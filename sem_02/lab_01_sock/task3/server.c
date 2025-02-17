#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>

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

int server_sock;

void handle_sigint(int sig)
{
    close(server_sock);
    unlink(SERVERNAME);
    exit(0);
}

void print_recv_op(const struct calcop *op)
{
    switch (op->op)
    {
    case ADD:
        printf("%.2f + %.2f\n", op->num1, op->num2);
        break;
    case SUB:
        printf("%.2f - %.2f\n", op->num1, op->num2);
        break;
    case MUL:
        printf("%.2f * %.2f\n", op->num1, op->num2);
        break;
    case DIV:
        printf("%.2f / %.2f\n", op->num1, op->num2);
        break;
    }
}

int main()
{
    double result;
    ssize_t recv_len;
    struct calcop op;
    struct sockaddr server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    if ((server_sock = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sa_family = AF_UNIX;
    strncpy(server_addr.sa_data, SERVERNAME, sizeof(server_addr.sa_data) - 1);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, handle_sigint);
    printf("Server is running\n");

    while (1)
    {
        recv_len = recvfrom(server_sock, &op, sizeof(op), 0, (struct sockaddr *)&client_addr, &client_len);
        if (recv_len == -1)
        {
            perror("recvfrom");
            continue;
        }
        printf("Received from %s: ", client_addr.sa_data);
        print_recv_op(&op);
        switch (op.op)
        {
            case ADD:
                result = op.num1 + op.num2;
                break;
            case SUB:
                result = op.num1 - op.num2;
                break;
            case MUL:
                result = op.num1 * op.num2;
                break;
            case DIV:
                result = op.num1 / op.num2;
                break;
            default:
                printf("Unknown operation\n");
                continue;
        }
        printf("Sent result to %s: %.2f\n", client_addr.sa_data, result);
        if (sendto(server_sock, &result, sizeof(result), 0, (struct sockaddr *)&client_addr, client_len) == -1)
            perror("sendto");
    }
    
    close(server_sock);
    unlink(SERVERNAME);
    return 0;
}
