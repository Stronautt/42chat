/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: phrytsenko <phrytsenko@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/08/03 17:29:59 by phrytsenko        #+#    #+#             */
/*   Updated: 2018/08/03 17:32:18 by phrytsenko       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_H
# define SERVER_H

# include "node.h"

# define MAX_ROOMS_NUMBER 1024

typedef struct	s_assocc
{
	char	* key;
	void (*func)();
}				t_assocc;

typedef struct	s_client
{
	int			sockfd;
	uint8_t		silent_mode;
	pthread_t	thread_data;
	char		nickname[(MAX_NICKNAME_LEN + 1) * 4];
	t_dlist		* chat_room_node;
	t_dlist		* node_in_room;
}				t_client;

typedef struct	s_chat_room
{
	char		* name;
	t_dlist		* users;
	int			log_fd;
	char		log_name[64];
	uint64_t	passwd;
}				t_chat_room;

extern t_dlist			* g_clients;
extern t_dlist			* g_chat_rooms;
extern pthread_mutex_t	g_mutex;

/*
**				Server.c
**				↓↓↓↓↓↓↓↓
*/

void			log_client_actions(t_client * client, const char * status, const char * public_status);

void			sync_chat_history(t_client * client);

void			update_clients_data(t_chat_room * room);

void			update_room_list(t_client * client);

/*
**				Cmd_interfaces.c
**				↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
*/

void			show_help(t_client * client);

void			toogle_silent_mode(t_client * client);

void			create_chat_room(t_client * client, const char ** args);

void			join_chat_room(t_client * client, const char ** args);

/*
**				Cmd_helpers.c
**				↓↓↓↓↓↓↓↓↓↓↓↓↓
*/

const char		*validate_room_data(const char ** args, t_dlist ** rooms,
									const t_client * client, t_client ** c_dup);

const char		*new_chat_room(const char * name, const char * passwd);

#endif
