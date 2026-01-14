/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   hdd_cache.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 08:55:00 by yunhpark          #+#    #+#             */
/*   Updated: 2026/01/14 08:55:00 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/stor3d.h"

int	hdd_cache_lookup(t_hdd_state *hdd, size_t lba)
{
	size_t	i;

	i = 0;
	while (i < HDD_CACHE_SIZE)
	{
		if (hdd->cache[i].valid && hdd->cache[i].lba == lba)
		{
			hdd->cache[i].last_access = hdd->cache_clock++;
			hdd->cache_hits++;
			return (1);
		}
		i++;
	}
	hdd->cache_misses++;
	return (0);
}

static size_t	find_lru_entry(t_hdd_state *hdd)
{
	size_t	i;
	size_t	lru_idx;
	size_t	min_access;

	lru_idx = 0;
	min_access = hdd->cache[0].last_access;
	i = 1;
	while (i < HDD_CACHE_SIZE)
	{
		if (hdd->cache[i].last_access < min_access)
		{
			min_access = hdd->cache[i].last_access;
			lru_idx = i;
		}
		i++;
	}
	return (lru_idx);
}

void	hdd_cache_insert(t_hdd_state *hdd, size_t lba)
{
	size_t	i;
	size_t	lru_idx;

	i = 0;
	while (i < HDD_CACHE_SIZE)
	{
		if (!hdd->cache[i].valid)
		{
			hdd->cache[i].lba = lba;
			hdd->cache[i].valid = 1;
			hdd->cache[i].last_access = hdd->cache_clock++;
			return ;
		}
		i++;
	}
	lru_idx = find_lru_entry(hdd);
	hdd->cache[lru_idx].lba = lba;
	hdd->cache[lru_idx].last_access = hdd->cache_clock++;
}

void	hdd_cache_invalidate(t_hdd_state *hdd, size_t lba)
{
	size_t	i;

	i = 0;
	while (i < HDD_CACHE_SIZE)
	{
		if (hdd->cache[i].valid && hdd->cache[i].lba == lba)
		{
			hdd->cache[i].valid = 0;
			return ;
		}
		i++;
	}
}
