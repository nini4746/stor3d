/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   hdd_init.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 14:30:00 by yunhpark          #+#    #+#             */
/*   Updated: 2026/01/12 14:30:00 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/stor3d.h"

int	hdd_init(t_hdd_state **hdd)
{
	*hdd = (t_hdd_state *)malloc(sizeof(t_hdd_state));
	if (!*hdd)
	{
		perror("hdd malloc failed");
		return (1);
	}
	memset(*hdd, 0, sizeof(t_hdd_state));
	(*hdd)->prev_cylinder = -1;
	(*hdd)->prev_head = -1;
	(*hdd)->prev_lba = (size_t)-1;
	return (0);
}

void	hdd_cleanup(t_hdd_state *hdd)
{
	if (hdd)
		free(hdd);
}

/* Spec v2 §9.4: 3+ consecutive LBAs -> prefetch next 8 blocks. */
static void	hdd_track_sequential(t_hdd_state *hdd, size_t lba)
{
	if (hdd->last_sequential_lba + 1 == lba)
		hdd->sequential_count++;
	else
		hdd->sequential_count = 1;
	hdd->last_sequential_lba = lba;
}

static void	hdd_prefetch(t_hdd_state *hdd, size_t lba)
{
	size_t	i;

	i = 1;
	while (i <= HDD_CACHE_SIZE && lba + i < BLOCK_COUNT)
	{
		hdd_cache_insert(hdd, lba + i);
		i++;
	}
}

int	hdd_read(t_hdd_state *hdd, int disk_fd, size_t lba, void *buf)
{
	int		cylinder;
	int		head;
	int		sector;

	hdd_lba_to_chs(lba, &cylinder, &head, &sector);
	hdd_track_sequential(hdd, lba);
	if (hdd_cache_lookup(hdd, lba))
	{
		if (hdd_do_physical_read(disk_fd, lba, buf))
			return (1);
		hdd->total_time_ms += hdd_get_transfer_time(hdd_get_zone(cylinder));
		hdd->total_reads++;
		if (hdd->sequential_count >= 3)
			hdd_prefetch(hdd, lba);
		return (0);
	}
	hdd->total_time_ms += hdd_calc_read_cost(hdd, cylinder, head,
			hdd_get_zone(cylinder));
	if (hdd_do_physical_read(disk_fd, lba, buf))
		return (1);
	hdd->prev_cylinder = cylinder;
	hdd->prev_head = head;
	hdd->prev_lba = lba;
	hdd->total_reads++;
	hdd_cache_insert(hdd, lba);
	if (hdd->sequential_count >= 3)
		hdd_prefetch(hdd, lba);
	return (0);
}

int	hdd_write(t_hdd_state *hdd, int disk_fd, size_t lba, const void *buf)
{
	int			cylinder;
	int			head;
	int			sector;
	int			zone;
	double		cost;

	hdd_lba_to_chs(lba, &cylinder, &head, &sector);
	cost = hdd_calculate_seek_cost(hdd, cylinder, head);
	cost += HDD_ROTATION_TIME;
	zone = hdd_get_zone(cylinder);
	cost += hdd_get_transfer_time(zone);
	hdd->total_time_ms += cost;
	if (hdd_do_physical_write(disk_fd, lba, buf))
		return (1);
	hdd->prev_cylinder = cylinder;
	hdd->prev_head = head;
	hdd->prev_lba = lba;
	hdd->total_writes++;
	hdd_cache_invalidate(hdd, lba);
	return (0);
}

static void	hdd_print_stats_json(t_hdd_state *hdd, double hit_rate)
{
	printf("{\"device\":\"hdd\",");
	printf("\"total_reads\":%zu,\"total_writes\":%zu,",
		hdd->total_reads, hdd->total_writes);
	printf("\"total_time_ms\":%.2f,",
		hdd->total_time_ms);
	printf("\"cache_hits\":%zu,\"cache_misses\":%zu,",
		hdd->cache_hits, hdd->cache_misses);
	printf("\"hit_rate_pct\":%.1f}\n", hit_rate);
}

static void	hdd_print_stats_prom(t_hdd_state *hdd, double hit_rate)
{
	printf("# TYPE stor3d_hdd_reads counter\n");
	printf("stor3d_hdd_reads %zu\n", hdd->total_reads);
	printf("# TYPE stor3d_hdd_writes counter\n");
	printf("stor3d_hdd_writes %zu\n", hdd->total_writes);
	printf("# TYPE stor3d_hdd_time_ms gauge\n");
	printf("stor3d_hdd_time_ms %.4f\n", hdd->total_time_ms);
	printf("# TYPE stor3d_hdd_cache_hits counter\n");
	printf("stor3d_hdd_cache_hits %zu\n", hdd->cache_hits);
	printf("# TYPE stor3d_hdd_cache_misses counter\n");
	printf("stor3d_hdd_cache_misses %zu\n", hdd->cache_misses);
	printf("# TYPE stor3d_hdd_cache_hit_rate gauge\n");
	printf("stor3d_hdd_cache_hit_rate %.4f\n", hit_rate / 100.0);
}

void	hdd_print_stats(t_hdd_state *hdd)
{
	size_t	lookups;
	double	hit_rate;
	char	*fmt;

	if (!hdd)
		return ;
	lookups = hdd->cache_hits + hdd->cache_misses;
	if (lookups > 0)
		hit_rate = (double)hdd->cache_hits * 100.0 / (double)lookups;
	else
		hit_rate = 0.0;
	fmt = getenv("STOR3D_OUTPUT");
	if (fmt && strcmp(fmt, "json") == 0)
	{
		hdd_print_stats_json(hdd, hit_rate);
		return ;
	}
	if (fmt && strcmp(fmt, "prometheus") == 0)
	{
		hdd_print_stats_prom(hdd, hit_rate);
		return ;
	}
	printf("[HDD] total_reads=%zu\n", hdd->total_reads);
	printf("[HDD] total_writes=%zu\n", hdd->total_writes);
	printf("[HDD] cylinder_seeks=%zu\n", hdd->cylinder_seeks);
	printf("[HDD] head_switches=%zu\n", hdd->head_switches);
	printf("[HDD] cache_hits=%zu\n", hdd->cache_hits);
	printf("[HDD] cache_misses=%zu\n", hdd->cache_misses);
	printf("[HDD] total_time_ms=%.2f\n", hdd->total_time_ms);
}
