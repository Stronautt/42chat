/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   update_data.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/09/02 13:55:58 by pgritsen          #+#    #+#             */
/*   Updated: 2018/09/02 15:25:16 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.h"

void			update_rooms(const char *raw)
{
	g_env.layot.u_online_offset = 0;
	ft_dlstclear(&g_env.rooms_avaliable.lines);
	if (raw)
	{
		g_env.rooms_avaliable.lines = ft_strsplit_dlst(raw, ' ');
		g_env.rooms_avaliable.size = ft_dlstsize(g_env.rooms_avaliable.lines);
	}
	render_call(display_rooms, g_env.ws.rooms_a);
}

void			update_users_online(const char *raw)
{
	char	*users;


	g_env.layot.u_online_offset = 0;
	ft_dlstclear(&g_env.users_online.lines);
	if (raw)
	{
		if (!(users = ft_strchr(raw, ':')))
			return ;
		g_env.room_name ? free(g_env.room_name) : 0;
		g_env.room_name = ft_get_content(raw, '[', ']');
		g_env.users_online.lines = ft_strsplit_dlst(users + 1, ' ');
		g_env.users_online.size = ft_dlstsize(g_env.users_online.lines);
	}
	render_call(display_users_online, g_env.ws.u_online);
}

void			update_chat_history(const char *raw)
{
	g_env.layot.chat_offset = 0;
	ft_dlstclear(&g_env.chat_history.lines);
	if (raw)
	{
		g_env.chat_history.lines = ft_strsplit_dlst(raw, '\n');
		g_env.chat_history.size = ft_dlstsize(g_env.chat_history.lines);
	}
	render_call(display_chat, g_env.ws.chat);
}
