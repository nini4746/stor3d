/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   trim_command.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/16 10:30:00 by yunhpark          #+#    #+#             */
/*   Updated: 2026/07/16 10:30:00 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/parser.h"

static char	*parse_trim_name(char *line)
{
	char	*name;

	line = skip_whitespace(line + 5);
	name = line;
	while (*line && *line != ' ' && *line != '\t' && *line != '\n')
		line++;
	if (*line)
	{
		*line++ = '\0';
		line = skip_whitespace(line);
		if (*line != '\0' && *line != '\n')
			return (NULL);
	}
	return (name);
}

/* Spec v2 §10.8: mark every page of the file invalid (SSD only, no-op
** on HDD where TRIM has no meaning). */
int	execute_trim_file(t_context *ctx, char *line)
{
	char			*name;
	size_t			b;
	size_t			lba;
	t_file_entry	*file;
	int				idx;

	name = parse_trim_name(line);
	if (!name)
		return (perror("invalid script line"), 1);
	idx = fs_find_file(ctx->fs, name);
	if (idx < 0)
		return (perror("file not found"), 1);
	if (ctx->mode != MODE_SSD)
		return (0);
	file = &ctx->fs->files[idx];
	b = 0;
	while (b * BLOCK_SIZE < file->size)
	{
		lba = fs_block_lba(file, b);
		if (lba != (size_t) - 1)
			ssd_trim_lba(ctx->ssd, lba);
		b++;
	}
	return (0);
}
