/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include "pc.h"
#include <stdio.h>
#include <stdlib.h>
#include <rpc/pmap_clnt.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/sem.h>
#include <pthread.h>

#ifndef SIG_PF
#define SIG_PF void(*)(int)
#endif

#define SIZE_BUF 256
#define BIN_SEM 0
#define BUFF_FULL 1
#define BUFF_EMPTY 2

pthread_t thread;
pthread_attr_t attr;
int semid;

struct thread_data {
    struct svc_req *rqstp;
    SVCXPRT *transp;
};

void *process_request(void *data)
{
    struct thread_data *td = (struct thread_data *)data;

    struct svc_req *rqstp = td->rqstp;
    register SVCXPRT *transp = td->transp;

	union {
		int fill;
	} argument;
	char *result;
	xdrproc_t _xdr_argument, _xdr_result;
	char *(*local)(char *, struct svc_req *);

	switch (rqstp->rq_proc) {
	case NULLPROC:
		(void) svc_sendreply (transp, (xdrproc_t) xdr_void, (char *)NULL);
        free(td);  
		return NULL;

	case PRODUCE:
		_xdr_argument = (xdrproc_t) xdr_void;
		_xdr_result = (xdrproc_t) xdr_void;
		local = (char *(*)(char *, struct svc_req *)) produce_1_svc;
		break;

	case CONSUME:
		_xdr_argument = (xdrproc_t) xdr_void;
		_xdr_result = (xdrproc_t) xdr_char;
		local = (char *(*)(char *, struct svc_req *)) consume_1_svc;
		break;

	default:
		svcerr_noproc (transp);
        free(td); 
        return NULL;
	}
	memset ((char *)&argument, 0, sizeof (argument));
	if (!svc_getargs (transp, (xdrproc_t) _xdr_argument, (caddr_t) &argument)) {
		svcerr_decode (transp);
        free(td); 
        return NULL;
	}
	result = (*local)((char *)&argument, rqstp);
	if (result != NULL && !svc_sendreply(transp, (xdrproc_t) _xdr_result, result)) {
		svcerr_systemerr (transp);
	}
	if (!svc_freeargs (transp, (xdrproc_t) _xdr_argument, (caddr_t) &argument)) {
		fprintf (stderr, "%s", "unable to free arguments");
		exit (1);
	}
    free(td); 
    return NULL;
}

static void pc_prog_1(struct svc_req *rqstp, register SVCXPRT *transp) {
    struct thread_data *td = (struct thread_data *)malloc(sizeof(struct thread_data));
    if (td == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        return;
    }

    td->rqstp = rqstp;
    td->transp = transp;

    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (pthread_create(&thread, &attr, process_request, (void *)td) != 0) {
        fprintf(stderr, "Thread creation error\n");
        free(td);
    }
}

int main (int argc, char **argv)
{
	register SVCXPRT *transp;
    key_t semkey = ftok("/dev/null", 4);
    if (semkey == (key_t)-1) {
        perror("Error: ftok\n");
        exit(1);
    }
    semid = semget(semkey, 3, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("Error: semget\n");
        exit(1);
    }
    int rc_bin_sem = semctl(semid, BIN_SEM, SETVAL, 1);
    int rc_buff_empty = semctl(semid, BUFF_EMPTY, SETVAL, SIZE_BUF);
    int rc_buff_full = semctl(semid, BUFF_FULL, SETVAL, 0);
    if (rc_bin_sem == -1 || rc_buff_empty == -1 || rc_buff_full == -1)
    {
        perror("Error: semctl.\n");
        exit(1);
    }

    pthread_attr_init(&attr);
	pmap_unset (PC_PROG, PC_VER);
	transp = svcudp_create(RPC_ANYSOCK);
	if (transp == NULL) {
		fprintf (stderr, "%s", "cannot create udp service.");
		exit(1);
	}
	if (!svc_register(transp, PC_PROG, PC_VER, pc_prog_1, IPPROTO_UDP)) {
		fprintf (stderr, "%s", "unable to register (PC_PROG, PC_VER, udp).");
		exit(1);
	}

	transp = svctcp_create(RPC_ANYSOCK, 0, 0);
	if (transp == NULL) {
		fprintf (stderr, "%s", "cannot create tcp service.");
		exit(1);
	}
	if (!svc_register(transp, PC_PROG, PC_VER, pc_prog_1, IPPROTO_TCP)) {
		fprintf (stderr, "%s", "unable to register (PC_PROG, PC_VER, tcp).");
		exit(1);
	}

	svc_run ();
	fprintf (stderr, "%s", "svc_run returned");
	exit (1);
}
