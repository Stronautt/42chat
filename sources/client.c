/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/07/30 13:18:50 by pgritsen          #+#    #+#             */
/*   Updated: 2018/08/01 18:25:53 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.h"

char			* g_error = NULL;
t_env			g_env;

const char		* get_error(void)
{
	return (g_error ? g_error : "Unkown error!");
}

void			set_error(const char * msg)
{
	g_error ? ft_memdel((void **)&g_error) : 0;
	g_error = ft_strdup(msg);
}

void	clear_prompt(void)
{
	struct winsize	w;

	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	ft_printf("\r%*s\r", w.ws_col, "");
}

void			get_messages(int * sockfd)
{
	char			* buffer;

	while (recieve_data(*sockfd, (void **)&buffer, MSG_WAITALL) > 0)
	{
		clear_prompt();
		ft_putstr(buffer);
		ft_memdel((void**)&buffer);
		rl_forced_update_display();
	}
}

int				init_socket(struct sockaddr_in * conn_data, const char * server_ip)
{
	int		ret_fd;

	if ((ret_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		set_error("Unable to create socket");
		return (-1);
	}
	memset(conn_data, '0', sizeof(*conn_data));
	conn_data->sin_family = AF_INET;
	conn_data->sin_port = htons(PORT);
	if(inet_pton(AF_INET, server_ip, &conn_data->sin_addr) <= 0)
	{
		set_error("Invalid address");
		return (-1);
	}
	else if (connect(ret_fd, (struct sockaddr *)conn_data, sizeof(*conn_data)) < 0)
	{
		set_error("Couldn't connect to server, try again later.\n");
		return (-1);
	}
	else if (send_command(ret_fd, CONNECT, 0) < 0)
	{
		set_error("Couldn't send data to server, try again later.\n");
		return (-1);
	}
	return (ret_fd);
}

int				try_reconnect(int * sockfd, struct sockaddr_in * conn_data, char ** nickname)
{
	pthread_t	thread;

	close(*sockfd);
	if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return (-1);
	else if (connect(*sockfd, (struct sockaddr *)conn_data, sizeof(*conn_data)) < 0)
		return (-1);
	else if (send_command(*sockfd, RECONNECT, 0) < 0)
		return (-1);
	else if (send_data(*sockfd, *nickname, ft_strlen(*nickname) + 1, 0) < 0)
		return (-1);
	ft_memdel((void**)nickname);
	recieve_data(*sockfd, (void **)nickname, 0);
	pthread_create(&thread, NULL, (void *(*)(void *))(get_messages), (void *)sockfd);
	pthread_detach(thread);
	return (1);
}

void			handle_input(int * sockfd, char ** nickname, struct sockaddr_in * conn_data)
{
	size_t	msg_len;
	char	* msg;
	char	* trash;
	char	prompt[128];

	sprintf(prompt, "You -> [%s]: ", *nickname);
	while ((msg = readline(prompt)))
	{
		trash = ft_strtrim(msg);
		free(msg);
		msg_len = ft_strlen(trash);
		if (msg_len > 255)
			ft_putendl("* Your message too long, it was trimmed to 255 symbols *");
		msg = ft_strsub(trash, 0, 255);
		free(trash);
		if ((msg_len = ft_strlen(msg)) > 0
			&& send_data(*sockfd, msg, msg_len + 1, 0) < 0)
		{
			ft_putendl("* You were diconnected from server, reconnecting... *");
			while (try_reconnect(sockfd, conn_data, nickname) < 0)
				;
			ft_putendl("* You were reconnected, enjoy! *");
			sprintf(prompt, "You -> [%s]: ", *nickname);
		}
		free(msg);
	}
}

void			get_startup_data(int sockfd, struct sockaddr_in * conn_data)
{
	char		* buffer;
	char		* nickname;
	char		* trash;
	pthread_t	thread;
	size_t		nickname_len;

	if (recieve_data(sockfd, (void **)&trash, MSG_WAITALL) < 0)
		exit(ft_printf("Couldn't connect to server, try again later.\n") * 0 + EXIT_FAILURE);
	ft_putendl(trash);
	ft_memdel((void **)&trash);
	do
	{
		if (!(nickname = readline("Enter your login: ")))
			return ;
		trash = ft_strtrim(nickname);
		free(nickname);
		nickname = ft_strsub(trash, 0, 15);
		free(trash);
		if (!nickname_is_valid(nickname))
		{
			ft_putendl("* Login must containt only ascii letters and numbers *");
			nickname[0] = 0;
		}
	}
	while ((nickname_len = ft_strlen(nickname)) <= 0);
	send_data(sockfd, nickname, nickname_len + 1, 0);
	ft_memdel((void**)&nickname);
	recieve_data(sockfd, (void **)&nickname, 0);
	while (recieve_data(sockfd, (void **)&buffer, MSG_WAITALL) > 0)
	{
		if (!*buffer)
		{
			ft_memdel((void **)&buffer);
			break ;
		}
		ft_putstr(buffer);
		ft_memdel((void **)&buffer);
	}
	pthread_create(&thread, NULL, (void *(*)(void *))(get_messages), (void *)&sockfd);
	pthread_detach(thread);
	handle_input(&sockfd, &nickname, conn_data);
}

const char * CHAT =	"[Pavel]: LAST Hi to all\n"\
				"[Pavel]: qHi to all\n"\
				"[Pavel]: wHi to all\n"\
				"[Pavel]: eHi to all\n"\
				"[Pavel]: rHi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: sdfgHi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: zzzsHi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: vbnvbHi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: qewqeHi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: 5Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: Hi to all\n"\
				"[Pavel]: 3Hi to all\n"\
				"[Pavel]: 2Hi to all asfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff\n"\
				"[Pavel]: 1Hi to all\n";

int				main(int ac, char **av)
{
	// struct sockaddr_in	conn_data;
	// int					sockfd;

	// if (ac < 2)
	// 	return (ft_printf("Usage: ./42chat [server_ip_address]\n") * 0 + EXIT_FAILURE);
	// else if ((sockfd = init_socket(&conn_data, av[1])) < 0)
	// 	return (ft_printf("%s\n", get_error()) * 0 + EXIT_FAILURE);
	// signal(SIGPIPE, SIG_IGN);
	// get_startup_data(sockfd, &conn_data);
	// return (0);

	(void)ac;
	(void)av;
	bzero(&g_env, sizeof(t_env));
	g_env.chat_history.lines = ft_strsplit_dlst(CHAT, '\n');
	g_env.chat_history.size = ft_dlstsize(g_env.chat_history.lines);
	setlocale(LC_ALL, "");
	init_design();
	init_readline();

	handle_input_tmp();

	curses_exit(NULL, NULL);
	return 0;
}
