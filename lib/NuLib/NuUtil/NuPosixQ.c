
#include "NuPosixQ.h"

/* private function */
/* ====================================================================== */
static inline void _get_attribute(NuPosixQ_t *pmq) {
    struct mq_attr attr;

    if (mq_getattr(pmq->mqHdl, &attr) == 0) {
        pmq->blockingMode = attr.mq_flags;
        pmq->maxMsgCnt = attr.mq_maxmsg;
        pmq->maxMsgSize = attr.mq_msgsize;
    }
}

static inline void _set_blocking(NuPosixQ_t *pmq, bool isBlocking) {
    struct mq_attr new_attr;
    struct mq_attr old_attr;

    if (isBlocking) {
        new_attr.mq_flags = 0;
    } else {
        new_attr.mq_flags = O_NONBLOCK;
    }

    mq_setattr(pmq->mqHdl, &new_attr, &old_attr);
}

static inline bool _is_blocking_mode(NuPosixQ_t *pmq) {
    return (pmq->blockingMode == O_NONBLOCK) ? false : true;
}

/* Queue function */
/* ====================================================================== */
int NuPosixQueueOpen(NuPosixQ_t *pmq, const char *name, long maxCnt, long maxSize) {
    return NuPosixQueueOpen2(pmq, name, maxCnt, maxSize, false);
}

int NuPosixQueueOpen2(NuPosixQ_t *pmq, const char *name, long maxCnt, long maxSize, bool isBlocking)
{
    int len = strlen(name);
    struct mq_attr attr;

    if (len >= ((sizeof(pmq->Name) / sizeof(char)) - 1)) {
        printf("[NuPosixQueueOpen] name size too large. name size = %d\n", len);
        return NU_FAIL;
    } else {
        if (name[0] != '/') {
            sprintf(pmq->Name, "/%s", name);
        } else {
            strcpy(pmq->Name, name);
        }
    }

    attr.mq_maxmsg = maxCnt;
    attr.mq_msgsize = maxSize;

    pmq->mqHdl = mq_open(pmq->Name, O_RDWR | O_CREAT | O_EXCL, 0666, &attr);
    if (pmq->mqHdl < 0) {
        if (errno != EEXIST) {
            return pmq->mqHdl;
        } else {
            pmq->mqHdl = mq_open(pmq->Name, O_RDWR);
        }
    }

    _set_blocking(pmq, isBlocking);

    _get_attribute(pmq);

    pmq->isQuit = false;

    return NU_OK; 
}

int NuPosixQueueClose(NuPosixQ_t *pmq) 
{ 
    pmq->isQuit = true;
    return mq_close(pmq->mqHdl); 
}

int NuPosixQueueDel(const char *name) 
{ 
    char mqName[63+1] = {0};
    if (name[0] != '/') {
        sprintf(mqName, "/%s", name);
    } else {
        strcpy(mqName, name);
    }
    return mq_unlink(mqName); 
}

int NuPosixQueueSendWithPrio(NuPosixQ_t *pmq, const char *msg, size_t msgLen, unsigned prio) {
    if (_is_blocking_mode(pmq)) {
        return mq_send(pmq->mqHdl, msg, msgLen, prio); 
    } else {
        int iRC = 0; 
        fd_set stWriteFDs;
        struct timeval stTimeOut;
        stTimeOut.tv_usec = 0;

        do {
            if (mq_send(pmq->mqHdl, msg, msgLen, prio) < 0) {
                switch(errno) 
                {
                    case EBADF:
                    case EINVAL:
                        break;
                    case ETIMEDOUT:
                    case EAGAIN:
                        FD_ZERO(&stWriteFDs);
                        FD_SET(pmq->mqHdl, &stWriteFDs);
                        stTimeOut.tv_sec = 1;
                        iRC = select(pmq->mqHdl + 1, NULL, &stWriteFDs, NULL, &stTimeOut);
                        if (iRC > 0) {
                            continue;
                        } else if (iRC == 0) { // timeout
                            continue;
                        } else {
                            return iRC;
                        }
                }

                continue;
            }
            break;
        } while(1);
    }
    return 0;
}

ssize_t NuPosixQueueReceiveWithPrio(NuPosixQ_t *pmq, char *buf, size_t bufLen, unsigned *prio) { 
    ssize_t rc = 0;

    if (_is_blocking_mode(pmq)) {
        struct timespec tm;
        do 
        {
            clock_gettime(CLOCK_REALTIME, &tm);
            tm.tv_sec += 1;
            rc = mq_timedreceive(pmq->mqHdl, buf, bufLen, prio, &tm);
            if (rc <= 0) {
                switch(errno) {
                    case EBADF:
                    case EINVAL:
                        rc = NU_FAIL;
                        break;
                    case ETIMEDOUT:
                    case EAGAIN:
                        rc = 0;
                        continue;
                }
            } 
        } while (rc == 0);
    } else {
        int iRC = 0; 
        fd_set stReadFDs;
        struct timeval stTimeOut;

        stTimeOut.tv_sec = 1;
        stTimeOut.tv_usec = 0;

        do {
            rc = mq_receive(pmq->mqHdl, buf, bufLen, prio);
            if (rc < 0) {
                switch(errno) 
                {
                    case EBADF:
                    case EINVAL:
                        rc = NU_FAIL;
                        break;
                    case EAGAIN:
                    case ETIMEDOUT:
                        FD_ZERO(&stReadFDs);
                        FD_SET(pmq->mqHdl, &stReadFDs);
                        stTimeOut.tv_sec = 1;
                        iRC = select(pmq->mqHdl + 1, &stReadFDs, NULL, NULL, &stTimeOut);
                        if (iRC > 0) {
                            continue;
                        } else if (iRC == 0) { // timeout
                            continue;
                        } else {
                            return NU_FAIL;
                        }
                }
                continue;
            }
            break;
        } while(1);

    }

    return rc;
}
