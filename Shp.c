#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>

#define BUFSIZE 256

void parseInput(char *input, char **args, int *redirectInput, int *redirectOutput, int *runInBackground);
void cdCommand(char **args);
void exitCommand();
void pwdCommand();
void catCommand(char **args);
void lsCommand();
void mkdirCommand(char **args);
void rmdirCommand(char **args);
void lnCommand(char **args);
void cpCommand(char **args);
void rmCommand(char **args);
void mvCommand(char **args);
void handleSignal(int signal);

int isInternalCommand(char *command) {
    return (
        strcmp(command, "cd") == 0 ||
        strcmp(command, "exit") == 0 ||
        strcmp(command, "pwd") == 0 ||
        strcmp(command, "cat") == 0 ||
        strcmp(command, "ls") == 0 ||
        strcmp(command, "mkdir") == 0 ||
        strcmp(command, "rmdir") == 0 ||
        strcmp(command, "ln") == 0 ||
        strcmp(command, "cp") == 0 ||
        strcmp(command, "rm") == 0 ||
        strcmp(command, "mv") == 0
    );
}

int main() {
    char input[BUFSIZE];
    char *args[BUFSIZE / 2 + 1];
    int redirectInput = 0;
    int redirectOutput = 0;
    int runInBackground = 0;
    
    printf("Welcome SHP(exit is quit) \n");

    signal(SIGINT, handleSignal);  // Ctrl-C
    signal(SIGQUIT, handleSignal); // Ctrl-Z

    while (1) {
    printf("command> ");
    fgets(input, sizeof(input), stdin);

    input[strcspn(input, "\n")] = 0;
    
    if (strlen(input) > 0 && input[strlen(input) - 1] == '&') {
            runInBackground = 1;
            input[strlen(input) - 1] = '\0'; // Remove the '&'
        } else {
            runInBackground = 0;
        }

    parseInput(input, args, &redirectInput, &redirectOutput, &runInBackground);

   	 if (isInternalCommand(args[0])) {
   	 if (strcmp(args[0], "cd") == 0) {
   	     cdCommand(args);
   	 } else if (strcmp(args[0], "exit") == 0) {
   	     exitCommand();
   	 } else if (strcmp(args[0], "pwd") == 0) {
   	     pwdCommand();
   	 } else if (strcmp(args[0], "cat") == 0) {
   	     catCommand(args);
   	 } else if (strcmp(args[0], "ls") == 0) {
   	     lsCommand();
   	 } else if (strcmp(args[0], "mkdir") == 0) {
   	     mkdirCommand(args);
   	 } else if (strcmp(args[0], "rmdir") == 0) {
   	     rmdirCommand(args);
   	 } else if (strcmp(args[0], "ln") == 0) {
   	     lnCommand(args);
   	 } else if (strcmp(args[0], "cp") == 0) {
   	     cpCommand(args);
   	 } else if (strcmp(args[0], "rm") == 0) {
   	     rmCommand(args);
   	 } else if (strcmp(args[0], "mv") == 0) {
   	     mvCommand(args);
   	 }
	} else {
    	pid_t pid = fork();

    if (pid == 0) {
        if (redirectInput) {
            int fd = open(args[1], O_RDONLY);
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        if (redirectOutput) {
            int fd = open(args[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        if (runInBackground) {
            close(STDIN_FILENO);
            close(STDOUT_FILENO);
   	 }    
   	  

        execvp(args[0], args);
        perror("shell");
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("shell");
    } else {
        if (!runInBackground) {
            wait(NULL);
        }
    }
}
    }
    return 0;
}

void parseInput(char *input, char **args, int *redirectInput, int *redirectOutput, int *runInBackground) {
    int i = 0;
    char *token = strtok(input, " ");

    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
   	 *redirectInput = 1;
  	  token = strtok(NULL, " ");
  	  args[i++] = NULL;  
        } else if (strcmp(token, ">") == 0) {
   	 *redirectOutput = 1;
  	  token = strtok(NULL, " ");
  	  args[i++] = NULL;
        } else {
  	  args[i++] = token;
  	  token = strtok(NULL, " ");
	}
  }

    if (args[i - 1] != NULL && strcmp(args[i - 1], "&") == 0) {
        *runInBackground = 1;
        args[i - 1] = NULL;
    } else {
        *runInBackground = 0;
    }

    args[i] = NULL;
}

void cdCommand(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "shell: cd: missing argument\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("shell");
        }
    }
}

void exitCommand() {
    printf("SHP is Died...\n");
    exit(0);
}

void pwdCommand() {
    char cwd[BUFSIZE];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("shell");
    }
}

void handleSignal(int signal) {
    printf("\n Press Ctl+Z to quit program\n");
    
    printf("shell> ");
    fflush(stdout);
}

void catCommand(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "shell: cat: missing argument\n");
    } else {
        int fd = open(args[1], O_RDONLY);

        if (fd == -1) {
            perror("shell");
        } else {
            char buffer[BUFSIZE];
            ssize_t bytesRead;

            while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
                write(STDOUT_FILENO, buffer, bytesRead);
            }

            close(fd);
        }
    }
}

void lsCommand() {
    struct dirent *entry;
    DIR *dir = opendir(".");

    if (dir == NULL) {
        perror("shell");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }

    closedir(dir);
}

void mkdirCommand(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "shell: mkdir: missing argument\n");
    } else {
        if (mkdir(args[1], 0777) == -1) {
            perror("shell");
        }
    }
}

void rmdirCommand(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "shell: rmdir: missing argument\n");
    } else {
        if (rmdir(args[1]) == -1) {
            perror("shell");
        }
    }
}

void lnCommand(char **args) {
    if (args[1] == NULL || args[2] == NULL) {
        fprintf(stderr, "shell: ln: missing argument\n");
    } else {
        if (link(args[1], args[2]) == -1) {
            perror("shell");
        }
    }
}

void cpCommand(char **args) {
    if (args[1] == NULL || args[2] == NULL) {
        fprintf(stderr, "shell: cp: missing argument\n");
    } else {
        FILE *source = fopen(args[1], "rb");
        FILE *destination = fopen(args[2], "wb");

        if (!source || !destination) {
            perror("shell");
        } else {
            char buffer[BUFSIZE];
            size_t bytesRead;

            while ((bytesRead = fread(buffer, 1, sizeof(buffer), source)) > 0) {
                fwrite(buffer, 1, bytesRead, destination);
            }

            fclose(source);
            fclose(destination);
        }
    }
}

void rmCommand(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "shell: rm: missing argument\n");
    } else {
        if (remove(args[1]) == -1) {
            perror("shell");
        }
    }
}

void mvCommand(char **args) {
    if (args[1] == NULL || args[2] == NULL) {
        fprintf(stderr, "shell: mv: missing argument\n");
    } else {
        if (rename(args[1], args[2]) == -1) {
            perror("shell");
        }
    }
}
