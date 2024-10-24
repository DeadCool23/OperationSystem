#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <syslog.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <sys/resource.h>

#define LOCKFILE "/var/run/daemon.pid"
#define CONFFILE "/etc/daemon.conf"
#define SLEEP_TIME 15

sigset_t mask;

int lockfile(int fd)
{
    struct flock fl;

    fl.l_type = F_WRLCK; // блокировка на запись
    fl.l_start = 0; // смещение относительно WHENCE, начало блокировки
    fl.l_whence = SEEK_SET; // курсор на начало
    fl.l_len = 0; // длина блокируемого участка

    return fcntl(fd, F_SETLK, &fl);
}

int already_running(void)
{
    int fd;
    char lockbuf[16];
    int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    fd = open(LOCKFILE, O_RDWR | O_CREAT, perms);
    if (fd < 0)
    {
        syslog(LOG_ERR, "невозможно открыть %s: %s", LOCKFILE, strerror(errno));
        exit(1);
    }

    if (lockfile(fd) < 0)
    {
        if (errno == EACCES || errno == EAGAIN)
        {
            close(fd);
            return 1;
        }
        syslog(LOG_ERR, "невозможно установить блокировку на %s: %s", LOCKFILE, strerror(errno));
        exit(1);
    }
    

    ftruncate(fd, 0);
    sprintf(lockbuf, "%ld", (long)getpid());
    write(fd, lockbuf, strlen(lockbuf) + 1);
    
    return 0;
}


void daemonize(const char *cmd)
{
    int i, fd0, fd1, fd2;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;

    /*
     * 1. Сбросить маску режима создания файла.
     */	
    umask(0);
    
    /*
     * Получить максимально возможный номер дескриптора файла.
     */
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
        printf("%s: невозможно получить максимальный номер дескриптора ", cmd);
    
    /*
     * 2. Стать лидером нового сеанса, чтобы утратить управляющий терминал.
     */
    if ((pid = fork()) < 0)
        printf("%s: ошибка вызова функции fork", cmd);
    else if (pid > 0) /* родительский процесс */
        exit(0);

    /*
     * Обеспечить невозможность обретения управляющего терминала в будущем.
     */
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGHUP, &sa, NULL) < 0)
        printf("%s: невозможно игнорировать сигнал SIGHUP", cmd);
    
    if(setsid() == -1)
    {
    	printf("Can't call setsid()\n");
    	exit(1);
    }
    
    /*
     * 4. Назначить корневой каталог текущим рабочим каталогом,
     * чтобы впоследствии можно было отмонтировать файловую систему.
     */
    if (chdir("/") < 0)
        printf("%s: невозможно сделать текущим рабочим каталогом", cmd);
    
    /*
     * 5. Закрыть все открытые файловые дескрипторы.
     */
    if (rl.rlim_max == RLIM_INFINITY)
        rl.rlim_max = 1024;
    for (i = 0; i < rl.rlim_max; i++)
        close(i);
    
    /*
     * Присоединить файловые дескрипторы 0, 1 и 2 к /dev/null.
     */
    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);
    
    /*
     * 6. Инициализировать файл журнала.
     */
    openlog(cmd, LOG_CONS, LOG_DAEMON);

    if (fd0 != 0 || fd1 != 1 || fd2 != 2)
    {
        syslog(LOG_ERR, "ошибочные файловые дескрипторы %d %d %d", fd0, fd1, fd2);
        exit(1);
    }
}

void reread(void)
{
    int fd;
    long uid;
    char uname[128];

    char buf[128];

    fd = open(CONFFILE, O_RDONLY);
    if (fd == -1) {
        syslog(LOG_ERR, "Невозможно открыть конфигурационный файл %s", CONFFILE);
        return;
    }

    ssize_t rbytes = read(fd, buf, 128 - 1);
    if (rbytes == -1) {
        syslog(LOG_ERR, "Невозможно прочитать конфигурационный файл %s", CONFFILE);
        close(fd);
        return;
    }

    buf[rbytes] = '\0';

    if (sscanf(buf, "%ld %s", &uid, uname) == 2)
        syslog(LOG_INFO, "UID: %ld, UNAME: %s", uid, uname);
    else
        syslog(LOG_ERR, "Невозможно прочитать конфигурационный файл %s", CONFFILE);

    close(fd);
}

void *thr_fn(void *arg)
{
    int err, signo;

    for (;;)
    {
        err = sigwait(&mask, &signo);
        if (err != 0)
        {
            syslog(LOG_ERR, "ошибка вызова функции sigwait");
            exit(1);
        }

        switch (signo)
        {
        case SIGHUP:
            syslog(LOG_INFO, "чтение конфигурационного файла");
            reread();
            break;
        case SIGTERM:
            syslog(LOG_INFO, "получен SIGTERM; выход");
            exit(0);   
        default:
            syslog(LOG_INFO, "получен сигнал %d\n", signo);
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int fd;
    int err;
    pthread_t tid;
    char *cmd;
    struct sigaction sa;

    if ((cmd = strrchr(argv[0], '/')) == NULL)
        cmd = argv[0];
    else
        cmd++;
    
    /*
     * Перейти в режим демона
     */
    daemonize(cmd);

    /*
     * Убедиться в том, что ранее не была запущенв другая копия демона
     */
    if (already_running())
    {
        syslog(LOG_ERR, "демон уже запущен");
        exit(1);
    }
    
    /*
     * Восстановить действия по умолчанию для сигнала SIGHUP и заблокировать все сигналы
     */
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0)
        syslog(LOG_SYSLOG, "невозможно восставновить действие SIG_DFL для SIGHUP");

    sigfillset(&mask);
    if ((err = pthread_sigmask(SIG_BLOCK, &mask, NULL)) != 0)
        printf("ошибка выполнения операции SIG_BLOCK");
    
    /*
     * Создать поток, который будет заниматься обработкой SIGHUP и SIGTERM
     */
    pthread_create(&tid, NULL, thr_fn, NULL);
    if (tid == -1)
        syslog(LOG_ERR, "невозможно создать поток");

    time_t raw_time;
    struct tm *timeinfo;
    
    for (;;)
    {
        time(&raw_time);
        timeinfo = localtime(&raw_time);
        syslog(LOG_INFO, "Current time is: %s", asctime(timeinfo));
        sleep(SLEEP_TIME);
    }
}
