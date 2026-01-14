/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fs_alloc.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 09:50:00 by yunhpark          #+#    #+#             */
/*   Updated: 2026/01/14 09:50:00 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/stor3d.h"

static size_t	find_free_block(t_fs_lite *fs)
{
	size_t	i;

	i = 0;
	while (i < 8192)
	{
		if (fs->free_blocks[i] == 0)
			return (i);
		i++;
	}
	return ((size_t) - 1);
}

size_t	fs_allocate_blocks(t_fs_lite *fs, size_t count)
{
	size_t	start;
	size_t	i;

	if (count == 0)
		return ((size_t) - 1);
	start = find_free_block(fs);
	if (start == (size_t) - 1)
		return ((size_t) - 1);
	i = 0;
	while (i < count)
	{
		if (start + i >= 8192 || fs->free_blocks[start + i] != 0)
			return ((size_t) - 1);
		i++;
	}
	i = 0;
	while (i < count)
	{
		fs->free_blocks[start + i] = 1;
		i++;
	}
	return (start);
}
