/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   file_commands.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 10:10:00 by yunhpark          #+#    #+#             */
/*   Updated: 2026/01/14 10:10:00 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/parser.h"

int	execute_create_file(t_context *ctx, char *line)
{
	char	*name;

	line = skip_whitespace(line + 2);
	name = line;
	while (*line && *line != ' ' && *line != '\t' && *line != '\n')
		line++;
	*line = '\0';
	return (fs_create_file(ctx->fs, name));
}

static char	*parse_wf_name(char *line, char **name)
{
	*name = line;
	while (*line && *line != ' ' && *line != '\t')
		line++;
	*line++ = '\0';
	return (skip_whitespace(line));
}

int	execute_write_file(t_context *ctx, char *line)
{
	char	*name;
	long	len;
	long	byte_val;

	line = parse_wf_name(skip_whitespace(line + 2), &name);
	if (parse_number(line, &len))
		return (perror("invalid script line"), 1);
	while (*line && *line != ' ' && *line != '\t')
		line++;
	line = skip_whitespace(line);
	if (parse_number(line, &byte_val) || byte_val < 0 || byte_val > 255)
		return (perror("invalid script line"), 1);
	return (fs_write_file(ctx, name, len, (unsigned char)byte_val));
}

int	execute_read_file(t_context *ctx, char *line)
{
	char	*name;
	long	len;

	line = skip_whitespace(line + 2);
	name = line;
	while (*line && *line != ' ' && *line != '\t')
		line++;
	*line++ = '\0';
	line = skip_whitespace(line);
	if (parse_number(line, &len))
		return (perror("invalid script line"), 1);
	return (fs_read_file(ctx, name, len));
}

int	execute_checksum(t_context *ctx, char *line)
{
	char	*name;

	line = skip_whitespace(line + 3);
	name = line;
	while (*line && *line != ' ' && *line != '\t' && *line != '\n')
		line++;
	*line = '\0';
	return (fs_checksum_file(ctx, name));
}
