#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <sched.h> // require for clone() function
#include <stdlib.h> // required for system() and exit() functions
#include <sys/wait.h> // required for wait_pid() function
#include <unistd.h> // required for getpid() and sleep() functions
#include <sys/mount.h>
#include <fcntl.h> // required for open and  O_WRONLY
#include <string.h>

#define STACK_SIZE (1024 * 1024) // Stack size for cloned child
static char child_stack[STACK_SIZE]; // Stack for child process

int child_main(void *arg) {

    int *checkpoint = (int *)arg;
    char c;
    close(checkpoint[1]);
    read(checkpoint[0], &c, 1);
    close(checkpoint[0]);
    
    printf("Tripwire realeased\n");
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
    int checkpoint[2];
    if(pipe(checkpoint) == -1){
	perror("Failed to implement pipe");
	return -1;
    }
    pid_t child_pid = clone(child_main, child_stack + STACK_SIZE, CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWNET | CLONE_NEWUSER | SIGCHLD, checkpoint);
    if(child_pid == -1){
        printf("failed to clone. Are you running as root");
        return -1;
    }
    printf("[Host] Child process has terminated\n");
    printf("[Host]Host PID : %d\n", getpid());
    printf("Child PID: %d\n",child_pid);

    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/setgroups", child_pid);
    int fd = open(path, O_WRONLY);
    if(fd == -1){
	perror("Failed to open setgroups");
	return -1;
    }
    
    if(write(fd, "deny", 4) == -1){
	perror("Failed to write to setgroups");
	close(fd);
 	return -1;
     }
     close(fd);
     close(checkpoint[0]);
     write(checkpoint[1],"1", 1);
     close(checkpoint[1]);
     waitpid(child_pid, NULL, 0);
    return 0;
}
