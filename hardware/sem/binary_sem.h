/*
 *  Reserve: Attempt to reserve semaphore for exclusive use. If the semaphore is already
 *           reserved by another process, then block until the semaphore is released.
 *
 *  Release: Free a currently reserved semaphore, so that it can be reserved by another
 *           process.
 *
 */
#ifndef BINARY_SEMS_H
#define BINARY_SEMS_H

#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <errno.h>
#include <stdbool.h>

bool bsUseSemUndo = false;
bool bsRetryOnEintr = true;

union semun {
    int                 val;
    struct semid_ds *   buf;
    unsigned short *    array;
#if defined(__linux__)
    struct seminfo *    __buf;
#endif
};

int init_sem_available(int semId, int semNum)
{
    union semun arg;

    arg.val = 1;
    return semctl(semId, semNum, SETVAL, arg);
}

int init_sem_in_use(int semId, int semNum)
{
    union semun arg;

    arg.val = 0;
    return semctl(semId, semNum, SETVAL, arg);
}

int reserve_sem(int semId, int semNum)
{
    struct sembuf sops;

    sops.sem_num = semNum;
    sops.sem_op = -1;
    sops.sem_flg = bsUseSemUndo ? SEM_UNDO : 0;
    while (semop(semId, &sops, 1) == -1)
        if (errno != EINTR || !bsRetryOnEintr)
            return -1;
    return 0;
}

int release_sem(int semId, int semNum)
{
    struct sembuf sops;

    sops.sem_num = semNum;
    sops.sem_op = 1;
    sops.sem_flg = bsUseSemUndo ? SEM_UNDO : 0;
    return semop(semId, &sops, 1);
}

#endif
