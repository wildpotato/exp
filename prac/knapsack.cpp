#include <iostream>
#include <vector>
#include <utility>

using namespace std;

struct Item {
	int size;
	int value;
};

template <typename T>
void PrintArr(vector<T> &arr) {
	cout << "\n-------------------------\n";
	for (const auto &t : arr) {
		cout << t << " ";
	}
	cout << "\n-------------------------\n";
}

int knapBU(int cap, vector<Item> &items, int n) {
	int space;
	vector<int> maxKnown(cap + 1, 0);
	for (int i = 1; i <= cap; ++i) {
		maxKnown[i] = maxKnown[i-1];
		for (int j = 0; j < n; ++j) {
			space = i - items[j].size;
			if ((space >= 0) && ((items[j].value + maxKnown[space]) > maxKnown[i])) {
				maxKnown[i] = items[j].value + maxKnown[space];
			}
		}
	}
	PrintArr(maxKnown);
	return maxKnown[cap];
}

int main() {
	// list of items with <size, value>
	vector<Item> items = {{2,3},{3,5},{4,4},{5,7},{6,9},{7,8}};
	int cap = 8;
	int profit = knapBU(cap, items, items.size());
	cout << "max value for cap = " << cap << " is " << profit << endl;
	return 0;
}
