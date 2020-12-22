/*
 * This program simulates the behavior of queuing transactions based with the time and
 * quantity associated with each transaction. The queue keeps track of all transactions
 * int the last TIME_WINDOW seconds and discards the older transactions. Here, a
 * double-ended queue (deque) is used to simulate the behavior of queue because we need
 * to print out the outputs and queue does not readily support iterators.
 *
 */

#include <iostream>
using std::cout;
using std::endl;

#include <vector>
using std::vector;

#include <deque>
using std::deque;

#include <utility>
using std::pair;
using std::make_pair;

#include <ctime>
#include <cstdlib>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

void printQueue(deque<pair<time_t, long>> &q, int iter, time_t now) {
    cout << "------ queue during the " << iter << " iteration ------ \n";
    cout << "current time = " << now << endl;
    for (const auto p : q) {
        cout << "time:" << p.first << " qty:" << p.second << endl;
    }
    cout << "------ queue during the " << iter << " iteration ------ \n";
}

int main() {
    constexpr int DURATION = 10;
    constexpr int MAX_TRANSACT_PER_SEC = 20;
    constexpr int MAX_QTY = 6;  // max quantity of each transaction
    constexpr int MIN_QTY = -6; // min quantity of each transaction
    constexpr int TIME_WINDOW = 3; // only keep the transactions in the last TIME_WINDOW seconds
    time_t now = 0;
    long sum = 0; // sum of all quantities in the last TIME_WINDOW seconds
    deque<pair<time_t, long>> que; // que stores pair of (time_t, qty) for each transaction
    vector<int> freq(DURATION, 0); // freq stores how many transactions occur within each second

    /* initialize number of transactions for each second of the duration */
    for (int i = 0; i < DURATION; ++i) {
        time(&now); // get number of seconds since epoch
        while (!que.empty() && (now - que.front().first > TIME_WINDOW)) {
            que.pop_front();
        }
        freq[i] = rand() % MAX_TRANSACT_PER_SEC;
        for (int j = 0; j < freq[i]; ++j) {
            que.push_back(make_pair(now, rand() % (MAX_QTY+abs(MIN_QTY)) - MAX_QTY));
        }
        printQueue(que, i+1, now);
        sleep(1);
    }
    return 0;
}
