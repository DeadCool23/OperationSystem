#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

#define PROC_CNT 2
#define SLEEP_TIME 2

static int flag = 0;

void signal_handler(int sig_numb)
{
    flag = 1;
    printf("\nSignal %d received. Sending messages is allowed.\n", sig_numb);
}

int main(void)
{
    pid_t child_pid[PROC_CNT];
    char *messages[PROC_CNT] = {"xxxxx", \
                         "yyyyyyyyyyyyyyyyyyyyyyyyyyy"};
    int fd[2];

    if (pipe(fd) == -1)
    {
        perror("Can't pipe\n");
        exit(EXIT_FAILURE);
    }

    if (signal(SIGINT, signal_handler) == SIG_ERR)
    {
        perror("Can't change handler\n");
        exit(EXIT_FAILURE);
    }
    printf("Press Ctrl+C to allow sending messages\n");
    sleep(SLEEP_TIME);

    for (size_t i = 0; i < PROC_CNT; i++)
    {
        if ((child_pid[i] = fork()) == -1)
        {
            perror("Can't fork\n");
            exit(EXIT_FAILURE);
        }
        else if (child_pid[i] == 0)
        {
            printf("Child: pid = %d, ppid = %d, gid = %d\n", \
                getpid(), getppid(), getpgrp());

            if (flag)
            {
                printf("Child pid: %d sent message: %s\n", \
                    getpid(), messages[i]);
                close(fd[0]);
                write(fd[1], messages[i], strlen(messages[i]));
            }
            else
                printf("No signal. Child pid: %d didn't send message\n", getpid());

            exit(EXIT_SUCCESS);
        }
    }

    for (size_t i = 0; i < PROC_CNT; i++)
    {
        int status;
        waitpid(child_pid[i], &status, 0);

        if (WIFEXITED(status))
            printf("Child pid: %d exited with code %d\n", \
                child_pid[i], WEXITSTATUS(status));
        else if (WIFSIGNALED(status))
            printf("Child pid: %d received signal %d\n", \
                child_pid[i], WTERMSIG(status));
        else if (WIFSTOPPED(status))
            printf("Child pid: %d received signal %d\n", \
                child_pid[i], WSTOPSIG(status));
    }

    if (flag) 
    {
        for (size_t i = 0; i < PROC_CNT; i++)
        {
            char buf[64];
            close(fd[1]);
            read(fd[0], buf, strlen(messages[i]));
            printf("Messages from children pid: %d: %s\n", child_pid[i], buf);
        }
    }
    char buf[64];
    close(fd[1]);
    int read_bytes_cnt = read(fd[0], buf, sizeof(buf) - 1);
    printf("Parent pid: %d recieved messages: %s\n", getpid(), buf);
    exit(EXIT_SUCCESS);
}