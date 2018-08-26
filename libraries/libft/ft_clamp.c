/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_clamp.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgritsen <pgritsen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/08/26 12:53:51 by pgritsen          #+#    #+#             */
/*   Updated: 2018/08/26 12:55:16 by pgritsen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

uint64_t	ft_clamp(uint64_t min, uint64_t max, uint64_t val)
{
	if (val < min)
		return (min);
	else if (val > max)
		return (max);
	else
		return (val);
}
