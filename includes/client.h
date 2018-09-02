/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/07/30 13:18:50 by pgritsen          #+#    #+#             */
/*   Updated: 2018/09/02 14:05:47 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_H
# define CLIENT_H

# include "node.h"
# include <curses.h>
# include <math.h>
# include <locale.h>

# define RL_KEY_UP 0x415b1b
# define RL_KEY_DOWN 0x425b1b
# define RL_KEY_PAGEUP 0x7E355B1B
# define RL_KEY_PAGEDOWN 0x7E365B1B
# define RL_KEY_ESC 0x1b

# define SELF_POINT "\02"

# define TERM_MIN_WIDTH 40
# define TERM_MIN_HEIGHT 9

# define C_COLOR_RED 1
# define C_COLOR_CYAN 2
# define C_COLOR_GREEN 3

typedef struct	s_workspaces
{
	WINDOW				*input;
	WINDOW				*chat;
	WINDOW				*u_online;
	WINDOW				*rooms_a;
	WINDOW				*input_b;
	WINDOW				*chat_b;
	WINDOW				*u_online_b;
	WINDOW				*rooms_a_b;
}				t_workspaces;

typedef struct	s_layot
{
	uint				chat_offset;
	uint				u_online_offset;
	uint				rooms_a_offset;
}				t_layot;

typedef struct	s_buffer
{
	t_dlist				*lines;
	size_t				size;
}				t_buffer;

typedef struct	s_env
{
	struct event		ev_getmsg;
	struct event		ev_input;
	int					sockfd;
	char				*nickname;
	char				*room_name;
	t_layot				layot;
	uint8_t				connection_lost;
	t_buffer			chat_history;
	t_buffer			users_online;
	t_buffer			rooms_avaliable;
	t_workspaces		ws;
	struct winsize		term_size;
	struct sockaddr_in	conn_data;
}				t_env;

extern t_env	g_env;

/*
**				Client.c
**				↓↓↓↓↓↓↓↓
*/

int				try_reconnect(void);

/*
**				IO.c
**				↓↓↓↓
*/

void			init_readline(void);

void			handle_input(void);

/*
**				Curses.c
**				↓↓↓↓↓↓↓↓
*/

void			resize_curses(void);

void			init_design(void);

void			curses_exit(void (*clear_callback)(), void *callback_data);

/*
**				Render.c
**				↓↓↓↓↓↓↓↓
*/

void			render_call(void (*func)(), WINDOW *target);

void			display_rooms(void);

void			display_users_online(void);

void			display_chat(void);

/*
**				Data_update.c
**				↓↓↓↓↓↓↓↓↓↓↓↓↓
*/

void			update_rooms(char *raw);

void			update_users_online(char *raw);

void			update_chat_history(char *raw);

/*
**				Key_handler.c
**				↓↓↓↓↓↓↓↓↓↓↓↓↓
*/

#endif
