#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE 20

int main(void) {
    char input_buffer[MAX_LINE];
    int should_run = 1;

    while (should_run) {
        printf("uinxsh> ");
        fflush(stdout);

        // read
        if (fgets(input_buffer, MAX_LINE, stdin) == NULL) {
            break;
        }

        // remove  newline
        input_buffer[strcspn(input_buffer, "\n")] = 0;

        // skip empty input
        if (strlen(input_buffer) == 0) {
            continue;
        }

        printf("You entered: %s\n", input_buffer);

        // i'm too sleepy right now to work on this part...farda ishalla
        if (strcmp(input_buffer, "exit") == 0) {
            should_run = 0;
        }
    }

    return 0;
}