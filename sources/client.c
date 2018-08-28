/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/07/30 13:18:50 by pgritsen          #+#    #+#             */
/*   Updated: 2018/08/26 21:40:30 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.h"

char			* g_error = NULL;
t_env			g_env;

const char		* get_error(void)
{
	return (g_error ? g_error : "Unkown error!");
}

int				set_error(const char * msg)
{
	g_error ? ft_memdel((void **)&g_error) : 0;
	g_error = ft_strdup(msg);
	return (-1);
}

void			get_messages(int fd, short ev, void *data)
{
	char			* buffer;
	char			* point;
	t_dlist			* lines;
	t_dlist			* tmp;

	(void)ev;
	(void)data;
	if (recieve_data(fd, (void **)&buffer, 0, MSG_WAITALL) <= 0)
		return ;
	lines = ft_strsplit_dlst(buffer, '\n');
	free(buffer);
	tmp = lines;
	while (tmp && (tmp = tmp->next) != lines)
		if ((point = ft_strchr(tmp->content, '\a')))
		{
			g_env.layot.chat_offset = 0;
			beep();
			ft_strclr(point);
		}
	g_env.chat_history.size += ft_dlstsize(lines);
	ft_dlstmerge(&g_env.chat_history.lines, &lines);
	display_chat();
}

int				init_socket(struct sockaddr_in * c_data, const char * server_ip)
{
	int		ret_fd;

	if ((ret_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return (set_error("Unable to create socket"));
	memset(c_data, '0', sizeof(*c_data));
	c_data->sin_family = AF_INET;
	c_data->sin_port = htons(PORT);
	if(inet_pton(AF_INET, server_ip, &c_data->sin_addr) <= 0)
		return (set_error("Invalid address"));
	else if (connect(ret_fd, (struct sockaddr *)c_data, sizeof(*c_data)) < 0)
		return (set_error("Couldn't connect to server, try again later.\n"));
	else if (send_data(ret_fd, 0, 0, CONNECT) < 0)
		return (set_error("Couldn't send data to server, try again later.\n"));
	return (ret_fd);
}

int				try_reconnect(void)
{
	close(g_env.sockfd);
	if ((g_env.sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return (-1);
	else if (connect(g_env.sockfd, (struct sockaddr *)&g_env.conn_data,
						sizeof(g_env.conn_data)) < 0)
		return (-1);
	else if (send_data(g_env.sockfd, 0, 0, RECONNECT) < 0)
		return (-1);
	else if (send_data(g_env.sockfd, g_env.nickname,
						ft_strlen(g_env.nickname) + 1, 0) < 0)
		return (-1);
	event_del(&g_env.ev_getmsg);
	event_set(&g_env.ev_getmsg, g_env.sockfd, EV_READ|EV_PERSIST, get_messages, NULL);
	event_add(&g_env.ev_getmsg, NULL);
	ft_memdel((void**)&g_env.nickname);
	recieve_data(g_env.sockfd, (void **)&g_env.nickname, 0, MSG_WAITALL);
	return (1);
}

static void			get_startup_data(void)
{
	char	* buffer = NULL;
	char	* tmp = NULL;
	char	* trash;

	handle_input(0, 0, true);
	nodelay(g_env.ws.input, true);
	while (recieve_data(g_env.sockfd, (void **)&tmp, 0, MSG_WAITALL) > 0)
	{
		if (!*tmp)
		{
			ft_memdel((void **)&tmp);
			break ;
		}
		trash = ft_strjoin(buffer, tmp);
		ft_memdel((void **)&tmp);
		ft_memdel((void **)&buffer);
		buffer = trash;
	}
	g_env.chat_history.lines = ft_strsplit_dlst(buffer, '\n');
	free(buffer);
	g_env.chat_history.size = ft_dlstsize(g_env.chat_history.lines);
	display_chat();
}

// Online users in 'general' room (1):
// Pavel.

// void			update_online_users(void)
// {
// 	const char	* command = "/online";
// 	char		* response;

// 	if (stdscr->_maxx <= TERM_MIN_WIDTH || stdscr->_maxy <= TERM_MIN_HEIGHT)
// 		return ;
// 	event_del(&g_env.ev_getmsg);
// 	send_data(g_env.sockfd, command, ft_strlen(command) + 1, 0);
// 	recieve_data(g_env.sockfd, (void **)&response, 0, MSG_WAITALL);
// 	free(g_env.room_name);
// 	g_env.room_name = ft_get_content(response, '\'', '\'');
// 	free(response);
// 	display_users_online();
// 	event_set(&g_env.ev_getmsg, g_env.sockfd,
// 				EV_READ | EV_PERSIST, get_messages, NULL);
// 	event_add(&g_env.ev_getmsg, NULL);
// }

void			initialize_events(void)
{
	event_set(&g_env.ev_getmsg, g_env.sockfd,
				EV_READ | EV_PERSIST, get_messages, NULL);
	event_add(&g_env.ev_getmsg, NULL);
	event_set(&g_env.ev_input, 0, EV_WRITE | EV_PERSIST,
				(void (*)(int, short, void *))handle_input, NULL);
	event_add(&g_env.ev_input, NULL);
}

int				main(int ac, char **av)
{
	setlocale(LC_ALL, "");
	bzero(&g_env, sizeof(t_env));
	if (ac < 2)
		return (ft_printf("Usage: ./42chat [server_ip_address]\n") * 0 + EXIT_FAILURE);
	else if ((g_env.sockfd = init_socket(&g_env.conn_data, av[1])) < 0)
		return (ft_printf("%s\n", get_error()) * 0 + EXIT_FAILURE);
	signal(SIGPIPE, SIG_IGN);
	event_init();
	init_design();
	get_startup_data();
	initialize_events();
	event_loop(0);
	curses_exit(NULL, NULL);
	return (0);
}
