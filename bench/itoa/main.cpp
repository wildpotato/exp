#include <iostream>
#include <cstring>
#include <string>
#include <cstdlib>
#include <unordered_map>
#include <chrono>

#include "tbl_itoa.h"

class Timer
{
public:
    Timer() : beg_(clock_::now()) {}
    void reset() { beg_ = clock_::now(); }
    double elapsed() const {
        return std::chrono::duration_cast<second_>
            (clock_::now() - beg_).count(); }

private:
    typedef std::chrono::high_resolution_clock clock_;
    typedef std::chrono::duration<double, std::ratio<1> > second_;
    std::chrono::time_point<clock_> beg_;
};

/* fast number to string convert */
#define NumTable    "0123456789"
#define NumCopyToStr(T, S, V, L) do { \
    unsigned int _idx = L; \
    T _val = V; \
    do{ \
        V = _val/10; \
        S[_idx - 1]=NumTable[(_val - V *10)]; \
        _val = V; \
        --_idx; \
    } while(_idx>0); \
} while(0)

int NuUIntCopyToLPadZero(char *str, unsigned int val, int padLen)
{
    NumCopyToStr(unsigned int, str, val, padLen);
    return padLen;
}

int main(int argc, char **argv)
{
	const int C_STR_SZ = 10;
	const int N = 1e1;
	const int MAX_INT = 9;
	char *c_str_arr[MAX_INT+1];
	Timer timer;
	double t1, t2;
	char *c_str = (char *) malloc(sizeof(char) * C_STR_SZ);

	timer.reset();
	// Benchmark approach 1
	std::cout << t1 << std::endl;
	for (int i = 0; i < N; ++i)
	{
		c_str = m_itoa[i];
	}
	t1 = timer.elapsed();

	std::cout << "Approach 1: " << t1 << std::endl;

	// Benchmark appraoch 2
	timer.reset();
	for (unsigned int i = 0; i < N; ++i)
	{
		NuUIntCopyToLPadZero(c_str, i, 0);
	}
	t2 = timer.elapsed();
	std::cout << "Approach 2: " << t2 << std::endl;

	return 0;
}
