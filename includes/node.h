/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   node.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: phrytsenko <phrytsenko@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/07/30 17:43:46 by pgritsen          #+#    #+#             */
/*   Updated: 2018/08/03 17:42:37 by phrytsenko       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NODE_H
# define NODE_H

# include "libft.h"
# include <errno.h>
# include <unistd.h>
# include <fcntl.h>
# include <stdio.h>
# include <sys/socket.h>
# include <sys/ioctl.h>
# include <stdlib.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <string.h>
# include <pthread.h>
# include <readline/readline.h>
# include <readline/history.h>
# include <signal.h>

#define PORT 2089

typedef enum	e_command
{
	CONNECT = 1, RECONNECT
}				t_command;

typedef struct	s_client
{
	int		sockfd;
	char	nickname[32];
}				t_client;

/*
**				Data_exchange.c
**				↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
*/

int				good_connection(int sockfd);

uint64_t		hash_data(const void * data, size_t size);

ssize_t			send_data(int sockfd, const void * data, size_t size, int flags);

ssize_t			send_command(int sockfd, t_command command, int flags);

ssize_t			recieve_data(int sockfd, void ** data, int flags);

ssize_t			recieve_command(int sockfd, t_command * data, int flags);

/*
**				Data_validation.c
**				↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
*/

int				nickname_is_valid(const char * nickname);

/*
**				Commands.c
**				↓↓↓↓↓↓↓↓↓↓
*/

void			show_help(t_client * client);

#endif
