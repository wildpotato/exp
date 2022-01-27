
#include "NuCommon.h"

int NuInitailFn_Default(void *Argu)
{
    return NU_OK;
}

int NuDestroyFn_Default(void *Argu)
{
    return NU_OK;
}

void NuEventFn_Default(void *Argu)
{
    return;
}

int msSleep(int msec) {
#if _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;
    return nanosleep(&ts, NULL);
#else
    return usleep(msec * 1000);
#endif
}

int usSleep(int usec) {
#if _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = usec / 1000000;
    ts.tv_nsec = (usec % 1000000) * 1000;
    return nanosleep(&ts, NULL);
#else
    return usleep(usec);
#endif
}
