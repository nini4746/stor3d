/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   stor3d.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/04 13:06:14 by yunhpark          #+#    #+#             */
/*   Updated: 2026/01/07 15:09:29 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef STOR3D_H
# define STOR3D_H

# include <stdlib.h>
# include <unistd.h>
# include <sys/time.h>
# include <string.h>
# include <ctype.h>
# include <errno.h>
# include <stdio.h>
# include <fcntl.h>
# include <sys/types.h>
# include <sys/stat.h>
# include "hdd.h"
# include "ssd.h"
# include "fs_lite.h"

# define BLOCK_SIZE 4096
# define BLOCK_COUNT 8192
# define DISK_SIZE 33554432

// FS_LITE
# define MAX_FILES 64

// Forward declarations
typedef struct s_hdd_state	t_hdd_state;
typedef struct s_ssd_state	t_ssd_state;
typedef struct s_fs_lite	t_fs_lite;

typedef enum e_mode
{
	MODE_HDD,
	MODE_SSD
}	t_mode;

typedef struct s_context
{
	int				disk_fd;
	int				script_fd;
	t_mode			mode;
	t_hdd_state		*hdd;
	t_ssd_state		*ssd;
	t_fs_lite		*fs;
}	t_context;

int		init_context(t_context **ctx, char **argv);
int		open_context_files(t_context *ctx, char **argv);
void	cleanup_context(t_context *ctx);
int		is_valid(int argc, char **argv);
int		valid_image(const char *image_path);
int		valid_script(const char *script_path);
int		create_image(const char *image_path);
int		read_block(t_context *ctx, size_t lba, void *buf);
int		write_block(t_context *ctx, size_t lba, const void *buf);
int		run_script(t_context *ctx);

#endif