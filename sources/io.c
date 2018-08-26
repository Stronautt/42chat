/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   io.c                                                                     */
/*                                                                            */
/*   By: phrytsenko                                                           */
/*                                                                            */
/*   Created: 2018/08/23 17:00:56 by phrytsenko                               */
/*   Updated: 2018/08/25 12:27:19 by phrytsenko                               */
/*                                                                            */
/* ************************************************************************** */

#include "client.h"

int		g_symb = 0;
bool	g_input_avb = false;

static int		readline_input_avail(void)
{
	return (g_input_avb);
}

static int		readline_getc(FILE *dummy)
{
	(void)dummy;
	g_input_avb = false;
	return (g_symb);
}

static void		got_command(char *line)
{
	if (line && *line)
	{
		ft_dlstpush(&g_env.chat_history.lines, ft_dlstnew(line, sizeof(void *)));
		g_env.chat_history.size++;
		g_env.layot.chat_offset = 0;
		display_chat();
	}
}

static void		readline_redisplay(void)
{
	size_t	cursor_col = ft_cinustrn(rl_line_buffer, rl_point);
	size_t	offset_x = cursor_col % (g_env.ws.input->_maxx + 1);
	size_t	rl_line_buffer_len = ft_cinustr(rl_line_buffer);
	char	* msg = ft_strsub(rl_line_buffer, ft_cinustrcn(rl_line_buffer, cursor_col / (g_env.ws.input->_maxx + 1) * (g_env.ws.input->_maxx + 1)),
							rl_point + ft_cinustrcn(rl_line_buffer + rl_point, g_env.ws.input->_maxx - offset_x + 1));

	werase(g_env.ws.input);
	rl_line_buffer_len > 255 ? wattron(g_env.ws.input, COLOR_PAIR(1)) : 0;
	wprintw(g_env.ws.input, "%s", msg);
	rl_line_buffer_len > 255 ? wattroff(g_env.ws.input, COLOR_PAIR(1)) : 0;
	free(msg);
	wrefresh(g_env.ws.input);
	wmove(g_env.ws.input, 0, offset_x);
}

void			handle_input_tmp(void)
{
	while ((g_symb = wgetch(g_env.ws.input)) != ERR)
	{
		if (g_symb == 0x1b)
		{
			uint64_t	utf = 0;

			((char *)&utf)[0] = g_symb;
			nodelay(g_env.ws.input, TRUE);
			for (size_t it = 1; it < sizeof(uint64_t); it++)
			{
				if ((g_symb = wgetch(g_env.ws.input)) == 0x1b)
				{
					ungetch(g_symb);
					break ;
				}
				((char *)&utf)[it] = (g_symb == ERR ? 0 : g_symb);
			}
			nodelay(g_env.ws.input, FALSE);
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
				case RL_KEY_ESC:
					return ;
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
			display_chat();
		}
		else if (g_symb == KEY_RESIZE)
			continue ;
		else
		{
			g_input_avb = true;
			rl_callback_read_char();
		}
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

	rl_getc_function = readline_getc;
	rl_input_available_hook = readline_input_avail;
	rl_redisplay_function = readline_redisplay;

	rl_callback_handler_install("Your message", got_command);
}
