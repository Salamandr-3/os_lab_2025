#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

long long k = -1;
int pnum = -1;
long long mod = -1;
long long result = 1;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

void* calculate_part(void* arg) {
    int thread_id = *(int*)arg;
    long long start = thread_id * (k / pnum) + 1;
    long long end = (thread_id == pnum - 1) ? k : (thread_id + 1) * (k / pnum);
    
    long long local_result = 1;
    
    for (long long i = start; i <= end; i++) {
        local_result = (local_result * (i % mod)) % mod;
    }
    
    pthread_mutex_lock(&mut);
    result = (result * local_result) % mod;
    pthread_mutex_unlock(&mut);
    
    return NULL;
}

int main(int argc, char* argv[]) {
    static struct option options[] = {
        {"k", required_argument, 0, 0},
        {"pnum", required_argument, 0, 0},
        {"mod", required_argument, 0, 0},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    
    while (1) {
        int c = getopt_long(argc, argv, "", options, &option_index);
        
        if (c == -1) break;
        
        switch (c) {
            case 0:
                switch (option_index) {
                    case 0:
                        k = atoll(optarg);
                        if (k <= 0) {
                            printf("k must be positive\n");
                            return 1;
                        }
                        break;
                    case 1:
                        pnum = atoi(optarg);
                        if (pnum <= 0) {
                            printf("pnum must be positive\n");
                            return 1;
                        }
                        break;
                    case 2:
                        mod = atoll(optarg);
                        if (mod <= 0) {
                            printf("mod must be positive\n");
                            return 1;
                        }
                        break;
                    default:
                        printf("Index %d is out of options\n", option_index);
                }
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

    if (k == -1 || pnum == -1 || mod == -1) {
        printf("Usage: %s --k num --pnum num --mod num\n", argv[0]);
        return 1;
    }

    if (pnum > k) {
        pnum = k;
    }

    pthread_t* threads = malloc(pnum * sizeof(pthread_t));
    int* thread_ids = malloc(pnum * sizeof(int));
    
    if (threads == NULL || thread_ids == NULL) {
        perror("malloc");
        exit(1);
    }

    for (int i = 0; i < pnum; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, calculate_part, 
                          (void*)&thread_ids[i]) != 0) {
            perror("pthread_create");
            exit(1);
        }
    }

    for (int i = 0; i < pnum; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join");
            exit(1);
        }
    }

    printf("%lld! mod %lld = %lld\n", k, mod, result);

    free(threads);
    free(thread_ids);
    pthread_mutex_destroy(&mut);

    return 0;
}