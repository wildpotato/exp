#include <stdio.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdlib.h>

#include "google/cmockery.h"

extern void* _test_malloc(const size_t size, const char* file, const int line);
extern void* _test_calloc(const size_t number_of_elements, const size_t size,
                          const char* file, const int line);
extern void _test_free(void* const ptr, const char* file, const int line);

#define malloc(size) _test_malloc(size, __FILE__, __LINE__)
#define calloc(num, size) _test_calloc(num, size, __FILE__, __LINE__)
#define free(ptr) _test_free(ptr, __FILE__, __LINE__)

#include "NuStrTest.c"
#include "NuCStrTest.c"

int main()
{
	const UnitTest tests[] = 
	{
		unit_test(nustr_test),
		unit_test(nustr_tools_test),
		unit_test(nucstr_test),
	};

	return run_tests(tests);
}

#undef malloc
#undef calloc
#undef free

