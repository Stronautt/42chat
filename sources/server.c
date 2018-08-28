/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: phrytsenko <phrytsenko@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/07/31 17:16:33 by pgritsen          #+#    #+#             */
/*   Updated: 2018/08/03 17:55:52 by phrytsenko       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

# define LOG_ERR_PATH "./logs/log_errs.txt"
# define LOG_SYS_PATH "./logs/log_sys.txt"

char			* g_error = NULL;
int				g_log_err_fd = 0;
int				g_log_sys_fd = 0;
t_dlist			* g_clients = NULL;
t_dlist			* g_chat_rooms = NULL;
pthread_mutex_t	g_mutex = PTHREAD_MUTEX_INITIALIZER;

const char		* get_error(void)
{
	return (g_error ? g_error : "Unkown error!");
}

void			set_error(const char * msg)
{
	g_error ? ft_memdel((void **)&g_error) : 0;
	g_error = ft_strdup(msg);
}

int				init_logs()
{
	if ((g_log_err_fd = open(LOG_ERR_PATH, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP)) < 0)
	{
		set_error("Err log file can't be created!");
		return (-1);
	}
	else if ((g_log_sys_fd = open(LOG_SYS_PATH, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP)) < 0)
	{
		set_error("Sys log file can't be created!");
		return (-1);
	}
	return (0);
}

//	Be careful with this function as it used in signal handle,
//	see `man -s7 signal` to check if function that you want to
//	use inside is allowed in handler

int				log_errors(int fd, const char * msg)
{
	char		buffer[32];
	char		log[1024];
	time_t		timer;
	struct tm	* tm_info;
	ssize_t		ret = 0;

	if (strlen(msg) + sizeof(buffer) > sizeof(log))
		return (-1);
	time(&timer);
	tm_info = localtime(&timer);
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
	ret = write(fd, log, sprintf(log, "[%s][pid: %d]: %s\n", buffer, getpid(), msg));
	return (ret);
}

int				init_socket(struct sockaddr_in * conn_data)
{
	int		ret_fd;

	if ((ret_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		set_error("Unable to create socket");
		return (-1);
	}
	else if (setsockopt(ret_fd, SOL_SOCKET, SO_REUSEPORT, (int[1]){1}, sizeof(int)))
	{
		set_error("Unable to set socket option: SO_REUSEPORT");
		return (-1);
	}
	memset(conn_data, '0', sizeof(*conn_data));
	conn_data->sin_family = AF_INET;
	conn_data->sin_addr.s_addr = INADDR_ANY;
	conn_data->sin_port = htons(PORT);
	if (bind(ret_fd, (struct sockaddr *)conn_data, sizeof(*conn_data)) < 0 || listen(ret_fd, 3) < 0)
	{
		set_error(strerror(errno));
		return (-1);
	}
	return (ret_fd);
}

void	sync_chat_history(t_client * client)
{
	int			log_fd;
	t_chat_room	* chat_room = client->chat_room_node->content;

	if ((log_fd = open(chat_room->log_name, O_RDONLY)) > 0)
	{
		char	buffer[1024];
		ssize_t	buf_len;

		while ((buf_len = read(log_fd, buffer, sizeof(buffer) - 1)) > 0)
		{
			buffer[buf_len] = 0;
			send_data(client->sockfd, buffer, buf_len + 1, 0);
		}
		if (buf_len < 0)
			send_data(client->sockfd, "* Unable to load history *", 28, 0);
	}
	else
		send_data(client->sockfd, "* Unable to load history *", 28, 0);
	send_data(client->sockfd, "", 1, 0);
}

void			log_client_actions(t_client * client, const char * status, const char * public_status)
{
	char		msg[128];
	char		buffer[32];
	time_t		timer;
	struct tm	* tm_info;
	t_chat_room	* chat_room = client->chat_room_node->content;
	t_dlist		* clients = chat_room->users;

	time(&timer);
	tm_info = localtime(&timer);
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
	pthread_mutex_lock(&g_mutex);
	dprintf(g_log_sys_fd, "[%s][%s] -> %s\n", buffer, client->nickname, status);
	sprintf(msg, "* %s %s *", client->nickname, public_status);
	dprintf(chat_room->log_fd, "%s\n", msg);
	while (clients && (clients = clients->next) != chat_room->users)
			if (clients->content && clients != client->node_in_room)
			send_data(((t_client *)clients->content)->sockfd, msg, ft_strlen(msg) + 1, 0);
	pthread_mutex_unlock(&g_mutex);
}

int				nickname_is_busy(const char * nickname, t_dlist * list)
{
	t_dlist		* clients = list;

	while (clients && (clients = clients->next) != list)
		if (!strcasecmp(((t_client *)clients->content)->nickname, nickname))
			return (1);
	return (0);
}

void			get_nickname(t_client * client)
{
	char		* ret;
	char		* raw;
	char		* trimmed;

	if (recieve_data(client->sockfd, (void **)&ret, 0, MSG_WAITALL) < 0)
		pthread_exit(NULL);
	else if (!(raw = ft_strsub(ret, 0, ft_cinustrcn(ret, MAX_NICKNAME_LEN))))
		pthread_exit(NULL);
	ft_memdel((void **)&ret);
	trimmed = ft_strtrim(raw);
	ft_memdel((void **)&raw);
	if (!nickname_is_valid(trimmed))
	{
		free(trimmed);
		trimmed = ft_strdup("H@ZZk3R");
	}
	if (nickname_is_busy(trimmed, g_clients))
	{
		char	* tmp;
		int		it = 1;

		tmp = malloc(strlen(trimmed) + 16);
		do
			sprintf(tmp, "%s(%d)", trimmed, it++);
		while (nickname_is_busy(tmp, g_clients));
		ft_memdel((void **)&trimmed);
		trimmed = tmp;
	}
	ft_strncpy(client->nickname, trimmed, sizeof(client->nickname) - 1);
	ft_memdel((void **)&trimmed);
	send_data(client->sockfd, client->nickname, sizeof(client->nickname), 0);
}

int				validate_msg(char * msg)
{
	ssize_t 		len = ft_cinustr(msg);
	unsigned char	*p_s = (unsigned char *)msg;

	if (!msg || len < 1 || len > MSG_MAX_LEN)
		return (-1);
	len = ft_strlen(msg);
	while (len > 0 && *p_s)
		if (*p_s == 0x1B && (p_s += 8))
			len -= 8;
		else if (*p_s >= 0xC2 && *p_s <= 0xDF && (p_s += 2))
			len -= 2;
		else if (*p_s >= 0xE0 && *p_s <= 0xEF && (p_s += 3))
			len -= 3;
		else if (*p_s >= 0xF0 && *p_s <= 0xF4 && (p_s += 3))
			len -= 4;
		else
		{
			!ft_isprint(*p_s++) ? *p_s = '*' : 0;
			len--;
		}
	return (1);
}

uint8_t			treated_as_command(char * msg, ssize_t msg_l, t_client * client)
{
	char	**args;
	char	buf[1024];
	ssize_t	len;
	uint8_t	found = 0;
	static const t_assocc cmds[] = {
		{"help", &show_help},
		{"silent", &toogle_silent_mode},
		{"showrooms", &show_all_rooms},
		{"newroom", &create_chat_room},
		{"joinroom", &join_chat_room},
		{"online", &show_users_in_room}
	};

	if (!msg || msg_l <= 2 || msg[0] != '/')
		return (0);
	else if (!(args = ft_strsplit(msg, ' ')))
		return (0);
	else if (!args[0])
	{
		free_splitted(args);
		return (0);
	}
	free(msg);
	for (size_t i = 0; i < sizeof(cmds) / sizeof(t_assocc); i++)
		if (!strcasecmp(cmds[i].key, args[0] + 1) && ++found)
			cmds[i].func(client, args + 1);
	if (!found)
	{
		len = sprintf(buf, "Unknown command: [%s]", args[0] + 1);
		send_data(client->sockfd, buf, len + 1, 0);
	}
	free_splitted(args);
	return (1);
}

void			send_msg(t_client * client, char * msg, ssize_t msg_l)
{
	if (!client || !msg)
		return ;
	else if (!client->silent_mode)
	{
		char	* trash;

		if (!(trash = ft_strjoin(msg, "\a")))
			return ;
		send_data(client->sockfd, trash, msg_l + 2, 0);
		free(trash);
	}
	else
		send_data(client->sockfd, msg, msg_l + 1, 0);
}

void			trace_income_msgs(t_client * client)
{
	ssize_t		msg_l;
	ssize_t		public_msg_l;
	char		* msg;
	char		* public_msg;

	while ((msg_l = recieve_data(client->sockfd, (void **)&msg, 0, MSG_WAITALL)) > 0)
	{
		t_chat_room	* chat_room = client->chat_room_node->content;
		t_dlist		* clients = chat_room->users;

		if (validate_msg(msg) < 0)
		{
			ft_memdel((void **)&msg);
			continue ;
		}
		else if (treated_as_command(msg, msg_l, client))
			continue ;
		else if (!(public_msg = ft_strnew(msg_l + ft_strlen(client->nickname) + 16)))
			pthread_exit(NULL);
		msg[msg_l] = 0;
		public_msg_l = sprintf(public_msg, "[%s]: %s", client->nickname, msg);
		ft_memdel((void **)&msg);
		public_msg[public_msg_l] = 0;
		pthread_mutex_lock(&g_mutex);
		msg_l = dprintf(chat_room->log_fd, "%s\n", public_msg);
		while (clients && (clients = clients->next) != chat_room->users)
			if (clients->content && clients != client->node_in_room)
				send_msg(clients->content, public_msg, public_msg_l);
		pthread_mutex_unlock(&g_mutex);
		ft_memdel((void **)&public_msg);
	}
}

void			disconnect_client(t_dlist * client_node)
{
	t_client	* client = client_node->content;

	if (nickname_is_valid(client->nickname))
		log_client_actions(client, "DISCONNECTED", "left the chat");
	pthread_mutex_lock(&g_mutex);
	close(client->sockfd);
	ft_dlstdelelem(&client->node_in_room);
	ft_dlstdelelem(&client_node);
	pthread_mutex_unlock(&g_mutex);
}

void			*handle_client(t_dlist * client_node)
{
	t_command	cmd;
	t_client	* client = client_node->content;
	t_client	* tmp;
	char		* sys_act;
	char		* public_act;

	pthread_cleanup_push((void (*)(void *))&disconnect_client, client_node);
	if (recieve_data(client->sockfd, 0, &cmd, MSG_WAITALL) > 0)
	{
		client->chat_room_node = g_chat_rooms->next;
		if (cmd == CONNECT)
		{
			get_nickname(client);
			sync_chat_history(client);
			sys_act = "CONNECTED";
			public_act = "joined the chat";
		}
		else if (cmd == RECONNECT)
		{
			get_nickname(client);
			sys_act = "RECONNECTED";
			public_act = "returned to the chat";
		}
		else
			pthread_exit(NULL);
		if (!(tmp = malloc(sizeof(t_client))))
			pthread_exit(NULL);
		ft_memcpy(tmp, client, sizeof(t_client));
		pthread_mutex_lock(&g_mutex);
		client->node_in_room = ft_dlstnew(tmp, sizeof(void *));
		ft_dlstpush(&((t_chat_room *)client->chat_room_node->content)->users, client->node_in_room);
		ft_dlstpush(&g_clients, client_node);
		pthread_mutex_unlock(&g_mutex);
		log_client_actions(client, sys_act, public_act);
		trace_income_msgs(client);
	}
	pthread_cleanup_pop(1);
	return (NULL);
}

void			handle_connections(int server, struct sockaddr_in * conn_data)
{
	int			accept_failed = 0;
	int			new_conn;
	t_client	* new_client;
	t_dlist		* new_node;
	socklen_t	conn_len = sizeof(*conn_data);

	while (accept_failed < 3)
	{
		if ((new_conn = accept(server, (struct sockaddr *)conn_data, &conn_len)) < 0 && accept_failed++)
		{
			log_errors(g_log_err_fd, strerror(errno));
			continue ;
		}
		else if (!(new_client = ft_memalloc(sizeof(t_client))))
		{
			close(new_conn);
			continue ;
		}
		accept_failed = 0;
		new_client->sockfd = new_conn;
		new_node = ft_dlstnew(new_client, sizeof(void *));
		pthread_create(&new_client->thread_data, NULL, (void *(*)(void *))handle_client, (void *)new_node);
		pthread_detach(new_client->thread_data);
	}
	if (accept_failed >= 3)
		log_errors(g_log_err_fd, "Too many accept system calls in line failed!");
}

int				new_chat_room(char * name, char * passwd)
{
	t_chat_room	* new_room;

	if (!name)
		return (-1);
	else if (!(new_room = ft_memalloc(sizeof(t_chat_room))))
		return (-1);
	new_room->name = ft_strsub(name, 0, 16);
	if (passwd)
		new_room->passwd = hash_data(passwd, ft_strlen(passwd));
	sprintf(new_room->log_name, "./logs/log_chat_%s_%ld.txt", new_room->name, time(NULL));
	if ((new_room->log_fd = open(new_room->log_name, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP)) < 0)
		return (-1);
	pthread_mutex_lock(&g_mutex);
	ft_dlstpush_back(&g_chat_rooms, ft_dlstnew(new_room, sizeof(void *)));
	pthread_mutex_unlock(&g_mutex);
	return (0);
}

/*
*	SIGHUP		1	Hangup (POSIX)
*	SIGINT		2	Terminal interrupt (ANSI)
*	SIGQUIT		3	Terminal quit (POSIX)
*	SIGILL		4	Illegal instruction (ANSI)
*	SIGTRAP		5	Trace trap (POSIX)
*	SIGIOT		6	IOT Trap (4.2 BSD)
*	SIGBUS		7	BUS error (4.2 BSD)
*	SIGFPE		8	Floating point exception (ANSI)
*	SIGKILL		9	Kill(can't be caught or ignored) (POSIX)
*	SIGUSR1		10	User defined signal 1 (POSIX)
*	SIGSEGV		11	Invalid memory segment access (ANSI)
*	SIGUSR2		12	User defined signal 2 (POSIX)
*	SIGPIPE		13	Write on a pipe with no reader, Broken pipe (POSIX)
*	SIGALRM		14	Alarm clock (POSIX)
*	SIGTERM		15	Termination (ANSI)
*	SIGSTKFLT	16	Stack fault
*	SIGCHLD		17	Child process has stopped or exited, changed (POSIX)
*	SIGCONT		18	Continue executing, if stopped (POSIX)
*	SIGSTOP		19	Stop executing(can't be caught or ignored) (POSIX)
*	SIGTSTP		20	Terminal stop signal (POSIX)
*	SIGTTIN		21	Background process trying to read, from TTY (POSIX)
*	SIGTTOU		22	Background process trying to write, to TTY (POSIX)
*	SIGURG		23	Urgent condition on socket (4.2 BSD)
*	SIGXCPU		24	CPU limit exceeded (4.2 BSD)
*	SIGXFSZ		25	File size limit exceeded (4.2 BSD)
*	SIGVTALRM	26	Virtual alarm clock (4.2 BSD)
*	SIGPROF		27	Profiling alarm clock (4.2 BSD)
*	SIGWINCH	28	Window size change (4.3 BSD, Sun)
*	SIGIO		29	I/O now possible (4.2 BSD)
*	SIGPWR		30	Power failure restart (System V)
*/

