#include <iostream>
#include <vector>
#include <functional>
#include <algorithm>

using namespace std;

struct EndGreater {
	template <typename T>
	bool operator()(const vector<T> &a, const vector<T> &b)
	{
		if (a[1] > b[1]) {
			return true;
		} else if (a[1] == b[1]) {
			if (a[0] > b[0]) {
				return true;
			} else {
				return false;
			}
		} else {
			return false;
		}
	}
};

template <typename T>
void PrintArr(vector<vector<T>> &arr)
{
	cout << "---------------------------------------------\n";
	for (size_t i = 0; i < arr.size(); ++i)
	{
		cout << "{";
		for (size_t j = 0; j < arr[0].size()-1; ++j)
		{
			cout << arr[i][j] << ",";
		}
		cout << arr[i][arr[0].size()-1] << "}" << "\n";
	}
	cout << "---------------------------------------------\n";
}

int main() {
	vector<vector<int>> arr1 {{3,5}, {2,4}, {9,1}, {1,7}, {2,6}, {8,1}, {7,4}, {5,9}, {4,1}, {5,2}, {4,4}};
	PrintArr(arr1);
	sort(arr1.begin(), arr1.end(), EndGreater());
	PrintArr(arr1);
	return 0;
}
