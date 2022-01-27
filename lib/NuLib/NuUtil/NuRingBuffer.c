#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include "NuRingBuffer.h"
/* ---------------------------------------------- */
/* ring buffer base                               */
/* ---------------------------------------------- */
/* how many elements can be read from  ring  buffer
 *           h
 * 0 1 2 3 4 5 6 7
 *     t        
 * rbuf_base_count        will be 3 (2, 3, 4)
 * rbuf_base_count_to_end will be 3 (2, 3, 4)
 *     h      
 * 0 1 2 3 4 5 6 7
 *           t       
 * rbuf_base_count        will be 5 (5, 6, 7, 0, 1)
 * rbuf_base_count_to_end will be 3 (5, 6, 7)
 * */
static inline size_t rbuf_base_count(NuRBufBase_t *rb) {
	return (rb->head - rb->tail) & (rb->caps - 1);
}

static inline size_t rbuf_base_count_to_end(NuRBufBase_t *rb) {
	size_t end = rb->caps - rb->tail;
	size_t n   = (rb->head + end) & (rb->caps - 1);
	return (n < end) ? n : end;
}

/* how many elements can be pushed into ring  buffer
 *           h
 * 0 1 2 3 4 5 6 7
 *     t        
 * rbuf_base_free_caps        will be 4 (5, 6, 7, 0)
 * rbuf_base_free_caps_to_end will be 3 (5, 6, 7)
 *
 * h          
 * 0 1 2 3 4 5 6 7
 *           t       
 * rbuf_base_free_caps        will be 4 (0, 1, 2, 0)
 * rbuf_base_free_caps_to_end will be 4 (0, 1, 2, 4)
 * */
static inline size_t rbuf_base_free_caps(NuRBufBase_t *rb) {
	return (rb->tail - (rb->head + 1)) & (rb->caps - 1);
}

static inline size_t rbuf_base_free_caps_to_end(NuRBufBase_t *rb) {
	size_t end = rb->caps - 1 - rb->head;
	size_t n   = (end + rb->tail) & (rb->caps - 1);
	return (n <= end) ? n : end + 1;
}

/* ring buffer base initialize */
static inline bool rbuf_base_init(NuRBufBase_t *rb, size_t sz) {
	if (!uint_is_power_of_two(sz)) {
		rb->caps = uint_ceil_pow_of_two(sz);
	} else {
		rb->caps = sz;
	}

	rb->head   = 0;
	rb->tail   = 0;

	rb->buffer = (char *)malloc(rb->caps * sizeof(char));
	if (rb->buffer == NULL) {
		return false;
	} else {
		memset(rb->buffer, ' ', rb->caps * sizeof(char));
	}

	return true;
}

static inline size_t rbuf_base_try_read_data(NuRBufBase_t *rb, char *data, size_t needSz, bool tryFlag) {
	size_t rc = needSz;
	size_t available_sz_end = rbuf_base_count_to_end(rb);

	if (needSz > available_sz_end) {
		size_t available_sz = rbuf_base_count(rb);
		if (needSz > available_sz) {
			NUGOTO(rc, 0, EXIT);
		}
		memcpy(data, rb->buffer + rb->tail, available_sz_end);
		memcpy(data + available_sz_end, rb->buffer, needSz - available_sz_end);
		rb->tail = (tryFlag == true) ? rb->tail : (needSz - available_sz_end);

	} else {
		memcpy(data, rb->buffer + rb->tail, needSz);
		rb->tail += (tryFlag == true) ? 0 : needSz;
		rb->tail &= rb->caps - 1;
	}
EXIT:
	return rc;
}

static inline size_t rbuf_base_skip_data(NuRBufBase_t *rb, size_t skipSz) {
	size_t rc = skipSz;
	size_t available_sz_end = rbuf_base_count_to_end(rb);

	if (skipSz > available_sz_end) {
		size_t available_sz = rbuf_base_count(rb);
		if (skipSz > available_sz) {
			NUGOTO(rc, 0, EXIT);
		}
		rb->tail = (skipSz - available_sz_end);

	} else {
		rb->tail += skipSz;
		rb->tail &= rb->caps - 1;
	}
EXIT:
	return rc;
}

/* fd read function */
/* set fd option */
static inline int rbuf_base_sockOptOn(int fd, int level, int optname) {
	socklen_t optval = 1;
	return setsockopt( fd, level, optname, &optval, sizeof(optval) );
}

static inline int rbuf_base_sock_setOpt(int fd, int flags) {
	if (NuBITCHK(flags, NURB_TCP_QUICKACK)) {
		return rbuf_base_sockOptOn(fd, IPPROTO_TCP, TCP_QUICKACK);
	}
	return 0;
}

/* if the fd in blocking mode, don't blocked it */
static inline ssize_t rbuf_base_read_fd(int fd, char *dst, size_t needSz, int waitSec) {
	struct timeval  tv = { .tv_sec = waitSec, .tv_usec = 0 };
    fd_set          fds;   
    int             sact;

    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    sact = select(fd + 1, &fds, NULL, NULL, &tv);
	if (sact > 0) {
		return read(fd, dst, needSz);
	} else if (sact == 0) {
        errno = EAGAIN;
        return 0;
	} else {
        return -1;
	}
}

