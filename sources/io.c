/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   io.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/07/30 13:18:50 by pgritsen          #+#    #+#             */
/*   Updated: 2018/08/26 21:40:30 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.h"

int		g_symb = 0;
bool	g_input_avb = false;
bool	g_prevent_update = false;

static int		readline_input_avail(void)
{
	return (g_input_avb);
}

static int		readline_getc(void)
{
	if (g_env.term_size.ws_col < TERM_MIN_WIDTH
		|| g_env.term_size.ws_row < TERM_MIN_HEIGHT)
		return (beep() * 0);
	g_prevent_update = false;
	if (g_env.connection_lost)
		return (beep() * 0);
	g_input_avb = false;
	return (g_symb == '\t' ? ' ' : g_symb);
}

static void		proceed_nickname(char *nickname)
{
	char	*err;

	err = "";
	if (ft_cinustr(nickname) > MAX_NICKNAME_LEN && (g_prevent_update = true))
		err = "* Nickname is too long *";
	else if (!nickname_is_valid(nickname) && (g_prevent_update = true))
		err = "* Invalid nickname *";
	else if (send_data(g_env.sockfd, nickname, ft_strlen(nickname) + 1, 0) < 0)
		return ;
	else if (recieve_data(g_env.sockfd, (void **)&g_env.nickname,
							0, MSG_WAITALL) < 0)
		return ;
	else if (ungetch(ERR) == OK)
		return (display_chat());
	werase(g_env.ws.input);
	mvwprintw(g_env.ws.input, 0, 0, err);
	wrefresh(g_env.ws.input);
}

static void		got_command(char *line)
{
	if (g_env.term_size.ws_col < TERM_MIN_WIDTH || g_env.term_size.ws_row < TERM_MIN_HEIGHT)
		return free(line);
	g_prevent_update = false;
	if (line && *line && !g_env.nickname)
		proceed_nickname(line);
	else if (line && *line)
	{
		char	tag[128];
		char	* trimmed;

		trimmed = ft_strtrim(line);
		free(line);
		line = trimmed;
		if (!(trimmed = ft_strsub(line, 0, ft_cinustrcn(line, MSG_MAX_LEN))))
			return (free(line));
		sprintf(tag, "["SELF_POINT"%s]: ", g_env.nickname);
		g_env.layot.chat_offset = 0;
		if (send_data(g_env.sockfd, trimmed, ft_strlen(trimmed) + 1, 0) < 0)
		{
			g_env.connection_lost = 1;
			display_chat();
			while (try_reconnect() < 0 || send_data(g_env.sockfd, trimmed, ft_strlen(trimmed) + 1, 0) < 0);
			g_env.connection_lost = 0;
		}
		ft_dlstpush(&g_env.chat_history.lines, ft_dlstnew(ft_strjoin(tag, trimmed), sizeof(void *)));
		g_env.chat_history.size++;
		display_chat();
	}
	free(line);
}

static void		readline_redisplay(void)
{
	if (g_prevent_update || g_env.term_size.ws_col < TERM_MIN_WIDTH || g_env.term_size.ws_row < TERM_MIN_HEIGHT)
		return ;
	char	* trimmed = ft_strtrim(rl_line_buffer);
	size_t	cursor_col = ft_cinustrn(rl_line_buffer, rl_point);
	size_t	offset_x = cursor_col % (g_env.ws.input->_maxx + 1);
	size_t	mult_x = cursor_col / (g_env.ws.input->_maxx + 1) * (g_env.ws.input->_maxx + 1);
	size_t	rl_line_buffer_len = ft_cinustr(trimmed);
	char	* msg = ft_strsub(rl_line_buffer, ft_cinustrcn(rl_line_buffer, mult_x),
							rl_point + ft_cinustrcn(rl_line_buffer + rl_point, g_env.ws.input->_maxx - offset_x + 1));

	free(trimmed);
	werase(g_env.ws.input);
	mvwhline(g_env.ws.input_b, 0, 1, 0, g_env.ws.input_b->_maxx - 1);
	if ((g_env.nickname && rl_line_buffer_len > MSG_MAX_LEN) || (!g_env.nickname && rl_line_buffer_len > MAX_NICKNAME_LEN))
		wattron(g_env.ws.input, COLOR_PAIR(C_COLOR_RED));
	mvwprintw(g_env.ws.input, 0, 0, "%s", msg);
	if ((g_env.nickname && rl_line_buffer_len > MSG_MAX_LEN) || (!g_env.nickname && rl_line_buffer_len > MAX_NICKNAME_LEN))
		wattroff(g_env.ws.input, COLOR_PAIR(C_COLOR_RED));
	free(msg);
	if (!g_env.nickname)
		mvwprintw(g_env.ws.input_b, 0, 1, "Enter nickname (%d/%d)", rl_line_buffer_len, MAX_NICKNAME_LEN);
	else if (g_env.connection_lost)
		mvwprintw(g_env.ws.input_b, 0, 1, " Wait, reconnecting... ");
	else
		mvwprintw(g_env.ws.input_b, 0, 1, "%s, %s (%d/%d)", rl_display_prompt, g_env.nickname, rl_line_buffer_len, MSG_MAX_LEN);
	wrefresh(g_env.ws.input_b);
	wmove(g_env.ws.input, 0, offset_x);
	wrefresh(g_env.ws.input);
}

void			handle_input(int fd, short ev, bool block)
{
	(void)fd;
	(void)ev;
	while ((g_symb = wgetch(g_env.ws.input)) != ERR)
		if (g_symb == 0x1b)
		{
			uint64_t	utf = 0;
			int			_delay = g_env.ws.input->_delay;

			((char *)&utf)[0] = g_symb;
			nodelay(g_env.ws.input, true);
			for (size_t it = 1; it < sizeof(uint64_t); it++)
			{
				if ((g_symb = wgetch(g_env.ws.input)) == 0x1b && ungetch(g_symb) == OK)
					break ;
				((char *)&utf)[it] = (g_symb == ERR ? 0 : g_symb);
			}
			g_env.ws.input->_delay = _delay;
			switch (utf)
			{
				case RL_KEY_UP:
					g_env.layot.chat_offset + g_env.ws.chat->_maxy + 1 < g_env.chat_history.size
						? g_env.layot.chat_offset++ : (uint)beep();
					display_chat();
					break ;
				case RL_KEY_DOWN:
					g_env.layot.chat_offset ? g_env.layot.chat_offset-- : (uint)beep();
					display_chat();
					break ;
				case RL_KEY_PAGEDOWN:
					g_env.layot.u_online_offset + g_env.ws.u_online->_maxy + 1 < g_env.users_online.size
						? g_env.layot.u_online_offset++ : 0;
					display_users_online();
					g_env.layot.rooms_a_offset + g_env.ws.rooms_a->_maxy + 1 < g_env.rooms_avaliable.size
						? g_env.layot.rooms_a_offset++ : 0;
					display_rooms();
					break ;
				case RL_KEY_PAGEUP:
					g_env.layot.u_online_offset ? g_env.layot.u_online_offset-- : 0;
					display_users_online();
					g_env.layot.rooms_a_offset ? g_env.layot.rooms_a_offset-- : 0;
					display_rooms();
					break ;
				case RL_KEY_ESC:
					curses_exit(NULL, NULL);
					break ;
				default:
					g_input_avb = true;
					for (size_t it = 0; it < sizeof(uint64_t) && (g_symb = ((char *)&utf)[it]); it++)
						rl_callback_read_char();
					break ;
			}
		}
		else if (g_symb == '\f')
		{
			g_env.layot.chat_offset = 0;
			g_env.layot.u_online_offset = 0;
			g_env.layot.rooms_a_offset = 0;
			display_chat();
		}
		else if (g_symb == KEY_RESIZE)
			continue ;
		else if ((g_input_avb = true))
		{
			rl_callback_read_char();
			if (!block)
				break ;
		}
}

void			init_readline(void)
{
	rl_bind_key('\t', rl_insert);
	rl_catch_signals = 0;
	rl_catch_sigwinch = 0;
	rl_deprep_term_function = NULL;
	rl_prep_term_function = NULL;
	rl_change_environment = 0;

	rl_getc_function = (int (*)(FILE *))&readline_getc;
	rl_input_available_hook = readline_input_avail;
	rl_redisplay_function = readline_redisplay;

	rl_callback_handler_install("Your message", got_command);
}
