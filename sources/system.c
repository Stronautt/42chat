/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   system.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/09/02 13:55:58 by pgritsen          #+#    #+#             */
/*   Updated: 2018/09/02 15:26:42 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

int		setnonblock(int fd)
{
	int flags;

	flags = fcntl(fd, F_GETFL);
	if (flags < 0)
		return (flags);
	else if ((flags = fcntl(fd, F_SETFL, flags | O_NONBLOCK)) < 0)
		return (flags);
	return (0);
}

char	*init_socket(void)
{
	struct sockaddr_in	conn_data;
	static int			h = sizeof(struct sockaddr_in);

	close(g_env.msocket);
	memset(&conn_data, '0', sizeof(conn_data));
	conn_data.sin_family = AF_INET;
	conn_data.sin_addr.s_addr = INADDR_ANY;
	conn_data.sin_port = htons(PORT);
	if ((g_env.msocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return ("Unable to create socket");
	else if (setnonblock(g_env.msocket) < 0)
		return ("Socket can't be in non-blocking mode");
	else if (setsockopt(g_env.msocket, SOL_SOCKET, SO_REUSEPORT, &h, sizeof(h)))
		return ("Unable to set socket option: SO_REUSEPORT");
	else if (bind(g_env.msocket, (struct sockaddr *)&conn_data, h) < 0
			|| listen(g_env.msocket, 3) < 0)
		return (strerror(errno));
	return (0);
}

int		log_errors(int fd, const char *msg)
{
	char		time_s[32];
	time_t		timer;
	struct tm	*tm_info;

	if (!msg)
		return (0);
	time(&timer);
	tm_info = localtime(&timer);
	strftime(time_s, sizeof(time_s), "%Y-%m-%d %H:%M:%S", tm_info);
	return (dprintf(fd, "[%s][pid: %d]: %s\n", time_s, getpid(), msg));
}

void	log_client_actions(t_client *client,
							const char *status, const char *public_status)
{
	time_t		timer;
	char		*msg;
	struct tm	*tm_info;
	t_chat_room	*chat_room;
	t_dlist		*clients;

	time(&timer);
	tm_info = localtime(&timer);
	if (!(msg = ft_strnew(256)))
		return ;
	strftime(msg, 256, "%Y-%m-%d %H:%M:%S", tm_info);
	dprintf(g_env.sys_fd, "[%s][%s] -> %s\n", msg, client->nickname, status);
	sprintf(msg, "* %s %s *", client->nickname, public_status);
	chat_room = client->chat_room_node->content;
	dprintf(chat_room->log_fd, "%s\n", msg);
	clients = chat_room->users;
	while (clients && (clients = clients->next) != chat_room->users)
		if (clients->content && clients != client->node_in_room)
			send_data(((t_client *)clients->content)->sockfd,
												msg, ft_strlen(msg) + 1, 0);
	free(msg);
}

void	disconnect_client(t_dlist *client_node)
{
	t_client	*client;
	t_chat_room	*room;

	client = client_node->content;
	room = client->chat_room_node->content;
	if (*client->nickname)
		log_client_actions(client, "DISCONNECTED", "left the chat");
	event_del(&client->ev);
	close(client->sockfd);
	ft_dlstdelelem_cs(&client->node_in_room);
	ft_dlstdelelem(&client_node);
	update_clients_data(room);
}
