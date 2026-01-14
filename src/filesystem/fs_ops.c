/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fs_ops.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 10:00:00 by yunhpark          #+#    #+#             */
/*   Updated: 2026/01/14 10:02:24 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/stor3d.h"

static int	write_extent(t_context *ctx, size_t lba, size_t count,
				unsigned char byte_val)
{
	char	buf[BLOCK_SIZE];
	size_t	i;

	memset(buf, byte_val, BLOCK_SIZE);
	i = 0;
	while (i < count)
	{
		if (write_block(ctx, lba + i, buf))
			return (1);
		i++;
	}
	return (0);
}

int	fs_write_file(t_context *ctx, const char *name, size_t len,
		unsigned char byte_val)
{
	int				idx;
	size_t			blocks_needed;
	size_t			new_lba;
	t_file_entry	*file;

	idx = fs_find_file(ctx->fs, name);
	if (idx < 0)
		return (perror("file not found"), 1);
	file = &ctx->fs->files[idx];
	blocks_needed = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;
	new_lba = fs_allocate_blocks(ctx->fs, blocks_needed);
	if (new_lba == (size_t) - 1)
		return (perror("no space left on device"), 1);
	if (write_extent(ctx, new_lba, blocks_needed, byte_val))
		return (1);
	file->extents[0].lba = new_lba;
	file->extents[0].length = blocks_needed;
	file->extent_count = 1;
	file->size = len;
	return (0);
}

int	fs_read_file(t_context *ctx, const char *name, size_t len)
{
	int		idx;
	char	buf[BLOCK_SIZE];
	size_t	i;

	idx = fs_find_file(ctx->fs, name);
	if (idx < 0)
		return (perror("file not found"), 1);
	if (len > ctx->fs->files[idx].size)
		return (perror("read beyond file size"), 1);
	i = 0;
	while (i < ctx->fs->files[idx].extents[0].length)
	{
		if (read_block(ctx, ctx->fs->files[idx].extents[0].lba + i, buf))
			return (1);
		i++;
	}
	(void)len;
	return (0);
}

static void	sum_block(char *buf, size_t bytes, unsigned long *checksum)
{
	size_t	j;

	j = 0;
	while (j < bytes)
	{
		*checksum += (unsigned char)buf[j];
		j++;
	}
}

int	fs_checksum_file(t_context *ctx, const char *name)
{
	int				idx;
	char			buf[BLOCK_SIZE];
	size_t			i;
	size_t			bytes;
	unsigned long	checksum;

	idx = fs_find_file(ctx->fs, name);
	if (idx < 0)
		return (perror("file not found"), 1);
	checksum = 0;
	i = 0;
	while (i < ctx->fs->files[idx].extents[0].length)
	{
		if (read_block(ctx, ctx->fs->files[idx].extents[0].lba + i, buf))
			return (1);
		bytes = BLOCK_SIZE;
		if (i * BLOCK_SIZE + bytes > ctx->fs->files[idx].size)
			bytes = ctx->fs->files[idx].size - i * BLOCK_SIZE;
		sum_block(buf, bytes, &checksum);
		i++;
	}
	printf("[CHK] %s: %lu\n", name, checksum);
	return (0);
}
