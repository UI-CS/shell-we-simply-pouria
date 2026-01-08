#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE 80

int main(void) {
    char *args[MAX_LINE/2 + 1];
    char *args_pipe[MAX_LINE/2 + 1];
    char input_buffer[MAX_LINE];
    char history_buffer[MAX_LINE];
    int has_history = 0;
    int should_run = 1;

    // I hardcoded this because you said so...
    char home_dir[1024];
    if (getcwd(home_dir, sizeof(home_dir)) == NULL) {
        perror("uinxsh init failed");
        return 1;
    }

    while (should_run) {
        // ZOMBIE CLEANUP
        while (waitpid(-1, NULL, WNOHANG) > 0);

        printf("uinxsh> ");
        fflush(stdout);

        if (fgets(input_buffer, MAX_LINE, stdin) == NULL) break;
        input_buffer[strcspn(input_buffer, "\n")] = 0;

        // history
        if (strcmp(input_buffer, "!!") == 0) {
            if (!has_history) {
                printf("No commands in history.\n");
                continue;
            }
            strcpy(input_buffer, history_buffer);
            printf("%s\n", input_buffer);
        } else {
            strcpy(history_buffer, input_buffer);
            has_history = 1;
        }

        if (strlen(input_buffer) == 0) continue;

        // parser
        int i = 0;
        char *token = strtok(input_buffer, " ");
        while (token != NULL) {
            if (i >= MAX_LINE/2) break; // PREVENT OVERFLOW
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;

        if (args[0] == NULL) continue;

        // CHECK FOR (&)
        int background = 0;
        if (i > 0 && strcmp(args[i-1], "&") == 0) {
            background = 1;
            args[i-1] = NULL;
            i--;
        }

        // CHECK FOR (|)
        int pipe_idx = -1;
        for (int j = 0; j < i; j++) {
            if (strcmp(args[j], "|") == 0) {
                pipe_idx = j;
                break;
            }
        }

        if (strcmp(args[0], "exit") == 0) {
            should_run = 0;
            continue;
        }

        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL) {
                fprintf(stderr, "uinxsh: expected argument to \"cd\"\n");
            } else {
                if (chdir(args[1]) != 0) {
                    perror("uinxsh");
                }
            }
            continue;
        }

        if (strcmp(args[0], "pwd") == 0) {
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("%s\n", cwd);
            } else {
                perror("pwd failed");
            }
            continue;
        }


        // help
        if (strcmp(args[0], "help") == 0) {
            printf("uinxsh built-in commands:\n");
            printf("  cd <path> : Change directory\n");
            printf("  pwd       : Print working directory\n");
            printf("  exit      : Exit the shell\n");
            printf("  help      : Show this help message\n");
            continue;
        }

        // echo
        if (strcmp(args[0], "echo") == 0) {
            for (int k = 1; args[k] != NULL; k++) {
                printf("%s ", args[k]);
            }
            printf("\n");
            continue;
        }

        if (pipe_idx != -1) {
            // pipe logic
            args[pipe_idx] = NULL;

            int pipe_arg_count = 0;
            for(int j = pipe_idx + 1; args[j] != NULL; j++) {
                args_pipe[pipe_arg_count++] = args[j];
            }
            args_pipe[pipe_arg_count] = NULL;

            int pipefd[2];
            if (pipe(pipefd) == -1) {
                perror("pipe failed");
                continue;
            }

            pid_t p1 = fork();
            if (p1 < 0) {
                perror("fork failed");
            } else if (p1 == 0) {
                // child 1
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
                if (execvp(args[0], args) == -1) {
                    perror("execvp child 1");
                    exit(1);
                }
            }

            pid_t p2 = fork();
            if (p2 < 0) {
                perror("fork failed");
            } else if (p2 == 0) {
                // child 2
                close(pipefd[1]);
                dup2(pipefd[0], STDIN_FILENO);
                close(pipefd[0]);
                if (execvp(args_pipe[0], args_pipe) == -1) {
                    perror("execvp child 2");
                    exit(1);
                }
            }

            // parent
            close(pipefd[0]);
            close(pipefd[1]);

            if (!background) {
                waitpid(p1, NULL, 0);
                waitpid(p2, NULL, 0);
            }
        }
        else {
            pid_t pid = fork();
            if (pid < 0) {
                perror("Fork failed");
            }
            else if (pid == 0) {

                // I hardcoded this because you all said so
                if (strcmp(args[0], "monte_carlo") == 0 || strcmp(args[0], "sudoku") == 0) {
                    // switch child process to the directory where main.c lives
                    if (chdir(home_dir) != 0) {
                        perror("Failed to switch to project dir");
                        exit(1);
                    }

                    // tell execvp to look in current directory
                    char cmd_path[MAX_LINE + 50];
                    snprintf(cmd_path, sizeof(cmd_path), "./%s", args[0]);

                    if (execvp(cmd_path, args) == -1) {
                        perror("External module execution failed");
                        exit(1);
                    }
                }

                if (execvp(args[0], args) == -1) {
                    printf("Command not found: %s\n", args[0]);
                    exit(1);
                }
            }
            else {
                if (!background) {
                    wait(NULL);
                } else {
                    printf("[Running in background] PID: %d\n", pid);
                }
            }
        }
    }

    return 0;
}