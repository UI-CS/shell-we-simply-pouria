#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>

/* structure for passing data to threads */
typedef struct {
    int row;
    int col;
    int thread_idx; // Index in the valid array (0-10)
} parameters;

/* * 11 Threads Total:
 * Index 0: Checks ALL rows
 * Index 1: Checks ALL columns
 * Indices 2-10: Check each 3x3 subgrid
 */

int valid[11] = {0};
int sudoku[9][9];

// Helper to check if an array of 9 numbers contains 1-9 exactly once
int check_set(int arr[]) {
    int counts[10] = {0};
    for (int i = 0; i < 9; i++) {
        if (arr[i] < 1 || arr[i] > 9 || counts[arr[i]] > 0) return 0;
        counts[arr[i]]++;
    }
    return 1;
}

// thread 1: Validate ALL rows
void *validate_rows(void *param) {
    parameters *p = (parameters *)param;
    for (int i = 0; i < 9; i++) {
        int row_data[9];
        for (int j = 0; j < 9; j++) {
            row_data[j] = sudoku[i][j];
        }
        if (!check_set(row_data)) {
            pthread_exit(NULL); // valid[0] remains 0
        }
    }
    valid[p->thread_idx] = 1;
    pthread_exit(NULL);
}

// thread 2: Validate ALL columns
void *validate_cols(void *param) {
    parameters *p = (parameters *)param;
    for (int j = 0; j < 9; j++) {
        int col_data[9];
        for (int i = 0; i < 9; i++) {
            col_data[i] = sudoku[i][j];
        }
        if (!check_set(col_data)) {
            pthread_exit(NULL);
        }
    }
    valid[p->thread_idx] = 1;
    pthread_exit(NULL);
}

// threads 3-11: Validate one 3x3 subgrid
void *validate_subgrid(void *param) {
    parameters *p = (parameters *)param;
    int grid_data[9];
    int count = 0;

    for (int i = p->row; i < p->row + 3; i++) {
        for (int j = p->col; j < p->col + 3; j++) {
            grid_data[count++] = sudoku[i][j];
        }
    }

    if (check_set(grid_data)) {
        valid[p->thread_idx] = 1;
    }
    pthread_exit(NULL);
}

void read_board(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Could not open file %s. Using default valid board.\n", filename);
        //default valid board for test :)
        int default_board[9][9] = {
                {6, 2, 4, 5, 3, 9, 1, 8, 7},
                {5, 1, 9, 7, 2, 8, 6, 3, 4},
                {8, 3, 7, 6, 1, 4, 2, 9, 5},
                {1, 4, 3, 8, 6, 5, 7, 2, 9},
                {9, 5, 8, 2, 4, 7, 3, 6, 1},
                {7, 6, 2, 3, 9, 1, 4, 5, 8},
                {3, 7, 1, 9, 5, 6, 8, 4, 2},
                {4, 9, 6, 1, 8, 2, 5, 7, 3},
                {2, 8, 5, 4, 7, 3, 9, 1, 6}
        };
        for(int i=0; i<9; i++)
            for(int j=0; j<9; j++)
                sudoku[i][j] = default_board[i][j];
        return;
    }

    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (fscanf(file, "%d", &sudoku[i][j]) != 1) {
                fprintf(stderr, "Error reading board\n");
                exit(1);
            }
        }
    }
    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        // Fallback to default if no file provided
        read_board("puzzle.txt");
    } else {
        read_board(argv[1]);
    }

    pthread_t threads[11];
    parameters *data[11];

    // thread for Rows
    data[0] = (parameters *)malloc(sizeof(parameters));
    data[0]->thread_idx = 0;
    pthread_create(&threads[0], NULL, validate_rows, (void *)data[0]);

    // thread for Columns
    data[1] = (parameters *)malloc(sizeof(parameters));
    data[1]->thread_idx = 1;
    pthread_create(&threads[1], NULL, validate_cols, (void *)data[1]);

    // threads for 3x3 Subgrids
    int thread_count = 2;
    for (int i = 0; i < 9; i += 3) {
        for (int j = 0; j < 9; j += 3) {
            data[thread_count] = (parameters *)malloc(sizeof(parameters));
            data[thread_count]->row = i;
            data[thread_count]->col = j;
            data[thread_count]->thread_idx = thread_count;
            pthread_create(&threads[thread_count], NULL, validate_subgrid, (void *)data[thread_count]);
            thread_count++;
        }
    }

    // wait for all threads
    for (int i = 0; i < 11; i++) {
        pthread_join(threads[i], NULL);
        free(data[i]);
    }

    // Check results
    int is_valid = 1;
    for (int i = 0; i < 11; i++) {
        if (valid[i] == 0) {
            is_valid = 0;
            // validation
            if (i == 0) printf("Row validation failed.\n");
            else if (i == 1) printf("Column validation failed.\n");
            else printf("Subgrid validation failed for thread %d.\n", i);
        }
    }

    if (is_valid)
        printf("Sudoku Solution is VALID\n");
    else
        printf("Sudoku Solution is INVALID\n");

    return 0;
}