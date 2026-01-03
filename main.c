#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE 80 /* Increased from 20 for practical use */

int main(void) {
    char *args[MAX_LINE/2 + 1];
    char input_buffer[MAX_LINE];
    char history_buffer[MAX_LINE]; // last command
    int has_history = 0;           // toggle so I won;t screw things up
    int should_run = 1;

    while (should_run) {
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
            // overwrite current input with history and priint it
            strcpy(input_buffer, history_buffer);
            printf("%s\n", input_buffer);
        } else {
            // Save this valid command for next time
            strcpy(history_buffer, input_buffer);
            has_history = 1;
        }

        if (strlen(input_buffer) == 0) continue;

        // parser
        int i = 0;
        char *token = strtok(input_buffer, " ");
        while (token != NULL) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }

        args[i] = NULL;

        // Commands

        // exit
        if (strcmp(args[0], "exit") == 0) {
            should_run = 0;
            continue;
        }

        // cd
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

        // pwd
        if (strcmp(args[0], "pwd") == 0) {
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("%s\n", cwd);
            } else {
                perror("pwd failed");
            }
            continue;
        }

        // executor
        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork failed");
        }
        else if (pid == 0) {
            // child
            if (execvp(args[0], args) == -1) {
                printf("Command not found: %s\n", args[0]);
                exit(1);
            }
        }
        else {
            // parent
            wait(NULL);
        }
    }

    return 0;
}