/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   node.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/07/30 17:43:46 by pgritsen          #+#    #+#             */
/*   Updated: 2018/09/15 12:41:45 by pgritsen         ###   ########.fr       */
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
# include <strings.h>
# include <readline/readline.h>
# include <readline/history.h>
# include <signal.h>
# include <event.h>
# include <math.h>

# define PORT 2089

# define MSG_POINT "\01"
# define SELF_POINT "\02"
# define ROOM_LOCKED "\03"
# define PRIVATE_MSG_POINT "\04"

# define MSG_MAX_LEN 255
# define MAX_NICKNAME_LEN 15
# define MAX_DATA_SIZE INT16_MAX

typedef enum	e_command
{
	NO_CMD = 0, UPDATE_USERS, UPDATE_ROOMS, UPDATE_HISTORY, RECONNECT
}				t_command;

typedef struct	s_packet
{
	ssize_t		size;
	size_t		count;
	uint64_t	crs_sum;
	t_command	cmd;
}				t_packet;

/*
**				Data_helpers.c
**				↓↓↓↓↓↓↓↓↓↓↓↓↓↓
*/

int				good_connection(int sockfd);

uint64_t		hash_data(const void *data, size_t size);

uint64_t		hash_packet(const t_packet *packet, const void *data);

/*
**				Data_exchange.c
**				↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
*/

ssize_t			send_data(int sockfd, const void *data,
							size_t size, t_command command);

ssize_t			recieve_data(int sockfd, void **data,
							t_command *command, int flags);

/*
**				Data_validation.c
**				↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
*/

int				nickname_is_valid(const char *nickname);

int				h_clean(void *data);

size_t			splitted_size(char **parts);

int				free_splitted(char **parts);

#endif
