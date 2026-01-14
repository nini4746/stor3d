/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   hdd_io.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 09:10:00 by yunhpark          #+#    #+#             */
/*   Updated: 2026/01/14 09:10:00 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/stor3d.h"

int	hdd_do_physical_read(int disk_fd, size_t lba, void *buf)
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
	return (0);
}

int	hdd_do_physical_write(int disk_fd, size_t lba, const void *buf)
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
	return (0);
}
