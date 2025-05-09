#include "shell.h"
#include "builtins.h"

static const builtin_cmd_t builtin_commands[] = {
    {"cd", "Change directory. Usage: cd [directory|~|-]", builtin_cd},
    {"exit", "Exit the shell. Usage: exit", builtin_exit},
    {"pwd", "Print current working directory. Usage: pwd", builtin_pwd },
    {"echo", "Display a line of text. Usage: echo [text...]", builtin_echo},
    {"help", "Display information about built-in commands. Usage: help [command...]", builtin_help},
    {NULL, NULL, NULL}
};


int is_builtin(const char *cmd) {
    if (!cmd) return 0;

    for (int i = 0; builtin_commands[i].name != NULL; i++) {
        if (strcmp(builtin_commands[i].name, cmd) == 0) {
            return 1;
        }
    }

    return 0;
}

int execute_builtin_command(command_t *cmd) {
    if (!cmd || cmd->arg_count == 0) {
        return -1;
    }

    for (int i = 0; builtin_commands[i].name != NULL; i++) {
        if (strcmp(builtin_commands[i].name, cmd->args[0]) == 0) {
            return builtin_commands[i].func(cmd);
        }
    }

    return -1;
}

int builtin_echo(command_t *cmd) {
    for (int i = 0; i < cmd->arg_count; i++) {
        printf("%s ", cmd->args[i]);
    }

    printf("\n");
    return 0;
}

int builtin_exit(command_t *cmd) {
    if (cmd->arg_count > 1) {
        fprintf(stderr, "minishell: exit: Too much arguments.\n");
        return 1;
    }

    global_state->is_running = 0;
    return 0;
}

int builtin_pwd(command_t *cmd) {
    if (cmd->arg_count > 1) {
        fprintf(stderr, "minishell: pwd: Too much argument.\n");
        return 1;
    }

    char cwd[MAX_PATH_LENGTH];
    if (getcwd(cwd, MAX_PATH_LENGTH) == NULL) {
        perror("getcwd");
        return 1;
    }

    printf("%s\n", cwd);
    return 0;
}

int builtin_cd(command_t *cmd) {
    char *dir;

    if (cmd->arg_count > 2) {
        fprintf(stderr, "%s\n", "minishell: cd: Too much arguments.\n");
        return 1;
    }

    if (cmd->arg_count == 1 || strcmp(cmd->args[1], "~") == 0) {
        dir = getenv("HOME");
        if (dir == NULL) {
            fprintf(stderr, "minishell: cd: HOME environment variable not defined.\n");
            return 1;
        }
    } else if (strcmp(cmd->args[1], "-") == 0) {
        dir = getenv("OLDPWD");
        if (dir == NULL) {
            fprintf(stderr, "minishell: cd: OLDPWD environment variable not defined.\n");
            return 1;
        }

        printf("%s\n", dir);
    } else {
        dir = cmd->args[1];
    }

    char old_dir[MAX_PATH_LENGTH];
    if (getcwd(old_dir, MAX_PATH_LENGTH) == NULL) {
        perror("getcwd");
        return 1;
    }

    if (chdir(dir) != 0) {
        fprintf(stderr, "minishell: cd: Unable to navigate to %s: %s\n", dir, strerror(errno));
        return 1;
    }

    setenv("OLDPWD", old_dir, 1);

    char new_dir[MAX_PATH_LENGTH];
    if (getcwd(new_dir, MAX_PATH_LENGTH) != NULL) {
        setenv("PWD", new_dir, 1);
        strcpy(global_state->crnt_dir, new_dir);
    }

    return 0;
}

int builtin_help(command_t *cmd) {
    if (cmd->arg_count > 2) {
        fprintf(stderr, "minishell: help: Too many arguments.\n");
        return 1;
    }

    if (cmd->arg_count == 1) {
        printf("Minishell built-in commands:\n");
        for (int i = 0; builtin_commands[i].name != NULL; i++) {
            printf("  %s - %s\n", builtin_commands[i].name, builtin_commands[i].help);
        }
    } else {
        // Display help for a specific command
        const char *cmd_name = cmd->args[1];
        int found = 0;

        for (int i = 0; builtin_commands[i].name != NULL; i++) {
            if (strcmp(builtin_commands[i].name, cmd_name) == 0) {
                printf("%s\n", builtin_commands[i].help);
                found = 1;
                break;
            }
        }

        if (!found) {
            fprintf(stderr, "minishell: help: no help found for '%s'\n", cmd_name);
            return 1;
        }
    }

    return 0;
}