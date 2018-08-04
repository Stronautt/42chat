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
		"\t1. /help -> Displays all avaliable commands.\n"
		"\t2. /silent -> To enable/disable sound on new messages.\n"
		"\t3. /showrooms -> Displays all avaliable rooms.\n"
		"\t4. /newroom [NAME] [PASSWORD]? -> Creates new room. Can be locked by [PASSWORD].\n"
		"\t5. /joinroom [NAME] [PASSWORD]? -> Relocates you to specified room. If it's locked, [PASSWORD] required.\n"
		"\n";

	send_data(client->sockfd, help_msg, sizeof(help_msg), 0);
}

void	toogle_silent_mode(t_client * client)
{
	char		msg_e[] = "* Silent mode enabled *\n";
	char		msg_d[] = "* Silent mode disabled *\n";

	client->silent_mode ^= 1;
	if (client->silent_mode)
		send_data(client->sockfd, msg_e, sizeof(msg_e), 0);
	else
		send_data(client->sockfd, msg_d, sizeof(msg_d), 0);
}

void	show_all_rooms(t_client * client)
{
	char	* trash;
	char	* anwser;
	t_dlist	* rooms = g_chat_rooms;
	int		first = 1;

	anwser = ft_strdup("Avaliable rooms: ");
	while (rooms && (rooms = rooms->next) != g_chat_rooms)
	{
		if (!first)
		{
			trash = ft_strjoin(anwser, ", ");
			free(anwser);
			anwser = trash;
		}
		first = 0;
		trash = ft_strjoin(anwser, ((t_chat_room *)rooms->content)->name);
		free(anwser);
		anwser = trash;
	}
	trash = ft_strjoin(anwser, ".\n");
	free(anwser);
	anwser = trash;
	send_data(client->sockfd, anwser, ft_strlen(anwser) + 1, 0);
}

void	show_users_in_room(t_client * client)
{
	char	* trash;
	char	* anwser;
	t_dlist	* clients = ((t_chat_room *)client->chat_room_node->content)->users;
	int		first = 1;

	anwser = ft_strnew(256);
	sprintf(anwser, "Online users in '%s' room (%zu):\n\t", ((t_chat_room *)client->chat_room_node->content)->name, ft_dlstsize(clients));
	while (clients && (clients = clients->next) != ((t_chat_room *)client->chat_room_node->content)->users)
	{
		if (!first)
		{
			trash = ft_strjoin(anwser, ", ");
			free(anwser);
			anwser = trash;
		}
		first = 0;
		trash = ft_strjoin(anwser, ((t_client *)clients->content)->nickname);
		free(anwser);
		anwser = trash;
	}
	trash = ft_strjoin(anwser, ".\n\n");
	free(anwser);
	anwser = trash;
	send_data(client->sockfd, anwser, ft_strlen(anwser) + 1, 0);
}

void	create_chat_room(t_client * client, char ** args)
{
	char		* msg;
	t_dlist		* rooms = g_chat_rooms;
	char		* room_name;
	size_t		rooms_c = 0;

	room_name = ft_strtrim(args[0]);
	if (!args[0] || ft_strlen(room_name) < 1)
	{
		msg = "* You sould specify room name *\n";
		send_data(client->sockfd, msg, ft_strlen(msg) + 1, 0);
		ft_memdel((void **)&room_name);
		return ;
	}
	rooms_c = ft_dlstsize(g_chat_rooms);
	if (rooms_c >= 1024)
	{
		msg = "* Maximum number of rooms created *\n";
		send_data(client->sockfd, msg, ft_strlen(msg) + 1, 0);
		ft_memdel((void **)&room_name);
		return ;
	}
	while (rooms && (rooms = rooms->next) != g_chat_rooms)
		if (!ft_strcmp(((t_chat_room *)rooms->content)->name, room_name))
		{
			msg = "* Room with this name already exists *\n";
			send_data(client->sockfd, msg, ft_strlen(msg) + 1, 0);
			ft_memdel((void **)&room_name);
			return ;
		}
	if (new_chat_room(room_name, args[1]) < 0)
	{
		msg = "* Couldn't create room *\n";
		send_data(client->sockfd, msg, ft_strlen(msg) + 1, 0);
		ft_memdel((void **)&room_name);
		return ;
	}
	ft_memdel((void **)&room_name);
	msg = "* Room successfully created *\n";
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
	pthread_mutex_lock(&g_mutex);
	ft_dlstdelelem(&client->node_in_room);
	client->node_in_room = ft_dlstnew(tmp, sizeof(void *));
	client->chat_room_node = rooms;
	ft_dlstpush(&((t_chat_room *)rooms->content)->users, client->node_in_room);
	pthread_mutex_unlock(&g_mutex);
	msg = "* You successfully entered room *\n";
	send_data(client->sockfd, msg, ft_strlen(msg) + 1, 0);
	sync_chat_history(client);
}
