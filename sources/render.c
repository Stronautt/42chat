/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/07/30 13:18:50 by pgritsen          #+#    #+#             */
/*   Updated: 2018/09/02 15:27:32 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.h"

void		render_call(void (*func)(), WINDOW *target)
{
	if (!func || !target || g_env.term_size.ws_col < TERM_MIN_WIDTH
		|| g_env.term_size.ws_row < TERM_MIN_HEIGHT)
		return ;
	werase(target);
	func();
	wrefresh(target);
	wrefresh(g_env.ws.input);
}

void		display_rooms(void)
{
	t_dlist	*tmp;
	int		my;
	int		offset;
	size_t	r_name_l;
	bool	r_locked;

	mvwhline(g_env.ws.rooms_a_b, g_env.ws.rooms_a_b->_maxy, 1, 0,
		g_env.ws.rooms_a_b->_maxx - 1);
	mvwprintw(g_env.ws.rooms_a_b, g_env.ws.rooms_a_b->_maxy, 1,
				"> %zu <", g_env.rooms_avaliable.size);
	my = wrefresh(g_env.ws.rooms_a_b) * 0;
	offset = g_env.layot.rooms_a_offset;
	tmp = g_env.rooms_avaliable.lines;
	while (tmp && (tmp = tmp->next) != g_env.rooms_avaliable.lines
			&& my <= g_env.ws.rooms_a->_maxy + 1)
		if (offset-- <= 0)
		{
			r_locked = *(char *)tmp->content == *ROOM_LOCKED ? true : false;
			r_name_l = ft_cinustr(tmp->content + r_locked);
			r_locked ? wattron(g_env.ws.rooms_a, COLOR_PAIR(C_COLOR_RED)) : 0;
			mvwprintw(g_env.ws.rooms_a, my, (g_env.ws.rooms_a->_maxx
						- r_name_l + 1) / 2, tmp->content + r_locked);
			r_locked ? wattroff(g_env.ws.rooms_a, COLOR_PAIR(C_COLOR_RED)) : 0;
			my += r_name_l / (g_env.ws.rooms_a->_maxx + 2) + 1;
		}
}

void		display_users_online(void)
{
	t_dlist	*tmp;
	int		my;
	int		offset;
	size_t	nickname_l;

	mvwhline(g_env.ws.chat_b, 0, 1, 0, g_env.ws.chat_b->_maxx - 1);
	mvwhline(g_env.ws.u_online_b, g_env.ws.u_online_b->_maxy, 1, 0,
		g_env.ws.u_online_b->_maxx - 1);
	g_env.room_name ? mvwprintw(g_env.ws.chat_b, 0,
			(g_env.ws.chat_b->_maxx - ft_cinustr(g_env.room_name) - 10) / 2,
			"> Room: %s <", g_env.room_name) : 0;
	mvwprintw(g_env.ws.u_online_b, g_env.ws.u_online_b->_maxy, 1,
			"> %zu <", g_env.users_online.size);
	my = (wrefresh(g_env.ws.u_online_b) + wrefresh(g_env.ws.chat_b)) * 0;
	tmp = g_env.users_online.lines;
	offset = g_env.layot.u_online_offset;
	while (tmp && (tmp = tmp->next) != g_env.users_online.lines
			&& my <= g_env.ws.u_online->_maxy + 1)
		if (offset-- <= 0)
		{
			mvwprintw(g_env.ws.u_online, my, (g_env.ws.u_online->_maxx -
				(nickname_l = ft_cinustr(tmp->content)) + 1) / 2, tmp->content);
			my += nickname_l / (g_env.ws.u_online->_maxx + 2) + 1;
		}
}

static void	draw_msg(int *l_n, const char *raw, WINDOW *tg, int *offset_row)
{
	char	*u_name;
	char	*msg;
	bool	own_msg;
	int		c;

	if (!raw || (*offset_row && (*offset_row)--)
		|| (*l_n -= ft_cinustr(raw) / (tg->_maxx + 4) + 1) < 0)
		return ;
	u_name = ft_get_content(raw, '[', ']');
	own_msg = (u_name && *u_name == *SELF_POINT && u_name++ ? 1 : 0);
	if (!u_name || !*u_name || !(msg = ft_strchr(raw, ':')) || !*msg
		|| (*(char *)raw != *MSG_POINT && *(char *)raw != *PRIVATE_MSG_POINT))
		mvwprintw(tg, *l_n, 0, raw);
	else
	{
		c = *(char *)raw == *PRIVATE_MSG_POINT ? C_COLOR_MAGENTA : C_COLOR_CYAN;
		mvwaddch(tg, *l_n, 0, '[');
		wattron(tg, COLOR_PAIR(own_msg ? C_COLOR_GREEN : c));
		wprintw(tg, "%s", u_name);
		wattroff(tg, COLOR_PAIR(own_msg ? C_COLOR_GREEN : c));
		wprintw(tg, "]:%s", msg + 1);
	}
	free(u_name - own_msg);
}

void		display_chat(void)
{
	t_dlist		*msg;
	int			l_n;
	int			offset;
	const char	*warning_msg = "Loading...";

	if (!g_env.nickname || g_env.connection_lost)
		mvwprintw(g_env.ws.chat, g_env.ws.chat->_maxy / 2,
			(g_env.ws.chat->_maxx - ft_cinustr(warning_msg)) / 2, warning_msg);
	else
	{
		mvwhline(g_env.ws.chat_b, g_env.ws.chat_b->_maxy, 1, 0,
			g_env.ws.chat_b->_maxx - 1);
		mvwprintw(g_env.ws.chat_b, g_env.ws.chat_b->_maxy,
			g_env.ws.chat_b->_maxx - ft_nbrullen(g_env.chat_history.size)
			- ft_nbrullen(g_env.layot.chat_offset) - 12, "> l: %zu,"
			" o: %zu <", g_env.chat_history.size, g_env.layot.chat_offset);
		wrefresh(g_env.ws.chat_b);
		l_n = g_env.ws.chat->_maxy + 1;
		msg = g_env.chat_history.lines;
		offset = g_env.layot.chat_offset;
		while (msg && (msg = msg->next) != g_env.chat_history.lines && l_n > 0)
			draw_msg(&l_n, msg->content, g_env.ws.chat, &offset);
	}
}
