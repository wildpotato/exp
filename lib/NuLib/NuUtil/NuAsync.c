
#include "NuAsync.h"

int NuAsyncQueNew(NuAsyncQue_t **que, int queCnt) {
	int rc = 0;
	NuAsyncQue_t *q = NULL;

	q = (NuAsyncQue_t *)malloc(sizeof(NuAsyncQue_t));
	if (q == NULL) {
		NUGOTO(rc, -1, EXIT);
	} 
	memset(q, 0x00, sizeof(NuAsyncQue_t));


	q->rBuf = NuRBufBaseNew(sizeof(NuAsync_Delegate_t) * queCnt);
	if (q->rBuf == NULL) {
		NUGOTO(rc, -2, EXIT);
	}

	q->isOpen = true;
	q->caps   = NuRBufBaseGetCapicity(q->rBuf) / sizeof(NuAsync_Delegate_t);

    NuMutexInit(&(q->wLock));
    NuMutexInit(&(q->rLock));
    NuCondVarInit(&(q->fullBlocking));
    NuCondVarInit(&(q->emptyBlocking));

	(*que) = q;
EXIT:	
	if (rc < 0) {
		NuAsyncQueFree(q);
		(*que) = NULL;
	}
	return rc;
}

void NuAsyncQueFree(NuAsyncQue_t *que) {
	if (que != NULL) {
		if (que->rBuf != NULL) {
			NuRBufBaseFree(que->rBuf);
		}
		free(que);
	}
	return;
}

void NuAsyncQueClose(NuAsyncQue_t *que) {
	if (que != NULL) {
		if (que->isOpen == true) {
			que->isOpen = false;
			NuCondVarWake(&(que->emptyBlocking));
			NuCondVarWake(&(que->fullBlocking));
		}
	}
	return;
}

int NuAsyncQueEnqueue(NuAsyncQue_t *que, NuAsync_Delegate_t *dlg) {
    int rc = NU_OK;

    NuMutexLock(&(que->wLock));

    while (que->isOpen) {
		rc = NuRBufBaseWrite(que->rBuf, (char *)dlg, sizeof(NuAsync_Delegate_t));
		if (rc <= 0) {
			if (que->isOpen == false) {
				NUGOTO(rc, NU_ENQINTERRUPT, EXIT);
			}
            NuCondVarBlock(&(que->fullBlocking));
		} else {
			NuCondVarWake(&(que->emptyBlocking));
			NUGOTO(rc, NU_OK, EXIT);
		}
    } 
	NUGOTO(rc, NU_FAIL, EXIT);

EXIT:
    NuMutexUnLock(&(que->wLock));
    return rc;
}

int NuAsyncQueDequeue(NuAsyncQue_t *que, NuAsync_Delegate_t *dlg) {
    int rc = NU_OK;

    NuMutexLock(&(que->rLock));

    while (que->isOpen) {
		rc = NuRBufBaseRead(que->rBuf, (char *)dlg, sizeof(NuAsync_Delegate_t));
		if (rc <= 0) {
			if (que->isOpen == false) {
				NUGOTO(rc, NU_DEQINTERRUPT, EXIT);
			} 
            NuCondVarBlock(&(que->emptyBlocking));
		} else {
			NuCondVarWake(&(que->fullBlocking));
			NUGOTO(rc, NU_OK, EXIT);
		}
    } 
	NUGOTO(rc, NU_FAIL, EXIT);

EXIT:
    NuMutexUnLock(&(que->rLock));
    return rc;
}

/* --------------------------------------------------------------------- */
typedef struct _NuAsyncThd_t {
	NuThread_t TID;
	int        BindCpuID;
	void       *owner;
} NuAsyncThd_t; 

