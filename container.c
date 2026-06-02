#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <sched.h> // require for clone() function
#include <stdlib.h> // required for system() and exit() functions
#include <sys/wait.h> // required for wait_pid() function
#include <unistd.h> // required for getpid() and sleep() functions
#include <sys/mount.h>

#define STACK_SIZE (1024 * 1024) // Stack size for cloned child
static char child_stack[STACK_SIZE]; // Stack for child process

int child_main(void *arg) {
    printf("[Container] PID : %d\n", getpid());
    
    sethostname("hasse-runtime", 13);
    if(mount("none", "/", NULL, MS_REC | MS_PRIVATE, NULL) != 0){
	perror("Failed to set mount propagation to private");
	return -1;
    }

    if(mount("proc","/proc","proc",0,NULL) != 0){
	perror("Failed to mount isolated /proc");
	return -1;
    }

    system("/bin/bash");
    return 0;
}


int main(){
    printf("[Host] PID : %d\n", getpid());
    //pid_t clone(int (*fn)(void *), void *stack, int flags, void *arg);
    pid_t child_pid = clone(child_main, child_stack + STACK_SIZE, CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD, NULL);
    if(child_pid == -1){
        printf("failed to clone. Are you running as root");
        return -1;
    }
    waitpid(child_pid, NULL, 0);
    printf("[Host] Child process has terminated\n");
    return 0;
}
