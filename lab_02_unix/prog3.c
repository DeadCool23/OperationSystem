/* EXEC */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/wait.h>
#include <sys/types.h>

#define PROC_CNT 2

int main(void) {
    pid_t childpid[PROC_CNT];
    char *apps[PROC_CNT] = {"./target/prog1.exe", "./target/prog2.exe"};

    char *argv1[] = {NULL};
    char *argv2[] = {NULL};
    char **argvs[PROC_CNT] = {argv1, argv2};

    for (size_t i = 0; i < PROC_CNT; i++) {
        if ((childpid[i] = fork()) == -1) {
            perror("Can't fork\n");
            exit(EXIT_FAILURE);
        } else if (childpid[i] == 0) {
            printf("Child[%d]: pid=%d, ppid=%d, gid=%d\n", i, getpid(), getppid(), getpgrp());
            if (execvp(apps[i], argvs[i])  == -1) {
                perror("Can't exec\n");
                exit(EXIT_FAILURE);
            }

            exit(EXIT_SUCCESS);
        } else {
         	printf("Parent: pid=%d, ppid=%d, childpid=%d, gid=%d\n", getpid(), getppid(), childpid[i],  getpgrp());
        }
    }

    for (size_t i = 0; i < PROC_CNT; i++) {
        int status;
        waitpid(childpid[i], &status, 0);

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
