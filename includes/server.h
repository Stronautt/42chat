/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: phrytsenko <phrytsenko@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/08/03 17:29:59 by phrytsenko        #+#    #+#             */
/*   Updated: 2018/08/03 17:32:18 by phrytsenko       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_H
# define SERVER_H

typedef struct	s_assocc
{
	char	* key;
	void (*func)();
}				t_assocc;

#endif
