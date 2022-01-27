
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "NuSocket.h"
#include "NuInvoker.h"

#define DEFAULT_THREADNO    3

static void MyPrint(void *Argu)
{
    printf("Printf via Socket invoke system...[%s]\n", (char *)Argu);

    return;
}

static void OnLog(char *Format, va_list ArguList, void *Argu)
{
    vprintf(Format, ArguList);
    return;
}

static void Signal(int Sig)
{
    printf("Sig[%d]\n", Sig);
    return;
}

int main(int Argc, char **Argv)
{
    int                     Cnt = 10, WorkThreadNo = DEFAULT_THREADNO;
    struct _NuSocket_t      *pSocket = NULL;
    char                    Msg[10][256];
    NuInvokerDelegate_t     Delegate = {&MyPrint, NULL};

    signal(SIGHUP, Signal);
    signal(SIGPIPE, Signal);

    if(NuSocketNew(&pSocket, WorkThreadNo, &OnLog) < 0)
    {
        printf("NuSocketNew Error!\n");
        return 0;
    }

    while(Cnt --)
    {
        sprintf(Msg[Cnt], "%d", Cnt);
    }

    Cnt = 0;
    while(1)
    {
        sleep(1);

        Delegate.Argu = Msg[Cnt];
        NuSocketInvoke(pSocket, &Delegate);

        ++ Cnt;

        Delegate.Argu = Msg[Cnt];
        NuSocketInvoke(pSocket, &Delegate);

        ++ Cnt;

        Delegate.Argu = Msg[Cnt];
        NuSocketInvoke(pSocket, &Delegate);

        ++ Cnt;

        Delegate.Argu = Msg[Cnt];
        NuSocketInvoke(pSocket, &Delegate);

        ++ Cnt;

        Delegate.Argu = Msg[Cnt];
        NuSocketInvoke(pSocket, &Delegate);

        ++ Cnt;

        Cnt %= 10;
        printf("----------------------\n");
    }

    return 0;
}

