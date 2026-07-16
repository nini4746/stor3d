/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ssd_gc_helpers.c                                    :+:      :+:    :+:  */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 10:40:00 by yunhpark          #+#    #+#             */
/*   Updated: 2026/07/16 10:20:00 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/stor3d.h"

static int	ssd_read_page(int disk_fd, size_t ppa, char *buf)
{
	off_t	offset;

	offset = ppa * BLOCK_SIZE;
	if (lseek(disk_fd, offset, SEEK_SET) != offset)
		return (1);
	if (read(disk_fd, buf, BLOCK_SIZE) != BLOCK_SIZE)
		return (1);
	return (0);
}

int	ssd_gc_collect(t_ssd_state *ssd, int victim, int disk_fd,
		t_gc_batch *batch)
{
	size_t	lba;
	size_t	ppa;

	batch->count = 0;
	batch->data = (char *)malloc(SSD_PAGES_PER_BLOCK * SSD_PAGE_SIZE);
	if (!batch->data)
		return (perror("malloc failed"), 1);
	lba = 0;
	while (lba < SSD_TOTAL_PAGES)
	{
		ppa = ssd->ftl_map[lba].ppa;
		if (ssd->ftl_map[lba].valid && ssd_get_block_index(ppa) == victim
			&& ssd->blocks[victim].pages[ssd_get_page_index(ppa)]
			== SSD_PAGE_VALID)
		{
			if (ssd_read_page(disk_fd, ppa,
					batch->data + batch->count * SSD_PAGE_SIZE))
				return (free(batch->data), 1);
			batch->lbas[batch->count++] = lba;
		}
		lba++;
	}
	return (0);
}

void	ssd_gc_erase_block(t_ssd_state *ssd, int victim)
{
	int	i;

	i = 0;
	while (i < SSD_PAGES_PER_BLOCK)
	{
		ssd->blocks[victim].pages[i] = SSD_PAGE_FREE;
		i++;
	}
	ssd->blocks[victim].valid_count = 0;
	ssd->blocks[victim].invalid_count = 0;
	ssd->blocks[victim].erase_count++;
	ssd->free_pages += SSD_PAGES_PER_BLOCK;
	ssd->erases++;
	ssd->total_time_ms += SSD_ERASE_COST;
	if (ssd->blocks[victim].erase_count > ssd->max_erase_count)
		ssd->max_erase_count = ssd->blocks[victim].erase_count;
}

int	ssd_gc_writeback(t_ssd_state *ssd, int disk_fd, int victim,
		t_gc_batch *batch)
{
	int		k;
	size_t	ppa;

	k = 0;
	while (k < batch->count)
	{
		ppa = (size_t)victim * SSD_PAGES_PER_BLOCK + k;
		if (ssd_physical_write(disk_fd, ppa,
				batch->data + (size_t)k * SSD_PAGE_SIZE))
			return (1);
		ssd->blocks[victim].pages[k] = SSD_PAGE_VALID;
		ssd->blocks[victim].valid_count++;
		ssd->free_pages--;
		ssd->ftl_map[batch->lbas[k]].ppa = ppa;
		ssd->nand_writes++;
		ssd->gc_moves++;
		ssd->total_time_ms += SSD_READ_COST + SSD_WRITE_COST;
		k++;
	}
	return (0);
}
