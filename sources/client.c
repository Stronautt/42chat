/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/07/30 13:18:50 by pgritsen          #+#    #+#             */
/*   Updated: 2018/07/30 21:58:31 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "node.h"

void	clear_prompt(void)
{
	struct winsize	w;

	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	ft_printf("\r%*s\r", w.ws_col, "");
}

void	display_messages(int * sockfd)
{
	char			buffer[1024] = {0};
	static char		welcome_msg = 1;

	while (read(*sockfd , buffer, sizeof(buffer)))
	{
		clear_prompt();
		ft_putstr(buffer);
		!welcome_msg ? rl_forced_update_display() : 0;
		welcome_msg = 0;
	}
}

int		main(int ac, char **av)
{
	struct sockaddr_in	address;
	socklen_t			addrlen = sizeof(address);
	int					sock;

	if (ac < 2)
	{
		ft_printf("Specify server address!\n");
		return (0);
	}

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		return (-1);
	}

	memset(&address, 0, addrlen);

	address.sin_family = AF_INET;
	address.sin_port = htons(PORT);

	if(inet_pton(AF_INET, av[1], &address.sin_addr) <= 0)
	{
		printf("Invalid address\n");
		return (-1);
	}

	if (connect(sock, (struct sockaddr *)&address, addrlen) < 0)
	{
		printf("Connection Failed\n");
		return (-1);
	}

	char			prompt[128];
	char			* buffer;
	pthread_t		thread;

	do
	{
		if (!(buffer = readline("Enter your login: ")))
			return (0);
	}
	while (ft_strlen(buffer) <= 0);
	send(sock, buffer, ft_strlen(buffer) + 1, 0);

	rl_bind_key ('c', (rl_command_func_t *)&clear_prompt);
	pthread_create(&thread, NULL, (void *(*)(void *))(display_messages), (void *)&sock);
	usleep(500);
	sprintf(prompt, "You -> [%s]: ", buffer);
	free(buffer);
	while ((buffer = readline(prompt)))
	{
		if (ft_strlen(buffer) > 0)
			send(sock, buffer, ft_strlen(buffer) + 1, 0);
		free(buffer);
	}
	return (0);
}