void			sig_handler(int sig)
{
	char	err_msg[256];
	int		critical = 0;
	int		is_critical[] = {
		0, 1, 1, 1, 1, 1, 1,
		1, 1, 0, 1, 0, 0, 0,
		1, 1, 1, 0, 1, 1, 0,
		0, 1, 1, 1, 0, 0, 0,
		0, 0
	};
	char	*signames[] = {
		"SIGHUP",	"SIGINT",	"SIGQUIT",	"SIGILL",	"SIGTRAP",	"SIGIOT",	"SIGBUS",
		"SIGFPE",	"SIGKILL",	"SIGUSR1",	"SIGSEGV",	"SIGUSR2",	"SIGPIPE",	"SIGALRM",
		"SIGTERM",	"SIGSTKFLT","SIGCHLD",	"SIGCONT",	"SIGSTOP",	"SIGTSTP",	"SIGTTIN",
		"SIGTTOU",	"SIGURG",	"SIGXCPU",	"SIGXFSZ",	"SIGVTALRM","SIGPROF",	"SIGWINCH",
		"SIGIO",	"SIGPWR"
	};

	if (sig >= 0 && sig < (int)(sizeof(signames) / sizeof(*signames)))
	{
		critical = is_critical[sig - 1];
		sprintf(err_msg, "[signal: %d -> %s]", sig, signames[sig - 1]);
	}
	else
		sprintf(err_msg, "[signal: %d -> Unknown]", sig);
	log_errors(g_log_err_fd, err_msg);
	critical ? exit(sig) : 0;
}

