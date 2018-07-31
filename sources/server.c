/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/07/30 13:18:30 by pgritsen          #+#    #+#             */
/*   Updated: 2018/07/31 13:18:23 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "node.h"

int				g_log_file_fd = 0;
t_dlist			* g_clients = NULL;
pthread_mutex_t	g_mutex = PTHREAD_MUTEX_INITIALIZER;

int		good_connection(int sockfd)
{
	int			error = 0;
	socklen_t	len = sizeof (error);

	if (getsockopt (sockfd, SOL_SOCKET, SO_ERROR, &error, &len) != 0)
		return (0);
	else if (error)
		return (0);
	return (1);
}

void	proccess_client_message(t_user * user)
{
	t_dlist	* tmp = g_clients;
	size_t	msg_len;
	size_t	send_buffer_len;
	char	buffer[256];
	char	send_buffer[320];

	while ((msg_len = recv(user->sockfd, buffer, sizeof(buffer), 0)) > 0)
	{
		buffer[msg_len] = 0;
		send_buffer_len = sprintf(send_buffer, "[%s]: %s\n", user->nickname, buffer);
		send_buffer[send_buffer_len] = 0;
		while ((tmp = tmp->next) != g_clients)
			if (tmp->content != user)
			{
				t_user	*tmp_user = tmp->content;
				if (!good_connection(tmp_user->sockfd))
				{
					t_dlist	*prev = tmp->prev;
					ft_dlstdelelem(&tmp);
					tmp = prev;
					continue ;
				}
				send(tmp_user->sockfd, send_buffer, send_buffer_len + 1, 0);
			}
		pthread_mutex_lock(&g_mutex);
		write(g_log_file_fd, send_buffer, send_buffer_len);
		pthread_mutex_unlock(&g_mutex);
	}
}

int		main(void)
{
	struct sockaddr_in	address;
	socklen_t			addrlen = sizeof(address);
	int					server_fd;
	int					new_socket;

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		return (EXIT_FAILURE);
	}

	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, (int[1]){1}, sizeof(int)))
	{
		perror("setsockopt");
		return (EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	if (bind(server_fd, (struct sockaddr *)&address, addrlen) < 0)
	{
		perror("bind failed");
		return (EXIT_FAILURE);
	}
	if (listen(server_fd, 3) < 0)
	{
		perror("listen");
		return (EXIT_FAILURE);
	}

	if ((g_log_file_fd = open("log_chat.txt", O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP)) < 0)
	{
		ft_putendl("Log file can't be created!");
		return (EXIT_FAILURE);
	}

	pthread_t	thread;
	char		invite_msg[] = "Welcome to Chat 1488!\n";

	while ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)))
	{
		send(new_socket, invite_msg, sizeof(invite_msg), 0);
		t_user	*new_user = ft_memalloc(sizeof(t_user));
		new_user->sockfd = new_socket;
		if (recv(new_socket, new_user->nickname, 32, 0) < 0)
		{
			free(new_user);
			continue ;
		}
		ft_dlstpush(&g_clients, ft_dlstnew(new_user, sizeof(t_user)));
		pthread_create(&thread, NULL, (void *(*)(void *))(proccess_client_message), (void *)new_user);
	}
	if (new_socket < 0)
		perror("accept");
	return (0);
}
