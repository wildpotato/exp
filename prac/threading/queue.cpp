#include <iostream>
#include <thread>

#include "circular.hpp"

using namespace std;

CircularBuffer<int> cbuf(2);

void t1_func() {
	for (int i = 0; i < 10; ++i) {
		int val = 2 * i + 1;
		this_thread::sleep_for(chrono::duration<int, milli>(800));
		cbuf.put(val);
		cout << "thread 1 put " << val << "\n";
	}
	cout << "Bye thread 1\n";
}

void t2_func() {
	for (int i = 0; i < 10; ++i) {
		int val = 2 * i;
		this_thread::sleep_for(chrono::duration<int, milli>(605));
		cbuf.put(val);
		cout << "thread 2 put " << val << "\n";
	}
	cout << "Bye thread 2\n";
}

void t3_func() {
	for (int i = 0; i < 100; ++i) {
		std::pair<int, int> val = cbuf.peekFirstNLast();
		cout << "First item: " << val.first << "   ";
		cout << "Last Item: " << val.second << "\n";
		this_thread::sleep_for(chrono::duration<int, milli>(100));
	}
	cout << "Bye thread 3\n";
}

int main() {
	thread t1(t1_func);
	thread t2(t2_func);
	thread t3(t3_func);
	cout << "join t1\n";
	t1.join();
	cout << "join t2\n";
	t2.join();
	cout << "join t3\n";
	t3.join();
	cout << "All finished\n";
	return 0;
}
