/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/07/31 17:16:33 by pgritsen          #+#    #+#             */
/*   Updated: 2018/07/31 18:21:01 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "node.h"

# define LOG_FILE_PATH "./log_chat.txt"

char			* g_error = NULL;
int				g_log_file_fd = 0;
t_dlist			* g_clients = NULL;
pthread_mutex_t	g_mutex = PTHREAD_MUTEX_INITIALIZER;

const char		* get_error(void)
{
	return (g_error ? g_error : "Unkown error!");
}

void			set_error(const char * msg)
{
	g_error ? ft_memdel((void **)&g_error) : 0;
	g_error = ft_strdup(msg);
}

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

int				init_socket(struct sockaddr_in * conn_data)
{
	int		ret_fd;

	if ((ret_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		set_error("Unable to create socket");
		return (-1);
	}
	else if (setsockopt(ret_fd, SOL_SOCKET, SO_REUSEPORT, (int[1]){1}, sizeof(int)))
	{
		set_error("Unable to set socket option: SO_REUSEPORT");
		return (-1);
	}
	memset(conn_data, '0', sizeof(*conn_data));
	conn_data->sin_family = AF_INET;
	conn_data->sin_addr.s_addr = INADDR_ANY;
	conn_data->sin_port = htons(PORT);
	if (bind(ret_fd, (struct sockaddr *)conn_data, sizeof(*conn_data)) < 0 || listen(ret_fd, 3) < 0)
	{
		set_error(strerror(errno));
		return (-1);
	}
	else if ((g_log_file_fd = open(LOG_FILE_PATH, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP)) < 0)
	{
		set_error("Log file can't be created!");
		return (-1);
	}
	return (ret_fd);
}

void	sync_chat_history(t_client * client)
{
	int		log_fd;

	if ((log_fd = open(LOG_FILE_PATH, O_RDONLY)) > 0)
	{
		char	buffer[1024];
		size_t	buf_len;

		while ((buf_len = read(log_fd, buffer, sizeof(buffer) - 1)))
		{
			buffer[buf_len] = 0;
			send(client->sockfd, buffer, sizeof(buffer), 0);
		}
		buffer[0] = 0;
		send(client->sockfd, buffer, sizeof(buffer), 0);
	}
}

void			handle_client(t_dlist * client_node)
{
	char		msg[256];
	size_t		msg_len;
	char		public_msg[512];
	size_t		public_msg_len;
	char		invite_msg[32] = "Welcome to 42Chat!\n";
	t_client	* client = client_node->content;
	t_dlist		* clients = g_clients;

	if (!good_connection(client->sockfd))
	{
		pthread_mutex_lock(&g_mutex);
		ft_dlstdelelem(&client_node);
		pthread_mutex_unlock(&g_mutex);
		return ;
	}
	send(client->sockfd, invite_msg, sizeof(invite_msg), 0);
	recv(client->sockfd, client->nickname, 32, 0);
	sync_chat_history(client);
	while ((msg_len = recv(client->sockfd, msg, sizeof(msg), 0)) > 0)
	{
		msg[msg_len] = 0;
		public_msg_len = sprintf(public_msg, "[%s]: %s\n", client->nickname, msg);
		public_msg[public_msg_len] = 0;
		pthread_mutex_lock(&g_mutex);
		write(g_log_file_fd, public_msg, public_msg_len);
		pthread_mutex_unlock(&g_mutex);
		while ((clients = clients->next) != g_clients)
			if (clients->content != client)
			{
				t_client	*tmp_client = clients->content;

				if (!good_connection(tmp_client->sockfd))
				{
					t_dlist	*prev = clients->prev;
					pthread_mutex_lock(&g_mutex);
					ft_dlstdelelem(&clients);
					pthread_mutex_unlock(&g_mutex);
					clients = prev;
					continue ;
				}
				send(tmp_client->sockfd, public_msg, public_msg_len + 1, 0);
			}
	}
}

void			handle_connections(int server, struct sockaddr_in * conn_data)
{
	pthread_t	thread;
	int			new_conn;
	t_client	* new_client;
	t_dlist		* new_node;
	socklen_t	conn_len = sizeof(*conn_data);

	while (1)
	{
		if ((new_conn = accept(server, (struct sockaddr *)conn_data, &conn_len)) < 0)
		{
			perror("Accept failed");
			break ;
		}
		if (!(new_client = ft_memalloc(sizeof(t_client))))
			continue ;
		new_client->sockfd = new_conn;
		new_node = ft_dlstnew(new_client, sizeof(t_client));
		ft_dlstpush(&g_clients, new_node);
		pthread_create(&thread, NULL, (void *(*)(void *))(handle_client), (void *)new_node);
		pthread_detach(thread);
	}
}

int				main(void)
{
	struct sockaddr_in	conn_data;
	int					server_socket;
	pid_t				server_pid = fork();

	if (server_pid)
		return (ft_printf(server_pid < 0 ? "Server start failed!"
			: "Server pid -> [%d]\n", server_pid) * 0 + EXIT_FAILURE);
	setsid();
	if ((server_socket = init_socket(&conn_data)) < 0)
		return (ft_printf("%s\n", get_error()) * 0 + EXIT_FAILURE);
	handle_connections(server_socket, &conn_data);
	return (0);
}
