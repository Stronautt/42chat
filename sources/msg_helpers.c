/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   msg_helpers.c                                                            */
/*                                                                            */
/*   By: phrytsenko                                                           */
/*                                                                            */
/*   Created: 2018/08/31 19:14:30 by phrytsenko                               */
/*   Updated: 2018/08/31 19:16:15 by phrytsenko                               */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

static inline int	nickname_is_busy(const char * nickname, t_dlist * list)
{
	t_dlist		* clients = list;

	while (clients && (clients = clients->next) != list)
		if (!strcasecmp(((t_client *)clients->content)->nickname, nickname))
			return (1);
	return (0);
}

int					get_nickname(t_client * client)
{
	int			it;
	char		* ret;
	char		* raw;
	char		* nname;

	nname = NULL;
	if (recieve_data(client->sockfd, (void **)&ret, 0, MSG_WAITALL) < 0)
		return (-1);
	else if (!(raw = ft_strsub(ret, 0, ft_cinustrcn(ret, MAX_NICKNAME_LEN))))
		return (_clean(ret) - 1);
	else if (!_clean(ret) && !nickname_is_valid((nname = ft_strtrim(raw))
		+ _clean(raw)) && !_clean(nname) && !(nname = ft_strdup("H@ZZk3R")))
		return (-1);
	else if (nickname_is_busy(nname, g_env.all_clients) && (it = 1))
	{
		if (!(raw = malloc(ft_strlen(nname) + 16)))
			return (_clean(nname) - 1);
		do
			sprintf(raw, "%s(%d)", nname, it++);
		while (nickname_is_busy(raw, g_env.all_clients));
		nname = raw - _clean(nname);
	}
	ft_strncpy(client->nickname, nname, sizeof(client->nickname) - 1);
	return (send_data(client->sockfd, client->nickname,
		sizeof(client->nickname), 0) < 0 ? _clean(nname) - 1 : _clean(nname));
}

void				send_msg(t_client *client, const char *msg, ssize_t msg_l)
{
	char	*trash;

	if (!client || !msg)
		return ;
	else if (!client->silent_mode)
	{
		if (!(trash = ft_strjoin("\a", msg)))
			return ;
		send_data(client->sockfd, trash, msg_l + 2, 0);
		free(trash);
	}
	else
		send_data(client->sockfd, msg, msg_l + 1, 0);
}

int					msg_valid(char * msg)
{
	ssize_t 		len;
	unsigned char	*p_s;

	len = ft_cinustr(msg);
	p_s = (unsigned char *)msg;
	if (!msg || len < 1 || len > MSG_MAX_LEN)
		return (-1);
	len = ft_strlen(msg);
	while (len > 0 && *p_s)
		if (*p_s == 0x1B && (p_s += 8))
			len -= 8;
		else if (*p_s >= 0xC2 && *p_s <= 0xDF && (p_s += 2))
			len -= 2;
		else if (*p_s >= 0xE0 && *p_s <= 0xEF && (p_s += 3))
			len -= 3;
		else if (*p_s >= 0xF0 && *p_s <= 0xF4 && (p_s += 3))
			len -= 4;
		else if (len--)
			!ft_isprint(*p_s++) ? *p_s = '*' : 0;
	return (1);
}

uint8_t				treated_as_command(const char * msg, ssize_t msg_l,
										t_client * client)
{
	char					**args;
	char					buf[512];
	ssize_t					len;
	ssize_t					it;
	static const t_assocc	cmds[] = {
		{"help", &show_help},
		{"silent", &toogle_silent_mode},
		{"newroom", &create_chat_room},
		{"joinroom", &join_chat_room}
	};

	if (!msg || msg_l <= 2 || msg[0] != '/')
		return (0);
	else if (!(args = ft_strsplit(msg, ' ')))
		return (0);
	else if (!args || !args[0])
		return (free_splitted(args));
	it = -1;
	while (++it < (ssize_t)(sizeof(cmds) / sizeof(t_assocc)))
		if (!strcasecmp(cmds[it].key, args[0] + 1))
		{
			cmds[it].func(client, args + 1);
			return (free_splitted(args) + 1);
		}
	len = sprintf(buf, "Unknown command: [%s]", args[0] + 1);
	send_data(client->sockfd, buf, len + 1, free_splitted(args));
	return (1);
}
