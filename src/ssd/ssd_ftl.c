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

size_t	ssd_allocate_page(t_ssd_state *ssd)
{
	size_t	ppa;
	int		block_idx;
	int		page_idx;

	if (ssd->free_pages == 0)
		return ((size_t) - 1);
	ppa = ssd->next_free_ppa;
	block_idx = ssd_get_block_index(ppa);
	page_idx = ssd_get_page_index(ppa);
	ssd->blocks[block_idx].pages[page_idx] = SSD_PAGE_VALID;
	ssd->blocks[block_idx].valid_count++;
	ssd->free_pages--;
	ssd->next_free_ppa++;
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