/* ---------------------------------------------- */
/* ring buffer base (public function)             */
/* ---------------------------------------------- */
NuRBufBase_t *NuRBufBaseNew(size_t sz) {
	NuRBufBase_t *rb = NULL;

	rb = (NuRBufBase_t *)malloc(sizeof(*rb));
	if (rb != NULL) {
		if (rbuf_base_init(rb, sz) == false) {
			NuRBufBaseFree(rb);
			NUGOTO(rb, NULL, EXIT);
		}
	}
EXIT:
	return rb;
}

void NuRBufBaseFree(NuRBufBase_t *rb) {
	if (rb != NULL) {
		if (rb->buffer != NULL) {
			free(rb->buffer);
		}
		free(rb);
	}
}

size_t NuRBufBaseGetCapicity(NuRBufBase_t *rb) {
	return rb->caps;
}

size_t NuRBufBaseGetSize(NuRBufBase_t *rb) {
	return rbuf_base_count(rb);
}

size_t NuRBufBaseWrite(NuRBufBase_t *rb, char *data, size_t dataSz) {
	size_t rc = dataSz;
	size_t free_caps_end = rbuf_base_free_caps_to_end(rb);

	/* if data size large to overlap, it must be copied in two actions */
	if (dataSz > free_caps_end) { 
		size_t free_caps = rbuf_base_free_caps(rb);
		if (dataSz > free_caps) {
			NUGOTO(rc, 0, EXIT);
		} 

		memcpy((rb->buffer + rb->head), data, free_caps_end);
		memcpy(rb->buffer, (data + free_caps_end), (dataSz - free_caps_end));
		rb->head = dataSz - free_caps_end;

	} else {
		memcpy(rb->buffer + rb->head, data, dataSz);
		rb->head += dataSz;
		rb->head &= rb->caps - 1;
	}
EXIT:
	return rc;
}

int NuRBufBaseRecv(NuRBufBase_t *rb, int fd, size_t needSz, size_t *readSz, int flags) {
	ssize_t recvN         = 0;
	int     rc            = 0;
	size_t  free_caps     = rbuf_base_free_caps(rb);
	size_t  free_caps_end = rbuf_base_free_caps_to_end(rb);

	*readSz = 0;

	if (needSz == 0) {
		needSz = free_caps;
	} else if (needSz > free_caps) {
		NUGOTO(rc, 0, EXIT);
	}

	/* if data size large to overlap, it must be copied in two actions */
	if (needSz > free_caps_end) {
		rbuf_base_sock_setOpt(fd, flags);
		recvN = read(fd, rb->buffer + rb->head, free_caps_end);
		if (recvN <= 0) {
			NUGOTO(rc, recvN, EXIT);
		} 

		*readSz += recvN;

		if (recvN == free_caps_end) { /* more data can be read */
			rb->head = 0;
			rbuf_base_sock_setOpt(fd, flags);
			recvN = read(fd, rb->buffer, needSz - free_caps_end);
			if (recvN >= 0) {
				*readSz += recvN;
				rb->head += recvN;
			} else {
				if (errno == EAGAIN) {
					/* 由於已經有讀到資料, 所以需要return 0, 讓呼叫者知道readSz有值 */
					NUGOTO(rc, 0, EXIT);
				} else {
					NUGOTO(rc, recvN, EXIT);
				}
			}
		} else {
			rb->head += recvN;
			rb->head &= rb->caps - 1;
		}
	} else {
		rbuf_base_sock_setOpt(fd, flags);
		recvN = read(fd, rb->buffer + rb->head, needSz);
		if (recvN <= 0) {
			NUGOTO(rc, recvN, EXIT);
		} 
		*readSz += recvN;
		rb->head += recvN;
		rb->head &= rb->caps - 1;
	}
EXIT:
	return rc;
}

int NuRBufBaseRecvInTime(NuRBufBase_t *rb, int fd, size_t needSz, size_t *readSz, int waitSec) {
	ssize_t recvN         = 0;
	int     rc            = 0;
	size_t  free_caps     = rbuf_base_free_caps(rb);
	size_t  free_caps_end = rbuf_base_free_caps_to_end(rb);

	*readSz = 0;

	if (needSz == 0) {
		needSz = free_caps;
	} else if (needSz > free_caps) {
		NUGOTO(rc, 0, EXIT);
	}

	/* if data size large to overlap, it must be copied in two actions */
	if (needSz > free_caps_end) { 
		recvN = rbuf_base_read_fd(fd, rb->buffer + rb->head, free_caps_end, waitSec);
		if (recvN <= 0) {
			NUGOTO(rc, recvN, EXIT);
		} 

		*readSz += recvN;

		if (recvN == free_caps_end) { /* more data can be read */
			rb->head = 0;
			recvN = rbuf_base_read_fd(fd, rb->buffer, needSz - free_caps_end, waitSec);
			if (recvN >= 0) {
				*readSz += recvN;
				rb->head += recvN;
			} else {
				NUGOTO(rc, recvN, EXIT);
			}
		} else {
			rb->head += recvN;
			rb->head &= rb->caps - 1;
		}
	} else {
		//recvN = read(fd, rb->buffer + rb->head, needSz);
		recvN = rbuf_base_read_fd(fd, rb->buffer + rb->head, needSz, waitSec);
		if (recvN <= 0) {
			NUGOTO(rc, recvN, EXIT);
		} 
		*readSz += recvN;
		rb->head += recvN;
		rb->head &= rb->caps - 1;
	}
EXIT:
	return rc;
}

