/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_str_replace.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/10/25 19:25:56 by pgritsen          #+#    #+#             */
/*   Updated: 2017/11/10 20:57:53 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_str_replace(char *src, char b, char t)
{
	size_t	it;

	if (!src)
		return (NULL);
	it = -1;
	while (src[++it])
		if (src[it] == b)
			src[it] = t;
	return (src);
}
