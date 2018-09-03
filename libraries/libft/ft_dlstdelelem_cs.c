/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_dlstdelelem_cs.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/01/08 20:53:36 by pgritsen          #+#    #+#             */
/*   Updated: 2018/01/09 17:51:26 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	ft_dlstdelelem_cs(t_dlist **del)
{
	if (!del || !*del)
		return ;
	(*del)->next->prev = (*del)->prev;
	(*del)->prev->next = (*del)->next;
	ft_memdel((void **)del);
}
