#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>
#include <time.h>

#define SERVERNAME "server.sock"

enum operation
{
    ADD,
    SUB,
    MUL,
    DIV
};

struct calcop
{
    double num1;
    double num2;
    enum operation op;
};

int clientsock;
char clientsockname[13];


void handle_sigint(int sig)
{
    close(clientsock);
    unlink(clientsockname);
    exit(0);
}

void init_random_op(struct calcop *op)
{
    op->num1 = rand() % 100 - 1;
    op->num2 = rand() % 100 + 1;
    op->op = rand() % 4;
}

void print_op(const struct calcop *op)
{
    switch (op->op)
    {
    case ADD:
        printf("%.2f + %.2f = ", op->num1, op->num2);
        break;
    case SUB:
        printf("%.2f - %.2f = ", op->num1, op->num2);
        break;
    case MUL:
        printf("%.2f * %.2f = ", op->num1, op->num2);
        break;
    case DIV:
        printf("%.2f / %.2f = ", op->num1, op->num2);
        break;
    }
}

int main()
{
    double result;
    struct calcop op;
    struct sockaddr server_addr, client_addr;

    srand(time(NULL));

    clientsock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (clientsock == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sa_family = AF_UNIX;
    snprintf(clientsockname, sizeof(clientsockname), "%d.sock", getpid());
    strncpy(client_addr.sa_data, clientsockname, sizeof(client_addr.sa_data) - 1);

    if (bind(clientsock, (struct sockaddr *)&client_addr, sizeof(client_addr)) == -1)
    {
        perror("bind");
        close(clientsock);
        exit(EXIT_FAILURE);
    }

    printf("Bind socket: %s\n", clientsockname);
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sa_family = AF_UNIX;
    strncpy(server_addr.sa_data, SERVERNAME, sizeof(server_addr.sa_data) - 1);

    signal(SIGINT, handle_sigint);

    while (1)
    {
        init_random_op(&op);
        if (sendto(clientsock, &op, sizeof(op), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        {
            printf("Server is closed\n");
            close(clientsock);
            unlink(clientsockname);
            exit(EXIT_FAILURE);
        }
        if (recvfrom(clientsock, &result, sizeof(result), 0, NULL, NULL) == -1)
        {
            perror("recvfrom");
            close(clientsock);
            unlink(clientsockname);
            exit(EXIT_FAILURE);
        }
        print_op(&op);
        printf("%.2f\n", result);
        sleep(rand() % 2 + 1);
    }
    
    close(clientsock);
    unlink(clientsockname);
    return 0;
}
    