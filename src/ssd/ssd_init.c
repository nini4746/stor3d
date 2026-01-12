/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ssd_init.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 14:30:00 by yunhpark          #+#    #+#             */
/*   Updated: 2026/01/12 14:30:00 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/stor3d.h"

int	ssd_init(t_ssd_state **ssd)
{
	*ssd = (t_ssd_state *)malloc(sizeof(t_ssd_state));
	if (!*ssd)
	{
		perror("ssd malloc failed");
		return (1);
	}
	memset(*ssd, 0, sizeof(t_ssd_state));
	(*ssd)->free_pages = SSD_TOTAL_PAGES;
	(*ssd)->min_erase_count = 0;
	(*ssd)->max_erase_count = 0;
	return (0);
}

void	ssd_cleanup(t_ssd_state *ssd)
{
	if (ssd)
		free(ssd);
}

int	ssd_read(t_ssd_state *ssd, int disk_fd, size_t lba, void *buf)
{
	off_t	offset;
	ssize_t	bytes_read;

	(void)ssd;
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

int	ssd_write(t_ssd_state *ssd, int disk_fd, size_t lba, const void *buf)
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
	ssd->host_writes++;
	ssd->nand_writes++;
	return (0);
}

void	ssd_print_stats(t_ssd_state *ssd)
{
	(void)ssd;
}
