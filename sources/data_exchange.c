/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   data_exchange.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/08/01 13:14:19 by pgritsen          #+#    #+#             */
/*   Updated: 2018/08/01 17:03:23 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "node.h"

int		good_connection(int sockfd)
{
	int			error = 0;
	socklen_t	len = sizeof(error);

	if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) != 0)
		return (0);
	else if (error)
		return (0);
	return (1);
}

uint64_t	hash_data(const void * data, size_t size)
{
	uint64_t		hash = 5381;
	unsigned char	* tmp = (unsigned char *)data;

	if (!data || !size)
		return (0);
	while (size--)
		hash = ((hash << 5) + hash) + *tmp++;
	return (hash);
}

ssize_t		send_data(int sockfd, const void * data,
						size_t size, t_command command)
{
	t_packet	packet;

	packet.size = size;
	packet.cmd = command;
	packet.crs_sum = hash_data(data, size);
	packet.crs_sum += (packet.crs_sum << (packet.size % 5));
	packet.crs_sum += packet.cmd;
	if (!good_connection(sockfd))
		return (-1);
	else if (send(sockfd, &packet, sizeof(t_packet), 0) < 0)
		return (-1);
	return (send(sockfd, data, size, 0));
}

ssize_t		recieve_data(int sockfd, void ** data,
							t_command * command, int flags)
{
	t_packet	packet;
	ssize_t		ret;
	void		* tmp;
	uint64_t	tmp_crs;

	data ? *data = 0 : 0;
	if (!good_connection(sockfd))
		return (-1);
	else if (recv(sockfd, &packet, sizeof(t_packet), flags) < 0)
		return (-1);
	else if (!(tmp = ft_memalloc(packet.size)))
		return (-1);
	else if ((ret = recv(sockfd, tmp, packet.size, flags)) < 0)
		return (_clean(tmp) - 1);
	tmp_crs = hash_data(tmp, packet.size);
	if (packet.crs_sum != tmp_crs + (tmp_crs << (packet.size % 5)) + packet.cmd)
		return (_clean(tmp) - 1);
	data ? *data = tmp : free(tmp);
	command ? *command = packet.cmd : 0;
	return (ret);
}
