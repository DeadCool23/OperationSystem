#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sys/types.h"
#include "sys/socket.h"

#define BUF_SIZE 256
#define SOCK_NAME "socket"

int main(void)
{
    int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr sockaddr = {.sa_family = AF_UNIX};
    strcpy(sockaddr.sa_data, SOCK_NAME);
    
    char buf[BUF_SIZE];
    sprintf(buf, "%d\n", getpid());
    printf("Client %d send\n", getpid());

    if (sendto(sockfd, buf, BUF_SIZE, 0, &sockaddr, sizeof(sockaddr))  == -1)
    {
        printf("sendto: No server\n");
        exit(EXIT_FAILURE);
    }
    close(sockfd);

    return 0;
}