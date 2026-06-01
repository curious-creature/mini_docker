#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <sched.h> // require for clone() function
#include <stdlib.h> // required for system() and exit() functions
#include <sys/wait.h> // required for wait_pid() function
#include <unistd.h> // required for getpid() and sleep() functions

#define STACK_SIZE (1024 * 1024) // Stack size for cloned child
static char child_stack[STACK_SIZE]; // Stack for child process

int child_main(void *arg) {
    printf("[Container] PID : %d\n", getpid());
    
    if(sethostname("hasse-runtime", 13) != 0){
        printf("Failed to set hostname\n");
        return -1;
    }

    system("/bin/bash");
    return 0;
}


int main(){
    printf("[Host] PID : %d\n", getpid());
    //pid_t clone(int (*fn)(void *), void *stack, int flags, void *arg);
    pid_t child_pid = clone(child_main, child_stack + STACK_SIZE, CLONE_NEWUTS | SIGCHLD, NULL);
    if(child_pid == -1){
        printf("failed to clone. Are you running as root");
        return -1;
    }
    waitpid(child_pid, NULL, 0);
    printf("[Host] Child process has terminated\n");
    return 0;
}