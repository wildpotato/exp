
#include "NuSpin.h"

int NuSpinInit(NuSpin_t *mtx) {
    return pthread_spin_init(mtx, 0);
}

