/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmd_interfaces.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/08/03 17:34:19 by phrytsenko        #+#    #+#             */
/*   Updated: 2018/09/02 14:12:15 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

void	show_help(t_client *client)
{
	const char	help_msg[] =

	"\nAvaliable commands:\n"
	"    1. /help -> Displays all avaliable commands.\n"
	"    2. /silent -> To enable/disable sound on new messages.\n"
	"    3. /newroom [NAME] [PASSWORD]? -> Creates new room."
	" Can be locked by [PASSWORD].\n"
	"    4. /joinroom [NAME] [PASSWORD]? -> Relocates you to specified room."
	" If it's locked, [PASSWORD] required.\n"
	"    5. /pm [NICKNAME] [MSG] -> Send private message.\n"
	"    6. /block [NICKNAME] -> To block/unblock certain user.";
	send_data(client->sockfd, help_msg, sizeof(help_msg), 0);
}

void	toogle_silent_mode(t_client *client)
{
	const char	*msg_e = "* Silent mode enabled *";
	const char	*msg_d = "* Silent mode disabled *";

	client->silent_mode ^= 1;
	client->silent_mode
		? send_data(client->sockfd, msg_e, ft_strlen(msg_e) + 1, 0)
		: send_data(client->sockfd, msg_d, ft_strlen(msg_d) + 1, 0);
}

void	create_chat_room(t_client *client, const char **args)
{
	const char	*msg;
	char		*room_name;

	room_name = ft_strtrim(args[0]);
	!(msg = new_chat_room(room_name, args[1], args[1] && *args[1]))
		? update_room_list(NULL) : 0;
	free(room_name);
	!msg ? msg = "* Room successfully created *" : 0;
	send_data(client->sockfd, msg, ft_strlen(msg) + 1, 0);
}

void	join_chat_room(t_client *client, const char **args)
{
	const char	*msg;
	t_dlist		*rooms;

	if ((msg = validate_room_data(args, &rooms, client)))
		return ((void)send_data(client->sockfd, msg, ft_strlen(msg) + 1, 0));
	log_client_actions(client, "LEFT_ROOM", "left the room");
	ft_dlstdelelem_cs(&client->node_in_room);
	update_clients_data(client->chat_room_node->content);
	client->node_in_room = ft_dlstnew(client, sizeof(void *));
	client->chat_room_node = rooms;
	ft_dlstpush(&((t_chat_room *)rooms->content)->users, client->node_in_room);
	update_clients_data(client->chat_room_node->content);
	msg = "* You successfully entered the room *";
	send_data(client->sockfd, msg, ft_strlen(msg) + 1, 0);
	sync_chat_history(client);
	log_client_actions(client, "ENTERED_ROOM", "entered the room");
}
