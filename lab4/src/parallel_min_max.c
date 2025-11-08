#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

pid_t *children_pids = NULL;
int pnum_global = 0;

void kill_children(int sig) {
    if (children_pids) {
        for (int i = 0; i < pnum_global; i++) {
            if (children_pids[i] > 0) {
                kill(children_pids[i], SIGKILL);
            }
        }
    }
    exit(1);
}

int main(int argc, char **argv) {
    int seed = -1;
    int array_size = -1;
    int pnum = -1;
    bool with_files = false;
    int timeout = 0;

    while (true) {
        int current_optind = optind ? optind : 1;

        static struct option options[] = {
            {"seed", required_argument, 0, 0},
            {"array_size", required_argument, 0, 0},
            {"pnum", required_argument, 0, 0},
            {"by_files", no_argument, 0, 'f'},
            {"timeout", required_argument, 0, 0},
            {0, 0, 0, 0}
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "f", options, &option_index);

        if (c == -1) break;

        switch (c) {
            case 0:
                switch (option_index) {
                    case 0:
                        seed = atoi(optarg);
                        if (seed <= 0) {
                            printf("seed must be positive\n");
                            return 1;
                        }
                        break;
                    case 1:
                        array_size = atoi(optarg);
                        if (array_size <= 0) {
                            printf("array_size must be positive\n");
                            return 1;
                        }
                        break;
                    case 2:
                        pnum = atoi(optarg);
                        if (pnum <= 0) {
                            printf("pnum must be positive\n");
                            return 1;
                        }
                        break;
                    case 3:
                        with_files = true;
                        break;
                    case 4:
                        timeout = atoi(optarg);
                        if (timeout <= 0) {
                            printf("timeout must be positive\n");
                            return 1;
                        }
                        break;
                    default:
                        printf("Index %d is out of options\n", option_index);
                }
                break;
            case 'f':
                with_files = true;
                break;
            case '?':
                break;
            default:
                printf("getopt returned character code 0%o?\n", c);
        }
    }

    if (optind < argc) {
        printf("Has at least one no option argument\n");
        return 1;
    }

    if (seed == -1 || array_size == -1 || pnum == -1) {
        printf("Usage: %s --seed num --array_size num --pnum num [--by_files] [--timeout sec]\n", argv[0]);
        return 1;
    }

    pnum_global = pnum;
    children_pids = malloc(pnum * sizeof(pid_t));
    int *array = malloc(sizeof(int) * array_size);
    GenerateArray(array, array_size, seed);

    if (timeout > 0) {
        signal(SIGALRM, kill_children);
    }

    int pipes[pnum][2];
    if (!with_files) {
        for (int i = 0; i < pnum; i++) {
            if (pipe(pipes[i]) == -1) {
                printf("Pipe creation failed!\n");
                return 1;
            }
        }
    }

    int active_child_processes = 0;

    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    for (int i = 0; i < pnum; i++) {
        pid_t child_pid = fork();
        if (child_pid >= 0) {
            active_child_processes += 1;
            children_pids[i] = child_pid;
            if (child_pid == 0) {
                int chunk_size = array_size / pnum;
                int start = i * chunk_size;
                int end = (i == pnum - 1) ? array_size - 1 : (i + 1) * chunk_size - 1;
                
                struct MinMax local_min_max = GetMinMax(array, start, end);

                if (with_files) {
                    char filename[30];
                    sprintf(filename, "min_max_%d.txt", i);
                    FILE *file = fopen(filename, "w");
                    if (file != NULL) {
                        fprintf(file, "%d %d", local_min_max.min, local_min_max.max);
                        fclose(file);
                    }
                } else {
                    close(pipes[i][0]);
                    write(pipes[i][1], &local_min_max.min, sizeof(int));
                    write(pipes[i][1], &local_min_max.max, sizeof(int));
                    close(pipes[i][1]);
                }
                free(array);
                return 0;
            }
        } else {
            printf("Fork failed!\n");   
            return 1;
        }
    }

    if (timeout > 0) {
        alarm(timeout);
    }

    while (active_child_processes > 0) {
        wait(NULL);
        active_child_processes -= 1;
    }

    if (timeout > 0) {
        alarm(0);
    }

    struct MinMax min_max;
    min_max.min = INT_MAX;
    min_max.max = INT_MIN;

    for (int i = 0; i < pnum; i++) {
        int min = INT_MAX;
        int max = INT_MIN;

        if (with_files) {
            char filename[30];
            sprintf(filename, "min_max_%d.txt", i);
            FILE *file = fopen(filename, "r");
            if (file != NULL) {
                fscanf(file, "%d %d", &min, &max);
                fclose(file);
                remove(filename);
            }
        } else {
            close(pipes[i][1]);
            read(pipes[i][0], &min, sizeof(int));
            read(pipes[i][0], &max, sizeof(int));
            close(pipes[i][0]);
        }

        if (min < min_max.min) min_max.min = min;
        if (max > min_max.max) min_max.max = max;
    }

    struct timeval finish_time;
    gettimeofday(&finish_time, NULL);

    double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
    elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

    free(array);
    free(children_pids);

    printf("Min: %d\n", min_max.min);
    printf("Max: %d\n", min_max.max);
    printf("Elapsed time: %fms\n", elapsed_time);
    fflush(NULL);
    return 0;
}