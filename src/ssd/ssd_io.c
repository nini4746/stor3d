/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ssd_io.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 10:30:00 by yunhpark          #+#    #+#             */
/*   Updated: 2026/01/14 10:30:00 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/stor3d.h"

int	ssd_physical_write(int disk_fd, size_t ppa, const void *buf)
{
	off_t		offset;
	ssize_t		bytes_written;

	offset = ppa * BLOCK_SIZE;
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

int	ssd_gc_if_needed(t_ssd_state *ssd, int disk_fd)
{
	while (ssd_gc_needed(ssd))
	{
		if (ssd_run_gc(ssd, disk_fd))
		{
			perror("no space left on device");
			return (1);
		}
	}
	return (0);
}

int	ssd_read(t_ssd_state *ssd, int disk_fd, size_t lba, void *buf)
{
	off_t	offset;
	ssize_t	bytes_read;
	size_t	ppa;

	if (!ssd->ftl_map[lba].valid)
	{
		memset(buf, 0, BLOCK_SIZE);
		return (0);
	}
	ppa = ssd->ftl_map[lba].ppa;
	offset = ppa * BLOCK_SIZE;
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
	ssd->total_time_ms += SSD_READ_COST;
	return (0);
}

int	ssd_write(t_ssd_state *ssd, int disk_fd, size_t lba, const void *buf)
{
	size_t	new_ppa;

	if (ssd->ftl_map[lba].valid)
		ssd_invalidate_page(ssd, ssd->ftl_map[lba].ppa);
	if (ssd_gc_if_needed(ssd, disk_fd))
		return (1);
	new_ppa = ssd_allocate_page(ssd);
	if (new_ppa == (size_t) - 1)
	{
		perror("no space left on device");
		return (1);
	}
	if (ssd_physical_write(disk_fd, new_ppa, buf))
		return (1);
	ssd->ftl_map[lba].ppa = new_ppa;
	ssd->ftl_map[lba].valid = 1;
	ssd->host_writes++;
	ssd->nand_writes++;
	ssd->total_time_ms += SSD_WRITE_COST;
	return (0);
}
