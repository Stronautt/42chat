/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   curses.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/09/02 13:55:58 by pgritsen          #+#    #+#             */
/*   Updated: 2018/09/02 15:18:43 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.h"

static void		redraw_windows(const short s_w)
{
	box(g_env.ws.chat_b, 0, 0);
	box(g_env.ws.input_b, 0, 0);
	box(g_env.ws.u_online_b, 0, 0);
	box(g_env.ws.rooms_a_b, 0, 0);
	wrefresh(g_env.ws.chat_b);
	mvwprintw(g_env.ws.u_online_b, 0, (s_w - 10) / 2, "> Online <");
	mvwprintw(g_env.ws.rooms_a_b, 0, (s_w - 9) / 2, "> Rooms <");
	wrefresh(g_env.ws.u_online_b);
	wrefresh(g_env.ws.rooms_a_b);
	g_env.layot.chat_offset + g_env.ws.chat->_maxy - g_env.layot.chat_msg_h_corr
		+ 1	> g_env.chat_history.size ? g_env.layot.chat_offset = 0 : 0;
	g_env.layot.u_online_offset + g_env.ws.u_online->_maxy
		> g_env.users_online.size ? g_env.layot.u_online_offset = 0 : 0;
	g_env.layot.rooms_a_offset + g_env.ws.rooms_a->_maxy
		> g_env.rooms_avaliable.size ? g_env.layot.rooms_a_offset = 0 : 0;
	render_call(display_rooms, g_env.ws.rooms_a);
	render_call(display_users_online, g_env.ws.u_online);
	render_call(display_chat, g_env.ws.chat);
	rl_forced_update_display();
}

static void		resize_windows(struct winsize size, const short s_w)
{
	curs_set(1);
	g_env.ws.input_b->_begy = size.ws_row - 3;
	g_env.ws.u_online_b->_begx = size.ws_col - s_w;
	g_env.ws.rooms_a_b->_begx = size.ws_col - s_w;
	g_env.ws.input->_begy = size.ws_row - 2;
	g_env.ws.u_online->_begx = size.ws_col - s_w + 1;
	g_env.ws.rooms_a->_begx = size.ws_col - s_w + 1;
	wresize(g_env.ws.input_b, 3, size.ws_col);
	wresize(g_env.ws.chat_b, size.ws_row - 3, size.ws_col - s_w);
	wresize(g_env.ws.u_online_b, ceil((size.ws_row - 3) / 2.0), s_w);
	g_env.ws.rooms_a_b->_begy = g_env.ws.u_online_b->_maxy + 1;
	wresize(g_env.ws.rooms_a_b, floor((size.ws_row - 3) / 2.0), s_w);
	wresize(g_env.ws.input, 1, size.ws_col - 2);
	wresize(g_env.ws.chat, size.ws_row - 5, size.ws_col - s_w - 2);
	wresize(g_env.ws.u_online, ceil((size.ws_row - 3) / 2.0) - 2, s_w - 2);
	g_env.ws.rooms_a->_begy = g_env.ws.u_online_b->_maxy + 2;
	wresize(g_env.ws.rooms_a, floor((size.ws_row - 3) / 2.0) - 2, s_w - 2);
	werase(g_env.ws.input_b);
	werase(g_env.ws.u_online_b);
	werase(g_env.ws.rooms_a_b);
	werase(g_env.ws.chat_b);
	redraw_windows(s_w);
}

void			resize_curses(void)
{
	int				offset;
	const short		s_w = MAX_NICKNAME_LEN + 5;

	ioctl(fileno(stdout), TIOCGWINSZ, (char *)&g_env.term_size);
	resizeterm(g_env.term_size.ws_row, g_env.term_size.ws_col);
	if (g_env.term_size.ws_col < TERM_MIN_WIDTH
		|| g_env.term_size.ws_row < TERM_MIN_HEIGHT)
	{
		offset = (g_env.term_size.ws_col - s_w) / 2;
		curs_set(0);
		erase();
		werase(g_env.ws.input);
		mvprintw(g_env.term_size.ws_row / 2, offset < 0 ? 0 : offset, "Window too small!");
		refresh();
	}
	else
		resize_windows(g_env.term_size, s_w);
}

void			init_design(void)
{
	const short	s_w = MAX_NICKNAME_LEN + 5;

	initscr();
	cbreak();
	noecho();
	signal(SIGWINCH, (void (*)(int))resize_curses);
	has_colors() == false ? curses_exit((void (*)())ft_printf,
		"Your terminal does not support color\n") : start_color();
	init_pair(C_COLOR_RED, COLOR_RED, COLOR_BLACK);
	init_pair(C_COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
	init_pair(C_COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
	init_pair(C_COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
	g_env.ws.input_b = newwin(3, COLS, LINES - 3, 0);
	g_env.ws.chat_b = newwin(LINES - 3, COLS - s_w, 0, 0);
	g_env.ws.u_online_b = newwin(ceil((LINES - 3) / 2.0), s_w, 0, COLS - s_w);
	g_env.ws.rooms_a_b = newwin(floor((LINES - 3) / 2.0), s_w,
								g_env.ws.u_online_b->_maxy + 1, COLS - s_w + 1);
	g_env.ws.input = newwin(1, COLS - 2, LINES - 2, 1);
	g_env.ws.chat = newwin(LINES - 5, COLS - s_w - 2, 1, 1);
	g_env.ws.u_online = newwin(ceil((LINES - 3) / 2.0) - 2, s_w - 2, 1,
								COLS - s_w + 1);
	g_env.ws.rooms_a = newwin(floor((LINES - 3) / 2.0) - 2, s_w - 2,
								g_env.ws.u_online_b->_maxy + 2, COLS - s_w + 1);
	nodelay(g_env.ws.input, true);
	resize_curses();
}

void			curses_exit(void (*clear_callback)(), void *callback_data)
{
	signal(SIGWINCH, SIG_IGN);
	delwin(g_env.ws.input);
	delwin(g_env.ws.chat);
	delwin(g_env.ws.u_online);
	delwin(g_env.ws.rooms_a);
	delwin(g_env.ws.input_b);
	delwin(g_env.ws.chat_b);
	delwin(g_env.ws.u_online_b);
	delwin(g_env.ws.rooms_a_b);
	endwin();
	clear_callback ? clear_callback(callback_data) : 0;
	exit(0);
}
