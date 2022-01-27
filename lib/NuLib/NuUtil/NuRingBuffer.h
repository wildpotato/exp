
#ifndef _NURINGBUFFER_H
#define _NURINGBUFFER_H

#include "NuCommon.h"
#include "NuUtil.h"
#include "NuSpin.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ring buffer base
 * -------------------------------------------------------------------- */
typedef struct _NuRBufBase_t {
	size_t     head;      /* the distance that pointer to buffer head */
	size_t     tail;      /* the distance that pointer to buffer end  */
	size_t     caps;      /* the maximum number of element in buffer  */
	char       *buffer;   /* pointer to ring buffer in memory         */
} NuRBufBase_t;

/** 
 * ring buffer create function 
 * @param[in ] sz : buffer size, it will be power of 2. 
 *
 * @return        : pointer of NuRBuf_t, if create fail will be NULL.
 */ 
NuRBufBase_t *NuRBufBaseNew(size_t sz);

/** 
 * ring buffer free function 
 * @param[in ] rb : the pointer of NuRBuf_t
 *
 * @return        : void
 */ 
void NuRBufBaseFree(NuRBufBase_t *rb);

/** 
 * get ring buffer capacity
 * @param[in ] rb : the pointer of NuRBufBase_t
 *
 * @return        : bytes of capacity.
 */ 
size_t NuRBufBaseGetCapicity(NuRBufBase_t *rb);

/** 
 * ring buffer free function 
 * @param[in ] rb : the pointer of NuRBufBase_t
 *
 * @return        : bytes of available data.
 */ 
size_t NuRBufBaseGetSize(NuRBufBase_t *rb);

/** 
 * write data to ring buffer. 
 * @param[in ] rb     : pointer of NuRBufBase_t.
 * @param[in ] data   : pointer of data source. 
 * @param[in ] dataSz : the number of bytes that want to write. 
 *
 * @return : bytes of writen successful. if the buffer not enougth, will be 0.
 */ 
size_t NuRBufBaseWrite(NuRBufBase_t *rb, char *data, size_t dataSz);

/** 
 * read data from ring buffer. it will be remove data from buffer.
 * @param[in ] rb     : pointer of NuRBufBase_t.
 * @param[out] data   : pointer of data destination.
 * @param[in ] needSz : the number of bytes that want to read. 
 *
 * @return : bytes of read successful. if the buffer not enougth, will be 0.
 */ 
size_t NuRBufBaseRead(NuRBufBase_t *rb, void *data, size_t needSz);

/** 
 * read data from ring buffer. it will not be remove data from buffer.
 * @param[in ] rb     : pointer of NuRBufBase_t.
 * @param[out] data   : pointer of data destination.
 * @param[in ] needSz : the number of bytes that want to read. 
 *
 * @return : bytes of read successful. if the buffer not enougth, will be 0.
 */ 
size_t NuRBufBaseTryRead(NuRBufBase_t *rb, void *data, size_t needSz);


#define NURB_TCP_QUICKACK 1 << 0
/** 
 * read data to ring buffer from file description.
 * @param[in ] rb     : pointer of NuRBufBase_t.
 * @param[in ] fd     : file description. 
 * @param[in ] needSz : the number of bytes that want to write. 
 *                      if dataSz equal 0, it will try to read as much as possible. 
 * @param[out] readSz : the number of bytes that be read. if readSz equal 0, the errno is EAGAIN
 * @param[in ] flags  : The flags argument is the bitwise OR of zero or more of the following flags. 
 *                      NURB_TCP_QUICKACK : turn on TCP_QUICKACK for tcp/ip fd. 
 *
 *
 * @return : fd read status.
 *           if the status less than zero. it's fail. please check errno.
 *           if the status equal to zero. there have data be read, readSz is the bytes of read.
 */ 
int NuRBufBaseRecv(NuRBufBase_t *rb, int fd, size_t needSz, size_t *readSz, int flags);

/** 
 * read data to ring buffer from file description.
 * it will be do select check before read from fd with wait sec. 
 * if there are data need to read, will read one time and return how many bytes be read.
 * @param[in ] rb     : pointer of NuRBufBase_t.
 * @param[in ] fd     : file description. 
 * @param[in ] needSz : the number of bytes that want to write. 
 *                      if dataSz equal 0, it will try to read as much as possible. 
 * @param[out] readSz : the number of bytes that be read. if readSz equal 0, the errno is EAGAIN
 *
 * @return : fd read status.
 *           if the status less than zero. it's fail. please check errno.
 *           if the status equal to zero. there have data be read, readSz is the bytes of read.
 */ 
int NuRBufBaseRecvInTime(NuRBufBase_t *rb, int fd, size_t needSz, size_t *readSz, int waitSec);

/** 
 * skip data from ring buffer. it will be remove data from buffer.
 * @param[in ] rb     : pointer of NuRBufBase_t.
 * @param[in ] skipSz : the number of bytes that want to skip.
 *
 * @return : bytes of skip successful. if the buffer not enougth, will be 0.
 */ 
