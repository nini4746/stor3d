/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fs_ops.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 10:00:00 by yunhpark          #+#    #+#             */
/*   Updated: 2026/07/16 10:00:00 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/stor3d.h"

static int	write_range_block(t_context *ctx, size_t lba, size_t range[2],
				unsigned char byte_val)
{
	char	buf[BLOCK_SIZE];

	if (range[0] == 0 && range[1] == BLOCK_SIZE)
		memset(buf, byte_val, BLOCK_SIZE);
	else
	{
		if (read_block(ctx, lba, buf))
			return (1);
		memset(buf + range[0], byte_val, range[1] - range[0]);
	}
	return (write_block(ctx, lba, buf));
}

static int	fs_write_blocks(t_context *ctx, t_file_entry *file, t_fs_io *io)
{
	size_t	b;
	size_t	end;
	size_t	lba;
	size_t	range[2];

	end = io->offset + io->len;
	b = io->offset / BLOCK_SIZE;
	while (b <= (end - 1) / BLOCK_SIZE)
	{
		lba = fs_block_lba(file, b);
		if (lba == (size_t) - 1)
			return (perror("invalid block address"), 1);
		range[0] = 0;
		range[1] = BLOCK_SIZE;
		if (b == io->offset / BLOCK_SIZE)
			range[0] = io->offset % BLOCK_SIZE;
		if (b == (end - 1) / BLOCK_SIZE)
			range[1] = (end - 1) % BLOCK_SIZE + 1;
		if (write_range_block(ctx, lba, range, (unsigned char)io->byte_val))
			return (1);
		b++;
	}
	return (0);
}

int	fs_write_file(t_context *ctx, const char *name, t_fs_io *io)
{
	int				idx;
	size_t			end;
	t_file_entry	*file;

	idx = fs_find_file(ctx->fs, name);
	if (idx < 0)
		return (perror("file not found"), 1);
	file = &ctx->fs->files[idx];
	if (io->len == 0)
		return (perror("invalid length"), 1);
	if (io->offset > file->size)
		return (perror("invalid offset"), 1);
	end = io->offset + io->len;
	if (fs_grow_file(ctx->fs, file, (end + BLOCK_SIZE - 1) / BLOCK_SIZE))
		return (1);
	if (fs_write_blocks(ctx, file, io))
		return (1);
	if (end > file->size)
		file->size = end;
	return (0);
}

int	fs_read_file(t_context *ctx, const char *name, t_fs_io *io)
{
	int				idx;
	char			buf[BLOCK_SIZE];
	size_t			b;
	t_file_entry	*file;

	idx = fs_find_file(ctx->fs, name);
	if (idx < 0)
		return (perror("file not found"), 1);
	file = &ctx->fs->files[idx];
	if (io->len == 0)
		return (perror("invalid length"), 1);
	if (io->offset + io->len > file->size)
		return (perror("read beyond file size"), 1);
	b = io->offset / BLOCK_SIZE;
	while (b <= (io->offset + io->len - 1) / BLOCK_SIZE)
	{
		if (fs_block_lba(file, b) == (size_t) - 1)
			return (perror("invalid block address"), 1);
		if (read_block(ctx, fs_block_lba(file, b), buf))
			return (1);
		b++;
	}
	return (0);
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
	while (i * BLOCK_SIZE < ctx->fs->files[idx].size)
	{
		if (read_block(ctx, fs_block_lba(&ctx->fs->files[idx], i), buf))
			return (1);
		bytes = ctx->fs->files[idx].size - i * BLOCK_SIZE;
		if (bytes > BLOCK_SIZE)
			bytes = BLOCK_SIZE;
		while (bytes--)
			checksum += (unsigned char)buf[bytes];
		i++;
	}
	printf("[CHK] %s: %lu\n", name, checksum);
	return (0);
}
