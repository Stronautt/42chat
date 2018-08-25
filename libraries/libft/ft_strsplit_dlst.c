/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strsplit_dlst.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/11/04 21:27:45 by pgritsen          #+#    #+#             */
/*   Updated: 2017/12/13 15:42:37 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

t_dlist			*ft_strsplit_dlst(char const *s, char c)
{
	t_dlist	*dlst;
	t_dlist	*tmp;
	char	**splitted;
	ssize_t	it;

	dlst = NULL;
	if (!s)
		return (NULL);
	else if (!(splitted = ft_strsplit(s, c)))
		return (NULL);
	it = -1;
	while (splitted[++it])
	{
		if (!(tmp = ft_dlstnew(splitted[it], sizeof(void *))))
		{
			ft_dlstclear(&dlst);
			return (NULL);
		}
		ft_dlstpush(&dlst, tmp);
	}
	free(splitted);
	return (dlst);
}
