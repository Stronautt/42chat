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

static int	_clean(void * data)
{
	free(data);
	return (-1);
}

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

	if (!data || !size)
		return (0);
	while (size--)
	{
		c = *tmp++;
		hash = ((hash << 5) + hash) + c;
	}
	return (hash);
}

ssize_t		send_data(int sockfd, const void * data, size_t size, t_command command)
{
	ssize_t			ret;
	uint64_t		hash = hash_data(data, size);
	unsigned char	*msg = ft_memalloc(size + sizeof(t_command));

	ft_memcpy(msg, &command, sizeof(t_command));
	ft_memcpy(msg + sizeof(t_command), data, size);
	if (!good_connection(sockfd))
		return (_clean(msg));
	else if (send(sockfd, &size, sizeof(size), 0) < 0)
		return (_clean(msg));
	else if (send(sockfd, &hash, sizeof(hash), 0) < 0)
		return (_clean(msg));
	ret = send(sockfd, msg, size + sizeof(t_command), 0);
	free(msg);
	return (ret);
}

ssize_t		recieve_data(int sockfd, void ** data, t_command * command, int fl)
{
	unsigned char	*msg;
	uint64_t		hash = 0;
	size_t			size = 0;
	ssize_t			ret = 0;

	data ? *data = 0 : 0;
	if (!good_connection(sockfd))
		return (-1);
	else if (recv(sockfd, &size, sizeof(size), fl) < 0)
		return (-1);
	else if (recv(sockfd, &hash, sizeof(hash), fl) < 0)
		return (-1);
	else if (!(msg = ft_memalloc(size + sizeof(t_command))))
		return (-1);
	else if ((ret = recv(sockfd, msg, size + sizeof(t_command), fl)) < 0)
		return (_clean(msg));
	else if (hash != hash_data(msg + sizeof(t_command), size))
		return (_clean(msg));
	command ? ft_memcpy(command, msg, sizeof(t_command)) : 0;
	data && size ? ft_memcpy((*data = malloc(size)), msg + sizeof(t_command), size) : 0;
	free(msg);
	return (ret);
}
