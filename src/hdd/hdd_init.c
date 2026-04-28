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

int	hdd_read(t_hdd_state *hdd, int disk_fd, size_t lba, void *buf)
{
	int		cylinder;
	int		head;
	int		sector;

	if (hdd_cache_lookup(hdd, lba))
	{
		if (hdd_do_physical_read(disk_fd, lba, buf))
			return (1);
		hdd->total_time_ms += hdd_get_transfer_time(0) / 4.0;
		hdd->total_reads++;
		return (0);
	}
	hdd_lba_to_chs(lba, &cylinder, &head, &sector);
	hdd->total_time_ms += hdd_calc_read_cost(hdd, cylinder, head,
			hdd_get_zone(cylinder));
	if (hdd_do_physical_read(disk_fd, lba, buf))
		return (1);
	hdd->prev_cylinder = cylinder;
	hdd->prev_head = head;
	hdd->prev_lba = lba;
	hdd->total_reads++;
	hdd_cache_insert(hdd, lba);
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
	cost += HDD_ROTATION_TIME / 2.0;
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

void	hdd_print_stats(t_hdd_state *hdd)
{
	size_t	lookups;
	double	hit_rate;

	if (!hdd)
		return ;
	printf("[HDD] total_reads=%zu\n", hdd->total_reads);
	printf("[HDD] total_writes=%zu\n", hdd->total_writes);
	printf("[HDD] total_time_ms=%.2f\n", hdd->total_time_ms);
	lookups = hdd->cache_hits + hdd->cache_misses;
	if (lookups > 0)
		hit_rate = (double)hdd->cache_hits * 100.0 / (double)lookups;
	else
		hit_rate = 0.0;
	printf("[HDD] cache_hits=%zu cache_misses=%zu hit_rate=%.1f%%\n",
		hdd->cache_hits, hdd->cache_misses, hit_rate);
}
