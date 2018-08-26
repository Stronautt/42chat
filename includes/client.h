/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/07/30 13:18:50 by pgritsen          #+#    #+#             */
/*   Updated: 2018/08/26 13:59:18 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_H
# define CLIENT_H

# include "node.h"
# include <curses.h>
# include <math.h>
# include <locale.h>
# include <wchar.h>

# define RL_KEY_UP 0x415b1b
# define RL_KEY_DOWN 0x425b1b
# define RL_KEY_ESC 0x1b

typedef struct	s_workspaces
{
	WINDOW			* input;
	WINDOW			* chat;
	WINDOW			* sidebar;
	WINDOW			* input_b;
	WINDOW			* chat_b;
	WINDOW			* sidebar_b;
}				t_workspaces;

typedef struct	s_layot
{
	uint			chat_offset;
}				t_layot;

typedef struct	s_buffer
{
	t_dlist			* lines;
	size_t			size;
}				t_buffer;

typedef struct	s_env
{
	t_workspaces	ws;
	t_layot			layot;
	t_buffer		chat_history;
}				t_env;

extern char		* g_error;
extern t_env	g_env;

void			resize_curses(int sig);

void			display_chat(void);

void			init_design(void);

void			init_readline(void);

void			handle_input_tmp(void);

int				curses_exit(void (*clear_callback)(), void * callback_data);

#endif
