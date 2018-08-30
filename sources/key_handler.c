/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   key_handler.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/07/30 13:18:50 by pgritsen          #+#    #+#             */
/*   Updated: 2018/08/26 21:40:30 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.h"
#include "handler.h"

int		g_symb = 0;
bool	g_input_avb = false;
bool	g_prevent_update = false;

int			readline_input_avail(void)
{
	return (g_input_avb);
}

int			readline_getc(void)
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

uint64_t	get_uni_key(void)
{
	size_t		it;
	uint64_t	utf;
	int			_delay;

	utf = 0;
	_delay = g_env.ws.input->_delay;
	((uint8_t *)&utf)[0] = g_symb;
	nodelay(g_env.ws.input, true);
	it = 0;
	while (++it < sizeof(uint64_t))
	{
		if ((g_symb = wgetch(g_env.ws.input)) == 0x1b && ungetch(g_symb) == OK)
			break ;
		((uint8_t *)&utf)[it] = (g_symb == ERR ? 0 : g_symb);
	}
	g_env.ws.input->_delay = _delay;
	return (utf);
}

static int	handle_uni_key_2(uint64_t utf)
{
	int		ret;

	ret = 0;
	if (utf == RL_KEY_PAGEDOWN && ++ret)
	{
		g_env.layot.u_online_offset + g_env.ws.u_online->_maxy + 1
			< g_env.users_online.size ? g_env.layot.u_online_offset++ : 0;
		render_call(display_users_online, g_env.ws.u_online);
		g_env.layot.rooms_a_offset + g_env.ws.rooms_a->_maxy + 1
			< g_env.rooms_avaliable.size ? g_env.layot.rooms_a_offset++ : 0;
		render_call(display_rooms, g_env.ws.rooms_a);
	}
	else if (utf == RL_KEY_PAGEUP && ++ret)
	{
		g_env.layot.u_online_offset ? g_env.layot.u_online_offset-- : 0;
		render_call(display_users_online, g_env.ws.u_online);
		g_env.layot.rooms_a_offset ? g_env.layot.rooms_a_offset-- : 0;
		render_call(display_rooms, g_env.ws.rooms_a);
	}
	return (ret);
}

void		handle_uni_key(uint64_t utf)
{
	size_t	it;

	if (utf == RL_KEY_UP)
	{
		g_env.layot.chat_offset + g_env.ws.chat->_maxy + 1
			< g_env.chat_history.size ? g_env.layot.chat_offset++ : (uint)beep();
		render_call(display_chat, g_env.ws.chat);
	}
	else if (utf == RL_KEY_DOWN)
	{
		g_env.layot.chat_offset ? g_env.layot.chat_offset-- : (uint)beep();
		render_call(display_chat, g_env.ws.chat);
	}
	else if (handle_uni_key_2(utf))
		return ;
	else if (utf == RL_KEY_ESC)
		curses_exit(NULL, NULL);
	else
	{
		g_input_avb = true;
		it = 0;
		while (it < sizeof(uint64_t) && (g_symb = ((char *)&utf)[it++]))
			rl_callback_read_char();
	}
}
