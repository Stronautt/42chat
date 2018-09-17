/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   data_exchange.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/08/01 13:14:19 by pgritsen          #+#    #+#             */
/*   Updated: 2018/09/15 12:42:07 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "node.h"

static ssize_t		send_parts(int sockfd, const void *data, ssize_t data_len)
{
	ssize_t	it;
	ssize_t	ret;
	ssize_t	tmp;

	it = -MAX_DATA_SIZE;
	ret = 0;
	while ((it += MAX_DATA_SIZE) <= data_len)
	{
		tmp = (data_len - it > MAX_DATA_SIZE ? MAX_DATA_SIZE : data_len - it);
		if ((tmp = send(sockfd, data + it, tmp, 0)) < 0)
			return (-1);
		ret += tmp;
	}
	return (ret);
}

ssize_t				send_data(int sockfd, const void *data,
								size_t size, t_command command)
{
	t_packet	*packet;

	packet = ft_memalloc(sizeof(t_packet));
	packet->size = data ? size : 0;
	packet->count = ceil((double)packet->size / (double)MAX_DATA_SIZE);
	packet->cmd = command;
	packet->crs_sum = hash_packet(packet, data);
	if (!good_connection(sockfd))
		return (h_clean(packet) - 1);
	else if (send(sockfd, packet, sizeof(t_packet), 0) < 0)
		return (h_clean(packet) - 1);
	else if (size > MAX_DATA_SIZE)
		return (send_parts(sockfd, data, size) - h_clean(packet));
	return (send(sockfd, data, size, 0) - h_clean(packet));
}

static ssize_t		recieve_parts(int sockfd, t_packet *packet,
									void **data, int flg)
{
	void		*ptr;
	ssize_t		it;
	ssize_t		tmp;
	ssize_t		size;

	if (!data || !packet)
		return (-1);
	*data = NULL;
	it = -MAX_DATA_SIZE;
	size = 0;
	while ((it += MAX_DATA_SIZE) <= packet->size)
	{
		tmp = (packet->size - it > MAX_DATA_SIZE
			? MAX_DATA_SIZE : packet->size - it);
		if (!(ptr = malloc(tmp)))
			return (h_clean(*data) - 1);
		else if (tmp != recv(sockfd, ptr, tmp, flg))
			return (h_clean(ptr) - h_clean(*data) - 1);
		*data = ft_memjoin(*data, ptr, size, tmp) - h_clean(*data);
		size += tmp;
	}
	if (size != packet->size || packet->crs_sum != hash_packet(packet, *data))
		return (-1);
	return (size);
}

ssize_t				recieve_data(int sockfd, void **data,
									t_command *command, int flg)
{
	t_packet	packet;
	void		*tmp;

	data ? *data = NULL : 0;
	tmp = NULL;
	if (!good_connection(sockfd)
		|| recv(sockfd, &packet, sizeof(t_packet), flg) < 0 || packet.size < 0
		|| (packet.count && packet.size / packet.count > MAX_DATA_SIZE)
		|| (packet.count > 1 && recieve_parts(sockfd, &packet, &tmp, flg) < 0))
		return (-1);
	else if (packet.count == 1)
	{
		if (!(tmp = ft_memalloc(packet.size))
			|| packet.size != recv(sockfd, tmp, packet.size, flg))
			return (h_clean(tmp) - 1);
		else if (packet.crs_sum != hash_packet(&packet, tmp))
			return (h_clean(tmp) - 1);
	}
	data ? *data = tmp : free(tmp);
	command ? *command = packet.cmd : 0;
	return (packet.size);
}
