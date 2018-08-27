/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   ft_dlstmerge.c                                                           */
/*                                                                            */
/*   By: phrytsenko                                                           */
/*                                                                            */
/*   Created: 2018/08/27 17:50:52 by phrytsenko                               */
/*   Updated: 2018/08/27 18:40:07 by phrytsenko                               */
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
