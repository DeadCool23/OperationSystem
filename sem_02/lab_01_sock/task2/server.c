#include "sys/socket.h"
#include "sys/types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#define TIMEOUT 30
#define BUF_SIZE 256
#define SOCK_NAME "socket"

int fd;

void sig_handler(int signal)
{
    printf("Recived signal %d\n", signal);
    close(fd);
    unlink(SOCK_NAME);
    exit(0);
}

int main()
{
    char buf[BUF_SIZE];
    fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd == -1)
    {
        perror("socket");
        exit(1);
    }
    struct sockaddr sockaddr = {.sa_family=AF_UNIX};
    strcpy(sockaddr.sa_data, SOCK_NAME);

    if (bind(fd, &sockaddr, sizeof(sockaddr)) == -1)
    {
        perror("bind");
        exit(1);
    }

    alarm(TIMEOUT);
    if (signal(SIGALRM, sig_handler) == (void *)-1)
    {
        perror("signal");
        exit(1);
    }

    while(1)
    {
        int bytes_read = recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL);

        if (bytes_read == -1) 
        {
            perror("Can't read()");
            exit(1);
        }    
        else
        {
            buf[bytes_read] = '\0';
            printf("Read message: %s\n", buf);
        }
    }
}