/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   block_device.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 13:35:00 by yunhpark          #+#    #+#             */
/*   Updated: 2026/01/12 13:35:00 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/stor3d.h"

int	read_block(t_context *ctx, size_t lba, void *buf)
{
	if (lba >= BLOCK_COUNT)
	{
		perror("invalid block address");
		return (1);
	}
	if (ctx->mode == MODE_HDD)
		return (hdd_read(ctx->hdd, ctx->disk_fd, lba, buf));
	else
		return (ssd_read(ctx->ssd, ctx->disk_fd, lba, buf));
}

int	write_block(t_context *ctx, size_t lba, const void *buf)
{
	if (lba >= BLOCK_COUNT)
	{
		perror("invalid block address");
		return (1);
	}
	if (ctx->mode == MODE_HDD)
		return (hdd_write(ctx->hdd, ctx->disk_fd, lba, buf));
	else
		return (ssd_write(ctx->ssd, ctx->disk_fd, lba, buf));
}
