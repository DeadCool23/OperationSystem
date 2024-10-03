/* SIROTA CHILD */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>

#define PROC_CNT 3
#define SLEEP_TIME 3

int main()
{
	pid_t childpid[PROC_CNT];
	for (int i = 0; i < PROC_CNT; ++i)
	{
		if ((childpid[i] = fork()) == -1)
		{
            printf("Can't fork");
            exit(EXIT_FAILURE);
        }
		else if (childpid[i] == 0)
		{
			printf("Child: pid=%d, ppid=%d, gid=%d\n", getpid(), getppid(), getpgrp());
			sleep(SLEEP_TIME);
			printf("After sleep\nChild: pid=%d, ppid=%d, gid=%d\n", getpid(), getppid(), getpgrp());
			exit(EXIT_SUCCESS);
		} 
		else
			printf("Parent: pid=%d, ppid=%d, childpid=%d, gid=%d\n", getpid(), getppid(), childpid[i],  getpgrp());
    }
    exit(EXIT_SUCCESS);
}