int NuAsyncNew(NuAsync_t **async, int queCnt) {
	int rc =NU_OK;
	NuAsync_t *obj = NULL;

	obj = (NuAsync_t *)malloc(sizeof(NuAsync_t));
	if (obj == NULL) {
		NUGOTO(rc, NU_FAIL, EXIT);
	} else {
		memset(obj, 0x00, sizeof(NuAsync_t));

		obj->isStop = true;

		rc = NuAsyncQueNew(&(obj->Queue), queCnt);
		if (rc < 0) {
			NUGOTO(rc, NU_FAIL, EXIT);
		}

		rc = base_vector_init(&(obj->vThds), 8);
		if (rc < 0) {
			NUGOTO(rc, NU_FAIL, EXIT);
		}
	}

	(*async) = obj;
EXIT:

	if (rc < 0) {
		NuAsyncFree(obj);
		(*async) = NULL;
	}
	return rc;
}

void NuAsyncFree(NuAsync_t *async) {
	if (async != NULL) {
		if (async->isStop == false) {
			NuAsyncStop(async);
		}

		if (async->Queue != NULL) {
			NuAsyncQueFree(async->Queue);
		}

		base_vector_destroy(&(async->vThds));
		free(async);
	}
}

int NuAsyncAddThreadBindCore(NuAsync_t *async, int cpuID) {
	int rc = 0;
	NuAsyncThd_t *thd = NULL;
	thd = (NuAsyncThd_t *)malloc(sizeof(NuAsyncThd_t));
	if (thd == NULL) {
		NUGOTO(rc, NU_FAIL, EXIT);	
	}

	thd->TID = 0;
	thd->BindCpuID = cpuID;
	thd->owner = (void *)async;

	rc = base_vector_push(&(async->vThds), (const void *)thd);

EXIT:
	return rc;
}

int NuAsyncAddThread(NuAsync_t *async, int cnt) {
	int rc = 0;

	for (int i = 0; i < cnt; i++) {
		rc = NuAsyncAddThreadBindCore(async, -1);
		if (rc < 0) {
			return rc;
		}
	}
	return rc;
}

static void *_bgThdWorkFn(void *args) {
	int rc = 0;
	NuAsyncThd_t   *thd = (NuAsyncThd_t *)args;
	NuAsync_t    *async = (NuAsync_t *)thd->owner;

	NuAsync_Delegate_t dlg;

	dlg.CBFn = NULL;
	dlg.Argu = NULL;

	while(async->isStop == false) {
		rc = NuAsyncQueDequeue(async->Queue, &dlg);
		if (rc != NU_OK) {
			break;
		} else {
			dlg.CBFn(dlg.Argu);
			dlg.CBFn = NULL;
			dlg.Argu = NULL;
		}
	}
	NuThdReturn();
	return NULL;
}

int NuAsyncStart(NuAsync_t *async) {
	int rc = 0;
	int thdCnt = base_vector_get_cnt(&(async->vThds));

	async->isStop = false;

	if (thdCnt > 0) {
		int i = 0; 
		NuAsyncThd_t *thd = NULL;

		for (i = 0; i < thdCnt; i++) {
			thd = (NuAsyncThd_t *)base_vector_get_by_index(&(async->vThds), i);
			rc = NuThdCreate(&_bgThdWorkFn, (void *)thd, &(thd->TID));
			if (rc < 0) {
				NUGOTO(rc, NU_FAIL, EXIT);
			}
		}
	} else {
		NUGOTO(rc, NU_FAIL, EXIT);
	}

	rc = NU_OK;
EXIT:
	return rc;
}

void NuAsyncStop(NuAsync_t *async) {
	int thdCnt = base_vector_get_cnt(&(async->vThds));

	async->isStop = true;
	NuAsyncQueClose(async->Queue);

	if (thdCnt > 0) {
		int i = 0; 
		NuAsyncThd_t *thd = NULL;
		for (i = 0; i < thdCnt; i++) {
			thd = (NuAsyncThd_t *)base_vector_get_by_index(&(async->vThds), i);
			if (thd->TID > 0) {
				NuThdJoin(thd->TID);
			}
			free(thd);
		}
		base_vector_clear(&(async->vThds));
	}
}

int NuAsyncInvoke(NuAsync_t *async, NuEventFn fn, void *argu) {
	NuAsync_Delegate_t dlg;
	dlg.CBFn = fn;
	dlg.Argu = argu;
	return NuAsyncQueEnqueue(async->Queue, &dlg);
}
