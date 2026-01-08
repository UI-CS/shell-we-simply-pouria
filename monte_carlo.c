#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <time.h>

/* *
 * ./monte_carlo <n_processes> <n_points>
 */

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <n_processes> <n_points>\n", argv[0]);
        return 1;
    }

    int n_processes = atoi(argv[1]);
    long long total_points = atoll(argv[2]);

    if (n_processes <= 0 || total_points <= 0) {
        fprintf(stderr, "Error: Arguments must be positive integers.\n");
        return 1;
    }



    // determine points per process
    long long points_per_process = total_points / n_processes;

    // create sharedd memory for results
    long long *results = mmap(NULL, n_processes * sizeof(long long),
                              PROT_READ | PROT_WRITE,
                              MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (results == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    // fork processes
    for (int i = 0; i < n_processes; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork failed");
            exit(1);
        } else if (pid == 0) {
            unsigned int seed = time(NULL) ^ (getpid() << 16);

            long long local_hits = 0;
            double x, y;

            for (long long j = 0; j < points_per_process; j++) {
                // random x, y
                x = (double)rand_r(&seed) / RAND_MAX * 2.0 - 1.0;
                y = (double)rand_r(&seed) / RAND_MAX * 2.0 - 1.0;

                // point is inside unit circle?
                if (x * x + y * y <= 1.0) {
                    local_hits++;
                }
            }

            // write result to shared memory
            results[i] = local_hits;


            exit(0);
        }
    }

    // parent process
    for (int i = 0; i < n_processes; i++) {
        wait(NULL);
    }

    // results and stuff
    long long total_hits = 0;
    for (int i = 0; i < n_processes; i++) {
        total_hits += results[i];
    }



    long long actual_total_points = points_per_process * n_processes;
    double pi_estimate = 4.0 * ((double)total_hits / actual_total_points);

    printf("Pi estimation with %d processes and %lld points: %f\n",
           n_processes, actual_total_points, pi_estimate);

    // clean up shared memory
    munmap(results, n_processes * sizeof(long long));

    return 0;
}


