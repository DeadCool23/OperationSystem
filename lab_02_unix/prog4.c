/* PIPE */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/wait.h>
#include <sys/types.h>

#define PROC_CNT 2

int main(void) {
    pid_t childpid[PROC_CNT];

    char fst[] = "xxxxx";
    char scd[] = "yyyyyyyyyyyyyyyyyyyyyyyyy";
    char *msgs[PROC_CNT] = {
        fst,
        scd
    };
    size_t str_sizes[PROC_CNT] = {
        sizeof(fst) - 1,
        sizeof(scd) - 1
    };

    int fd[2];
    if (pipe(fd) == -1) {
        perror("Can't pipe\n");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < PROC_CNT; i++) {
        if ((childpid[i] = fork()) == -1) {
            perror("Can't fork\n");
            exit(EXIT_FAILURE);
        } else if (childpid[i] == 0) {
            printf("Child[%d]: pid=%d, ppid=%d, gid=%d\n", i, getpid(), getppid(), getpgrp());
            
            printf("Child (pid: %d) sent message: %s\n", getpid(), msgs[i]);

            close(fd[0]);
            write(fd[1], msgs[i], str_sizes[i]);

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

    for (size_t i = 0; i < PROC_CNT; i++) {
        char buf[128];

        close(fd[1]);
        read(fd[0], buf, str_sizes[i]);

        buf[str_sizes[i]] = '\0';

        printf("Parent received message: %s\n", buf);
    }

    exit(EXIT_SUCCESS);
}
