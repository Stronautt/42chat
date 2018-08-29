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
#include "server.h"

void	show_help(t_client * client)
{
	char	help_msg[] =
		"\nAvaliable commands:\n"
		"    1. /help -> Displays all avaliable commands.\n"
		"    2. /silent -> To enable/disable sound on new messages.\n"
		"    3. /newroom [NAME] [PASSWORD]? -> Creates new room. Can be locked by [PASSWORD].\n"
		"    4. /joinroom [NAME] [PASSWORD]? -> Relocates you to specified room. If it's locked, [PASSWORD] required.\n"
		"\n";

	send_data(client->sockfd, help_msg, sizeof(help_msg), 0);
}

void	toogle_silent_mode(t_client * client)
{
	const char	* msg_e = "* Silent mode enabled *\n";
	const char	* msg_d = "* Silent mode disabled *\n";

	client->silent_mode ^= 1;
	((t_client *)client->node_in_room->content)->silent_mode ^= 1;
	if (client->silent_mode)
		send_data(client->sockfd, msg_e, sizeof(msg_e), 0);
	else
		send_data(client->sockfd, msg_d, sizeof(msg_d), 0);
}

void	create_chat_room(t_client * client, char ** args)
{
	char		* msg;
	char		* room_name;

	room_name = ft_strtrim(args[0]);
	if (!args[0] || ft_strlen(room_name) < 1)
		msg = "* You sould specify room name *\n";
	else if (ft_dlstsize(g_chat_rooms) >= MAX_ROOMS_NUMBER)
		msg = "* Maximum number of rooms created *\n";
	else if (new_chat_room(room_name, args[1]) < 0)
		msg = "* Couldn't create the room *\n";
	else if ((msg = "* Room successfully created *\n"))
		update_room_list(NULL);
	ft_memdel((void **)&room_name);
	send_data(client->sockfd, msg, ft_strlen(msg) + 1, 0);
}

void	join_chat_room(t_client * client, char ** args)
{
	char		* msg;
	char		* room_name;
	uint8_t		room_found = 0;
	t_dlist		* rooms = g_chat_rooms;
	t_client	* tmp;

	room_name = ft_strtrim(args[0]);
	if (!args[0] || ft_strlen(room_name) < 1)
	{
		msg = "* You sould specify room name *\n";
		send_data(client->sockfd, msg, ft_strlen(msg) + 1, 0);
		ft_memdel((void **)&room_name);
		return ;
	}
	while (rooms && (rooms = rooms->next) != g_chat_rooms)
		if (!ft_strcmp(((t_chat_room *)rooms->content)->name, room_name) && ++room_found)
			break ;
	ft_memdel((void **)&room_name);
	if (!room_found)
	{
		msg = "* Room with this name doesn't exists *\n";
		send_data(client->sockfd, msg, ft_strlen(msg) + 1, 0);
		return ;
	}
	else if (rooms == client->chat_room_node)
	{
		msg = "* You are already in this room *\n";
		send_data(client->sockfd, msg, ft_strlen(msg) + 1, 0);
		return ;
	}
	else if (!args[1] && ((t_chat_room *)rooms->content)->passwd)
	{
		msg = "* Room locked by password *\n";
		send_data(client->sockfd, msg, ft_strlen(msg) + 1, 0);
		return ;
	}
	else if (((t_chat_room *)rooms->content)->passwd != hash_data(args[1], ft_strlen(args[1])))
	{
		msg = "* Invalid access password to the room *\n";
		send_data(client->sockfd, msg, ft_strlen(msg) + 1, 0);
		return ;
	}
	else if (!(tmp = malloc(sizeof(t_client))))
		return ;
	ft_memcpy(tmp, client, sizeof(t_client));
	log_client_actions(client, "LEFT_ROOM", "left the room");
	pthread_mutex_lock(&g_mutex);
	ft_dlstdelelem(&client->node_in_room);
	update_clients_data(client->chat_room_node->content);
	client->node_in_room = ft_dlstnew(tmp, sizeof(void *));
	client->chat_room_node = rooms;
	ft_dlstpush(&((t_chat_room *)rooms->content)->users, client->node_in_room);
	pthread_mutex_unlock(&g_mutex);
	update_clients_data(client->chat_room_node->content);
	msg = "* You successfully entered the room *\n";
	send_data(client->sockfd, msg, ft_strlen(msg) + 1, 0);
	sync_chat_history(client);
	log_client_actions(client, "ENTERED_ROOM", "entered the room");
}
