
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

#include "NuSocketType_Pipe.h"

static void SetFD(int FD)
{
    int Flag = 0;

    Flag = fcntl(FD, F_GETFL, 0);
    fcntl(FD, F_SETFL, Flag | O_NONBLOCK);

    Flag = 1;
    setsockopt(FD, SOL_SOCKET, SO_REUSEADDR, &Flag, sizeof(Flag));

    return;
}

static int SetOnline(NuSocketTypeNode_t *TypeNode)
{
    int FDS[2] = {0, 0};

    if(pipe(FDS) < 0)
    {
        NuSocketTypeLog(TypeNode, "pipe(%s)\n", strerror(errno));
        return NuSocketTypeCBError;
    }

    SetFD(TypeNode->InFD = FDS[0]);
    SetFD(TypeNode->OutFD = FDS[1]);

    return NuSocketTypeCBPass;
}

NuSocketType_t Pipe = {"Pipe", 0, &SetOnline, NULL, NULL, NULL, NULL, NULL, NULL};

