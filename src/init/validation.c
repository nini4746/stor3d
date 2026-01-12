/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   validation.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/04 13:14:22 by yunhpark          #+#    #+#             */
/*   Updated: 2026/01/07 15:05:01 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/stor3d.h"

int	is_valid(int argc, char **argv)
{
	if (argc != 4)
	{
		perror("usage : ./stor3D <mode> <disk.img> <script.txt>");
		return (1);
	}
	if (strcmp(argv[1], "hdd") != 0 && strcmp(argv[1], "ssd") != 0)
	{
		perror("mode is only hdd, ssd");
		return (1);
	}
	if (valid_image(argv[2]))
		return (1);
	if (valid_script(argv[3]))
		return (1);
	return (0);
}

int	create_image(const char *image_path)
{
	int		fd;
	char	buf[BLOCK_SIZE];
	size_t	i;

	fd = open(image_path, O_RDWR | O_CREAT, 0644);
	if (fd < 0)
	{
		perror("cannot open disk image");
		return (1);
	}
	memset(buf, 0, BLOCK_SIZE);
	i = 0;
	while (i < BLOCK_COUNT)
	{
		if (write(fd, buf, BLOCK_SIZE) != BLOCK_SIZE)
		{
			perror("cannot create disk image");
			close(fd);
			return (1);
		}
		i++;
	}
	close(fd);
	return (0);
}

int	valid_image(const char *image_path)
{
	int		fd;
	long	size;

	fd = open(image_path, O_RDWR);
	if (fd < 0)
	{
		if (create_image(image_path))
			return (1);
	}
	else
	{
		size = lseek(fd, 0, SEEK_END);
		if (size != DISK_SIZE)
		{
			perror("invalid disk image size");
			close(fd);
			return (1);
		}
		close(fd);
	}
	return (0);
}

int	valid_script(const char *script_path)
{
	int fd;

	fd = open(script_path, O_RDONLY);
	if (fd < 0)
	{
		perror("cannot open script");
		return (1);
	}
	close(fd);
	return (0);
}
