/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   validation.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/04 13:14:22 by yunhpark          #+#    #+#             */
/*   Updated: 2026/01/04 14:45:26 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/stor3d.h"

int is_valid(int argc, char **argv)
{
    if (argc != 4)
	{
		perror("usage : ./stor3D <mode> <disk.img> <script.txt>");
		return (1);
	}
    if (strcmp(argv[1],"hdd") && strcmp(argv[1],"ssd") != 0)
    {
        perror("mode is only hdd, ssd");
		return (1);
    }
    if (valid_image(argv[1], argv[2])) 
        return (1);

}

int valid_image(char *mod,const char *image_path)
{
    // 도커 이미지가 없으면 생성, 있으면 크기 맞는지 확인해야함
    int fd;
    
    if ( (fd = open(image_path, O_RDWR) )< 0)
    {
        
    }
}
