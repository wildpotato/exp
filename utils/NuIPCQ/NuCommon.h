
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/time.h>
#include <unistd.h>

#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <errno.h>

#ifndef _NUCOMMON_H
#define _NUCOMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#define NU_INLINE inline

/* Return MESSAAGE */
#define NU_NOT_EXIST            2
#define NU_OK                   0
#define NU_FAIL                -1 
#define NU_NULL                -2
#define NU_NOTNULL             -3
#define NU_MALLOC_FAIL         -4
#define NU_NOTOPEN             -5
#define NU_NOTCLEAR            -6
#define NU_STRNULL             -7
#define NU_CASTFAIL            -8
#define NU_NOTFOUND            -9
#define NU_PARAMERROR          -10
#define NU_DEADLCK             -11
#define NU_DUPLICATE           -12
#define NU_EMPTY               -13
#define NU_MMAPERR             -14

#define NU_FILENOTEXIST        -20
#define NU_FILEUNSTABLE        -21
#define NU_FILENOTTODAY        -22
#define NU_OPENFILEFAIL        -23
#define NU_READFAIL            -24
#define NU_WRITEFAIL           -25
#define NU_LOCALTIMEFAIL       -30
#define NU_MAPKEY_DUP          -40
#define NU_MAPKEY_NOT_FOUND    -41
#define NU_MAPIT_END           -42
#define NU_DBM_COL_TOO_LONG    -100
#define NU_FULL                -101
#define NU_DBM_LOCKFAIL        -102

/* socket */
#define NU_GETSOCKET_FAIL      -200
#define NU_BIND_FAIL           -201
#define NU_GETHOSTNAME_FAIL    -202
#define NU_SELECT_FAIL         -203
#define NU_LISTEN_FAIL         -204
#define NU_ACCEPT_FAIL         -205
#define NU_CONNECT_FAIL        -206
#define NU_DISCONNECT_FAIL     -207
#define NU_SEND_FAIL           -208
#define NU_RECV_FAIL           -209

/* Queue */
#define NU_QNOTEXIST           -300
#define NU_QACCESSFAIL         -301
#define NU_QRESOURCEDEFICIENT  -302
#define NU_CREATEQFAIL         -303
#define NU_ENQINTERRUPT        -304
#define NU_ENQFAIL             -305
#define NU_DEQFAIL             -306
#define NU_RMQFAIL             -307
#define NU_DEQINTERRUPT        -308

#define NU_TIMEOUT             -900

#define NU_DONTKNOW_FAIL        -999

#define NUTRUE                  1
#define NUFALSE                 0

/* Macro definition section */
#ifndef NUGOTO
#define NUGOTO(xrv, xrc, xlabel) \
do { \
    (xrv) = (xrc); \
    goto xlabel; \
} while(0)
#endif

#ifndef NUCHKRC
#define NUCHKRC(xrc, xlabel) \
do { \
    if ((xrc) < 0) \
        goto xlabel; \
} while(0)
#endif

#ifndef NUCHK_EQUAL_GOTO
#define NUCHK_EQUAL_GOTO(xrc, xrv, xlabel) \
do { \
    if ((xrc) == (xrv)) \
        goto xlabel; \
} while(0)
#endif

#ifndef NuMax
#define NuMax(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef NuMin
#define NuMin(a,b) (((a) < (b)) ? (a) : (b))
#endif


/**
 * copy from kernel.h
 * NuContainerOf - cast a member of a structure out to the containing structure
 * @ptr:    the pointer to the member.
 * @type:   the type of the container struct this is embedded in.
 * @member: the name of the member within the struct.
 *
 **/
#ifndef offsetof
#define offsetof(st, m) __builtin_offsetof(st, m)
#endif

#ifndef container_of
/* for c11 migration
   https://zh.wikipedia.org/wiki/Offsetof 
#define NuContainerOf(ptr, type, member) ({          \
		const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
		(type *)( (char *)__mptr - offsetof(type,member) );})
*/
#define NuContainerOf(ptr, type, member) ((type *)((char *)(ptr) - offsetof(type, member)))
#else
#define NuContainerOf container_of
#endif

#ifndef likely
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif


#ifdef _GNU_SOURCE
#define NuBITSET(S, A) __sync_or_and_fetch(&(S), (A))
#define NuBITUNSET(S, A) __sync_and_and_fetch(&(S), ~(A)) 
#else
#define NuBITSET(S, A) ((S) = (S) | (A))
#define NuBITUNSET(S, A) ((S) = (S) & ~(A))
#endif

#define NuBITCHK(S, A) ((S) & (A))

typedef int (*NuConstructor)(void *);
typedef int (*NuCompareFn)(void *, void *);
typedef int (*NuDestructor)(void *);
typedef void *(*NuCopyFn)(void *);
typedef void (*NuEventFn)(void *);

void NuEventFn_Default(void *Argu);
int NuInitailFn_Default(void *Argu);
int NuDestroyFn_Default(void *Argu);

/* sleep function */
int msSleep(int msec);
int usSleep(int usec);

#if _POSIX_C_SOURCE >= 199309L
#define usleep(usec) usSleep(usec)
#endif


#ifdef __cplusplus
}
#endif

#endif /* _NUCOMMON_H */

