#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <wait.h>

#define PROD_CNT 5
#define CONS_CNT 3

#define BUF_SIZE 1024
#define MAX_SLEEP 5

#define BIN_SEM 0
#define BUFF_FULL 1
#define BUFF_EMPTY 2

#define ERROR_BUF_SIZE 64

struct sembuf consumer_start[] = {{BUFF_FULL, -1, 0},
                             {BIN_SEM, -1, 0}};

struct sembuf consumer_stop[] = {{BUFF_EMPTY, 1, 0},
                                {BIN_SEM, 1, 0}};

struct sembuf producer_start[] = {{BUFF_EMPTY, -1, 0},
                             {BIN_SEM, -1, 0}};

struct sembuf producer_stop[] = {{BUFF_FULL, 1, 0},
                                {BIN_SEM, 1, 0}};

char flag = 1;

void sig_handler(int sig_numb)
{
    printf("Received signal %d pid=%d errno=%d\n", sig_numb, getpid(), errno);
    flag = 0;
}

void consumer(char **p_cons, int semid)
{
    srand(getpid());
    while (flag)
    {
        sleep(rand() % MAX_SLEEP + 1);
        if (semop(semid, consumer_start, 2) == -1)
        {
            char buf[ERROR_BUF_SIZE];
            sprintf(buf, "ERROR: semop consumer pid=%d errno=%d\n", getpid(), errno);
            perror(buf);
            exit(1);
        }
        
        printf("consumer %d get: %c\n", getpid(), **p_cons);
        (*p_cons)++;

        if (semop(semid, consumer_stop, 2) == -1)
        {
            char buf[ERROR_BUF_SIZE];
            sprintf(buf, "ERROR: semop consumer pid=%d errno=%d\n", getpid(), errno);
            perror(buf);
            exit(1);
        }
    }
}


void producer(char **p_prod, char *p_char, int semid)
{
    srand(getpid());
    while (flag)
    {
        sleep(rand() % MAX_SLEEP + 1);
        if (semop(semid, producer_start, 2) == -1)
        {
            char buf[ERROR_BUF_SIZE];
            sprintf(buf, "ERROR: semop producer pid=%d errno=%d\n", getpid(), errno);
            perror(buf);
            exit(1);
        }
      
        **p_prod = *p_char;
        printf("producer %d put: %c\n", getpid(), **p_prod);
        (*p_prod)++;
        *p_char = ((*p_char) >= 'z') ? 'a' : (*p_char) + 1;
        
        if (semop(semid, producer_stop, 2) == -1)
        {
            char buf[ERROR_BUF_SIZE];
            sprintf(buf, "ERROR: semop producer pid=%d errno=%d\n", getpid(), errno);
            perror(buf);
            exit(1);
        } 
    }
}


int main(void)
{
    key_t key = ftok("/dev/null", 1);
    if (key == -1)
    {
        perror("ERROR: ftok\n");
        exit(1);
    }

    int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int shmid = shmget(key, BUF_SIZE + sizeof(char *) * 2 + sizeof(char), IPC_CREAT | perms);

    if (shmid == -1)
    {
        perror("ERROR: shmget\n");
        exit(1);
    }

    char **p_cons, **p_prod, *buf, *p_char;
    buf = shmat(shmid, NULL, 0);
    if (buf == (void*)-1)
    {
        perror("ERROR: shmat\n");
        exit(1);
    }
    
    p_cons = buf;
    p_prod = buf + sizeof(char*);
    p_char = p_prod + sizeof(char *);

    *p_cons = p_char + sizeof(char);
    *p_prod = *p_cons;
    *p_char = 'a';

    int semid = semget(key, 3, IPC_CREAT | perms);
    if (semid == -1)
    {
        perror("ERROR: semget\n");
        exit(1);
    }
    if (semctl(semid, BIN_SEM, SETVAL, 1) == -1)
    {
        perror("ERROR: semctl BIN_SEM, SETVAL\n");
        exit(1);
    }
    if (semctl(semid, BUFF_FULL, SETVAL, 0) == -1)
    {
        perror("ERROR: semctl BUF_FULL, SETVAL\n");
        exit(1);
    }
    if (semctl(semid, BUFF_EMPTY, SETVAL, BUF_SIZE) == -1)
    {
        perror("ERROR: semctl BUF_EMPTY, SETVAL\n");
        exit(1);
    }

    if (signal(SIGINT, sig_handler) == SIG_ERR)
    {
        perror("ERROR: signal\n");
        exit(1);
    }

    for (size_t i = 0; i < CONS_CNT; i++)
    {
        int childpid;
        if ((childpid = fork()) == -1)
        {
            perror("ERROR: consumer fork\n");
            exit(1);
        } 
        else if (childpid == 0)
        {
            consumer(p_cons, semid);
            return 0;
        }
    }
    for (size_t i = 0; i < PROD_CNT; i++)
    {
        int childpid;
        if ((childpid = fork()) == -1)
        {
            perror("ERROR: producer fork\n");
            exit(1);
        } 
        else if (childpid == 0)
        {
            producer(p_prod, p_char, semid);
            return 0;
        }
    }
    for (size_t i = 0; i < PROD_CNT + CONS_CNT; i++)
    {
        int status;
        pid_t pid;
        if ((pid = wait(&status)) == -1)
        {
            perror("ERROR: wait\n");
            exit(1);
        }
        if (WIFEXITED(status)) {
            printf("exited pid=%d, status=%d, errno=%d\n", pid, WEXITSTATUS(status), errno);
        } else if (WIFSIGNALED(status)) {
            printf("killed pid=%d by signal %d, errno=%d\n", pid, WTERMSIG(status), errno);
        } else if (WIFSTOPPED(status)) {
            printf("stopped pid=%d by signal %d, errno=%d\n", pid, WSTOPSIG(status), errno);
        } else if (WIFCONTINUED(status)) {
            printf("continued pid=%d, errno=%d\n", pid, errno);
        }
    }
    if (shmdt(buf) == -1)
    {
        perror("ERROR: shmdt\n");
        exit(1);
    }
    if (semctl(semid, 0, IPC_RMID, 0) == -1)
    {
        perror("ERROR: semctl IPC_RMID\n");
        exit(1);
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
    {
        perror("ERROR: shmctl IPC_RMID\n");
        exit(1);
    }
    return 0;
}