/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/07/30 13:18:50 by pgritsen          #+#    #+#             */
/*   Updated: 2018/07/31 20:24:47 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "node.h"

char			* g_error = NULL;

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
	char			buffer[320];

	while (recv(*sockfd, buffer, sizeof(buffer), 0) > 0)
	{
		clear_prompt();
		ft_putstr(buffer);
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
	return (ret_fd);
}

void			handle_input(int sockfd, char * nickname)
{
	size_t	msg_len;
	char	* msg;
	char	* trash;
	char	prompt[128];

	sprintf(prompt, "You -> [%s]: ", nickname);
	free(nickname);
	while ((msg = readline(prompt)))
	{
		trash = ft_strtrim(msg);
		free(msg);
		msg_len = ft_strlen(trash);
		if (msg_len > 250)
			ft_putendl("* Your message too long, it was trimmed to 250 symbols *");
		msg = ft_strnew(255);
		ft_strncpy(msg, trash, 255);
		free(trash);
		if ((msg_len = ft_strlen(msg)) > 0)
			send(sockfd, msg, 256, 0);
		free(msg);
	}
}

void			get_startup_data(int sockfd)
{
	char		buffer[1024];
	char		* nickname;
	char		* trash;
	pthread_t	thread;

	recv(sockfd, buffer, 32, 0);
	ft_putendl(buffer);
	do
	{
		if (!(nickname = readline("Enter your login: ")))
			return ;
		trash = ft_strtrim(nickname);
		free(nickname);
		nickname = ft_strnew(32);
		ft_strncpy(nickname, trash, 31);
		free(trash);
	}
	while (ft_strlen(nickname) <= 0);
	send(sockfd, nickname, 32, 0);
	while (recv(sockfd, buffer, sizeof(buffer), 0) > 0)
		if (!*buffer)
			break ;
		else
			ft_putstr(buffer);
	pthread_create(&thread, NULL, (void *(*)(void *))(get_messages), (void *)&sockfd);
	handle_input(sockfd, nickname);
}

int				main(int ac, char **av)
{
	struct sockaddr_in	conn_data;
	int					sockfd;

	if (ac < 2)
		return (ft_printf("Usage: ./42chat [server_ip_address]\n") * 0 + EXIT_FAILURE);
	else if ((sockfd = init_socket(&conn_data, av[1])) < 0)
		return (ft_printf("%s\n", get_error()) * 0 + EXIT_FAILURE);
	get_startup_data(sockfd);
	return (0);
}
