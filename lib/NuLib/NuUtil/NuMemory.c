
#include "NuUtil.h"
#include "NuMemory.h"

int NuMemoryNew(NuMemory_t **mem, size_t initCap) {
    int RC = NU_OK;
    NuMemory_t *ptr = NULL;
    
    ptr = (NuMemory_t *)malloc(sizeof(NuMemory_t));
    if (ptr == NULL) {
        NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
    } else {
        ptr->data = (char *)(malloc(NU_ALIGN8(sizeof(char) * initCap)));
        if (ptr->data == NULL) {
            NUGOTO(RC, NU_MALLOC_FAIL, EXIT);
        } else{
            ptr->cap    = initCap;
            ptr->length = 0;
        }
    }
EXIT:

    if (RC < 0) {
        NuMemoryFree(ptr);
        *mem = NULL;
    } else {
        *mem = ptr;
    }
    return RC;
}

void NuMemoryFree(NuMemory_t *mem) {
    if (mem != NULL) {
        if (mem->data != NULL) {
            free(mem->data);
        }
        free(mem);
    }
    return;
}


int NuMemoryAppend(NuMemory_t *mem, void *data, size_t dataLen) {
    size_t len = mem->length + dataLen;
    if (len > mem->cap) {
        size_t sz = len + ((mem->cap > 1024) ? 1024 : mem->cap);
        char *ptr = mem->data;
        mem->data = (char *)realloc(mem->data, NU_ALIGN8(sz));
        if (mem->data == NULL) {
            mem->data = ptr;
            return NU_MALLOC_FAIL;
        } 
        mem->cap = sz;
    }

    memcpy((mem->data + mem->length), data, dataLen);
    mem->length = len;

    return mem->length;
}

