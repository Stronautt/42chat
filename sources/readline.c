/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   readline.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/07/30 13:18:50 by pgritsen          #+#    #+#             */
/*   Updated: 2018/08/26 13:59:18 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.h"
#include "handler.h"

static inline void	readline_redraw(const char * msg, size_t cursor_p,
														size_t m_l)
{
	werase(g_env.ws.input);
	mvwhline(g_env.ws.input_b, 0, 1, 0, g_env.ws.input_b->_maxx - 1);
	if ((g_env.nickname && m_l > MSG_MAX_LEN)
		|| (!g_env.nickname && m_l > MAX_NICKNAME_LEN))
		wattron(g_env.ws.input, COLOR_PAIR(C_COLOR_RED));
	mvwprintw(g_env.ws.input, 0, 0, "%s", msg);
	if ((g_env.nickname && m_l > MSG_MAX_LEN)
		|| (!g_env.nickname && m_l > MAX_NICKNAME_LEN))
		wattroff(g_env.ws.input, COLOR_PAIR(C_COLOR_RED));
	if (!g_env.nickname)
		mvwprintw(g_env.ws.input_b, 0, 1, "> Enter nickname (%d/%d) <",
					m_l, MAX_NICKNAME_LEN);
	else if (g_env.connection_lost)
		mvwprintw(g_env.ws.input_b, 0, 1, "> Wait, reconnecting... <");
	else
		mvwprintw(g_env.ws.input_b, 0, 1, "> %s, %s (%d/%d) <",
					rl_display_prompt, g_env.nickname, m_l, MSG_MAX_LEN);
	wrefresh(g_env.ws.input_b);
	wmove(g_env.ws.input, 0, cursor_p);
	wrefresh(g_env.ws.input);
}

void				readline_redisplay(void)
{
	size_t	cursor_col;
	size_t	offset_x;
	size_t	m_x;
	size_t	rl_line_buffer_len;
	char	* msg;

	if (g_prevent_update || g_env.term_size.ws_col < TERM_MIN_WIDTH
		|| g_env.term_size.ws_row < TERM_MIN_HEIGHT)
		return ;
	msg = ft_strtrim(rl_line_buffer);
	rl_line_buffer_len = ft_cinustr(msg);
	free(msg);
	cursor_col = ft_cinustrn(rl_line_buffer, rl_point);
	offset_x = cursor_col % (g_env.ws.input->_maxx + 1);
	m_x = cursor_col / (g_env.ws.input->_maxx + 1) * (g_env.ws.input->_maxx + 1);
	msg = ft_strsub(rl_line_buffer, ft_cinustrcn(rl_line_buffer, m_x),
							rl_point + ft_cinustrcn(rl_line_buffer + rl_point,
								g_env.ws.input->_maxx - offset_x + 1));
	readline_redraw(msg, offset_x, rl_line_buffer_len);
	free(msg);
}
