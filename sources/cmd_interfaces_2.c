/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmd_interfaces_2.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/08/03 17:34:19 by phrytsenko        #+#    #+#             */
/*   Updated: 2018/09/02 14:12:15 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

void	send_pm(t_client *client, const char **args)
{
	t_dlist		*user_node;
	t_client	*user;
	char		*err;
	char		*msg;
	size_t		it;

	if ((err = NULL) || !args || !args[0] || !args[1])
		err = "* You should specify all parameters *";
	else if (!(user_node = find_user_nickname(args[0], g_env.all_clients))
		|| !(user = user_node->content))
		err = "* User is offline now *";
	else if (user == client)
		err = "* Sir, you have schizophrenia ;) *";
	else if (find_user_addr(client, user->blacklist))
		err = "* You are in user's blacklist! *";
	if (err)
		return ((void)send_data(client->sockfd, err, ft_strlen(err) + 1, 0));
	msg = malloc((int)MAX_NICKNAME_LEN * 8 + 16);
	it = sprintf(msg, PRIVATE_MSG_POINT"[%s]:", client->nickname) * 0;
	while (args[++it])
		msg = ft_vstrjoin(3, msg, " ", args[it]) - h_clean(msg);
	send_data(user->sockfd, msg, ft_strlen(msg) + 1, 0) < 0
		&& (err = "* User is offline now *") ? disconnect_client(user_node) : 0;
	!err ? err = "* Private message successfully sent *" : 0;
	send_data(client->sockfd, err, ft_strlen(err) + 1, h_clean(msg));
}

void	block_user(t_client *client, const char **args)
{
	t_client	*user;
	t_dlist		*user_node;
	t_dlist		*bl_node;
	char		*err;

	if (!args || !args[0])
		err = "* You should specify user nickname *";
	else if (!(user_node = find_user_nickname(args[0], g_env.all_clients))
		|| !(user = user_node->content))
		err = "* There is nobody with this nickname *";
	else if (user == client)
		err = "* You can't block yourself, it's stupid ;) *";
	else if ((bl_node = find_user_addr(user, client->blacklist)))
	{
		err = "* User successfully unblocked *";
		ft_dlstdelelem_cs(&bl_node);
	}
	else
	{
		err = "* User successfully blocked *";
		ft_dlstpush(&client->blacklist, ft_dlstnew(user, sizeof(void *)));
	}
	send_data(client->sockfd, err, ft_strlen(err) + 1, 0);
}
