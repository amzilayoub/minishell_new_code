/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shell_loop.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aamzil <aamzil@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/18 16:17:47 by aamzil            #+#    #+#             */
/*   Updated: 2020/10/26 11:32:20 by aamzil           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int		prompt(void)
{
	return (write(1, "$ ", 2));
}

void	ft_sigint(int num)
{
	(void)num;
	FT_PUTSTR("\n$ ");
}

/*
** To see the commands with their parmams,
** add this line print_list(g_cmd_list)
** before close_fd() function
*/

void	shell_loop(char **envp)
{
	char *line;

	signal(SIGINT, ft_sigint);
	signal(SIGQUIT, ft_sigint);
	while (prompt() && get_next_line(0, &line) > 0)
	{
		add_mem(line);
		if (check_semicolons(line))
		{
			FT_PUTSTR_ERR(ERROR_MSG);
			continue;
		}
		treat_line(line);
		treat_list(g_cmd_list);
		if (THERE_IS_ERROR)
		{
			FT_PUTSTR_ERR(ERROR_MSG);
			continue;
		}
		treat_cmd(g_cmd_list, envp);
		call_commands(g_cmd_list, &envp);
		close_fd();
		clear_cmd_list(&g_cmd_list);
		free_memory(&g_mem_alloc, FREE_MODE);
	}
}
