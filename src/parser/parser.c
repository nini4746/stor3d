/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/07 15:05:25 by yunhpark          #+#    #+#             */
/*   Updated: 2026/01/12 15:00:00 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/parser.h"

int	parse_line(t_context *ctx, char *line)
{
	line = skip_whitespace(line);
	if (*line == '\0' || *line == '\n' || *line == '#')
		return (0);
	if (*line == 'R')
		return (execute_read(ctx, line));
	else if (*line == 'W')
		return (execute_write(ctx, line));
	else
	{
		perror("invalid script line");
		return (1);
	}
}

char	*read_script_to_buffer(int fd)
{
	char	*buffer;
	char	temp[BUFFER_SIZE];
	ssize_t	bytes_read;
	size_t	total_size;
	char	*new_buffer;

	buffer = NULL;
	total_size = 0;
	while ((bytes_read = read(fd, temp, BUFFER_SIZE)) > 0)
	{
		new_buffer = malloc(total_size + bytes_read + 1);
		if (!new_buffer)
		{
			free(buffer);
			perror("malloc failed");
			return (NULL);
		}
		if (buffer)
		{
			memcpy(new_buffer, buffer, total_size);
			free(buffer);
		}
		memcpy(new_buffer + total_size, temp, bytes_read);
		total_size += bytes_read;
		new_buffer[total_size] = '\0';
		buffer = new_buffer;
	}
	return (buffer);
}

int	run_script(t_context *ctx)
{
	char	*buffer;
	char	line_buf[BUFFER_SIZE];
	size_t	offset;

	buffer = read_script_to_buffer(ctx->script_fd);
	if (!buffer)
		return (1);
	offset = 0;
	while (read_next_line(buffer, line_buf, &offset))
	{
		if (parse_line(ctx, line_buf))
		{
			free(buffer);
			return (1);
		}
	}
	free(buffer);
	return (0);
}
