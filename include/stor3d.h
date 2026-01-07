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
# include<sys/types.h>
# include<sys/stat.h>

# define BLOCK_SIZE 4096
# define BLOCK_COUNT 8192
# define DISK_SIZE (BLOCK_SIZE * BLOCK_COUNT)

// FS_LITE
# define MAX_FILES 64

// SSD Flash Model
# define PAGE_SIZE 4096
# define PAGES_PER_BLOCK 256
# define ERASE_BLOCK_COUNT 32
# define SSD_GC_THRESHOLD 0.5

// HDD Cost Model (in milliseconds)
# define HDD_SEEK_COST 0.05
# define HDD_ROTATIONAL_LATENCY 2.0
# define HDD_TRANSFER_COST 0.1

typedef struct s_context
{
    int a;
}t_context;

#endif