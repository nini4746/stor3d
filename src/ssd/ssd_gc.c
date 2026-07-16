/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ssd_gc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 09:35:00 by yunhpark          #+#    #+#             */
/*   Updated: 2026/07/16 10:20:00 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/stor3d.h"

static int	ssd_has_invalid(t_ssd_state *ssd)
{
	int	i;

	i = 0;
	while (i < SSD_ERASE_BLOCK_COUNT)
	{
		if (ssd->blocks[i].invalid_count > 0)
			return (1);
		i++;
	}
	return (0);
}

/* Spec v2 §10.6: trigger when free pages < 10% of OP area (82 pages).
** GC can only reclaim invalid pages, so it is a no-op without any. */
int	ssd_gc_needed(t_ssd_state *ssd)
{
	if (ssd->free_pages >= SSD_GC_TRIGGER_THRESHOLD)
		return (0);
	return (ssd_has_invalid(ssd));
}

int	ssd_select_victim_block(t_ssd_state *ssd)
{
	int	i;
	int	victim;
	int	max_invalid;

	victim = -1;
	max_invalid = 0;
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

/* Valid pages are buffered in RAM, the block is erased, then they are
** written back into the freed block. Net gain: the victim's invalid count. */
int	ssd_run_gc(t_ssd_state *ssd, int disk_fd)
{
	int			victim;
	t_gc_batch	batch;

	victim = ssd_select_victim_block(ssd);
	if (victim < 0)
		return (1);
	if (ssd_gc_collect(ssd, victim, disk_fd, &batch))
		return (1);
	ssd_gc_erase_block(ssd, victim);
	if (ssd_gc_writeback(ssd, disk_fd, victim, &batch))
	{
		free(batch.data);
		return (1);
	}
	free(batch.data);
	ssd->gc_count++;
	return (0);
}
