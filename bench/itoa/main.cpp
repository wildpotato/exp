#include <iostream>
#include <cstring>
#include <string>
#include <cstdlib>
#include <unordered_map>
#include <chrono>
#include <algorithm>

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

// convert unsigned int to c-string, no null at end, return position after last char
template<typename Uint_type> char *ez_utoa(Uint_type number, char *output)
{
    char *b = output;
    do {
		std::cout << "*b=" << *b << std::endl;
        *b++ = '0' + (number % 10);
        number /= 10;
    } while(number);

    // Reverse the digits in-place.
    std::reverse(output, b);
    return b;
}

int main(int argc, char **argv)
{
	const int C_STR_SZ = 10;
	const int MAX_INT = 9999;
	char ** c_str_arr = (char **) malloc((MAX_INT + 1) * sizeof(char *));
	Timer timer;
	double t1, t2;

	for (int i = 0; i <= MAX_INT; ++i)
	{
		c_str_arr[i] = (char *) malloc(C_STR_SZ * sizeof(char));
	}

	timer.reset();
	// Benchmark approach 1
	for (int i = 0; i < MAX_INT; ++i)
	{
		c_str_arr[i] = m_itoa[i];
	}
	t1 = timer.elapsed();

	std::cout << "Approach 1: " << t1 << std::endl;

	// Benchmark appraoch 2
	timer.reset();
	for (unsigned int i = 0; i < MAX_INT; ++i)
	{
		ez_utoa(i, c_str_arr[i]);
	}
	t2 = timer.elapsed();
	std::cout << "Approach 2: " << t2 << std::endl;

	return 0;
}
