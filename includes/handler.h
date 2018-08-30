/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handler.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/07/30 13:18:50 by pgritsen          #+#    #+#             */
/*   Updated: 2018/08/26 13:59:18 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HANDLER_H
# define HANDLER_H

extern int	g_symb;
extern bool	g_input_avb;
extern bool	g_prevent_update;

/*
**			Key_handler.c
**			↓↓↓↓↓↓↓↓↓↓↓↓↓
*/

void		readline_redisplay(void);

int			readline_input_avail(void);

int			readline_getc(void);

uint64_t	get_uni_key(void);

void		handle_uni_key(uint64_t utf);

#endif
