/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ssd_init.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 14:30:00 by yunhpark          #+#    #+#             */
/*   Updated: 2026/01/12 14:30:00 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/stor3d.h"

static void	init_ftl_map(t_ssd_state *ssd)
{
	size_t	i;

	i = 0;
	while (i < 8192)
	{
		ssd->ftl_map[i].valid = 0;
		ssd->ftl_map[i].ppa = 0;
		i++;
	}
}

static void	init_blocks(t_ssd_state *ssd)
{
	int	block_idx;
	int	page_idx;

	block_idx = 0;
	while (block_idx < SSD_ERASE_BLOCK_COUNT)
	{
		page_idx = 0;
		while (page_idx < SSD_PAGES_PER_BLOCK)
		{
			ssd->blocks[block_idx].pages[page_idx] = SSD_PAGE_FREE;
			page_idx++;
		}
		ssd->blocks[block_idx].valid_count = 0;
		ssd->blocks[block_idx].invalid_count = 0;
		ssd->blocks[block_idx].erase_count = 0;
		block_idx++;
	}
}

int	ssd_init(t_ssd_state **ssd)
{
	*ssd = (t_ssd_state *)malloc(sizeof(t_ssd_state));
	if (!*ssd)
	{
		perror("ssd malloc failed");
		return (1);
	}
	memset(*ssd, 0, sizeof(t_ssd_state));
	(*ssd)->free_pages = SSD_TOTAL_PAGES;
	(*ssd)->next_free_ppa = 0;
	(*ssd)->min_erase_count = 0;
	(*ssd)->max_erase_count = 0;
	init_ftl_map(*ssd);
	init_blocks(*ssd);
	return (0);
}

void	ssd_cleanup(t_ssd_state *ssd)
{
	if (ssd)
		free(ssd);
}

static void	ssd_print_stats_json(t_ssd_state *ssd, double write_amp)
{
	printf("{\"device\":\"ssd\",");
	printf("\"host_writes\":%zu,\"nand_writes\":%zu,",
		ssd->host_writes, ssd->nand_writes);
	printf("\"erases\":%zu,\"gc_count\":%zu,\"gc_moves\":%zu,",
		ssd->erases, ssd->gc_count, ssd->gc_moves);
	printf("\"free_pages\":%zu,\"max_erase_count\":%d,",
		ssd->free_pages, ssd->max_erase_count);
	printf("\"write_amplification\":%.2f}\n", write_amp);
}

void	ssd_print_stats(t_ssd_state *ssd)
{
	double	write_amp;
	char	*fmt;

	if (!ssd)
		return ;
	if (ssd->host_writes > 0)
		write_amp = (double)ssd->nand_writes / (double)ssd->host_writes;
	else
		write_amp = 0.0;
	fmt = getenv("STOR3D_OUTPUT");
	if (fmt && strcmp(fmt, "json") == 0)
	{
		ssd_print_stats_json(ssd, write_amp);
		return ;
	}
	printf("[SSD] host_writes=%zu\n", ssd->host_writes);
	printf("[SSD] nand_writes=%zu\n", ssd->nand_writes);
	printf("[SSD] erases=%zu gc_count=%zu gc_moves=%zu\n",
		ssd->erases, ssd->gc_count, ssd->gc_moves);
	printf("[SSD] write_amp=%.2f\n", write_amp);
}
