/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/08/03 17:29:59 by phrytsenko        #+#    #+#             */
/*   Updated: 2018/09/02 14:08:25 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_H
# define SERVER_H

# include "node.h"

# define MAX_ROOMS_NUMBER 1024

# define O_LOG_FLAGS O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP

# define HISTORY_ERR "* Unable to load history *"

# define LOG_ERR_PATH "./logs/log_errs.txt"
# define LOG_SYS_PATH "./logs/log_sys.txt"

typedef struct	s_assocc
{
	char	* key;
	void	(*func)();
}				t_assocc;

typedef struct	s_client
{
	int				sockfd;
	uint8_t			silent_mode;
	struct event	ev;
	char			nickname[(MAX_NICKNAME_LEN + 1) * 4];
	t_dlist			*chat_room_node;
	t_dlist			*node_in_room;
}				t_client;

typedef struct	s_chat_room
{
	char		* name;
	t_dlist		* users;
	int			log_fd;
	char		log_name[64];
	uint64_t	passwd;
}				t_chat_room;

typedef struct	s_env
{
	int				err_fd;
	int				sys_fd;
	int				msocket;
	t_dlist			*all_clients;
	t_dlist			*all_rooms;
	struct event	m_ev;
}				t_env;

extern t_env	g_env;

/*
**				Server.c
**				↓↓↓↓↓↓↓↓
*/

/*
**	SIGHUP		1	Hangup (POSIX)
**	SIGINT		2	Terminal interrupt (ANSI)
**	SIGQUIT		3	Terminal quit (POSIX)
**	SIGILL		4	Illegal instruction (ANSI)
**	SIGTRAP		5	Trace trap (POSIX)
**	SIGIOT		6	IOT Trap (4.2 BSD)
**	SIGBUS		7	BUS error (4.2 BSD)
**	SIGFPE		8	Floating point exception (ANSI)
**	SIGKILL		9	Kill(can't be caught or ignored) (POSIX)
**	SIGUSR1		10	User defined signal 1 (POSIX)
**	SIGSEGV		11	Invalid memory segment access (ANSI)
**	SIGUSR2		12	User defined signal 2 (POSIX)
**	SIGPIPE		13	Write on a pipe with no reader, Broken pipe (POSIX)
**	SIGALRM		14	Alarm clock (POSIX)
**	SIGTERM		15	Termination (ANSI)
**	SIGSTKFLT	16	Stack fault
**	SIGCHLD		17	Child process has stopped or exited, changed (POSIX)
**	SIGCONT		18	Continue executing, if stopped (POSIX)
**	SIGSTOP		19	Stop executing(can't be caught or ignored) (POSIX)
**	SIGTSTP		20	Terminal stop signal (POSIX)
**	SIGTTIN		21	Background process trying to read, from TTY (POSIX)
**	SIGTTOU		22	Background process trying to write, to TTY (POSIX)
**	SIGURG		23	Urgent condition on socket (4.2 BSD)
**	SIGXCPU		24	CPU limit exceeded (4.2 BSD)
**	SIGXFSZ		25	File size limit exceeded (4.2 BSD)
**	SIGVTALRM	26	Virtual alarm clock (4.2 BSD)
**	SIGPROF		27	Profiling alarm clock (4.2 BSD)
**	SIGWINCH	28	Window size change (4.3 BSD, Sun)
**	SIGIO		29	I/O now possible (4.2 BSD)
**	SIGPWR		30	Power failure restart (System V)
*/

void			sig_handler(int sig);

void			handle_con(void);

void			listen_client(int fd, short ev, t_dlist *c_node);

int				read_client_msg(t_client *user);

/*
**				System.c
**				↓↓↓↓↓↓↓↓
*/

int				setnonblock(int fd);

char			*init_socket(void);

int				log_errors(int fd, const char *msg);

void			log_client_actions(t_client *client,
							const char *status, const char *public_status);

void			disconnect_client(t_dlist *client_node);

/*
**				Msg_helpers.c
**				↓↓↓↓↓↓↓↓↓↓↓↓↓
*/

int				get_nickname(t_client *client);

void			send_msg(t_client *client, const char *msg, ssize_t msg_l);

int				msg_valid(char *msg);

uint8_t			treated_as_command(const char *msg, ssize_t msg_l,
									t_client *client);

/*
**				Msg_helpers_2.c
**				↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
*/

char			*get_room_data(t_chat_room *room);

char			*get_room_list(void);

void			update_clients_data(t_chat_room *room);

void			update_room_list(t_client *client);

void			sync_chat_history(t_client *c);

/*
**				Cmd_interfaces.c
**				↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
*/

void			show_help(t_client *client);

void			toogle_silent_mode(t_client *client);

void			create_chat_room(t_client *client, const char **args);

void			join_chat_room(t_client *client, const char **args);

/*
**				Cmd_helpers.c
**				↓↓↓↓↓↓↓↓↓↓↓↓↓
*/

const char		*validate_room_data(const char **args, t_dlist **rooms,
									const t_client *client, t_client **c_dup);

const char		*new_chat_room(const char *name, const char *passwd);

#endif
