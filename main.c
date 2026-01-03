#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE 20

int main(void) {
    char *args[MAX_LINE/2 + 1];
    char input_buffer[MAX_LINE];
    int should_run = 1;

    while (should_run) {
        printf("uinxsh> ");
        fflush(stdout);

        if (fgets(input_buffer, MAX_LINE, stdin) == NULL) break;
        input_buffer[strcspn(input_buffer, "\n")] = 0;
        if (strlen(input_buffer) == 0) continue;

        // parser
        int i = 0;
        char *token = strtok(input_buffer, " ");
        while (token != NULL) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL; // I don't really get this

        // i'm just gonna procrastinate this for now as Maduro's government has fell
        if (strcmp(args[0], "exit") == 0) {
            should_run = 0;
            continue;
        }

        // executor
        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork failed");
        }
        else if (pid == 0) {
            // Child Process
            if (execvp(args[0], args) == -1) {
                printf("Command not found: %s\n", args[0]);
                exit(1);
            }
        }
        else {
            // Parent Process: wait for the child to finish
            wait(NULL);
        }
    }
    return 0;
}