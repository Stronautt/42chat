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

static inline void 	*protocol_clean(t_dlist **dlst, void *data)
{
	ft_dlstclear(dlst);
	free(data);
	return (NULL);
}

t_dlist				*ft_strsplit_dlst(char const *s, char c)
{
	size_t	offset;
	char	*part;
	t_dlist	*dlst;
	t_dlist	*new;

	if (!s--)
		return (NULL);
	dlst = NULL;
	while (*++s)
		if (*s != c && (offset = ft_strclen(s, c)))
		{
			if (!(part = ft_strsub(s, 0, offset)))
				return (protocol_clean(&dlst, part));
			else if (!(new = ft_dlstnew(part, offset)))
				return (protocol_clean(&dlst, part));
			ft_dlstpush(&dlst, new);
			if (!dlst)
				return (protocol_clean(&dlst, part));
			s += offset - 1;
		}
	return (dlst);
}
