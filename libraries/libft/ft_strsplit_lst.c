/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strsplit_lst.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/11/04 21:27:45 by pgritsen          #+#    #+#             */
/*   Updated: 2017/12/13 15:42:37 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static void		node_del(void *content, size_t content_size)
{
	ft_bzero(content, content_size);
	ft_memdel(&content);
}

t_list			*ft_strsplit_lst(char const *s, char c)
{
	t_list	*list;
	t_list	*tmp;
	char	**splitted;
	ssize_t	it;

	list = NULL;
	if (!s)
		return (NULL);
	else if (!(splitted = ft_strsplit(s, c)))
		return (NULL);
	it = -1;
	while (splitted[++it])
	{
		if (!(tmp = ft_lstnew(splitted[it], sizeof(void *))))
		{
			ft_lstdel(&list, &node_del);
			return (NULL);
		}
		ft_lstadd_back(&list, tmp);
	}
	free(splitted);
	return (list);
}
