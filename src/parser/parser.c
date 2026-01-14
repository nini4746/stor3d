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

static int	parse_file_command(t_context *ctx, char *line)
{
	if (strncmp(line, "CF", 2) == 0)
		return (execute_create_file(ctx, line));
	else if (strncmp(line, "WF", 2) == 0)
		return (execute_write_file(ctx, line));
	else if (strncmp(line, "RF", 2) == 0)
		return (execute_read_file(ctx, line));
	else if (strncmp(line, "CHK", 3) == 0)
		return (execute_checksum(ctx, line));
	else
	{
		perror("invalid script line");
		return (1);
	}
}

int	parse_line(t_context *ctx, char *line)
{
	line = skip_whitespace(line);
	if (*line == '\0' || *line == '\n' || *line == '#')
		return (0);
	if (*line == 'R' && (line[1] == ' ' || line[1] == '\t'))
		return (execute_read(ctx, line));
	else if (*line == 'W' && (line[1] == ' ' || line[1] == '\t'))
		return (execute_write(ctx, line));
	else if (*line == 'C' || *line == 'W' || *line == 'R')
		return (parse_file_command(ctx, line));
	else
	{
		perror("invalid script line");
		return (1);
	}
}

static char	*expand_buffer(char *old_buf, size_t old_size, char *data,
		ssize_t len)
{
	char	*new_buf;

	new_buf = malloc(old_size + len + 1);
	if (!new_buf)
	{
		free(old_buf);
		perror("malloc failed");
		return (NULL);
	}
	if (old_buf)
	{
		memcpy(new_buf, old_buf, old_size);
		free(old_buf);
	}
	memcpy(new_buf + old_size, data, len);
	new_buf[old_size + len] = '\0';
	return (new_buf);
}

char	*read_script_to_buffer(int fd)
{
	char	*buffer;
	char	temp[BUFFER_SIZE];
	ssize_t	bytes_read;
	size_t	total_size;

	buffer = NULL;
	total_size = 0;
	bytes_read = read(fd, temp, BUFFER_SIZE);
	while (bytes_read > 0)
	{
		buffer = expand_buffer(buffer, total_size, temp, bytes_read);
		if (!buffer)
			return (NULL);
		total_size += bytes_read;
		bytes_read = read(fd, temp, BUFFER_SIZE);
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
