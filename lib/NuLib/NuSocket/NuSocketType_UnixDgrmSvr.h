
#include <sys/un.h>
#include "NuSocketType.h"

#ifndef _NUSOCKETTYPE_UNIXDGRMSVR_H
#define _NUSOCKETTYPE_UNIXDGRMSVR_H

#ifdef __cplusplus
extern "C" {
#endif

extern NuSocketType_t UnixDgrmSvr;

typedef struct _NuSocketType_UnixDgrmSvrMsg
{
    struct sockaddr_un  Addr;
    char                Msg[1];
} NuSocketType_UnixDgrmSvrMsg;

#ifdef __cplusplus
}
#endif

#endif /* _NUSOCKETTYPE_UNIXDGRMSVR_H */

