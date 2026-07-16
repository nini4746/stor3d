/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   file_commands.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 10:10:00 by yunhpark          #+#    #+#             */
/*   Updated: 2026/07/16 10:10:00 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/parser.h"

/* Cuts next whitespace-separated token; advances *line past it. */
static char	*next_token(char **line)
{
	char	*tok;

	*line = skip_whitespace(*line);
	tok = *line;
	while (**line && **line != ' ' && **line != '\t' && **line != '\n')
		(*line)++;
	if (**line)
	{
		**line = '\0';
		(*line)++;
	}
	return (tok);
}

static int	line_is_done(char *line)
{
	line = skip_whitespace(line);
	return (*line == '\0' || *line == '\n');
}

int	execute_create_file(t_context *ctx, char *line)
{
	char	*name;

	line += 2;
	name = next_token(&line);
	if (!line_is_done(line))
		return (perror("invalid script line"), 1);
	return (fs_create_file(ctx->fs, name));
}

int	execute_write_file(t_context *ctx, char *line)
{
	char	*name;
	long	nums[3];
	int		i;
	t_fs_io	io;

	line += 2;
	name = next_token(&line);
	i = 0;
	while (i < 3)
	{
		if (parse_number(next_token(&line), &nums[i]))
			return (perror("invalid script line"), 1);
		i++;
	}
	if (!line_is_done(line) || nums[0] < 0 || nums[1] < 0
		|| nums[2] < 0 || nums[2] > 255)
		return (perror("invalid script line"), 1);
	io.offset = (size_t)nums[0];
	io.len = (size_t)nums[1];
	io.byte_val = nums[2];
	return (fs_write_file(ctx, name, &io));
}

int	execute_read_file(t_context *ctx, char *line)
{
	char	*name;
	long	nums[2];
	int		i;
	t_fs_io	io;

	line += 2;
	name = next_token(&line);
	i = 0;
	while (i < 2)
	{
		if (parse_number(next_token(&line), &nums[i]))
			return (perror("invalid script line"), 1);
		i++;
	}
	if (!line_is_done(line) || nums[0] < 0 || nums[1] < 0)
		return (perror("invalid script line"), 1);
	io.offset = (size_t)nums[0];
	io.len = (size_t)nums[1];
	io.byte_val = 0;
	return (fs_read_file(ctx, name, &io));
}

int	execute_checksum(t_context *ctx, char *line)
{
	char	*name;

	line += 3;
	name = next_token(&line);
	if (!line_is_done(line))
		return (perror("invalid script line"), 1);
	return (fs_checksum_file(ctx, name));
}
