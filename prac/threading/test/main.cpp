#include <iostream>
#include <cassert>
#include "../circular.hpp"
#include "../spinlock.hpp"

void test_circularBuffer_single() {
	CircularBuffer<int> cb(1);
	assert(cb.size() == 0);
	assert(!cb.full());
	assert(cb.capacity() == 1);

	cb.put(100);
	assert(cb.peekFirstItem() == 100);
	assert(cb.peekLastItem() == 100);
	assert(cb.peekFirstNLast() == std::make_pair(100, 100));
	assert(cb.full());
	assert(cb.size() == cb.capacity() == 1);

	cb.reset();
	assert(!cb.full());
	assert(cb.size() == 0);
	assert(cb.capacity() == 1);
	assert(cb.peekFirstItem() == int());
	assert(cb.peekLastItem() == int());
	assert(cb.peekFirstNLast() == std::make_pair(int(), int()));
}

void test_circularBuffer_double() {
	CircularBuffer<double> cb(2);
	assert(cb.size() == 0);
	assert(!cb.full());
	assert(cb.capacity() == 2);

	cb.put(1.1);
	assert(!cb.full());
	assert(cb.peekFirstItem() == 1.1);
	assert(cb.peekLastItem() == 1.1);
	assert(cb.size() == 1);
	assert(cb.capacity() == 2);
	assert(cb.peekFirstNLast() == std::make_pair(1.1, 1.1));

	cb.put(2.2);
	assert(cb.peekFirstItem() == 2.2);
	assert(cb.peekLastItem() == 1.1);
	assert(cb.size() == 2);
	assert(cb.capacity() == 2);
	assert(cb.peekFirstNLast() == std::make_pair(2.2, 1.1));

	cb.put(3.3);
	assert(cb.peekFirstItem() == 3.3);
	assert(cb.peekLastItem() == 2.2);
	assert(cb.size() == 2);
	assert(cb.capacity() == 2);
	assert(cb.peekFirstNLast() == std::make_pair(3.3, 2.2));

	cb.put(4.4);
	assert(cb.peekFirstItem() == 4.4);
	assert(cb.peekLastItem() == 3.3);
	assert(cb.size() == 2);
	assert(cb.capacity() == 2);
	assert(cb.peekFirstNLast() == std::make_pair(4.4, 3.3));

	cb.put(5.5);
	assert(cb.peekFirstItem() == 5.5);
	assert(cb.peekLastItem() == 4.4);
	assert(cb.size() == 2);
	assert(cb.capacity() == 2);
	assert(cb.peekFirstNLast() == std::make_pair(5.5, 4.4));

	cb.reset();
	assert(!cb.full());
	assert(cb.size() == 0);
	assert(cb.capacity() == 2);
	assert(cb.peekFirstItem() == double());
	assert(cb.peekLastItem() == double());
	assert(cb.peekFirstNLast() == std::make_pair(double(), double()));
}

void test_circularBuffer_triple()
{
	CircularBuffer<int> cb(3);
	assert(cb.size() == 0);
	assert(!cb.full());
	assert(cb.capacity() == 3);

	cb.put(111);
	assert(!cb.full());
	assert(cb.peekFirstItem() == 111);
	assert(cb.peekLastItem() == 111);
	assert(cb.size() == 1);
	assert(cb.capacity() == 3);
	assert(cb.peekFirstNLast() == std::make_pair(111, 111));

	cb.put(222);
	assert(!cb.full());
	assert(cb.peekFirstItem() == 222);
	assert(cb.peekLastItem() == 111);
	assert(cb.size() == 2);
	assert(cb.capacity() == 3);
	assert(cb.peekFirstNLast() == std::make_pair(222, 111));

	cb.put(333);
	assert(cb.full());
	assert(cb.peekFirstItem() == 333);
	assert(cb.peekLastItem() == 111);
	assert(cb.size() == 3);
	assert(cb.capacity() == 3);
	assert(cb.peekFirstNLast() == std::make_pair(333, 111));

	cb.put(444);
	assert(cb.full());
	assert(cb.peekFirstItem() == 444);
	assert(cb.peekLastItem() == 222);
	assert(cb.size() == 3);
	assert(cb.capacity() == 3);
	assert(cb.peekFirstNLast() == std::make_pair(444, 222));

}

int main() {
	test_circularBuffer_single();
	test_circularBuffer_double();
	test_circularBuffer_triple();
	std::cout << "All tests passed!\n";
	return 0;
}
