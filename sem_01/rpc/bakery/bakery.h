/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#ifndef _BAKERY_H_RPCGEN
#define _BAKERY_H_RPCGEN

#include <rpc/rpc.h>


#ifdef __cplusplus
extern "C" {
#endif

#define ADD 0
#define SUB 1
#define MUL 2
#define DIV 3

struct BAKERY {
	int num;
	int op;
	double arg1;
	double arg2;
	double result;
};
typedef struct BAKERY BAKERY;

#define BAKERY_PROG 0x20000001
#define BAKERY_VER 1

#if defined(__STDC__) || defined(__cplusplus)
#define GET_NUM 1
extern  struct BAKERY * get_num_1(struct BAKERY *, CLIENT *);
extern  struct BAKERY * get_num_1_svc(struct BAKERY *, struct svc_req *);
#define CALCULATOR_PROC 2
extern  struct BAKERY * calculator_proc_1(struct BAKERY *, CLIENT *);
extern  struct BAKERY * calculator_proc_1_svc(struct BAKERY *, struct svc_req *);
extern int bakery_prog_1_freeresult (SVCXPRT *, xdrproc_t, caddr_t);

#else /* K&R C */
#define GET_NUM 1
extern  struct BAKERY * get_num_1();
extern  struct BAKERY * get_num_1_svc();
#define CALCULATOR_PROC 2
extern  struct BAKERY * calculator_proc_1();
extern  struct BAKERY * calculator_proc_1_svc();
extern int bakery_prog_1_freeresult ();
#endif /* K&R C */

/* the xdr functions */

#if defined(__STDC__) || defined(__cplusplus)
extern  bool_t xdr_BAKERY (XDR *, BAKERY*);

#else /* K&R C */
extern bool_t xdr_BAKERY ();

#endif /* K&R C */

#ifdef __cplusplus
}
#endif

#endif /* !_BAKERY_H_RPCGEN */
