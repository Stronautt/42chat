/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/07/31 17:16:33 by pgritsen          #+#    #+#             */
/*   Updated: 2018/09/02 15:33:27 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

t_env	g_env;

int					read_client_msg(t_client *user)
{
	char		*msg;
	char		*p_msg;
	ssize_t		ml;
	t_dlist		*clients;
	t_chat_room	*chat_room;

	if ((ml = recieve_data(user->sockfd, (void **)&msg, 0, MSG_WAITALL)) < 0)
		return (-1);
	else if (msg_valid(msg) < 0)
		return (h_clean(msg));
	else if (treated_as_command(msg, ml, user))
		return (h_clean(msg));
	else if (!(p_msg = ft_strnew(ml + ft_strlen(user->nickname) + 16)))
		return (h_clean(msg));
	msg[ml - 1] = 0;
	ml = sprintf(p_msg, MSG_POINT"[%s]: %s", user->nickname, msg);
	p_msg[ml] = h_clean(msg);
	chat_room = user->chat_room_node->content;
	dprintf(chat_room->log_fd, "%s\n", p_msg);
	clients = chat_room->users;
	while (clients && (clients = clients->next) != chat_room->users)
		if (clients->content && clients != user->node_in_room)
			send_msg(clients->content, p_msg, ml);
	return (h_clean(p_msg) + ml);
}

void				listen_client(int fd, short ev, t_dlist *c_node)
{
	t_client *user;

	(void)fd;
	(void)ev;
	if (!(user = (t_client *)c_node->content))
		return ;
	else if (!*user->nickname)
	{
		if (get_nickname(user) < 0)
			return (disconnect_client(c_node));
		sync_chat_history(user);
		ft_dlstpush(&((t_chat_room *)user->chat_room_node->content)->users,
			(user->node_in_room = ft_dlstnew(ft_memcpy(malloc(sizeof(t_client)),
								user, sizeof(t_client)), sizeof(void *))));
		ft_dlstpush(&g_env.all_clients, c_node);
		log_client_actions(user, "CONNECTED", "joined the chat");
		update_clients_data(user->chat_room_node->content);
		update_room_list(user);
	}
	else if (read_client_msg(user) < 0)
		return (disconnect_client(c_node));
}

void				handle_con(void)
{
	int					new_c;
	t_client			*new_client;
	t_dlist				*c_node;
	struct sockaddr_in	c_data;
	static socklen_t	c_len = sizeof(c_data);

	if (!good_connection(g_env.msocket))
		return ((void)log_errors(g_env.err_fd, init_socket()));
	if ((new_c = accept(g_env.msocket, (struct sockaddr*)&c_data, &c_len)) < 0)
		return ((void)log_errors(g_env.err_fd, strerror(errno)));
	else if (!(new_client = ft_memalloc(sizeof(t_client))) && !close(new_c))
		return ((void)log_errors(g_env.err_fd, strerror(errno)));
	new_client->sockfd = new_c;
	new_client->chat_room_node = g_env.all_rooms->next;
	if (!(c_node = ft_dlstnew(new_client, sizeof(void *)))
		&& !h_clean(new_client) && !close(new_c))
		return ((void)log_errors(g_env.err_fd, strerror(errno)));
	event_set(&new_client->ev, new_c, EV_READ | EV_PERSIST,
		(void (*)(int, short, void *))&listen_client, c_node);
	event_add(&new_client->ev, NULL);
}

void				sig_handler(int sig)
{
	char		err_msg[256];
	int			critical;
	static int	is_critical[] = {
		0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0,
		0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1,
		1, 0, 0, 0, 0, 0
	};
	static char	*signames[] = {
		"Unknown", "SIGHUP", "SIGINT", "SIGQUIT", "SIGILL", "SIGTRAP", "SIGIOT",
		"SIGBUS", "SIGFPE", "SIGKILL", "SIGUSR1", "SIGSEGV", "SIGUSR2",
		"SIGPIPE", "SIGALRM", "SIGTERM", "SIGSTKFLT", "SIGCHLD", "SIGCONT",
		"SIGSTOP", "SIGTSTP", "SIGTTIN", "SIGTTOU", "SIGURG", "SIGXCPU",
		"SIGXFSZ", "SIGVTALRM", "SIGPROF", "SIGWINCH", "SIGIO", "SIGPWR"};

	if (sig < 0 || sig >= (int)(sizeof(signames) / sizeof(*signames)))
		sig = 0;
	critical = is_critical[sig];
	sprintf(err_msg, "[signal: %d -> %s]", sig, signames[sig]);
	log_errors(g_env.err_fd, err_msg);
	critical ? exit(sig) : 0;
}

int					main(void)
{
	const char	*err;
	short int	sig;
	pid_t		server_pid;

	if ((server_pid = fork()) < 0)
		return (ft_printf("Server start failed!\n") * 0 - EXIT_FAILURE);
	else if (server_pid > 0)
		return (ft_printf("Server pid -> [%d]\n", server_pid) * 0);
	server_pid = setsid();
	event_init();
	ft_memset(&g_env, (sig = 0), sizeof(t_env));
	while (++sig <= 30)
		signal(sig != SIGKILL && sig != SIGSTOP ? sig : 1, sig_handler);
	if ((g_env.sys_fd = open(LOG_SYS_PATH, O_LOG_FLAGS)) < 0
			|| (g_env.err_fd = open(LOG_ERR_PATH, O_LOG_FLAGS)) < 0)
		return (EXIT_FAILURE);
	else if ((err = new_chat_room("general", 0)))
		return (log_errors(g_env.err_fd, err) * 0 + EXIT_FAILURE);
	else if ((err = init_socket()))
		return (log_errors(g_env.err_fd, err) * 0 + EXIT_FAILURE);
	event_set(&g_env.m_ev, g_env.msocket, EV_READ | EV_PERSIST,
				(void (*)(int, short, void *))&handle_con, NULL);
	event_add(&g_env.m_ev, NULL);
	event_dispatch();
	return (0);
}
