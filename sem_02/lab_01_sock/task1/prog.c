#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#define MAX_MSG_LEN 100

int main(void)
{
    pid_t pid;
    int fdsock[2];

    char buffer[MAX_MSG_LEN];
    const char msgs[2][MAX_MSG_LEN] = {"xxxxxxxx", "yyyyyyyyyyyyyyyyy"};


    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fdsock) == -1)
    {
        perror("socketpair");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }    

    if (pid == 0)
    {
        printf("Child write: %s\n", msgs[1]);
        if (write(fdsock[1], msgs[1], MAX_MSG_LEN) == -1)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
        if (read(fdsock[1], buffer, MAX_MSG_LEN) == -1)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        printf("Child received: %s\n", buffer);
    }
    else
    {
        if (read(fdsock[0], buffer, MAX_MSG_LEN) == -1)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        printf("Parent received: %s\n", buffer);
        printf("Parent write: %s\n", msgs[0]);
        if (write(fdsock[0], msgs[0], MAX_MSG_LEN) == -1)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
    }


    return 0;
}