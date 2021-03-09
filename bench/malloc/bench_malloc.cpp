#include <iostream>
#include <cstdlib>
#include <vector>
#include <functional>
#include <time.h>

#include "matplotlibcpp.h"
#include "mimalloc.h"
#include "tcmalloc.h"

namespace plt = matplotlibcpp;

enum Mode {
    MALLOC = 0,
    TC_MALLOC = 1,
    MI_MALLOC = 2
};

constexpr int one_million = 1e6;
constexpr int number_of_sets = 1e6;
constexpr int regular_ascii_code = 128;

class Test {
    public:
        Test(enum Mode m, int i, int sz) : mode(m), iter(i), size(sz) { time.resize(size); }
        void run() {
            switch (mode) {
                case MALLOC:
                    run_test(malloc, free);
                    break;
                case TC_MALLOC:
                    run_test(tc_new, tc_delete);
                    break;
                case MI_MALLOC:
                    run_test(mi_malloc, mi_free);
                    break;
                default:
                    std::cout << "[ERR] invalid mode\n";
                    break;
            }
        }
        enum Mode mode;
        int iter;
        int size;
        std::vector<int> time;
    private:
        void run_test(std::function<void *(size_t)> alloc_fn, std::function<void (void *)> free_fn)
        {
            for (int i = 0 ; i < iter; ++i) {
                struct timeval start, end;
                char * buffer;
                int sec_elapsed = 0;
                int usec_elapsed = 0;

                gettimeofday(&start, NULL);
                buffer = (char *) alloc_fn(size);
                if (buffer == NULL) exit (1);
                do_task(buffer);
                free_fn(buffer);
                gettimeofday(&end, NULL);

                sec_elapsed = end.tv_sec - start.tv_sec;
                usec_elapsed = end.tv_usec - start.tv_usec;
                time[i] = sec_elapsed * one_million + usec_elapsed;
            }
        }
        void do_task(char *buffer)
        {
            for (int i = 0; i < number_of_sets; ++i)
                buffer[0] = i % regular_ascii_code;
        }
};

void plot(Test *t) {
    std::vector<int> x (t->size, 0);
    plt::plot(x, t->time, "o");
    plt::show();
}

int main() {
    constexpr int iteration = 100;
    constexpr int size = 8192;
    std::vector<Test *> tests;
    tests.resize(3);
    tests[0] = new Test(MALLOC, iteration, size);
    tests[1] = new Test(TC_MALLOC, iteration, size);
    tests[2] = new Test(MI_MALLOC, iteration, size);

    for (auto t : tests) {
        t->run();
    }
    plot(tests[0]);
    return 0;
}
