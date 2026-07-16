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
# define SSD_TOTAL_PAGES 8192
# define SSD_USER_PAGES 7372
# define SSD_OP_PAGES 820
# define SSD_GC_TRIGGER_THRESHOLD 82

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

typedef struct s_ssd_state
{
	t_ftl_entry		ftl_map[8192];
	t_flash_block	blocks[SSD_ERASE_BLOCK_COUNT];
	size_t			free_pages;
	size_t			next_free_ppa;
	size_t			host_writes;
	size_t			nand_writes;
	size_t			erases;
	size_t			gc_count;
	size_t			gc_moves;
	int				max_erase_count;
	int				min_erase_count;
	double			total_time_ms;
}	t_ssd_state;

// GC batch: valid pages of the victim block buffered in RAM during erase
typedef struct s_gc_batch
{
	char	*data;
	size_t	lbas[SSD_PAGES_PER_BLOCK];
	int		count;
}	t_gc_batch;

int		ssd_init(t_ssd_state **ssd);
void	ssd_cleanup(t_ssd_state *ssd);
int		ssd_read(t_ssd_state *ssd, int disk_fd, size_t lba, void *buf);
int		ssd_write(t_ssd_state *ssd, int disk_fd, size_t lba, const void *buf);
void	ssd_print_stats(t_ssd_state *ssd);
int		ssd_physical_write(int disk_fd, size_t ppa, const void *buf);
int		ssd_gc_if_needed(t_ssd_state *ssd, int disk_fd);

// FTL Functions
size_t	ssd_allocate_page(t_ssd_state *ssd);
int		ssd_invalidate_page(t_ssd_state *ssd, size_t ppa);
int		ssd_get_block_index(size_t ppa);
int		ssd_get_page_index(size_t ppa);

int		ssd_gc_needed(t_ssd_state *ssd);
int		ssd_run_gc(t_ssd_state *ssd, int disk_fd);
int		ssd_select_victim_block(t_ssd_state *ssd);
int		ssd_gc_collect(t_ssd_state *ssd, int victim, int disk_fd,
			t_gc_batch *batch);
void	ssd_gc_erase_block(t_ssd_state *ssd, int victim);
int		ssd_gc_writeback(t_ssd_state *ssd, int disk_fd, int victim,
			t_gc_batch *batch);
int		ssd_trim_lba(t_ssd_state *ssd, size_t lba);

#endif
