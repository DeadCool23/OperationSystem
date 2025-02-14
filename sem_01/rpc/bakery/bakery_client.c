/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "bakery.h"


void
bakery_prog_1(char *host)
{
	CLIENT *clnt;
	struct BAKERY  *result_1;
	struct BAKERY  get_num_1_arg;
	struct BAKERY  *result_2;
	struct BAKERY  calculator_proc_1_arg;

#ifndef	DEBUG
	clnt = clnt_create (host, BAKERY_PROG, BAKERY_VER, "udp");
	if (clnt == NULL) {
		clnt_pcreateerror (host);
		exit (1);
	}
#endif	/* DEBUG */

	result_1 = get_num_1(&get_num_1_arg, clnt);
	if (result_1 == (struct BAKERY *) NULL) {
		clnt_perror (clnt, "call failed");
	}
	result_2 = calculator_proc_1(&calculator_proc_1_arg, clnt);
	if (result_2 == (struct BAKERY *) NULL) {
		clnt_perror (clnt, "call failed");
	}
#ifndef	DEBUG
	clnt_destroy (clnt);
#endif	 /* DEBUG */
}


int
main (int argc, char *argv[])
{
	char *host;

	if (argc < 2) {
		printf ("usage: %s server_host\n", argv[0]);
		exit (1);
	}
	host = argv[1];
	bakery_prog_1 (host);
exit (0);
}
