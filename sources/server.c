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

# define LOG_FILE_PATH "./log_chat.txt"
# define LOG_SYS_FILE_PATH "./log_sys.txt"

char			* g_error = NULL;
int				g_log_file_fd = 0;
int				g_log_sys_file_fd = 0;
t_dlist			* g_clients = NULL;
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
	else if ((g_log_file_fd = open(LOG_FILE_PATH, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP)) < 0)
	{
		set_error("Log file can't be created!");
		return (-1);
	}
	else if ((g_log_sys_file_fd = open(LOG_SYS_FILE_PATH, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP)) < 0)
	{
		set_error("Sys log file can't be created!");
		return (-1);
	}
	return (ret_fd);
}

void	sync_chat_history(t_client * client)
{
	int		log_fd;

	if ((log_fd = open(LOG_FILE_PATH, O_RDONLY)) > 0)
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
		buf_len = sprintf(buffer, "Online users: %zu\n", ft_dlstsize(g_clients));
		send_data(client->sockfd, buffer, buf_len + 1, 0);
	}
	else
		send_data(client->sockfd, "* Unable to load history *\n", 28, 0);
	send_data(client->sockfd, "", 1, 0);
}

void			log_client_actions(t_client * client, const char * status, const char * public_status)
{
	time_t		timer;
	char		buffer[32];
	char		msg[128];
	struct tm	* tm_info;
	t_dlist		* clients = g_clients;

	time(&timer);
	tm_info = localtime(&timer);
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
	pthread_mutex_lock(&g_mutex);
	dprintf(g_log_sys_file_fd, "[%s][%s] -> %s\n", buffer, client->nickname, status);
	sprintf(msg, "* %s %s the chat *\n", client->nickname, public_status);
	dprintf(g_log_file_fd, "%s", msg);
	while (clients && (clients = clients->next) != g_clients)
		if (clients->content != client)
		{
			t_client	*tmp_client = clients->content;
			send_data(tmp_client->sockfd, msg, ft_strlen(msg) + 1, 0);
		}
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
	if (ft_strlen(trimmed) == 0 || !nickname_is_valid(trimmed))
		ft_strncpy(client->nickname, "H@ZZk3R", 31);
	else
		ft_strncpy(client->nickname, trimmed, 31);
	ft_memdel((void **)&trimmed);
}

int				validate_msg(char * msg, ssize_t len)
{
	if (!msg || len > 255)
		return (-1);
	while (len-- > 1)
		if (!ft_isprint(*msg++))
			*(msg - 1) = '*';
	return (1);
}

size_t			splitted_size(char ** parts)
{
	size_t	size = 0;

	if (!parts)
		return (0);
	while (parts[size])
		size++;
	return (size);
}

void			free_splitted(char ** parts)
{
	char	** s_p = parts;

	if (!parts)
		return ;
	while (*parts)
		free(*parts++);
	free(s_p);
}

uint8_t			treated_as_command(char * msg, ssize_t msg_l, t_client * client)
{
	char	**parts;
	char	buf[1024];
	ssize_t	len;
	uint8_t	found = 0;
	static const t_assocc cmds[] = {
		{"help", &show_help}
	};

	if (!msg || msg_l <= 2 || msg[0] != '/')
		return (0);
	else if (!(parts = ft_strsplit(msg, ' ')))
		return (0);
	else if (!parts[0])
	{
		free_splitted(parts);
		return (0);
	}
	free(msg);
	for (size_t i = 0; i < sizeof(cmds) / sizeof(t_assocc); i++)
		if (!strcasecmp(cmds[i].key, parts[0] + 1) && ++found)
			cmds[i].func(client, parts[1]);
	if (!found)
	{
		len = sprintf(buf, "Unknown command: [%s]\n", parts[0] + 1);
		send_data(client->sockfd, buf, len + 1, 0);
	}
	free_splitted(parts);
	return (1);
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
		pthread_mutex_lock(&g_mutex);
		public_msg[public_msg_l] = 0;
		msg_l = write(g_log_file_fd, public_msg, public_msg_l);
		while (clients && (clients = clients->next) != g_clients)
			if (clients->content != client)
				send_data(((t_client *)(clients->content))->sockfd,
							public_msg, public_msg_l + 1, 0);
		ft_memdel((void **)&public_msg);
		pthread_mutex_unlock(&g_mutex);
	}
}

void			disconnect_client(t_dlist * client_node)
{
	(void)client_node;
}

void			handle_client(t_dlist * client_node)
{
	t_command	cmd;
	char		invite_msg[] = "Welcome to 42Chat!\n";
	t_client	* client = client_node->content;

	if (recieve_command(client->sockfd, &cmd, MSG_WAITALL) > 0)
	{
		if (cmd == CONNECT)
		{
			if (send_data(client->sockfd, invite_msg, sizeof(invite_msg), 0) < 0)
				pthread_exit(NULL);
			get_nickname(client);
			sync_chat_history(client);
			log_client_actions(client, "CONNECTED", "joined");
		}
		else if (cmd == RECONNECT)
		{
			get_nickname(client);
			log_client_actions(client, "RECONNECTED", "return to");
		}
		else
			pthread_exit(NULL);
		pthread_mutex_lock(&g_mutex);
		ft_dlstpush(&g_clients, client_node);
		pthread_mutex_unlock(&g_mutex);
		trace_income_msgs(client);
		log_client_actions(client, "DISCONNECTED", "left");
	}
	pthread_mutex_lock(&g_mutex);
	ft_dlstdelelem(&client_node);
	pthread_mutex_unlock(&g_mutex);
}

void			handle_connections(int server, struct sockaddr_in * conn_data)
{
	pthread_t	thread;
	int			new_conn;
	t_client	* new_client;
	t_dlist		* new_node;
	socklen_t	conn_len = sizeof(*conn_data);

	while (1)
	{
		if ((new_conn = accept(server, (struct sockaddr *)conn_data, &conn_len)) < 0)
		{
			perror("Accept failed");
			break ;
		}
		if (!(new_client = ft_memalloc(sizeof(t_client))))
			continue ;
		new_client->sockfd = new_conn;
		new_node = ft_dlstnew(new_client, sizeof(t_client));
		pthread_create(&thread, NULL, (void *(*)(void *))(handle_client), (void *)new_node);
		pthread_cleanup_push((void (*)(void *))&disconnect_client, (void *)new_node);
		pthread_detach(thread);
	}
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
	if ((server_socket = init_socket(&conn_data)) < 0)
		return (ft_printf("%s\n", get_error()) * 0 + EXIT_FAILURE);
	handle_connections(server_socket, &conn_data);
	return (0);
}
