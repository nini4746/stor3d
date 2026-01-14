/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ssd_gc_helpers.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 10:40:00 by yunhpark          #+#    #+#             */
/*   Updated: 2026/01/14 10:40:00 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/stor3d.h"

size_t	ssd_find_lba_for_ppa(t_ssd_state *ssd, size_t ppa)
{
	size_t	lba;

	lba = 0;
	while (lba < 8192)
	{
		if (ssd->ftl_map[lba].valid && ssd->ftl_map[lba].ppa == ppa)
			return (lba);
		lba++;
	}
	return ((size_t) - 1);
}

int	ssd_gc_move_one_page(t_ssd_state *ssd, size_t old_ppa, int disk_fd)
{
	size_t	new_ppa;
	size_t	lba;
	char	buf[BLOCK_SIZE];

	lba = ssd_find_lba_for_ppa(ssd, old_ppa);
	if (lba == (size_t) - 1)
		return (0);
	if (lseek(disk_fd, old_ppa * BLOCK_SIZE, SEEK_SET) < 0
		|| read(disk_fd, buf, BLOCK_SIZE) != BLOCK_SIZE)
		return (1);
	new_ppa = ssd_allocate_page(ssd);
	if (new_ppa == (size_t) - 1)
		return (1);
	if (lseek(disk_fd, new_ppa * BLOCK_SIZE, SEEK_SET) < 0
		|| write(disk_fd, buf, BLOCK_SIZE) != BLOCK_SIZE)
		return (1);
	ssd->ftl_map[lba].ppa = new_ppa;
	ssd->nand_writes++;
	ssd->gc_moves++;
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
	if (ssd->blocks[victim].erase_count > ssd->max_erase_count)
		ssd->max_erase_count = ssd->blocks[victim].erase_count;
}
