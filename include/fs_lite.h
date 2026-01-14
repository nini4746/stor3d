/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fs_lite.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 09:40:00 by yunhpark          #+#    #+#             */
/*   Updated: 2026/01/14 09:40:00 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FS_LITE_H
# define FS_LITE_H

# include <stddef.h>

# define FS_MAX_FILENAME 32
# define FS_MAX_EXTENTS 8

typedef struct s_extent
{
	size_t	lba;
	size_t	length;
}	t_extent;

typedef struct s_file_entry
{
	char		name[FS_MAX_FILENAME];
	int			valid;
	size_t		size;
	t_extent	extents[FS_MAX_EXTENTS];
	int			extent_count;
}	t_file_entry;

typedef struct s_fs_lite
{
	t_file_entry	files[64];
	int				free_blocks[8192];
}	t_fs_lite;

// Forward declaration
typedef struct s_context	t_context;

// FS_LITE functions
int		fs_init(t_fs_lite **fs);
void	fs_cleanup(t_fs_lite *fs);

// File operations
int		fs_create_file(t_fs_lite *fs, const char *name);
int		fs_write_file(t_context *ctx, const char *name, size_t len,
			unsigned char byte_val);
int		fs_read_file(t_context *ctx, const char *name, size_t len);
int		fs_checksum_file(t_context *ctx, const char *name);

// Helper functions
int		fs_find_file(t_fs_lite *fs, const char *name);
size_t	fs_allocate_blocks(t_fs_lite *fs, size_t count);

#endif
