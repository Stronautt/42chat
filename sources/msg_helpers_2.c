/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   msg_helpers_2.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/09/02 15:12:36 by pgritsen          #+#    #+#             */
/*   Updated: 2018/09/15 12:45:08 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

char	*get_room_data(t_chat_room *room)
{
	char	*ret;
	t_dlist	*user;

	if (!room)
		return (NULL);
	user = room->users;
	ret = malloc((int)MAX_NICKNAME_LEN * 8 + 16);
	sprintf(ret, "[%s]: ", room->name);
	while (user && (user = user->next) != room->users)
	{
		ret = ft_strjoin(ret, ((t_client *)user->content)->nickname)
													- h_clean(ret);
		ret = ft_strjoin(ret, " ") - h_clean(ret);
	}
	return (ret);
}

char	*get_room_list(void)
{
	char		*ret;
	t_dlist		*room_node;
	t_chat_room	*room;

	ret = ft_strnew(0);
	room_node = g_env.all_rooms;
	while (room_node && (room_node = room_node->next) != g_env.all_rooms)
	{
		room = (t_chat_room *)room_node->content;
		room->passwd ? (ret = ft_strjoin(ret, ROOM_LOCKED) - h_clean(ret)) : 0;
		ret = ft_strjoin(ret, room->name) - h_clean(ret);
		ret = ft_strjoin(ret, " ") - h_clean(ret);
	}
	return (ret);
}

void	update_clients_data(t_chat_room *room)
{
	size_t		data_len;
	char		*data;
	t_dlist		*user;

	data = get_room_data(room);
	data_len = ft_strlen(data);
	user = room->users;
	while (user && (user = user->next) != room->users)
		if (send_data(((t_client *)user->content)->sockfd,
				data, data_len + 1, UPDATE_USERS) < 0 && (user = user->prev))
			disconnect_client(
				find_user_addr(user->next->content, g_env.all_clients));
	free(data);
}

void	update_room_list(t_client *client)
{
	t_dlist	*client_node;
	char	*r_list;

	r_list = get_room_list();
	if (client)
		send_data(client->sockfd, r_list, ft_strlen(r_list) + 1, UPDATE_ROOMS);
	else if ((client_node = g_env.all_clients))
		while ((client_node = client_node->next) != g_env.all_clients)
			if (send_data(((t_client *)client_node->content)->sockfd, r_list,
					ft_strlen(r_list) + 1, UPDATE_ROOMS) < 0
				&& (client_node = client_node->prev))
				disconnect_client(client_node->next);
	free(r_list);
}
