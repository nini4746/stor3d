/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ssd_gc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 09:35:00 by yunhpark          #+#    #+#             */
/*   Updated: 2026/01/14 09:35:00 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/stor3d.h"

int	ssd_gc_needed(t_ssd_state *ssd)
{
	if (ssd->free_pages < SSD_GC_TRIGGER_THRESHOLD)
		return (1);
	return (0);
}

int	ssd_select_victim_block(t_ssd_state *ssd)
{
	int	i;
	int	victim;
	int	max_invalid;

	victim = -1;
	max_invalid = -1;
	i = 0;
	while (i < SSD_ERASE_BLOCK_COUNT)
	{
		if (ssd->blocks[i].invalid_count > max_invalid)
		{
			max_invalid = ssd->blocks[i].invalid_count;
			victim = i;
		}
		i++;
	}
	return (victim);
}

int	ssd_gc_move_valid_pages(t_ssd_state *ssd, int victim, int disk_fd)
{
	int		page_idx;
	size_t	old_ppa;

	page_idx = 0;
	while (page_idx < SSD_PAGES_PER_BLOCK)
	{
		if (ssd->blocks[victim].pages[page_idx] == SSD_PAGE_VALID)
		{
			old_ppa = victim * SSD_PAGES_PER_BLOCK + page_idx;
			if (ssd_gc_move_one_page(ssd, old_ppa, disk_fd))
				return (1);
		}
		page_idx++;
	}
	return (0);
}

int	ssd_run_gc(t_ssd_state *ssd, int disk_fd)
{
	int	victim;

	victim = ssd_select_victim_block(ssd);
	if (victim < 0)
		return (1);
	if (ssd_gc_move_valid_pages(ssd, victim, disk_fd))
		return (1);
	ssd_gc_erase_block(ssd, victim);
	ssd->gc_count++;
	return (0);
}
