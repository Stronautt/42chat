/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_dlstmerge.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/09/02 14:09:32 by pgritsen          #+#    #+#             */
/*   Updated: 2018/09/02 14:09:39 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

t_dlist		*ft_dlstmerge(t_dlist **a, t_dlist **b)
{
	if (!a)
		return (NULL);
	else if (!b)
		return (*a);
	else if (!*a || !*b)
		return (*a = *b);
	(*a)->next->prev = (*b)->prev;
	(*b)->next->prev = (*a);
	(*b)->prev->next = (*a)->next;
	(*a)->next = (*b)->next;
	ft_memdel((void **)&(*b)->content);
	ft_memdel((void **)b);
	return (*a);
}
