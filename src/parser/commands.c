/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   commands.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 15:20:00 by yunhpark          #+#    #+#             */
/*   Updated: 2026/01/12 15:20:00 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/parser.h"

/* True if only whitespace / end-of-line remains after the last expected token. */
static int	no_trailing_tokens(char *line)
{
	while (*line && *line != ' ' && *line != '\t' && *line != '\n')
		line++;
	line = skip_whitespace(line);
	return (*line == '\0' || *line == '\n');
}

int	execute_read(t_context *ctx, char *line)
{
	long	lba;
	char	buf[BLOCK_SIZE];

	line = skip_whitespace(line + 1);
	if (parse_number(line, &lba) || !no_trailing_tokens(line))
	{
		perror("invalid script line");
		return (1);
	}
	if (read_block(ctx, (size_t)lba, buf))
		return (1);
	return (0);
}

int	execute_write(t_context *ctx, char *line)
{
	long	lba;
	long	byte_val;
	char	buf[BLOCK_SIZE];

	line = skip_whitespace(line + 1);
	if (parse_number(line, &lba))
	{
		perror("invalid script line");
		return (1);
	}
	while (*line && *line != ' ' && *line != '\t')
		line++;
	line = skip_whitespace(line);
	if (parse_number(line, &byte_val) || byte_val < 0 || byte_val > 255
		|| !no_trailing_tokens(line))
	{
		perror("invalid script line");
		return (1);
	}
	memset(buf, (unsigned char)byte_val, BLOCK_SIZE);
	if (write_block(ctx, (size_t)lba, buf))
		return (1);
	return (0);
}
