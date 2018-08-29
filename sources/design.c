/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   design.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/07/30 13:18:50 by pgritsen          #+#    #+#             */
/*   Updated: 2018/08/26 21:40:33 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.h"

void			update_rooms(char * raw)
{
	if (!raw)
		return ;
	g_env.layot.u_online_offset = 0;
	ft_dlstclear(&g_env.rooms_avaliable.lines);
	g_env.rooms_avaliable.lines = ft_strsplit_dlst(raw, ' ');
	g_env.rooms_avaliable.size = ft_dlstsize(g_env.rooms_avaliable.lines);
	free(raw);
	display_rooms();
}

void			display_rooms(void)
{
	t_dlist			* tmp;
	int				my;
	int				offset;
	size_t			r_name_l;
	char			* r_name;

	if (g_env.term_size.ws_col < TERM_MIN_WIDTH || g_env.term_size.ws_row < TERM_MIN_HEIGHT)
		return ;
	werase(g_env.ws.rooms_a);
	my = 0;
	tmp = g_env.rooms_avaliable.lines;
	offset = g_env.layot.rooms_a_offset;
	while (tmp && (tmp = tmp->next) != g_env.rooms_avaliable.lines && my <= g_env.ws.rooms_a->_maxy + 1)
	{
		if (offset && offset--)
			continue ;
		r_name = *(char *)tmp->content == *ROOM_LOCKED
			? tmp->content + 1 : tmp->content;
		r_name_l = ft_cinustr(r_name);
		if (*(char *)tmp->content == *ROOM_LOCKED)
			wattron(g_env.ws.rooms_a, COLOR_PAIR(C_COLOR_RED));
		mvwprintw(g_env.ws.rooms_a, my, (g_env.ws.rooms_a->_maxx - r_name_l + 1) / 2, "%s", r_name);
		if (*(char *)tmp->content == *ROOM_LOCKED)
			wattroff(g_env.ws.rooms_a, COLOR_PAIR(C_COLOR_RED));
		my += r_name_l / (g_env.ws.rooms_a->_maxx + 2) + 1;
	}
	wrefresh(g_env.ws.rooms_a);
	rl_forced_update_display();
}

void			update_users_online(char * raw)
{
	if (!raw)
		return ;
	char	* users = ft_strchr(raw, ':');

	if (!users)
		return (free(raw));
	g_env.room_name = ft_get_content(raw, '[', ']');
	g_env.layot.u_online_offset = 0;
	ft_dlstclear(&g_env.users_online.lines);
	g_env.users_online.lines = ft_strsplit_dlst(users + 1, ' ');
	g_env.users_online.size = ft_dlstsize(g_env.users_online.lines);
	free(raw);
	display_users_online();
}

void			display_users_online(void)
{
	t_dlist			* tmp;
	int				my;
	int				offset;
	size_t			nickname_l;

	if (g_env.term_size.ws_col < TERM_MIN_WIDTH || g_env.term_size.ws_row < TERM_MIN_HEIGHT)
		return ;
	werase(g_env.ws.u_online);
	mvwhline(g_env.ws.chat_b, 0, 1, 0, g_env.ws.chat_b->_maxx - 1);
	g_env.room_name ? mvwprintw(g_env.ws.chat_b, 0,
			(g_env.ws.chat_b->_maxx - ft_cinustr(g_env.room_name) - 6) / 2,
			"Room: %s", g_env.room_name) : 0;
	wrefresh(g_env.ws.chat_b);
	my = 0;
	tmp = g_env.users_online.lines;
	offset = g_env.layot.u_online_offset;
	while (tmp && (tmp = tmp->next) != g_env.users_online.lines && my <= g_env.ws.u_online->_maxy + 1)
	{
		if (offset && offset--)
			continue ;
		nickname_l = ft_cinustr(tmp->content);
		mvwprintw(g_env.ws.u_online, my, (g_env.ws.u_online->_maxx - nickname_l + 1) / 2, "%s", tmp->content);
		my += nickname_l / (g_env.ws.u_online->_maxx + 2) + 1;
	}
	wrefresh(g_env.ws.u_online);
	rl_forced_update_display();
}

void			update_chat_history(char * raw)
{
	if (!raw)
		return ;
	g_env.layot.chat_offset = 0;
	ft_dlstclear(&g_env.chat_history.lines);
	g_env.chat_history.lines = ft_strsplit_dlst(raw, '\n');
	g_env.chat_history.size = ft_dlstsize(g_env.chat_history.lines);
	free(raw);
	display_chat();
}

void			display_chat(void)
{
	t_dlist			* tmp;
	int				my;
	int				offset;
	const char		* warning_msg = "Loading...";

	if (g_env.term_size.ws_col < TERM_MIN_WIDTH || g_env.term_size.ws_row < TERM_MIN_HEIGHT)
		return ;
	werase(g_env.ws.chat);
	if (!g_env.nickname || g_env.connection_lost)
		mvwprintw(g_env.ws.chat, g_env.ws.chat->_maxy / 2,
			(g_env.ws.chat->_maxx - ft_cinustr(warning_msg)) / 2, warning_msg);
	else
	{
		my = g_env.ws.chat->_maxy + 1;
		tmp = g_env.chat_history.lines;
		offset = g_env.layot.chat_offset;
		while (tmp && (tmp = tmp->next) != g_env.chat_history.lines && my > 0)
		{
			char	* nickname = ft_get_content(tmp->content, '[', ']');
			char	* msg = ft_strchr(tmp->content, ':');
			bool	self_msg = (nickname && *nickname == *SELF_POINT && nickname++ ? true : false);

			if (offset && offset--)
				continue ;
			if ((my -= ft_cinustr(tmp->content) / (g_env.ws.chat->_maxx + 2) + 1) < 0)
				break ;
			if (!nickname || !*nickname || !msg || !*msg || (tmp->content && *(char *)tmp->content != '['))
				mvwprintw(g_env.ws.chat, my, 0, "%s", tmp->content);
			else
			{
				mvwaddch(g_env.ws.chat, my, 0, '[');
				wattron(g_env.ws.chat, COLOR_PAIR(self_msg ? C_COLOR_GREEN : C_COLOR_CYAN));
				wprintw(g_env.ws.chat, "%s", nickname);
				wattroff(g_env.ws.chat, COLOR_PAIR(self_msg ? C_COLOR_GREEN : C_COLOR_CYAN));
				wprintw(g_env.ws.chat, "]:%s", msg + 1);
				free(nickname - self_msg);
			}
		}
	}
	wrefresh(g_env.ws.chat);
	rl_forced_update_display();
}

void			resize_curses(int sig)
{
	const short		sidebar_w = MAX_NICKNAME_LEN + 2;
	struct winsize	size;

	(void)sig;
	ioctl(fileno(stdout), TIOCGWINSZ, (char*)&g_env.term_size);
	size = g_env.term_size;
	resizeterm(size.ws_row, size.ws_col);
	if (size.ws_col < TERM_MIN_WIDTH || size.ws_row < TERM_MIN_HEIGHT)
	{
		int		offset = (size.ws_col - sidebar_w) / 2;

		curs_set(0);
		erase();
		g_env.ws.input->_begy = size.ws_row - 2;
		wresize(g_env.ws.input, 1, size.ws_col - 2);
		werase(g_env.ws.input);
		offset < 0 ? offset = 0 : 0;
		mvprintw(size.ws_row / 2, offset, "Window too small!");
		wrefresh(g_env.ws.input);
		refresh();
	}
	else
	{
		curs_set(1);
		g_env.ws.input_b->_begy = size.ws_row - 3;
		g_env.ws.u_online_b->_begx = size.ws_col - sidebar_w;
		g_env.ws.rooms_a_b->_begx = size.ws_col - sidebar_w;
		g_env.ws.input->_begy = size.ws_row - 2;
		g_env.ws.u_online->_begx = size.ws_col - sidebar_w + 1;
		g_env.ws.rooms_a->_begx = size.ws_col - sidebar_w + 1;
		wresize(g_env.ws.input_b, 3, size.ws_col);
		wresize(g_env.ws.chat_b, size.ws_row - 3, size.ws_col - sidebar_w);
		wresize(g_env.ws.u_online_b, ceil((size.ws_row - 3) / 2.0), sidebar_w);
		g_env.ws.rooms_a_b->_begy = g_env.ws.u_online_b->_maxy + 1;
		wresize(g_env.ws.rooms_a_b, floor((size.ws_row - 3) / 2.0), sidebar_w);
		wresize(g_env.ws.input, 1, size.ws_col - 2);
		wresize(g_env.ws.chat, size.ws_row - 5, size.ws_col - sidebar_w - 2);
		wresize(g_env.ws.u_online, ceil((size.ws_row - 3) / 2.0) - 2, sidebar_w - 2);
		g_env.ws.rooms_a->_begy = g_env.ws.u_online_b->_maxy + 2;
		wresize(g_env.ws.rooms_a, floor((size.ws_row - 3) / 2.0) - 2, sidebar_w - 2);
		werase(g_env.ws.input_b);
		werase(g_env.ws.u_online_b);
		werase(g_env.ws.rooms_a_b);
		werase(g_env.ws.chat_b);
		box(g_env.ws.chat_b, 0, 0);
		box(g_env.ws.input_b, 0, 0);
		box(g_env.ws.u_online_b, 0, 0);
		box(g_env.ws.rooms_a_b, 0, 0);
		wrefresh(g_env.ws.chat_b);
		mvwprintw(g_env.ws.u_online_b, 0, (sidebar_w - 6) / 2, "Online");
		mvwprintw(g_env.ws.rooms_a_b, 0, (sidebar_w - 5) / 2, "Rooms");
		wrefresh(g_env.ws.u_online_b);
		wrefresh(g_env.ws.rooms_a_b);
		if (g_env.layot.chat_offset + g_env.ws.chat->_maxy > g_env.chat_history.size)
			g_env.layot.chat_offset = 0;
		if (g_env.layot.u_online_offset + g_env.ws.u_online->_maxy > g_env.users_online.size)
			g_env.layot.u_online_offset = 0;
		if (g_env.layot.rooms_a_offset + g_env.ws.rooms_a->_maxy > g_env.rooms_avaliable.size)
			g_env.layot.rooms_a_offset = 0;
		display_rooms();
		display_users_online();
		display_chat();
	}
}

void			init_design(void)
{
	const short	sidebar_w = 20;

	initscr();
	cbreak();
	noecho();
	signal(SIGWINCH, resize_curses);
	if (has_colors() == false)
		curses_exit((void (*)())ft_printf, "Your terminal does not support color\n");
	start_color();
	init_pair(C_COLOR_RED, COLOR_RED, COLOR_BLACK);
	init_pair(C_COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
	init_pair(C_COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
	g_env.ws.input_b = newwin(3, COLS, LINES - 3, 0);
	g_env.ws.chat_b = newwin(LINES - 3, COLS - sidebar_w, 0, 0);
	g_env.ws.u_online_b = newwin(ceil((LINES - 3) / 2.0), sidebar_w, 0, COLS - sidebar_w);
	g_env.ws.rooms_a_b = newwin(floor((LINES - 3) / 2.0), sidebar_w, g_env.ws.u_online_b->_maxy + 1, COLS - sidebar_w + 1);
	g_env.ws.input = newwin(1, COLS - 2, LINES - 2, 1);
	g_env.ws.chat = newwin(LINES - 5, COLS - sidebar_w - 2, 1, 1);
	g_env.ws.u_online = newwin(ceil((LINES - 3) / 2.0) - 2, sidebar_w - 2, 1, COLS - sidebar_w + 1);
	g_env.ws.rooms_a = newwin(floor((LINES - 3) / 2.0) - 2, sidebar_w - 2, g_env.ws.u_online_b->_maxy + 2, COLS - sidebar_w + 1);
	init_readline();
	resize_curses(0);
}

int				curses_exit(void (*clear_callback)(), void * callback_data)
{
	delwin(g_env.ws.input);
	delwin(g_env.ws.chat);
	delwin(g_env.ws.u_online);
	endwin();
	clear_callback ? clear_callback(callback_data) : 0;
	exit(0);
	return (-1);
}
