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

static size_t	ssd_find_free_ppa(t_ssd_state *ssd, size_t start)
{
	size_t	scanned;
	size_t	ppa;
	int		bi;
	int		pi;

	scanned = 0;
	ppa = start;
	while (scanned < SSD_TOTAL_PAGES)
	{
		bi = ssd_get_block_index(ppa);
		pi = ssd_get_page_index(ppa);
		if (ssd->blocks[bi].pages[pi] == SSD_PAGE_FREE)
			return (ppa);
		ppa++;
		if (ppa >= SSD_TOTAL_PAGES)
			ppa = 0;
		scanned++;
	}
	return ((size_t) - 1);
}

size_t	ssd_allocate_page(t_ssd_state *ssd)
{
	size_t	ppa;
	int		block_idx;
	int		page_idx;

	if (ssd->free_pages == 0)
		return ((size_t) - 1);
	ppa = ssd_find_free_ppa(ssd, ssd->next_free_ppa);
	if (ppa == (size_t) - 1)
		return ((size_t) - 1);
	block_idx = ssd_get_block_index(ppa);
	page_idx = ssd_get_page_index(ppa);
	ssd->blocks[block_idx].pages[page_idx] = SSD_PAGE_VALID;
	ssd->blocks[block_idx].valid_count++;
	ssd->free_pages--;
	ssd->next_free_ppa = ppa + 1;
	if (ssd->next_free_ppa >= SSD_TOTAL_PAGES)
		ssd->next_free_ppa = 0;
	return (ppa);
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
