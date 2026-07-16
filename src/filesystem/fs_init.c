/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fs_init.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 09:45:00 by yunhpark          #+#    #+#             */
/*   Updated: 2026/01/14 09:45:00 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/stor3d.h"

static void	init_file_table(t_fs_lite *fs)
{
	int	i;

	i = 0;
	while (i < 64)
	{
		fs->files[i].valid = 0;
		fs->files[i].size = 0;
		fs->files[i].extent_count = 0;
		i++;
	}
}

/* Block 0 is reserved for the metadata table (spec v2 §XI). */
static void	init_free_blocks(t_fs_lite *fs)
{
	int	i;

	i = 0;
	while (i < 8192)
	{
		fs->free_blocks[i] = 0;
		i++;
	}
	fs->free_blocks[0] = 1;
}

int	fs_init(t_fs_lite **fs)
{
	*fs = (t_fs_lite *)malloc(sizeof(t_fs_lite));
	if (!*fs)
	{
		perror("fs malloc failed");
		return (1);
	}
	memset(*fs, 0, sizeof(t_fs_lite));
	init_file_table(*fs);
	init_free_blocks(*fs);
	return (0);
}

void	fs_cleanup(t_fs_lite *fs)
{
	if (fs)
		free(fs);
}

int	fs_find_file(t_fs_lite *fs, const char *name)
{
	int	i;

	i = 0;
	while (i < 64)
	{
		if (fs->files[i].valid && strncmp(fs->files[i].name, name,
				FS_MAX_FILENAME) == 0)
			return (i);
		i++;
	}
	return (-1);
}
