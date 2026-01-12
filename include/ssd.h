/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ssd.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 14:00:00 by yunhpark          #+#    #+#             */
/*   Updated: 2026/01/12 14:00:00 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SSD_H
# define SSD_H

# include <stddef.h>

// SSD Flash Geometry
# define SSD_PAGE_SIZE 4096
# define SSD_PAGES_PER_BLOCK 256
# define SSD_ERASE_BLOCK_COUNT 32
# define SSD_TOTAL_PAGES (SSD_PAGES_PER_BLOCK * SSD_ERASE_BLOCK_COUNT)

// Over-Provisioning (10%)
# define SSD_USER_CAPACITY_RATIO 0.9
# define SSD_USER_PAGES ((size_t)(SSD_TOTAL_PAGES * SSD_USER_CAPACITY_RATIO))
# define SSD_OP_PAGES (SSD_TOTAL_PAGES - SSD_USER_PAGES)

// GC Trigger
# define SSD_GC_TRIGGER_THRESHOLD (SSD_OP_PAGES / 10)

// Performance Costs (in milliseconds)
# define SSD_READ_COST 0.05
# define SSD_WRITE_COST 0.1
# define SSD_ERASE_COST 1.5

// Page states
# define SSD_PAGE_FREE 0
# define SSD_PAGE_VALID 1
# define SSD_PAGE_INVALID -1

// FTL Entry (LBA -> PPA mapping)
typedef struct s_ftl_entry
{
	size_t	ppa;
	int		valid;
}	t_ftl_entry;

// Flash Erase Block
typedef struct s_flash_block
{
	int		pages[SSD_PAGES_PER_BLOCK];
	int		valid_count;
	int		invalid_count;
	int		erase_count;
}	t_flash_block;

// SSD State
typedef struct s_ssd_state
{
	// FTL (Flash Translation Layer)
	t_ftl_entry		ftl_map[8192];

	// Flash blocks
	t_flash_block	blocks[SSD_ERASE_BLOCK_COUNT];

	// Free page management
	size_t			free_pages;
	size_t			next_free_ppa;

	// Statistics
	size_t			host_writes;
	size_t			nand_writes;
	size_t			erases;
	size_t			gc_count;
	size_t			gc_moves;
	int				max_erase_count;
	int				min_erase_count;
	double			total_time_ms;
}	t_ssd_state;

// SSD Functions
int		ssd_init(t_ssd_state **ssd);
void	ssd_cleanup(t_ssd_state *ssd);
int		ssd_read(t_ssd_state *ssd, int disk_fd, size_t lba, void *buf);
int		ssd_write(t_ssd_state *ssd, int disk_fd, size_t lba, const void *buf);
void	ssd_print_stats(t_ssd_state *ssd);

// FTL Functions
size_t	ssd_allocate_page(t_ssd_state *ssd);
int		ssd_invalidate_page(t_ssd_state *ssd, size_t ppa);
int		ssd_get_block_index(size_t ppa);
int		ssd_get_page_index(size_t ppa);

// GC Functions
int		ssd_gc_needed(t_ssd_state *ssd);
int		ssd_run_gc(t_ssd_state *ssd, int disk_fd);
int		ssd_select_victim_block(t_ssd_state *ssd);

// TRIM Functions
int		ssd_trim_lba(t_ssd_state *ssd, size_t lba);

#endif
