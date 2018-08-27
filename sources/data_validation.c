/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   data_validation.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/08/01 14:35:41 by pgritsen          #+#    #+#             */
/*   Updated: 2018/08/01 14:45:23 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "node.h"

int		nickname_is_valid(const char * nickname)
{
	if (ft_strlen(nickname) <= 0)
		return (0);
	while (*nickname++)
		if ((*(nickname - 1) > 0 && !ft_isprint(*(nickname - 1)))
			|| *(nickname - 1) == '[' || *(nickname - 1) == ']'
			|| *(nickname - 1) == ':' || *(nickname - 1) == ' ')
			return (0);
	return (1);
}

size_t	splitted_size(char ** parts)
{
	size_t	size = 0;

	if (!parts)
		return (0);
	while (parts[size])
		size++;
	return (size);
}

void	free_splitted(char ** parts)
{
	char	** s_p = parts;

	if (!parts)
		return ;
	while (*parts)
		free(*parts++);
	free(s_p);
}
