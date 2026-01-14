/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   hdd.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 14:00:00 by yunhpark          #+#    #+#             */
/*   Updated: 2026/01/12 14:00:00 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HDD_H
# define HDD_H

# include <stddef.h>

// HDD Physical Geometry
# define HDD_CYLINDERS 4
# define HDD_HEADS 16
# define HDD_SECTORS_PER_TRACK 256

// Zone Bit Recording (3 zones)
# define HDD_ZONE_COUNT 3

// Read-Ahead Cache
# define HDD_CACHE_SIZE 8

// Cost Model (in milliseconds)
# define HDD_CYLINDER_SEEK_COST 0.5
# define HDD_HEAD_SWITCH_COST 0.1
# define HDD_ROTATION_TIME 4.17
# define HDD_ZONE_TRANSFER_TIME_0 0.08
# define HDD_ZONE_TRANSFER_TIME_1 0.10
# define HDD_ZONE_TRANSFER_TIME_2 0.12

// Cache entry for LRU
typedef struct s_cache_entry
{
	size_t	lba;
	int		valid;
	size_t	last_access;
}	t_cache_entry;

typedef struct s_hdd_state
{
	int				prev_cylinder;
	int				prev_head;
	size_t			prev_lba;
	t_cache_entry	cache[HDD_CACHE_SIZE];
	size_t			cache_clock;
	size_t			sequential_count;
	size_t			last_sequential_lba;
	size_t			total_reads;
	size_t			total_writes;
	size_t			cylinder_seeks;
	size_t			head_switches;
	size_t			cache_hits;
	size_t			cache_misses;
	double			total_time_ms;
}	t_hdd_state;

// HDD Functions
int		hdd_init(t_hdd_state **hdd);
void	hdd_cleanup(t_hdd_state *hdd);
int		hdd_read(t_hdd_state *hdd, int disk_fd, size_t lba, void *buf);
int		hdd_write(t_hdd_state *hdd, int disk_fd, size_t lba, const void *buf);
void	hdd_print_stats(t_hdd_state *hdd);

// Helper functions
int		hdd_lba_to_chs(size_t lba, int *cylinder, int *head, int *sector);
int		hdd_get_zone(int cylinder);
double	hdd_get_transfer_time(int zone);
double	hdd_calculate_seek_cost(t_hdd_state *hdd, int cylinder, int head);
double	hdd_calc_read_cost(t_hdd_state *hdd, int cyl, int head, int zone);

// Cache functions
int		hdd_cache_lookup(t_hdd_state *hdd, size_t lba);
void	hdd_cache_insert(t_hdd_state *hdd, size_t lba);
void	hdd_cache_invalidate(t_hdd_state *hdd, size_t lba);

// I/O functions
int		hdd_do_physical_read(int disk_fd, size_t lba, void *buf);
int		hdd_do_physical_write(int disk_fd, size_t lba, const void *buf);

#endif
