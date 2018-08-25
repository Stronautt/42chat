/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   readline_kostulb.c                                                       */
/*                                                                            */
/*   By: phrytsenko                                                           */
/*                                                                            */
/*   Created: 2018/08/23 17:00:56 by phrytsenko                               */
/*   Updated: 2018/08/25 12:27:19 by phrytsenko                               */
/*                                                                            */
/* ************************************************************************** */

#include "client.h"

int				g_symb = 0;
bool			g_input_avb = FALSE;

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
	}
}

static void		readline_redisplay(void)
{
	// size_t	prompt_width = strwidth(rl_display_prompt, 0);
	// size_t	cursor_col = prompt_width +
	// 					strnwidth(rl_line_buffer, rl_point, prompt_width);

	werase(g_env.ws.input);
	mvwprintw(g_env.ws.input, 0, 0, "%s%s", rl_display_prompt, rl_line_buffer);
	wrefresh(g_env.ws.input);
}

void			handle_input_tmp(void)
{
	while ((g_symb = wgetch(g_env.ws.input)) != ERR)
	{
		if (g_symb == 27)
		{
			uint64_t	utf = 0;

			((char *)&utf)[0] = g_symb;
			nodelay(g_env.ws.input, TRUE);
			for (int it = 0; it < 2; it++)
				((char *)&utf)[it + 1] = (g_symb = wgetch(g_env.ws.input)) == ERR ? 0 : g_symb;
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
				default:
					// For debug
					// werase(g_env.ws.input);
					// mvwprintw(g_env.ws.input, 0, 0, "%ld", utf);
					// wrefresh(g_env.ws.input);
					werase(g_env.ws.input);
					g_input_avb = true;
					for (size_t it = 0; it < sizeof(uint64_t); it++)
					{
						g_symb = ((char *)&utf)[it];
						rl_callback_read_char();
					}
					wrefresh(g_env.ws.input);
					break ;
			}
		}
		else if (g_symb == KEY_RESIZE)
			continue ;
		else
		{
			werase(g_env.ws.input);
			g_input_avb = true;
			rl_callback_read_char();
			wrefresh(g_env.ws.input);
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

	rl_callback_handler_install("Message: ", got_command);
}
