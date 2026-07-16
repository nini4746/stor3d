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

size_t	fs_block_lba(t_file_entry *file, size_t block_idx)
{
	int		i;
	size_t	base;

	base = 0;
	i = 0;
	while (i < file->extent_count)
	{
		if (block_idx < base + file->extents[i].length)
			return (file->extents[i].lba + (block_idx - base));
		base += file->extents[i].length;
		i++;
	}
	return ((size_t) - 1);
}

static void	fs_release_blocks(t_fs_lite *fs, size_t lba, size_t count)
{
	while (count--)
		fs->free_blocks[lba + count] = 0;
}

int	fs_grow_file(t_fs_lite *fs, t_file_entry *file, size_t need_blocks)
{
	size_t		cur;
	size_t		new_lba;
	t_extent	*last;
	int			i;

	cur = 0;
	i = 0;
	while (i < file->extent_count)
		cur += file->extents[i++].length;
	if (need_blocks <= cur)
		return (0);
	new_lba = fs_allocate_blocks(fs, need_blocks - cur);
	if (new_lba == (size_t) - 1)
		return (perror("no space left on device"), 1);
	last = NULL;
	if (file->extent_count > 0)
		last = &file->extents[file->extent_count - 1];
	if (last && last->lba + last->length == new_lba)
		last->length += need_blocks - cur;
	else if (file->extent_count < FS_MAX_EXTENTS)
	{
		file->extents[file->extent_count].lba = new_lba;
		file->extents[file->extent_count].length = need_blocks - cur;
		file->extent_count++;
	}
	else
		return (fs_release_blocks(fs, new_lba, need_blocks - cur),
			perror("no space left on device"), 1);
	return (0);
}
