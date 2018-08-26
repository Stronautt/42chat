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

void			display_chat(void)
{
	t_dlist			* tmp;
	int				my;
	int				offset;

	werase(g_env.ws.chat);
	my = g_env.ws.chat->_maxy + 1;
	tmp = g_env.chat_history.lines;
	offset = g_env.layot.chat_offset;
	while (tmp && (tmp = tmp->next) != g_env.chat_history.lines && my > 0)
	{
		if (offset && offset--)
			continue ;
		my -= strlen(tmp->content) / (g_env.ws.chat->_maxx + 2) + 1;
		mvwprintw(g_env.ws.chat, my, 0, "%s", tmp->content);
	}
	wrefresh(g_env.ws.chat);
	rl_forced_update_display();
}

void			resize_curses(int sig)
{
	const short		sidebar_w = 20;
	struct winsize	size;

	(void)sig;
	ioctl(fileno(stdout), TIOCGWINSZ, (char*)&size);
	resizeterm(size.ws_row, size.ws_col);
	if (size.ws_col < 50 || size.ws_row < 8)
	{
		int		offset = (size.ws_col - sidebar_w) / 2;

		erase();
		offset < 0 ? offset = 0 : 0;
		mvprintw(size.ws_row / 2, offset, "Window too small!");
		refresh();
	}
	else
	{
		mvwin(g_env.ws.input_b, size.ws_row - 3, 0);
		mvwin(g_env.ws.sidebar_b, 0, size.ws_col - sidebar_w);
		mvwin(g_env.ws.input, size.ws_row - 2, 1);
		mvwin(g_env.ws.sidebar, 1, size.ws_col - sidebar_w + 1);
		wresize(g_env.ws.input_b, 3, size.ws_col);
		wresize(g_env.ws.chat_b, size.ws_row - 3, size.ws_col - sidebar_w);
		wresize(g_env.ws.sidebar_b, size.ws_row - 3, sidebar_w);
		wresize(g_env.ws.input, 1, size.ws_col - 2);
		wresize(g_env.ws.chat, size.ws_row - 5, size.ws_col - sidebar_w - 2);
		wresize(g_env.ws.sidebar, size.ws_row - 5, sidebar_w - 2);
		werase(g_env.ws.input_b);
		werase(g_env.ws.sidebar_b);
		werase(g_env.ws.chat_b);
		box(g_env.ws.input_b, 0, 0);
		box(g_env.ws.chat_b, 0, 0);
		box(g_env.ws.sidebar_b, 0, 0);
		mvwprintw(g_env.ws.input_b, 0, 1, rl_display_prompt);
		wrefresh(g_env.ws.input_b);
		wrefresh(g_env.ws.chat_b);
		wrefresh(g_env.ws.sidebar_b);
		if (g_env.layot.chat_offset + g_env.ws.chat->_maxy > g_env.chat_history.size)
			g_env.layot.chat_offset = 0;
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
		curses_exit((void (*)())printf, "Your terminal does not support color\n");
	start_color();
	init_pair(1, COLOR_RED, COLOR_BLACK);
	g_env.ws.input = newwin(1, COLS - 2, LINES - 2, 1);
	g_env.ws.chat = newwin(LINES - 5, COLS - sidebar_w - 2, 1, 1);
	g_env.ws.sidebar = newwin(LINES - 5, sidebar_w - 2, 1, COLS - sidebar_w + 1);
	g_env.ws.input_b = newwin(3, COLS, LINES - 3, 0);
	g_env.ws.chat_b = newwin(LINES - 3, COLS - sidebar_w, 0, 0);
	g_env.ws.sidebar_b = newwin(LINES - 3, sidebar_w, 0, COLS - sidebar_w);
	init_readline();
	resize_curses(0);
}

int				curses_exit(void (*clear_callback)(), void * callback_data)
{
	delwin(g_env.ws.input);
	delwin(g_env.ws.chat);
	delwin(g_env.ws.sidebar);
	endwin();
	clear_callback ? clear_callback(callback_data) : 0;
	exit(0);
}
