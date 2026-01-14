/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   context.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 14:20:00 by yunhpark          #+#    #+#             */
/*   Updated: 2026/01/12 14:20:00 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/stor3d.h"

// TODO: static으로 변경 예정
int	open_context_files(t_context *ctx, char **argv)
{
	ctx->disk_fd = open(argv[2], O_RDWR);
	if (ctx->disk_fd < 0)
	{
		perror("cannot open disk image");
		free(ctx);
		return (1);
	}
	ctx->script_fd = open(argv[3], O_RDONLY);
	if (ctx->script_fd < 0)
	{
		perror("cannot open script");
		close(ctx->disk_fd);
		free(ctx);
		return (1);
	}
	return (0);
}

static int	init_device(t_context *ctx, char **argv)
{
	if (strcmp(argv[1], "hdd") == 0)
	{
		ctx->mode = MODE_HDD;
		return (hdd_init(&ctx->hdd));
	}
	else
	{
		ctx->mode = MODE_SSD;
		return (ssd_init(&ctx->ssd));
	}
}

int	init_context(t_context **ctx, char **argv)
{
	*ctx = (t_context *)malloc(sizeof(t_context));
	if (!*ctx)
		return (perror("malloc failed"), 1);
	memset(*ctx, 0, sizeof(t_context));
	if (init_device(*ctx, argv))
		return (1);
	if (fs_init(&(*ctx)->fs))
		return (1);
	if (open_context_files(*ctx, argv))
		return (1);
	return (0);
}

void	cleanup_context(t_context *ctx)
{
	if (!ctx)
		return ;
	if (ctx->disk_fd >= 0)
		close(ctx->disk_fd);
	if (ctx->script_fd >= 0)
		close(ctx->script_fd);
	if (ctx->mode == MODE_HDD && ctx->hdd)
		hdd_cleanup(ctx->hdd);
	if (ctx->mode == MODE_SSD && ctx->ssd)
		ssd_cleanup(ctx->ssd);
	if (ctx->fs)
		fs_cleanup(ctx->fs);
	free(ctx);
}
