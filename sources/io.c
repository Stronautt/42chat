/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   io.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/07/30 13:18:50 by pgritsen          #+#    #+#             */
/*   Updated: 2018/09/02 15:07:37 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.h"
#include "handler.h"

static void		proceed_nickname(char *nickname)
{
	const char	*err;

	err = 0;
	if (ft_cinustr(nickname) > MAX_NICKNAME_LEN && (g_prevent_update = true))
		err = "* Nickname is too long *";
	else if (!nickname_is_valid(nickname) && (g_prevent_update = true))
		err = "* Invalid nickname *";
	else if (send_data(g_env.sockfd, nickname, ft_strlen(nickname) + 1, 0) < 0)
		return ;
	else if (recieve_data(g_env.sockfd, (void **)&g_env.nickname,
							0, MSG_WAITALL) <= 0)
		return (ft_memdel((void **)&g_env.nickname));
	else
		return (render_call(display_chat, g_env.ws.chat));
	if (err)
	{
		werase(g_env.ws.input);
		mvwprintw(g_env.ws.input, 0, 0, err);
		wrefresh(g_env.ws.input);
	}
}

static void		proceed_msg(char *msg)
{
	char	tag[128];
	char	*trimmed;

	trimmed = ft_strtrim(msg);
	msg = trimmed;
	if (!(trimmed = ft_strsub(msg, 0, ft_cinustrcn(msg, MSG_MAX_LEN))))
		return (free(msg));
	sprintf(tag, MSG_POINT"["SELF_POINT"%s]: ", g_env.nickname);
	g_env.layot.chat_offset = 0;
	if (send_data(g_env.sockfd, trimmed, ft_strlen(trimmed) + 1, 0) < 0)
	{
		g_env.connection_lost = 1;
		render_call(display_chat, g_env.ws.chat);
		while (try_reconnect() < 0
			|| send_data(g_env.sockfd, trimmed, ft_strlen(trimmed) + 1, 0) < 0)
			;
		g_env.connection_lost = 0;
	}
	free(trimmed);
	free(msg);
}

static void		catch_line(char *line)
{
	if (g_env.term_size.ws_col < TERM_MIN_WIDTH
		|| g_env.term_size.ws_row < TERM_MIN_HEIGHT)
		return (free(line));
	g_prevent_update = false;
	if (line && *line && !g_env.nickname)
		proceed_nickname(line);
	else if (line && *line)
		proceed_msg(line);
	free(line);
}

void			handle_input(void)
{
	if ((g_symb = wgetch(g_env.ws.input)) == ERR)
		return ;
	else if (g_symb == 0x1b)
		handle_uni_key(get_uni_key());
	else if (g_symb == '\f')
	{
		g_env.layot.chat_offset = 0;
		g_env.layot.u_online_offset = 0;
		g_env.layot.rooms_a_offset = 0;
		render_call(display_chat, g_env.ws.chat);
	}
	else if (g_symb == KEY_RESIZE)
		return ;
	else if ((g_input_avb = true))
		rl_callback_read_char();
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
	rl_callback_handler_install("Your message", catch_line);
}
