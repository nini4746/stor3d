/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 15:20:00 by yunhpark          #+#    #+#             */
/*   Updated: 2026/01/12 15:20:00 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_H
# define PARSER_H

# include "stor3d.h"

# define BUFFER_SIZE 4096

// Utils functions
char	*skip_whitespace(char *str);
int		parse_number(char *str, long *result);
char	*read_next_line(char *buffer, char *line_buf, size_t *offset);

// Command execution functions
int		execute_read(t_context *ctx, char *line);
int		execute_write(t_context *ctx, char *line);
int		execute_create_file(t_context *ctx, char *line);
int		execute_write_file(t_context *ctx, char *line);
int		execute_read_file(t_context *ctx, char *line);
int		execute_checksum(t_context *ctx, char *line);

// Parser functions
int		parse_line(t_context *ctx, char *line);
char	*read_script_to_buffer(int fd);

#endif
