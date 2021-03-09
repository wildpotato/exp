#include <iostream>
#include <cstdlib>
#include <vector>
#include <functional>

#include "mimalloc.h"
#include "tcmalloc.h"

enum Mode {
    MALLOC = 0,
    TC_MALLOC = 1,
    MI_MALLOC = 2
};

void run_once (std::function<void *(size_t)> alloc_fn, std::function<void (void *)> free_fn,
               std::string mode)
{
    std::vector<int> size {128, 512, 4096, 8192};
    std::vector<char *> buffer;
    int sz = size.size();
    buffer.resize(sz);

    for (int i = 0; i < sz; ++i) {
        buffer[i] = (char *) alloc_fn(size[i]);
        if (buffer[i] == NULL) exit (1);
        std::cout << mode << " New'd " << size[i] << "bytes\n";
    }

    for (int i = 0; i < sz; ++i) {
        free_fn(buffer[i]);
    }
}

int main() {
    enum Mode m = TC_MALLOC;
    switch (m) {
        case MALLOC:
            run_once(malloc, free, "malloc");
            break;
        case TC_MALLOC:
            run_once(tc_new, tc_delete, "tcmalloc");
            break;
        case MI_MALLOC:
            mi_version();
            run_once(mi_malloc, mi_free, "mimalloc");
            break;
        default:
            break;
    }
    return 0;
}
