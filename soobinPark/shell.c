#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h> 

#define MAX_BACKGROUND_PROCESSES 10


#define MAX_COMMAND_LEN 1024
#define MAX_ARGS 64

pid_t background_processes[MAX_BACKGROUND_PROCESSES];
int num_background_processes = 0;

volatile sig_atomic_t sigint_flag = 0;

void handle_signal(int signo) {
    if (signo == SIGINT) {
          printf("\n");
        sigint_flag = 1;
    }
}

void execute_command(char *command, int background) {
    char *args[MAX_ARGS];
    int arg_count = 0;
    int redirect_out = 0;
    int pipe_pos = -1;

    char *token = strtok(command, " ");
    while (token != NULL && arg_count < MAX_ARGS - 1) {
        args[arg_count++] = token;
        if (strcmp(token, "|") == 0) {
            pipe_pos = arg_count - 1;
            args[arg_count - 1] = NULL;
        }
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

    if (strcmp(args[0], "fg") == 0) {
        if (num_background_processes > 0) {
            pid_t pid = background_processes[num_background_processes - 1];
            waitpid(pid, NULL, 0);
            num_background_processes--;
        } else {
            fprintf(stderr, "No background process to bring to foreground\n");
        }
        return;
    }

    if (strcmp(args[0], "sleep") == 0 && background) {
        pid_t pid = fork();
        if (pid == 0) {
            if (execvp(args[0], args) == -1) {
                perror("execvp failed");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        } else if (pid > 0) {
            background_processes[num_background_processes++] = pid;
            printf("[%d] %s\n", pid, command); 
        } else {
            perror("fork failed");
        }
        return;
    }

    if (strcmp(args[0], "jobs") == 0) {
        printf("Running background processes:\n");
        for (int i = 0; i < num_background_processes; ++i) {
            printf("[%d] Running\n", background_processes[i]);
        }
        return;
    }


    for (int i = 0; args[i] != NULL; ++i) {
        if (strcmp(args[i], ">") == 0) {
            redirect_out = i;
            args[i] = NULL;
        }
    }

    if (pipe_pos >= 0) {
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            perror("pipe");
            return;
        }

        pid_t pid1, pid2;

        pid1 = fork();
        if (pid1 < 0) {
            perror("fork");
            return;
        } else if (pid1 == 0) {
            close(pipefd[0]); 
            dup2(pipefd[1], STDOUT_FILENO); 
            close(pipefd[1]); 

            execvp(args[0], args);
            perror("execvp");
            exit(EXIT_FAILURE);
        } else {
            waitpid(pid1, NULL, 0);

            pid2 = fork();
            if (pid2 < 0) {
                perror("fork");
                return;
            } else if (pid2 == 0) {
                close(pipefd[1]); 
                dup2(pipefd[0], STDIN_FILENO); 
                close(pipefd[0]); 

                execvp(args[pipe_pos + 1], &args[pipe_pos + 1]);
                perror("execvp");
                exit(EXIT_FAILURE);
            } else {
                close(pipefd[0]);
                close(pipefd[1]);
                waitpid(pid2, NULL, 0);
            }
        }
    } else if (redirect_out > 0 && args[redirect_out + 1] != NULL) {
        int fd = open(args[redirect_out + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            perror("Error opening output file");
            return;
        }

        pid_t pid = fork();
        if (pid == 0) {
            dup2(fd, STDOUT_FILENO);
            close(fd);

            if (execvp(args[0], args) == -1) {
                perror("execvp failed");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
        } else {
            perror("fork failed");
        }
    } else {
        pid_t pid = fork();
        if (pid == 0) {
            if (execvp(args[0], args) == -1) {
                perror("execvp failed");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        } else if (pid > 0) {
            int status;
            if (!background) {
                waitpid(pid, &status, 0);
            }
        } else {
            perror("fork failed");
        }
    }
}


int main() {
    char command[MAX_COMMAND_LEN];

    printf("Welcome Shell (exit to quit) \n");

    signal(SIGINT, handle_signal);

 while (1) {
        if (!sigint_flag) {
            printf("command> ");
            fflush(stdout);
        }
        
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

        if (sigint_flag) {
            sigint_flag = 0;
        }
    }

    return 0;
}