size_t NuRBufBaseSkip(NuRBufBase_t *rb, size_t skipSz);

/**
 * ring buffer (thread safe)
 * -------------------------------------------------------------------- */
typedef struct _NuRingBuffer_t {
	NuRBufBase_t *base;

	NuSpin_t     rlock;  
	NuSpin_t     wlock;  
	NuSpin_t     lock;  
} NuRBuf_t;

/** 
 * ring buffer create function 
 * @param[in ] sz : buffer size, it will be power of 2. 
 *
 * @return        : pointer of NuRBuf_t, if create fail will be NULL.
 */ 
NuRBuf_t *NuRBufNew(size_t sz);

/** 
 * ring buffer free function 
 * @param[in ] rb : the pointer of NuRBuf_t
 *
 * @return        : void
 */ 
void NuRBufFree(NuRBuf_t *rb);

/** 
 * get ring buffer capacity
 * @param[in ] rb : the pointer of NuRBuf_t
 *
 * @return        : bytes of capacity.
 */ 
size_t NuRBufGetCapicity(NuRBuf_t *rb);

/** 
 * ring buffer free function 
 * @param[in ] rb : the pointer of NuRBuf_t
 *
 * @return        : bytes of available data.
 */ 
size_t NuRBufGetSize(NuRBuf_t *rb);

/** 
 * write data to ring buffer. 
 * @param[in ] rb     : pointer of NuRBuf_t.
 * @param[in ] data   : pointer of data source. 
 * @param[in ] dataSz : the number of bytes that want to write. 
 *
 * @return : bytes of writen successful. if the buffer not enougth, will be 0.
 */ 
size_t NuRBufWrite(NuRBuf_t *rb, char *data, size_t dataSz);

/** 
 * read data from ring buffer. it will be remove data from buffer.
 * @param[in ] rb     : pointer of NuRBuf_t.
 * @param[out] data   : pointer of data destination.
 * @param[in ] needSz : the number of bytes that want to read. 
 *
 * @return : bytes of read successful. if the buffer not enougth, will be 0.
 */ 
size_t NuRBufRead(NuRBuf_t *rb, void *data, size_t needSz);

/** 
 * read data from ring buffer. it will not be remove data from buffer.
 * @param[in ] rb     : pointer of NuRBuf_t.
 * @param[out] data   : pointer of data destination.
 * @param[in ] needSz : the number of bytes that want to read. 
 *
 * @return : bytes of read successful. if the buffer not enougth, will be 0.
 */ 
size_t NuRBufTryRead(NuRBuf_t *rb, void *data, size_t needSz);

/** 
 * read data to ring buffer from file description.
 * @param[in ] rb     : pointer of NuRBufBase_t.
 * @param[in ] fd     : file description. 
 * @param[in ] needSz : the number of bytes that want to write. 
 *                      if dataSz equal 0, it will try to read as much as possible. 
 * @param[out] readSz : the number of bytes that be read. if readSz equal 0, the errno is EAGAIN
 * @param[in ] flags  : The flags argument is the bitwise OR of zero or more of the following flags. 
 *                      NURB_TCP_QUICKACK : turn on TCP_QUICKACK for tcp/ip fd. 
 *
 * @return : fd read status.
 *           if the status less than zero. it's fail. please check errno.
 *           if the status equal to zero. there have data be read, readSz is the bytes of read.
 */ 
int NuRBufRecv(NuRBuf_t *rb, int fd, size_t needSz, size_t *readSz, int flags);

/** 
 * read data to ring buffer from file description.
 * it will be do select check before read from fd with wait sec. 
 * if there are data need to read, will read one time and return how many bytes be read.
 * @param[in ] rb     : pointer of NuRBufBase_t.
 * @param[in ] fd     : file description. 
 * @param[in ] needSz : the number of bytes that want to write. 
 *                      if dataSz equal 0, it will try to read as much as possible. 
 * @param[out] readSz : the number of bytes that be read. if readSz equal 0, the errno is EAGAIN
 *
 * @return : fd read status.
 *           if the status less than zero. it's fail. please check errno.
 *           if the status equal to zero. there have data be read, readSz is the bytes of read.
 */ 
int NuRBufRecvInTime(NuRBuf_t *rb, int fd, size_t needSz, size_t *readSz, int waitSec);

/** 
 * skip data from ring buffer. it will be remove data from buffer.
 * @param[in ] rb     : pointer of NuRBuf_t.
 * @param[in ] skipSz : the number of bytes that want to skip.
 *
 * @return : bytes of skip successful. if the buffer not enougth, will be 0.
 */ 
size_t NuRBufSkip(NuRBuf_t *rb, size_t skipSz);


#ifdef __cplusplus
}
#endif

#endif /* _NURINGBUFFER_H */