size_t NuRBufBaseRead(NuRBufBase_t *rb, void *data, size_t needSz) {
	return rbuf_base_try_read_data(rb, data, needSz, false);
}

size_t NuRBufBaseTryRead(NuRBufBase_t *rb, void *data, size_t needSz) {
	return rbuf_base_try_read_data(rb, data, needSz, true);
}

size_t NuRBufBaseSkip(NuRBufBase_t *rb, size_t skipSz) {
	return rbuf_base_skip_data(rb, skipSz);
}


/* ---------------------------------------------- */
/* ring buffer                                    */
/* ---------------------------------------------- */
#define rbuf_w_lock(o)   do {     \
	NuSpinLock(&((o)->wlock));   \
	NuSpinLock(&((o)->lock));    \
} while(0)

#define rbuf_w_unlock(o) do {     \
	NuSpinUnLock(&((o)->lock));  \
	NuSpinUnLock(&((o)->wlock)); \
} while(0)

#define rbuf_r_lock(o)   do {     \
	NuSpinLock(&((o)->rlock));   \
	NuSpinLock(&((o)->lock));    \
} while(0)

#define rbuf_r_unlock(o) do {     \
	NuSpinUnLock(&((o)->lock));  \
	NuSpinUnLock(&((o)->rlock)); \
} while(0)


static inline size_t rbuf_try_read_data(NuRBuf_t *rb, char *data, size_t needSz, bool tryFlag) {
	size_t rc = 0;
	rbuf_r_lock(rb);
	rc = rbuf_base_try_read_data(rb->base, data, needSz, tryFlag);
	rbuf_r_unlock(rb);
	return rc;
}

static inline size_t rbuf_skip_data(NuRBuf_t *rb, size_t skipSz) {
	size_t rc = 0;
	rbuf_r_lock(rb);
	rc = rbuf_base_skip_data(rb->base, skipSz);
	rbuf_r_unlock(rb);
	return rc;
}


/* ---------------------------------------------- */
/* public function                                */
/* ---------------------------------------------- */
NuRBuf_t *NuRBufNew(size_t sz) {
	NuRBuf_t *rb = NULL;

	rb = (NuRBuf_t *)malloc(sizeof(*rb));
	if (rb != NULL) {
		rb->base = NuRBufBaseNew(sz);
		if (rb->base == NULL) {
			NuRBufFree(rb);
			NUGOTO(rb, NULL, EXIT);
		}
		
		NuSpinInit(&(rb->rlock));
		NuSpinInit(&(rb->wlock));
		NuSpinInit(&(rb->lock));
	}
EXIT:
	return rb;
}

void NuRBufFree(NuRBuf_t *rb) {
	if (rb != NULL) {
		if (rb->base != NULL) {
			NuRBufBaseFree(rb->base);
		}
		free(rb);
	}
}

size_t NuRBufGetCapicity(NuRBuf_t *rb) {
	return NuRBufBaseGetCapicity(rb->base);
}

size_t NuRBufGetSize(NuRBuf_t *rb) {
	return NuRBufBaseGetSize(rb->base);
}

size_t NuRBufWrite(NuRBuf_t *rb, char *data, size_t dataSz) {
	size_t rc = 0;
	rbuf_w_lock(rb);
	rc = NuRBufBaseWrite(rb->base, data, dataSz);
	rbuf_w_unlock(rb);
	return rc;
}

int NuRBufRecv(NuRBuf_t *rb, int fd, size_t needSz, size_t *readSz, int flags) {
	int rc = 0;
	rbuf_w_lock(rb);
	rc = NuRBufBaseRecv(rb->base, fd, needSz, readSz, flags);
	rbuf_w_unlock(rb);
	return rc;
}

int NuRBufRecvInTime(NuRBuf_t *rb, int fd, size_t needSz, size_t *readSz, int waitSec) {
	int rc = 0;
	rbuf_w_lock(rb);
	rc = NuRBufBaseRecvInTime(rb->base, fd, needSz, readSz, waitSec);
	rbuf_w_unlock(rb);
	return rc;
}

size_t NuRBufRead(NuRBuf_t *rb, void *data, size_t needSz) {
	return rbuf_try_read_data(rb, data, needSz, false);
}

size_t NuRBufTryRead(NuRBuf_t *rb, void *data, size_t needSz) {
	return rbuf_try_read_data(rb, data, needSz, true);
}

size_t NuRBufSkip(NuRBuf_t *rb, size_t skipSz) {
	return rbuf_skip_data(rb, skipSz);
}

