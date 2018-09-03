/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_vstrjoin.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/12/08 19:53:23 by pgritsen          #+#    #+#             */
/*   Updated: 2018/02/02 18:21:52 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include <stdarg.h>

int		s_clean(void *data)
{
	free(data);
	return (0);
}

char	*ft_vstrjoin(int n, ...)
{
	va_list	args;
	char	*ret;

	if (n <= 0)
		return (NULL);
	va_start(args, n);
	ret = ft_strdup(va_arg(args, char *));
	if (n == 1)
		return (ret);
	while (n-- > 1)
		ret = ft_strjoin(ret, va_arg(args, char *)) - s_clean(ret);
	va_end(args);
	return (ret);
}