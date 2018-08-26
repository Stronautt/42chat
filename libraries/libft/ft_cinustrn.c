/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_cinustrn.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/08/26 19:49:17 by pgritsen          #+#    #+#             */
/*   Updated: 2018/08/26 20:09:26 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

size_t	ft_cinustrn(const char *s, size_t n)
{
	size_t				ret;
	const unsigned char	*p_s;

	if (!s)
		return (0);
	p_s = (const unsigned char *)s;
	ret = 0;
	while (*p_s && (size_t)(p_s - (const unsigned char *)s) < n && ++ret)
		if (*p_s == 0x1B)
			p_s += 8;
		else if (*p_s >= 0xC2 && *p_s <= 0xDF)
			p_s += 2;
		else if (*p_s >= 0xE0 && *p_s <= 0xEF)
			p_s += 3;
		else if (*p_s >= 0xF0 && *p_s <= 0xF4)
			p_s += 4;
		else
			p_s += 1;
	return (ret);
}
