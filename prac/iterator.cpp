#include <iostream>
#include <vector>
#include <cstdlib>
#include <map>

using namespace std;

template <typename T>
void PrintArr2D(vector<vector<T>> &arr)
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

template <typename T>
void PrintMap(map<T, T> &map)
{
	cout << "---------------------------------------------\n";
	for (const auto &m : map)
	{
		cout << "{" << m.first << "," << m.second << "}" << endl;
	}
	cout << "---------------------------------------------\n";
}

template <typename T>
void PrintKV(pair<T, T> &pair)
{
	cout << "---------------------------------------------\n";
	cout << "{" << pair.first << "," << pair.second << "}" << endl;
	cout << "---------------------------------------------\n";
}

template <typename T>
void PrintArr1D(vector<T> &arr)
{
	cout << "---------------------------------------------\n";
	cout << "{";
	for (size_t i = 0; i < arr.size(); ++i)
	{
		{
			cout << arr[i];
			if (i != arr.size()-1)
				cout << ",";
		}
	}
	cout << "}\n";
	cout << "---------------------------------------------\n";
}

int main() {
	constexpr int ROW = 12;
	constexpr int COL = 3;
	constexpr int MOD = 10;
	vector<vector<int>> arr(ROW, vector<int>(COL, 0));
	for (int i = 0; i < ROW; ++i) {
		for (int j = 0; j < COL; ++j) {
			arr[i][j] = rand() % MOD;
		}
	}

	PrintArr2D(arr);
	auto itr_arr1D = arr.begin();
	PrintArr1D(*itr_arr1D);

	map<int, int> map{{1,6},{9,6},{6,3},{4,9},{3,8},{5,2},{7,1},{2,2},{3,1},{8,4}};
	PrintMap(map);
	auto itr_kv = map.upper_bound(5.5);
	PrintKV(*itr_kv);

	return 0;
}
