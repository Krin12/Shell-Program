#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_COMMAND_LEN 1024
#define MAX_ARGS 64

void execute_command(char *command, int background) {
    char *args[MAX_ARGS];
    int arg_count = 0;

    char *token = strtok(command, " ");
    while (token != NULL && arg_count < MAX_ARGS - 1) {
        args[arg_count++] = token;
        token = strtok(NULL, " ");
    }
    args[arg_count] = NULL;

    if (strcmp(args[0], "cd") == 0) {
        if (arg_count < 2) {
            fprintf(stderr, "Usage: cd <directory>\n");
            return;
        }

        if (chdir(args[1]) != 0) {
            perror("chdir() error");
            return;
        }
        return;
    }

    if (strcmp(args[0], "cat") == 0) {
        if (arg_count < 2) {
            fprintf(stderr, "Usage: cat <filename>\n");
            return;
        }

        FILE *file = fopen(args[1], "r");
        if (file == NULL) {
            perror("Error opening file");
            return;
        }

        char buffer[MAX_COMMAND_LEN];
        while (fgets(buffer, sizeof(buffer), file) != NULL) {
            printf("%s", buffer);
        }

        fclose(file);
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("execvp failed");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    } else if (pid > 0) {
        if (!background) {
            int status;
            waitpid(pid, &status, 0);
        }
    } else {
        perror("fork failed");
    }
}

int main() {
    char command[MAX_COMMAND_LEN];

    while (1) {
        printf("Shell> ");
        fgets(command, MAX_COMMAND_LEN, stdin);
        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "exit") == 0) {
            printf("Exiting shell...\n");
            break;
        }

        int background = 0;
        if (command[strlen(command) - 1] == '&') {
            background = 1;
            command[strlen(command) - 1] = '\0';
        }

        execute_command(command, background);
    }

    return 0;
}
