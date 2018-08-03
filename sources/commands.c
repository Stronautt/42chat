/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   commands.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: phrytsenko <phrytsenko@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/08/03 17:34:19 by phrytsenko        #+#    #+#             */
/*   Updated: 2018/08/03 17:57:31 by phrytsenko       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "node.h"

void	show_help(t_client * client)
{
	char	send_msg[] =
		"\nAvaliable commands:\n"
		"\t1. /help -> Displays all avaliable commands.\n"
		"\n";
	send_data(client->sockfd, send_msg, sizeof(send_msg), 0);
}
