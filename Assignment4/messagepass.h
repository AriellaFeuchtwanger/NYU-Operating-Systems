/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#ifndef _MESSAGEPASS_H_RPCGEN
#define _MESSAGEPASS_H_RPCGEN

#define RPCGEN_VERSION	199506

#include <rpc/rpc.h>


#define MESSAGE_PROG ((rpc_uint)0x30000824)
#define MESSAGE_VERS ((rpc_uint)1)

#ifdef __cplusplus
#define MESSAGEPASS ((rpc_uint)1)
extern "C" char ** messagepass_1(char **, CLIENT *);
extern "C" char ** messagepass_1_svc(char **, struct svc_req *);

#elif __STDC__
#define MESSAGEPASS ((rpc_uint)1)
extern  char ** messagepass_1(char **, CLIENT *);
extern  char ** messagepass_1_svc(char **, struct svc_req *);

#else /* Old Style C */
#define MESSAGEPASS ((rpc_uint)1)
extern  char ** messagepass_1();
extern  char ** messagepass_1_svc();
#endif /* Old Style C */

#endif /* !_MESSAGEPASS_H_RPCGEN */
