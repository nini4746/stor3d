/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ssd_ftl.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 09:30:00 by yunhpark          #+#    #+#             */
/*   Updated: 2026/01/14 09:30:00 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/stor3d.h"

int	ssd_get_block_index(size_t ppa)
{
	return (ppa / SSD_PAGES_PER_BLOCK);
}

int	ssd_get_page_index(size_t ppa)
{
	return (ppa % SSD_PAGES_PER_BLOCK);
}

/* Spec v2 §10.7 wear leveling: prefer the free block with the lowest
** erase count so writes spread evenly across all blocks. */
static int	ssd_pick_block(t_ssd_state *ssd)
{
	int	i;
	int	best;

	best = -1;
	i = 0;
	while (i < SSD_ERASE_BLOCK_COUNT)
	{
		if (ssd->blocks[i].valid_count + ssd->blocks[i].invalid_count
			< SSD_PAGES_PER_BLOCK
			&& (best < 0 || ssd->blocks[i].erase_count
				< ssd->blocks[best].erase_count))
			best = i;
		i++;
	}
	return (best);
}

size_t	ssd_allocate_page(t_ssd_state *ssd)
{
	int	block_idx;
	int	page_idx;

	if (ssd->free_pages == 0)
		return ((size_t) - 1);
	block_idx = ssd_pick_block(ssd);
	if (block_idx < 0)
		return ((size_t) - 1);
	page_idx = 0;
	while (ssd->blocks[block_idx].pages[page_idx] != SSD_PAGE_FREE)
		page_idx++;
	ssd->blocks[block_idx].pages[page_idx] = SSD_PAGE_VALID;
	ssd->blocks[block_idx].valid_count++;
	ssd->free_pages--;
	return ((size_t)block_idx * SSD_PAGES_PER_BLOCK + page_idx);
}

int	ssd_invalidate_page(t_ssd_state *ssd, size_t ppa)
{
	int	block_idx;
	int	page_idx;

	if (ppa >= SSD_TOTAL_PAGES)
		return (1);
	block_idx = ssd_get_block_index(ppa);
	page_idx = ssd_get_page_index(ppa);
	if (ssd->blocks[block_idx].pages[page_idx] == SSD_PAGE_VALID)
	{
		ssd->blocks[block_idx].pages[page_idx] = SSD_PAGE_INVALID;
		ssd->blocks[block_idx].valid_count--;
		ssd->blocks[block_idx].invalid_count++;
	}
	return (0);
}

/* Spec v2 §10.8 TRIM: mark the page invalid, do not trigger GC. */
int	ssd_trim_lba(t_ssd_state *ssd, size_t lba)
{
	if (lba >= SSD_TOTAL_PAGES)
		return (1);
	if (ssd->ftl_map[lba].valid)
	{
		ssd_invalidate_page(ssd, ssd->ftl_map[lba].ppa);
		ssd->ftl_map[lba].valid = 0;
	}
	return (0);
}
