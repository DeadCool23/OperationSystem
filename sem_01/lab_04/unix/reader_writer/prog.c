#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define WRITER_CNT 3
#define READER_CNT 5

#define READERS_QUEUE 0
#define ACTIVE_READERS 1
#define ACTIVE_WRITER 2
#define WRITERS_QUEUE 3
#define BIN_WRITER 4

struct sembuf start_read[] =
{
    {READERS_QUEUE, 1, 0},
    {ACTIVE_WRITER, 0, 0}, 
    {WRITERS_QUEUE, 0, 0}, 
    {ACTIVE_READERS, 1, 0},
    {READERS_QUEUE, -1, 0}
};

struct sembuf stop_read[] =
{
    {ACTIVE_READERS, -1, 0}
};

struct sembuf start_write[] =
{
    {WRITERS_QUEUE, 1, 0},
    {ACTIVE_READERS, 0, 0},
    {BIN_WRITER, -1, 0}, 
    {ACTIVE_WRITER, 1, 0},
    {WRITERS_QUEUE, -1, 0}
};

struct sembuf stop_write[] =
{
    {ACTIVE_WRITER, -1, 0},
    {BIN_WRITER, 1, 0}
};

const int PERMS =  S_IRWXU | S_IRWXG | S_IRWXO;

int *shm_buf;
int sem_id;
int shm_id;

int flag = 1;

void sig_handler(int sig_num)
{
    flag = 0;
        printf("Process %d catch signal %d\n", getpid(), sig_num);
}

int reader(int semid, int* shm)
{
    srand(getpid());
    while(flag)
    {
        sleep(rand() % 2);
        if (semop(sem_id, start_read, 5) == -1)
        {
            printf("semop %d errno: %d\n", getpid(), errno);
            perror("semop error");
            exit(1);
        }
        
        char ch = 'a' + (*shm - 1) % 26;
		printf("Reader %d: %c\n", getpid(), ch);

        if (semop(sem_id, stop_read, 1) == -1)
        {
            printf("semop %d errno: %d\n", getpid(), errno);
            perror("semop error");
            exit(1);
        }
    }

    return 0;
}

int writer(int semid, int* shm)
{
    srand(getpid());
    while(flag)
    {
        sleep(rand() % 3);
        if (semop(sem_id, start_write, 5) == -1)
        {
            printf("semop %d errno: %d\n", getpid(), errno);
            perror("semop error");
            exit(1);
        }

        char ch = 'a' + *shm % 26;
        (*shm)++;
        printf("Writer %d: %c\n", getpid(), ch);

        if (semop(sem_id, stop_write, 2) == -1)
        {
            printf("semop %d errno: %d\n", getpid(), errno);
            perror("semop error");
            exit(1);
        }
    }

    return 0;
}

int main()
{
    if (signal(SIGINT, sig_handler) == SIG_ERR)
    {
        perror("Can't signal.");
        exit(1);
    }

    key_t key = ftok("/dev/null",0);
    if (key == -1)
    {
        printf("ftok error.");
        exit(1);
    }

    if ((shm_id = shmget(key, sizeof(int), IPC_CREAT | PERMS)) == -1)
    {
		perror("shmget error\n");
		exit(1);
	}

    shm_buf = shmat(shm_id, NULL, 0);
    if (shm_buf == (void*) -1)
    {
        perror("shmat error\n");
        exit(1);
    }
    (*shm_buf) = 0;

    if ((sem_id = semget(key, 5, IPC_CREAT | PERMS)) == -1)
    {
		perror("semget error\n");
		exit(1);
	}

    if ((semctl(sem_id, READERS_QUEUE, SETVAL, 0)) == -1)
    {
        perror("semctl error\n");
		exit(1);
    }
    if ((semctl(sem_id, ACTIVE_READERS, SETVAL, 0)) == -1)
    {
        perror("semctl error\n");
		exit(1);
    }
    if ((semctl(sem_id, WRITERS_QUEUE, SETVAL, 0)) == -1)
    {
        perror("semctl error\n");
		exit(1);
    }
    if ((semctl(sem_id, ACTIVE_WRITER, SETVAL, 0)) == -1)
    {
        perror("semctl error\n");
		exit(1);
    }
    if ((semctl(sem_id, BIN_WRITER, SETVAL, 1)) == -1)
    {
        perror("semctl error\n");
		exit(1);
    }

    pid_t pid;

	for (int i = 0; i < WRITER_CNT && pid != 0; i++)
    {
        pid = fork();
        if (pid == -1)
        {
            perror("fork error\n");
            exit(1);
        }

        if (pid == 0)
        {
            writer(sem_id, shm_buf);
            exit(0);
        }
	}

    for (int i = 0; i < READER_CNT && pid != 0; i++)
    {
        pid = fork();
        if (pid == -1)
        {
            perror("fork error\n");
            exit(1);
        }

        if (pid == 0)
        {
            reader(sem_id, shm_buf);
            exit(0);
        }
	}

    int wstatus;
    for (short i = 0; i < WRITER_CNT + READER_CNT; ++i)
    {
        pid_t w = wait(&wstatus);
        if (w == -1)
        {
            perror("wait error");

            switch (errno)
            {
                case ECHILD:
                    fprintf(stderr, "No child processes left to wait for.\n");
                    break;
                case EINTR:
                    fprintf(stderr, "The wait was interrupted by a signal.\n");
                    break;
                default:
                    fprintf(stderr, "Unknown error occurred.\n");
                    break;
            }
            exit(1);
        }

        if (WIFEXITED(wstatus))
            printf("Exited %d status=%d\n", w, WEXITSTATUS(wstatus));
        else if (WIFSIGNALED(wstatus))
            printf("Killed %d by signal %d\n", w, WTERMSIG(wstatus));
        else if (WIFSTOPPED(wstatus))
            printf("Stopped %d by signal %d\n", w, WSTOPSIG(wstatus));
    }

    if (shmdt((void *)shm_buf) == -1)
    {
        perror("shmdt error");
        exit(1);
    }

    if (shmctl(shm_id, IPC_RMID, NULL) == -1)
    {
        perror("shmctl error");
        exit(1);
    }

    if (semctl(sem_id, IPC_RMID, 0) == -1)
    {
        perror("semctl error");
        exit(1);
    }

    exit(0);
}