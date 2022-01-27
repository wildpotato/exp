
#include "NuTuneTools.h"

static struct timespec g_tv_start;
static struct timespec g_tv_end;

static long diff_in_ns(struct timespec t1, struct timespec t2)
{
    struct timespec diff;
    if (t2.tv_nsec-t1.tv_nsec < 0) {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return (diff.tv_sec * 1000000000.0 + diff.tv_nsec);
}

void NuTuneSetStart()
{
	clock_gettime(CLOCK_REALTIME, &g_tv_start);
}

void NuTuneSetStop()
{
	clock_gettime(CLOCK_REALTIME, &g_tv_end);
}

time_t NuTuneGetSecElapse()
{
	return (g_tv_end.tv_sec - g_tv_start.tv_sec);
}

long NuTuneGetNanoSecElapse()
{
    return diff_in_ns(g_tv_start, g_tv_end);
}
