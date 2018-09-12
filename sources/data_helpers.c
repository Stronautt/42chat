/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   data_helpers.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/08/01 13:14:19 by pgritsen          #+#    #+#             */
/*   Updated: 2018/09/02 15:34:38 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "node.h"

int			good_connection(int sockfd)
{
	int			error;
	socklen_t	len;

	error = 0;
	len = sizeof(error);
	if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) != 0)
		return (0);
	else if (error)
		return (0);
	return (1);
}

uint64_t	hash_data(const void *data, size_t size)
{
	uint64_t		hash;
	unsigned char	*tmp;

	hash = 5381;
	tmp = (unsigned char *)data;
	if (!data || !size)
		return (0);
	while (size--)
		hash = ((hash << 5) + hash) + *tmp++;
	return (hash);
}

uint64_t	hash_packet(const t_packet *packet, const void *data)
{
	uint64_t	ret;

	if (!packet)
		return (0);
	ret = hash_data(data, packet->size);
	ret += (ret << (packet->size % 5));
	ret += hash_data(&packet->cmd, sizeof(packet->cmd))
			* hash_data(&packet->count, sizeof(packet->count));
	return (ret);
}
