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
	int				c;

	if (!data)
		return (0);
	while (size--)
	{
		c = *tmp++;
		hash = ((hash << 5) + hash) + c;
	}
	return (hash);
}

ssize_t		send_data(int sockfd, const void * data, size_t size, int flags)
{
	uint64_t	hash = hash_data(data, size);

	if (!good_connection(sockfd))
		return (-1);
	else if (size > INT16_MAX || !size)
		return (-1);
	else if (send(sockfd, &size, sizeof(size), flags) < 0)
		return (-1);
	else if (send(sockfd, &hash, sizeof(hash), flags) < 0)
		return (-1);
	return (send(sockfd, data, size, flags));
}

ssize_t		send_command(int sockfd, t_command command, int flags)
{
	if (!good_connection(sockfd))
		return (-1);
	return (send(sockfd, &command, sizeof(command), flags));
}

ssize_t		recieve_data(int sockfd, void ** data, int flags)
{
	uint64_t	recieved_hash = 0;
	size_t		size = 0;
	ssize_t		ret = 0;

	if (!good_connection(sockfd))
		return (-1);
	else if (recv(sockfd, &size, sizeof(size), flags) < 0)
		return (-1);
	else if (size > INT16_MAX || !size)
		return (-1);
	else if (recv(sockfd, &recieved_hash, sizeof(recieved_hash), flags) < 0)
		return (-1);
	else if (!(*data = ft_memalloc(size)))
		return (-1);
	else if ((ret = recv(sockfd, *data, size, flags)) < 0)
		return (-1);
	else if (recieved_hash != hash_data(*data, size))
		return (-1);
	return (ret);
}

ssize_t		recieve_command(int sockfd, t_command * data, int flags)
{
	if (!good_connection(sockfd))
		return (-1);
	return (recv(sockfd, data, sizeof(*data), flags));
}
