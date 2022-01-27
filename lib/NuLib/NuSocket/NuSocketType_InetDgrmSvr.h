
#include <netinet/in.h>
#include "NuSocketType.h"

#ifndef _NUSOCKETTYPE_INETDGRMSVR_H
#define _NUSOCKETTYPE_INETDGRMSVR_H

#ifdef __cplusplus
extern "C" {
#endif

extern NuSocketType_t InetDgrmSvr;

typedef struct _NuSocketType_InetDgrmSvrMsg
{
    struct sockaddr_in  Addr;
    char                Msg[1];
} NuSocketType_InetDgrmSvrMsg;

#ifdef __cplusplus
}
#endif

#endif /* _NUSOCKETTYPE_INETDGRMSVR_H */

