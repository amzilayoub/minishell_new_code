#include "minishell.h"

void	treat_line(char *line)
{
	int	i;
	int	start;
	char	quote;

	i = -1;
	start = 0;
	quote = 0;
	while (line[++i])
	{
		if (line[i] == '\\')
		{
			i++;
			continue;
		}
		if ((line[i] == '"' || line[i] == '\'') && !quote)
			quote = line[i];
		else if (quote && line[i] == quote)
			quote = 0;
		else if (!quote && line[i] == ';')
		{
			create_cmd_list(&g_cmd_list, ft_substr(line, start, i - start));
			start = i + 1;
		}
	}
	create_cmd_list(&g_cmd_list, ft_substr(line, start, i + 1 - start));
}

int	is_redirection(char *red, char *line)
{
	if (!ft_strncmp(red, ">>", 2))
		return (0);
	else if (red[0] == '>' || red[0] == '<')
		return (0);
	else
		return (1 + skip_char(line + 1, ' '));
}

int	fd_num(char *line, int index)
{
	int nb;

	--index;
	while (index >= 0 && ft_isalnum(line[index]))
		--index;
	if (line[index] != ' ')
		return (1);
	nb = ft_atoi(line + index + 1);
	while (line[++index] != '>')
		line[index] = ' ';
	return (nb);
}

void	treat_list(t_cmd *cmd_list)
{
	int i;
	int j;
	int start;
	char *tmp;

	if (!cmd_list)
		return ;
	if (check_pipes_error(&cmd_list->line))
		return ;
	i = -1;
	start = 0;
	while (cmd_list->line[++i])
	{
		if (cmd_list->line[i] == '\\')
		{
			i++;
			continue;
		}
		j = -1;
		while (g_pipe_cmd[++j])
		{
			if (g_pipe_cmd[j][0] == cmd_list->line[i])
			{
				if ((tmp = ft_substr(cmd_list->line, start, i - start))[0])
					add_cmd(&cmd_list->cmd, tmp);
				//printf("XX = %s\n",cmd_list->line + i);
				start = i + is_redirection(g_pipe_cmd[j], cmd_list->line + i)/* + skip_char(cmd_list->line + i + 1, ' ')*/;
				i += (!ft_strncmp(cmd_list->line + i, ">>", 2));
				break;
			}
		}
	}
	add_cmd(&cmd_list->cmd, ft_substr(cmd_list->line, start, i - start + 1));
	treat_list(cmd_list->next);
}

int	prompt(void)
{
	return (write(1, "$ ", 2));
}

void	print_cmd(t_piped_cmd *list)
{
	static int j = 1;
	int i;

	if (list)
	{
		i = -1;
		//printf("--------------------  FD = %d ------------------\n", list->fd);
		while (list->params[++i])
		{
			printf("   %i-- |%s|\n",j, list->params[i]);
			//printf("first CHAR = %d\n", list->params[i][0]);
		}
		j++;
		print_cmd(list->next);
	}
	j = 1;
}

void	print_list(t_cmd *list)
{
	if (list)
	{
		printf("%s\n", list->line);
		print_cmd(list->cmd);
		print_list(list->next);
	}
}

char	*shift_char(char *str)
{
	int i;

	i = -1;
	while (str[++i])
		str[i] = str[i + 1];
}

int	skip_char(char *line, char c)
{
	int i;

	i = 0;
	while (line[i] == c)
		i++;
	return (i);
}

int	get_arg_len(char *line)
{
	int i;
	int len;
	char quote;

	quote = 0;
	i = -1;
	len = ((*line) != 0);
	while (line[++i])
	{
		if (line[i] == '\\' && ++i)
			continue;
		if ((line[i] == '\'' || line[i] == '"') && !quote)
			quote = line[i];
		else if (line[i] == quote)
		{
			len++;
			quote = 0;
		}
		else if (!quote && line[i] == ' ')
		{
			i += skip_char(&line[i], ' ') - 1;
			len++;
		}
	}
	return (len);
}

int	join_env_var(char **str, int index, char **envp)
{
	int i;
	char *key;
	char *pre;
	char *suff;

	i = index + 1;
	if ((*str)[i] != '?')
	{
		while ((*str)[i] && (*str)[i] != '\\' && (*str)[i] != ' ' && ft_isalnum((*str)[i]))
			i++;
		key = ft_substr((*str), index, i - index);
		if (!(key = ft_get_env_value(key, envp)))
			key = "";
	}
	else
	{
		i++;
		key = ft_itoa(g_status);
	}
	pre = ft_substr((*str), 0, index);
	suff = ft_substr((*str), i, ft_strlen((*str)) - i);
	(*str) = ft_strjoin(ft_strjoin(pre, key), suff);
	// substract 1 bcuz of the pre increment in the get_arg loop
	return (ft_strlen(key) - 1);
}

