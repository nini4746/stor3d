/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yunhpark <yunhpark@student.42gyeongsan.kr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/04 13:05:57 by yunhpark          #+#    #+#             */
/*   Updated: 2026/01/07 15:12:55 by yunhpark         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/stor3d.h"

int	main(int argc, char **argv)
{
	t_context	*context;

	context = NULL;
	if (is_valid(argc, argv))
		return (1);
	if (init_context(&context, argv))
		return (1);
	if (run_script(context))
	{
		cleanup_context(context);
		return (1);
	}
	if (context->mode == MODE_HDD)
		hdd_print_stats(context->hdd);
	else
		ssd_print_stats(context->ssd);
	cleanup_context(context);
	return (0);
}
