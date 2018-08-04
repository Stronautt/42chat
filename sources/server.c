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

#include "node.h"
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

int				log_errors(int fd, const char * msg)
{
	char		buffer[32];
	char		* trimmed;
	time_t		timer;
	struct tm	* tm_info;
	ssize_t		ret = 0;

	time(&timer);
	tm_info = localtime(&timer);
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
	trimmed = ft_strtrim(msg);
	pthread_mutex_lock(&g_mutex);
	ret = dprintf(fd, "[%s]: %s\n", buffer, trimmed);
	pthread_mutex_unlock(&g_mutex);
	free(trimmed);
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
			send_data(client->sockfd, "* Unable to load history *\n", 28, 0);
	}
	else
		send_data(client->sockfd, "* Unable to load history *\n", 28, 0);
	send_data(client->sockfd, "", 1, 0);
}

void			log_client_actions(t_client * client, const char * status, const char * public_status)
{
	char		msg[128];
	char		buffer[32];
	time_t		timer;
	struct tm	* tm_info;
	t_dlist		* clients = g_clients;
	t_chat_room	* chat_room = client->chat_room_node->content;

	time(&timer);
	tm_info = localtime(&timer);
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
	pthread_mutex_lock(&g_mutex);
	dprintf(g_log_sys_fd, "[%s][%s] -> %s\n", buffer, client->nickname, status);
	sprintf(msg, "* %s %s *\n", client->nickname, public_status);
	dprintf(chat_room->log_fd, "%s", msg);
	while (clients && (clients = clients->next) != g_clients)
		if (clients->content && clients->content != client && ((t_client *)clients->content)->chat_room_node == client->chat_room_node)
			send_data(((t_client *)clients->content)->sockfd, msg, ft_strlen(msg) + 1, 0);
	pthread_mutex_unlock(&g_mutex);
}

void			get_nickname(t_client * client)
{
	char		* ret;
	char		* raw;
	char		* trimmed;

	if (recieve_data(client->sockfd, (void **)&ret, MSG_WAITALL) < 0)
		pthread_exit(NULL);
	else if (!(raw = ft_strsub(ret, 0, 31)))
		pthread_exit(NULL);
	ft_memdel((void **)&ret);
	trimmed = ft_strtrim(raw);
	ft_memdel((void **)&raw);
	if (!nickname_is_valid(trimmed))
		ft_strncpy(client->nickname, "H@ZZk3R", 31);
	else
		ft_strncpy(client->nickname, trimmed, 31);
	ft_memdel((void **)&trimmed);
}

int				validate_msg(char * msg, ssize_t len)
{
	if (!msg || len < 1 || len > 255)
		return (-1);
	while (len-- > 1)
		if (!ft_isprint(*msg++))
			*(msg - 1) = '*';
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
		len = sprintf(buf, "Unknown command: [%s]\n", args[0] + 1);
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

		trash = ft_strjoin(msg, "\a");
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
	t_dlist		* clients = g_clients;

	while ((msg_l = recieve_data(client->sockfd, (void **)&msg, MSG_WAITALL)) > 0)
	{
		t_chat_room	* chat_room = client->chat_room_node->content;

		if (validate_msg(msg, msg_l) < 0)
		{
			ft_memdel((void **)&msg);
			continue ;
		}
		else if (treated_as_command(msg, msg_l, client))
			continue ;
		else if (!(public_msg = ft_strnew(msg_l + ft_strlen(client->nickname) + 16)))
			pthread_exit(NULL);
		msg[msg_l] = 0;
		public_msg_l = sprintf(public_msg, "[%s]: %s\n", client->nickname, msg);
		ft_memdel((void **)&msg);
		public_msg[public_msg_l] = 0;
		pthread_mutex_lock(&g_mutex);
		msg_l = write(chat_room->log_fd, public_msg, public_msg_l);
		while (clients && (clients = clients->next) != g_clients)
			if (clients->content && clients->content != client && ((t_client *)clients->content)->chat_room_node == client->chat_room_node)
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
	char		invite_msg[] = "Welcome to 42Chat!\n";
	t_client	* client = client_node->content;
	t_client	* tmp;
	char		* sys_act;
	char		* public_act;

	pthread_cleanup_push((void (*)(void *))&disconnect_client, client_node);
	if (recieve_command(client->sockfd, &cmd, MSG_WAITALL) > 0)
	{
		client->chat_room_node = g_chat_rooms->next;
		if (cmd == CONNECT)
		{
			if (send_data(client->sockfd, invite_msg, sizeof(invite_msg), 0) < 0)
				pthread_exit(NULL);
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
		pthread_mutex_lock(&g_mutex);
		if (!(tmp = malloc(sizeof(t_client))))
		{
			pthread_mutex_unlock(&g_mutex);
			pthread_exit(NULL);
		}
		ft_memcpy(tmp, client, sizeof(t_client));
		client->node_in_room = ft_dlstnew(tmp, sizeof(void *));
		ft_dlstpush(&((t_chat_room *)client->chat_room_node->content)->users, client->node_in_room);
		ft_dlstpush(&g_clients, client_node);
		pthread_mutex_unlock(&g_mutex);
		log_client_actions(client, sys_act, public_act);
		show_users_in_room(client);
		show_help(client);
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

void			sig_handler(int sig)
{
	char	err_msg[256];

	sprintf(err_msg, "[signal: %d]", sig);
	log_errors(g_log_err_fd, err_msg);
	exit(sig);
}

int				main(void)
{
	struct sockaddr_in	conn_data;
	int					server_socket;
	pid_t				server_pid = fork();

	if (server_pid)
		return (server_pid < 0 ?
			ft_printf("Server start failed!\n") * 0 + EXIT_FAILURE :
			ft_printf("Server pid -> [%d]\n", server_pid) * 0);
	setsid();
	signal(SIGPIPE, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGSEGV, sig_handler);
	signal(SIGBUS, sig_handler);
	if (new_chat_room("general", 0) < 0)
		return (EXIT_FAILURE);
	else if (init_logs() < 0)
		return (EXIT_FAILURE);
	else if ((server_socket = init_socket(&conn_data)) < 0)
		return (log_errors(g_log_err_fd, get_error()) * 0 + EXIT_FAILURE);
	handle_connections(server_socket, &conn_data);
	return (0);
}
