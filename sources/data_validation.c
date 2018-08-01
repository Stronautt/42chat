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
	while (*nickname)
		if (!ft_isalnum(*nickname++))
			return (0);
	return (1);
}
