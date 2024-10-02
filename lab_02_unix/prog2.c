/* WAIT */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/wait.h>
#include <sys/types.h>

#define PROC_CNT 2

int main(void) {
    pid_t childpid[PROC_CNT];
    int is_inf[PROC_CNT] = {0, 1};

    for (size_t i = 0; i < PROC_CNT; i++) {
        if ((childpid[i] = fork()) == -1) {
            perror("Can't fork\n");
            exit(EXIT_FAILURE);
        } else if (childpid[i] == 0) {
            printf("Child[%d]: pid=%d, ppid=%d, gid=%d\n", i, getpid(), getppid(), getpgrp());
            if (is_inf[i]) {
                while (1);
            }
            exit(EXIT_SUCCESS);
        } else {
         	printf("Parent: pid=%d, ppid=%d, childpid=%d, gid=%d\n", getpid(), getppid(), childpid[i],  getpgrp());
        }
    }

    for (size_t i = 0; i < PROC_CNT; i++) {
        int status;
        int w = waitpid(childpid[i], &status, 0);
        if (w == -1) {
            perror("Can't waitpid\n");
            exit(EXIT_FAILURE);
        }
        
        if (WIFEXITED(status)) {
            printf("Child (pid: %d) exited with code %d\n", childpid[i], WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Child (pid: %d) received signal %d\n", childpid[i], WTERMSIG(status));
        } else if (WIFSTOPPED(status)) {
            printf("Child (pid: %d) received signal %d\n", childpid[i], WSTOPSIG(status));
        }
    }

    exit(EXIT_SUCCESS);
}