char	**get_arg(char *line, char **envp)
{
	int i;
	char **args;
	char quote;
	int j;
	int start;
	char *tmp;

	i = -1;
	start = 0;
	j = -1;
	//printf("LEN = %i\n", get_arg_len(line));
	args = (char**)malloc(sizeof(char*) * (get_arg_len(line) + 1));
	quote = 0;
	while (line[++i])
	{
		if (line[i] == '\\')
		{
			if (!quote || line[i + 1] == '\\')
				shift_char(line + i);
			else if (quote && line[i + 1] == quote)
				shift_char(line + i);
		}
		else if (line[i] == '$')
			i += join_env_var(&line, i, envp);
		else if ((line[i] == '"' || line[i] == '\'') && !quote)
		{
			start = i + 1;
			quote = line[i];
		}
		else if (line[i] == quote && !(quote = 0))
		{
			args[++j] = ft_substr(line, start, i - start);
			start = i + 1;
		}
		else if (!quote && line[i] == ' '/* && (i += skip_char(line + i, ' ') - 1)*/)
		{
			if ((tmp = ft_substr(line, start, i - start))[0])
				args[++j] = tmp;
			i += skip_char(line + i, ' ') - 1;
			start = i + 1;
		}
	}
	if ((tmp = ft_substr(line, start, i - start))[0])
		args[++j] = tmp;
	args[++j] = NULL;
	return (args);
}

void	treat_cmd_helper(t_piped_cmd *list, char **envp)
{
	if (!list)
		return ;
	list->params = get_arg(list->line, envp);
	treat_cmd_helper(list->next, envp);
}

void	treat_cmd(t_cmd *list, char **envp)
{
	if (!list)
		return ;
	treat_cmd_helper(list->cmd, envp);
	treat_cmd(list->next, envp);
}


void	sort_cmd_for_redirections(t_piped_cmd **current, t_piped_cmd **next)
{
	char **tmp;

	//printf("CUR = %s\n", (*current)->params[0]);
	if (!(*current) || !(*next))
		return ;
	if ((*next)->params[0][0] == '<')
	{
		if ((*current)->params[0][0] == '>')
			(*next) = (*next)->next;
		else
		{
			tmp = (*current)->params;
			(*current)->params = (*next)->params;
			(*next)->params = tmp;
		}
	}
	sort_cmd_for_redirections(next, &(*next)->next);
}

void	call_commands_helper(t_piped_cmd *list, char ***envp, int pipe_index)
{
	int i;

	if (!list)
		return ;
	i = -1;
	if (pipe_index < g_pipes_count)
	{
		dup2(g_pipes_fd[pipe_index][1], 1);
		close(g_pipes_fd[pipe_index][1]);
	}
	if (!g_read_from_file && (pipe_index != 0 && pipe_index < g_pipes_count))
	{
		dup2(g_pipes_fd[pipe_index - 1][0], 0);
		close(g_pipes_fd[pipe_index - 1][0]);
	}
	if (!list->next && list->params[0][0] != '>')
	{
		dup2(g_stdio_fd[2], 2);
		dup2(g_stdio_fd[1], 1);
	}
	g_read_from_file = 0;
	g_next_cmd = (list->next) ? list->next->params[0] : NULL;
	if (THERE_IS_ERROR)
		return ;
	while (g_cmd_char[++i])
	{
		//printf("%s\n", g_cmd_char[i]);
		if (!ft_strcmp(g_cmd_char[i], list->params[0]))
		{
			g_status = g_builtins[i](list->params + 1, envp);
			break;
		}
	}
	if (!g_cmd_char[i])
	{
		if (!fork())
		{
			execve(list->params[0], list->params, (*envp));
			FT_PUTSTR_ERR("Minishell: Command not found\n");
			exit(-1);
		}
		else
		{
			wait(&g_status);
		}
	}
	call_commands_helper(list->next, envp, ++pipe_index);
}

void	set_pipes(void)
{
	int i;

	g_pipes_fd = (int**)malloc(sizeof(int*) * g_pipes_count);
	i = -1;
	while (++i < g_pipes_count)
	{
		g_pipes_fd[i] = (int*)malloc(sizeof(int) * 2);
		pipe(g_pipes_fd[i]);
	}
}


void	call_commands(t_cmd *list, char ***envp)
{
	int i;

	if (!list)
		return ;
	sort_cmd_for_redirections(&list->cmd, &list->cmd->next);
	g_pipes_count = list_cmd_size(list->cmd);
	if (g_pipes_count > 1)
	{
		set_pipes();
	}
	else
		g_pipes_count = 0;
	//printf("NB_PIPE = %d\n", g_pipes_count);
	call_commands_helper(list->cmd, envp, 0);
	if (!THERE_IS_ERROR)
		call_commands(list->next, envp);
}

void	ft_sigint(int num)
{
	FT_PUTSTR("\n$ ");
}

void	shell_loop(char **envp)
{
	char *line;

	signal(SIGINT, ft_sigint);
	signal(SIGQUIT, ft_sigint);
	while (prompt() && get_next_line(0, &line) > 0)
	{
		clear_cmd_list(&g_cmd_list);
		if ((check_semicolons(line)))
		{
			FT_PUTSTR_ERR(ERROR_MSG);
			continue;
		}
		treat_line(line);
		treat_list(g_cmd_list);
		if (THERE_IS_ERROR)
		{
			FT_PUTSTR_ERR(ERROR_MSG);
			//print_list(g_cmd_list);
			continue;
		}
		treat_cmd(g_cmd_list, envp);
		call_commands(g_cmd_list, &envp);
		dup2(g_stdio_fd[0], 0);
		dup2(g_stdio_fd[1], 1);
		dup2(g_stdio_fd[2], 2);
		//printf("--------------------------FINISH--------------------\n");
				//print_list(g_cmd_list);

	}
}
