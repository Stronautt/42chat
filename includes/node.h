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
# include <sys/types.h>
# include <sys/wait.h>
# include <sys/ptrace.h>
# include <stdlib.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <string.h>
# include <pthread.h>
# include <readline/readline.h>
# include <readline/history.h>
# include <signal.h>

# define PORT 2089

# define MSG_POINT "\03"

# define MSG_MAX_LEN 255
# define MAX_NICKNAME_LEN 15

typedef enum	e_command
{
	CONNECT = 1, RECONNECT
}				t_command;

/*
**				Data_exchange.c
**				↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
*/

int				good_connection(int sockfd);

uint64_t		hash_data(const void * data, size_t size);

ssize_t			send_data(int sockfd, const void * data, size_t size, t_command command);

ssize_t			recieve_data(int sockfd, void ** data, t_command * command, int flags);

/*
**				Data_validation.c
**				↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
*/

int				nickname_is_valid(const char * nickname);

size_t			splitted_size(char ** parts);

void			free_splitted(char ** parts);

#endif