void			garbage_collector(void)
{
	t_dlist		* clients = g_clients;
	t_client	* client;

	while (1)
	{
		while (clients && (clients = clients->next) != g_clients)
		{
			client = clients->content;
			if (!good_connection(client->sockfd))
				pthread_cancel(client->thread_data);
		}
		sleep(3);
	}
}

int				main(void)
{
	struct sockaddr_in	conn_data;
	int					server_socket;
	short int			sig = 0;
	pthread_t			gc_thread;
	pid_t				server_pid = fork();

	if (server_pid)
		return (server_pid < 0 ?
			ft_printf("Server start failed!\n") * 0 + EXIT_FAILURE :
			ft_printf("Server pid -> [%d]\n", server_pid) * 0);
	setsid();
	while (++sig <= 30)
		if (sig != SIGKILL && sig != SIGSTOP)
			signal(sig, sig_handler);
	if (new_chat_room("general", 0) < 0)
		return (EXIT_FAILURE);
	else if (init_logs() < 0)
		return (EXIT_FAILURE);
	else if ((server_socket = init_socket(&conn_data)) < 0)
		return (log_errors(g_log_err_fd, get_error()) * 0 + EXIT_FAILURE);
	handle_connections(server_socket, &conn_data);
	pthread_create(&gc_thread, NULL, (void *(*)(void *))garbage_collector, NULL);
	pthread_detach(gc_thread);
	return (0);
}
