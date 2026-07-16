/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   hdd_helpers.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 08:50:00 by yunhpark          #+#    #+#             */
/*   Updated: 2026/01/14 08:50:00 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/stor3d.h"

int	hdd_lba_to_chs(size_t lba, int *cylinder, int *head, int *sector)
{
	size_t	sectors_per_cylinder;

	sectors_per_cylinder = HDD_HEADS * HDD_SECTORS_PER_TRACK;
	*cylinder = lba / sectors_per_cylinder;
	*head = (lba % sectors_per_cylinder) / HDD_SECTORS_PER_TRACK;
	*sector = lba % HDD_SECTORS_PER_TRACK;
	return (0);
}

/* Spec v2 §9.2: zone 0 = cylinders 0-1, zone 1 = cylinder 2, zone 2 = 3. */
int	hdd_get_zone(int cylinder)
{
	if (cylinder <= 1)
		return (0);
	else if (cylinder == 2)
		return (1);
	else
		return (2);
}

double	hdd_get_transfer_time(int zone)
{
	if (zone == 0)
		return (HDD_ZONE_TRANSFER_TIME_0);
	else if (zone == 1)
		return (HDD_ZONE_TRANSFER_TIME_1);
	else
		return (HDD_ZONE_TRANSFER_TIME_2);
}

double	hdd_calculate_seek_cost(t_hdd_state *hdd, int cylinder, int head)
{
	double	cost;

	cost = 0.0;
	if (hdd->prev_cylinder >= 0)
	{
		if (cylinder != hdd->prev_cylinder)
		{
			cost = (double)(cylinder - hdd->prev_cylinder);
			cost *= HDD_CYLINDER_SEEK_COST;
			if (cost < 0)
				cost = -cost;
			hdd->cylinder_seeks++;
		}
		if (head != hdd->prev_head)
		{
			cost += (double)abs(head - hdd->prev_head)
				* HDD_HEAD_SWITCH_COST;
			hdd->head_switches++;
		}
	}
	return (cost);
}

double	hdd_calc_read_cost(t_hdd_state *hdd, int cyl, int head, int zone)
{
	double	cost;

	cost = hdd_calculate_seek_cost(hdd, cyl, head);
	cost += HDD_ROTATION_TIME;
	cost += hdd_get_transfer_time(zone);
	return (cost);
}
