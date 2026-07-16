/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fs_file.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 09:55:00 by yunhpark          #+#    #+#             */
/*   Updated: 2026/01/14 09:55:00 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/stor3d.h"

static int	valid_name_char(char c)
{
	return (isalnum((unsigned char)c) || c == '.' || c == '_' || c == '-');
}

static int	validate_filename(t_fs_lite *fs, const char *name, size_t *len)
{
	size_t	i;

	if (fs_find_file(fs, name) >= 0)
		return (perror("file already exists"), 1);
	*len = strnlen(name, FS_MAX_FILENAME);
	if (*len == 0 || *len >= FS_MAX_FILENAME)
		return (perror("invalid filename"), 1);
	i = 0;
	while (i < *len)
	{
		if (!valid_name_char(name[i]))
			return (perror("invalid filename"), 1);
		i++;
	}
	return (0);
}

int	fs_create_file(t_fs_lite *fs, const char *name)
{
	int		i;
	size_t	name_len;

	if (validate_filename(fs, name, &name_len))
		return (1);
	i = 0;
	while (i < 64)
	{
		if (!fs->files[i].valid)
		{
			fs->files[i].valid = 1;
			memcpy(fs->files[i].name, name, name_len);
			fs->files[i].name[name_len] = '\0';
			fs->files[i].size = 0;
			fs->files[i].extent_count = 0;
			return (0);
		}
		i++;
	}
	return (perror("no space left on device"), 1);
}
