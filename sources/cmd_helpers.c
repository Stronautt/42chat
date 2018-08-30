/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmd_helpers.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: phrytsenko <phrytsenko@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/08/03 17:34:19 by phrytsenko        #+#    #+#             */
/*   Updated: 2018/08/03 17:57:31 by phrytsenko       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

const char	*validate_room_data(const char ** args, t_dlist ** rooms,
								const t_client * client, t_client ** c_dup)
{
	char		*room_name;

	if (!args || !rooms || !client || !c_dup)
		return ("* Incorrect data passed *");
	*rooms = g_chat_rooms;
	room_name = ft_strtrim(args[0]);
	if (!args[0] || !room_name || !*room_name)
		return ("* You sould specify room name *" + _clean(room_name));
	while (*rooms && (*rooms = (*rooms)->next) != g_chat_rooms)
		if (!strcasecmp(((t_chat_room *)(*rooms)->content)->name, room_name))
			break ;
	*c_dup = _clean(room_name) ? NULL : NULL;
	if (!*rooms || *rooms == g_chat_rooms)
		return ("* Room with this name doesn't exists *");
	else if (*rooms == client->chat_room_node)
		return ("* You are already in this room *");
	else if (!args[1] && ((t_chat_room *)(*rooms)->content)->passwd)
		return ("* Room locked by password *");
	else if (((t_chat_room *)(*rooms)->content)->passwd
				!= hash_data(args[1], ft_strlen(args[1])))
		return ("* Invalid access password to the room *");
	else if (!(*c_dup = malloc(sizeof(t_client))))
		return ("* Server feels bad *");
	ft_memcpy(*c_dup, client, sizeof(t_client));
	return (NULL);
}

const char	*new_chat_room(const char * name, const char * passwd)
{
	t_dlist		* rooms = g_chat_rooms;
	t_chat_room	* new_room;

	if (!name || !*name)
		return ("* You sould specify room name *");
	else if (!nickname_is_valid(name))
		return ("* Invalid room name *");
	else if (!(new_room = ft_memalloc(sizeof(t_chat_room))))
		return ("* Server feels bad *");
	else if (ft_dlstsize(g_chat_rooms) >= MAX_ROOMS_NUMBER)
		return ("* Maximum number of rooms created *" - _clean(new_room));
	new_room->name = ft_strsub(name, 0, ft_cinustrcn(name, MAX_NICKNAME_LEN));
	while (rooms && (rooms = rooms->next) != g_chat_rooms)
		if (!strcasecmp(((t_chat_room *)rooms->content)->name, new_room->name))
			return ("* Room with this name already exists *"
					- _clean(new_room - _clean(new_room->name)));
	new_room->passwd = hash_data(passwd, ft_strlen(passwd));
	sprintf(new_room->log_name, "./logs/log_chat_%s_%ld.txt",
			new_room->name, time(NULL));
	if ((new_room->log_fd = open(new_room->log_name,
		O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP)) < 0)
		return ("* Server feels bad *" - _clean(new_room));
	pthread_mutex_lock(&g_mutex);
	ft_dlstpush_back(&g_chat_rooms, ft_dlstnew(new_room, sizeof(void *)));
	pthread_mutex_unlock(&g_mutex);
	return (NULL);
}
