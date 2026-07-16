/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 15:20:00 by yunhpark          #+#    #+#             */
/*   Updated: 2026/01/12 15:20:00 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/parser.h"

char	*skip_whitespace(char *str)
{
	while (*str == ' ' || *str == '\t' || *str == '\r')
		str++;
	return (str);
}

int	parse_number(char *str, long *result)
{
	char	*endptr;
	int		base;

	base = 10;
	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
		base = 16;
	*result = strtol(str, &endptr, base);
	if (endptr == str || (*endptr != ' ' && *endptr != '\t'
			&& *endptr != '\n' && *endptr != '\0'))
		return (1);
	return (0);
}

char	*read_next_line(char *buffer, char *line_buf, size_t *offset)
{
	size_t	i;

	i = 0;
	while (buffer[*offset] && buffer[*offset] != '\n' && i < BUFFER_SIZE - 1)
	{
		line_buf[i++] = buffer[(*offset)++];
	}
	line_buf[i] = '\0';
	if (buffer[*offset] == '\n')
		(*offset)++;
	if (i == 0 && buffer[*offset] == '\0')
		return (NULL);
	return (line_buf);
}
