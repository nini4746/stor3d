/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   hdd_init.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 14:30:00 by yunhpark          #+#    #+#             */
/*   Updated: 2026/01/12 14:30:00 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/stor3d.h"

int	hdd_init(t_hdd_state **hdd)
{
	*hdd = (t_hdd_state *)malloc(sizeof(t_hdd_state));
	if (!*hdd)
	{
		perror("hdd malloc failed");
		return (1);
	}
	memset(*hdd, 0, sizeof(t_hdd_state));
	(*hdd)->prev_cylinder = -1;
	(*hdd)->prev_head = -1;
	(*hdd)->prev_lba = (size_t)-1;
	return (0);
}

void	hdd_cleanup(t_hdd_state *hdd)
{
	if (hdd)
		free(hdd);
}

int	hdd_read(t_hdd_state *hdd, int disk_fd, size_t lba, void *buf)
{
	off_t	offset;
	ssize_t	bytes_read;

	offset = lba * BLOCK_SIZE;
	if (lseek(disk_fd, offset, SEEK_SET) != offset)
	{
		perror("lseek failed");
		return (1);
	}
	bytes_read = read(disk_fd, buf, BLOCK_SIZE);
	if (bytes_read != BLOCK_SIZE)
	{
		perror("read failed");
		return (1);
	}
	hdd->total_reads++;
	return (0);
}

int	hdd_write(t_hdd_state *hdd, int disk_fd, size_t lba, const void *buf)
{
	off_t		offset;
	ssize_t		bytes_written;

	offset = lba * BLOCK_SIZE;
	if (lseek(disk_fd, offset, SEEK_SET) != offset)
	{
		perror("lseek failed");
		return (1);
	}
	bytes_written = write(disk_fd, buf, BLOCK_SIZE);
	if (bytes_written != BLOCK_SIZE)
	{
		perror("write failed");
		return (1);
	}
	hdd->total_writes++;
	return (0);
}

void	hdd_print_stats(t_hdd_state *hdd)
{
	(void)hdd;
}
