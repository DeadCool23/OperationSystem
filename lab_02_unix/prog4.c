/* PIPE */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/wait.h>
#include <sys/types.h>

#define PROC_CNT 2

int main(void)
{
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
    if (pipe(fd) == -1)
    {
        perror("Can't pipe\n");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < PROC_CNT; ++i)
    {
        if ((childpid[i] = fork()) == -1)
        {
            perror("Can't fork\n");
            exit(EXIT_FAILURE);
        }
        else if (childpid[i] == 0)
        {
            printf("Child (PID: %d) sent message: %s\n", getpid(), msgs[i]);
            close(fd[0]);
            write(fd[1], msgs[i], str_sizes[i]);
            exit(EXIT_SUCCESS);
        }
    }

    for (size_t i = 0; i < PROC_CNT; ++i)
    {
        int status;
        waitpid(childpid[i], &status, 0);
        if (WIFEXITED(status))
            printf("Child (PID: %d) exited with code %d\n", childpid[i], WEXITSTATUS(status));
        else if (WIFSIGNALED(status))
            printf("Child (PID: %d) received signal %d\n", childpid[i], WTERMSIG(status));
        else if (WIFSTOPPED(status))
            printf("Child (PID: %d) received signal %d\n", childpid[i], WSTOPSIG(status));
    }

    char buf[128];
    for (size_t i = 0; i < PROC_CNT; ++i)
    {
        close(fd[1]);
        read(fd[0], buf, str_sizes[i]);
        buf[str_sizes[i]] = '\0';
        printf("Parent (PID: %d) received message: %s\n", getpid(), buf);
    }

    buf[0] = '\0';
    close(fd[1]);
    int read_bytes_cnt = read(fd[0], buf, sizeof(buf) - 1);
    printf("Parent (PID: %d) recieved messages: %s\n", getpid(), buf);
    exit(EXIT_SUCCESS);
}
