/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   node.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/07/30 17:43:46 by pgritsen          #+#    #+#             */
/*   Updated: 2018/07/31 17:44:22 by pgritsen         ###   ########.fr       */
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

#define PORT 1488

typedef struct	s_client
{
	int		sockfd;
	char	nickname[32];
}				t_client;

# define VERSION "Chat1488: v1.0.3               " // Important to keep size of this string to 32 characters

#endif
