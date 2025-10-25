#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "find_min_max.h"
#include "utils.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <seed> <array_size>\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();
    
    if (pid == 0) {
        printf("Launching sequential_min_max with seed=%s, array_size=%s\n", 
               argv[1], argv[2]);
        
        execl("./sequential_min_max", "sequential_min_max", argv[1], argv[2], NULL);
        
        perror("execl failed");
        exit(1);
    }
    else {
        wait(NULL);
        printf("sequential_min_max finished\n");
    }
    
    return 0;
}