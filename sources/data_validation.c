/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   data_validation.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/08/01 14:35:41 by pgritsen          #+#    #+#             */
/*   Updated: 2018/09/02 14:57:30 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "node.h"

int		nickname_is_valid(const char *nickname)
{
	if (!nickname || ft_strlen(nickname) <= 0)
		return (0);
	while (*nickname++)
		if ((*(nickname - 1) > 0 && !ft_isprint(*(nickname - 1)))
			|| *(nickname - 1) == '[' || *(nickname - 1) == ']'
			|| *(nickname - 1) == ':' || *(nickname - 1) == ' '
			|| *(nickname - 1) == '/')
			return (0);
	return (1);
}

int		h_clean(void *data)
{
	free(data);
	return (0);
}

size_t	splitted_size(char **parts)
{
	size_t	size;

	size = 0;
	if (!parts)
		return (0);
	while (parts[size])
		size++;
	return (size);
}

int		free_splitted(char **parts)
{
	char	**s_p;

	s_p = parts;
	if (!parts)
		return (0);
	while (*parts)
		free(*parts++);
	free(s_p);
	return (0);
}
