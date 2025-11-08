#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    printf("Parent PID: %d\n", getpid());
    
    pid_t child_pid = fork();
    
    if (child_pid == 0) {
        printf("Child PID: %d\n", getpid());
        exit(0);
    } else {
        printf("Created child with PID: %d\n", child_pid);
        
        sleep(10);
        
        int status;
        waitpid(child_pid, &status, 0);
        printf("Collected child's exit status.\n");
        
        sleep(5);
    }
    
    return 0;
}