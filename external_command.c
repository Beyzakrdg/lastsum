/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   external_command.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bekarada <bekarada@student.42istanbul.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/15 17:19:31 by bekarada          #+#    #+#             */
/*   Updated: 2025/04/15 18:28:32 by bekarada         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

char	**token_list_to_args(t_token *tokens)
{
	int		count = 0;
	t_token	*tmp = tokens;
	char	**args;
	int		i = 0;

	while (tmp)
	{
		count++;
		tmp = tmp->next;
	}

	args = malloc(sizeof(char *) * (count + 1));
	if (!args)
		return NULL;

	tmp = tokens;
	while ( i < count)
	{
		args[i] = ft_strdup(tmp->value);
		tmp = tmp->next;
		i++;
	}
	args[count] = NULL;
	return args;
}
char **env_list_to_array(t_environment *env_list)
{
    int i = 0;
    int len = 0;
    t_environment *tmp = env_list;
    char **env_array;

    while (tmp)
    {
        len++;
        tmp = tmp->next;
    }

    env_array = malloc(sizeof(char *) * (len + 1));
    if (!env_array)
        return NULL;

    tmp = env_list;
    while (tmp)
    {
        int var_len = ft_strlen(tmp->variable);
        int val_len = ft_strlen(tmp->value);
        env_array[i] = malloc(var_len + val_len + 2);
        if (!env_array[i])
            return NULL;

        ft_strcpy(env_array[i], tmp->variable);
        ft_strcat(env_array[i], "=");
        ft_strcat(env_array[i], tmp->value);
        i++;
        tmp = tmp->next;
    }
    env_array[i] = NULL;
    return env_array;
}

char *ft_strjoin_path(char *s1, char *s2)
{
	char *result;
	size_t len1 = strlen(s1);
	size_t len2 = strlen(s2);

	result = malloc(len1 + len2 + 2);
	if (!result)
		return NULL;

	strcpy(result, s1);
	result[len1] = '/';
	strcpy(result + len1 + 1, s2);
	return result;
}

char *ft_getenv(t_environment *env_list, const char *variable)
{
	while (env_list)
	{
		if (strcmp(env_list->variable, variable) == 0)
			return env_list->value;
		env_list = env_list->next;
	}
	return NULL;
}

char *get_command_path(char *cmd, t_environment *env)
{
	char	*path_env;
	char	**paths;
	char	*full_path;
	int		i;

	if (access(cmd, X_OK) == 0)
		return strdup(cmd);

	path_env = ft_getenv(env, "PATH");
	if (!path_env)
		return NULL;

	paths = ft_split(path_env, ':');
	if (!paths)
		return NULL;

	i = 0;
	while (paths[i])
	{
		full_path = ft_strjoin_path(paths[i], cmd);
		if (!full_path)
			break;
		if (access(full_path, X_OK) == 0)
		{
			int j = 0;
			while (paths[j])
				free(paths[j++]);
			free(paths);
			return full_path;
		}
		free(full_path);
		i++;
	}
	i = 0;
	while (paths[i])
		free(paths[i++]);
	free(paths);
	return NULL;
}

void	ft_exec_not_built(t_program *prgm)
{
	pid_t	pid;
	int		status;
	char	**args;
	char	**envp;
	char	*command_path;
	int		i = 0;

	if (!prgm || !prgm->tokens || !prgm->tokens->value)
		return;
	args = token_list_to_args(prgm->tokens);
	if (!args)
		return;
	command_path = get_command_path(args[0], prgm->env_list);
	if (!command_path)
	{
		perror("Command not found");
		free(args);
		return;
	}
	pid = fork();
	if (pid < 0)
		perror("Fork");
	else if (pid == 0)
	{
		envp = env_list_to_array(prgm->env_list);
		if(execve(command_path, args, envp) == -1)
		{
			perror("Execve");
			exit(127);
		}
	}
	else
		waitpid(pid, &status, 0);
	free(command_path);
	while (args[i])
	{
		free(args[i]);
		i++;
	}
	free(args[i]);
}