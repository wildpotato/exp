#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <cassert>

using std::cout;
using std::vector;
using std::sort;
using std::function;

typedef function<void()> Test;
typedef vector<Test> TestSuites;

/*
 *  Requirement: Array must be non-empty and in sorted order
 *  Modifies:    Nothing
 *  Effect:      Returns index matching the target, -1 if no match found
 *
 */
int binarySearch(vector<int> &array, int lower_bound, int higher_bound, int target) {
    int low = lower_bound, high = higher_bound;
    while (low <= high) {
        int mid = (low + high) / 2;
        if (array[mid] < target) {
            low = mid + 1;
        } else if (array[mid] > target) {
            high = mid - 1;
        } else { // bingo!
            return mid;
        }
    } // while
    return -1;
}

void printArray(vector<int> &array) {
    for (int element : array) {
        cout << element << " ";
    }
    cout << "\n";
}

void test1() {
    vector<int> arr {1, 4, 8, 2, 7, 9, 0};
    sort(arr.begin(), arr.end());
    printArray(arr);
    cout << "4 at position " << binarySearch(arr, 0, arr.size() - 1, 4) << "\n";
    assert(binarySearch(arr, 0, arr.size(), 4) == 3);
}

void test2() {
    vector<int> arr {102, 74, 2, 3, 12, 81, 45};
    sort(arr.begin(), arr.end());
    printArray(arr);
    cout << "3 at position " << binarySearch(arr, 0, arr.size() - 1, 3) << "\n";
    assert(binarySearch(arr, 0, arr.size(), 3) == 1);
}

void test3() {
    vector<int> arr {6,5,4,3,2,1,0};
    sort(arr.begin(), arr.end());
    printArray(arr);
    cout << "6 at position " << binarySearch(arr, 0, arr.size() - 1, 6) << "\n";
    assert(binarySearch(arr, 0, arr.size(), 6) == 6);
}

void test4() {
    vector<int> arr {6,5,4,3,2,1,0};
    sort(arr.begin(), arr.end());
    printArray(arr);
    cout << "0 at position " << binarySearch(arr, 0, arr.size() - 1, 0) << "\n";
    assert(binarySearch(arr, 0, arr.size(), 0) == 0);
}

void test5() {
    vector<int> arr {6,5,4,3,2,1,0};
    sort(arr.begin(), arr.end());
    printArray(arr);
    cout << "1 at position " << binarySearch(arr, 0, arr.size() - 1, 1) << "\n";
    assert(binarySearch(arr, 0, arr.size(), 1) == 1);
}

void test6() {
    vector<int> arr {6,5,4,3,2,1,0};
    sort(arr.begin(), arr.end());
    printArray(arr);
    cout << "5 at position " << binarySearch(arr, 0, arr.size() - 1, 5) << "\n";
    assert(binarySearch(arr, 0, arr.size(), 5) == 5);
}

void test7() {
    vector<int> arr {6};
    sort(arr.begin(), arr.end());
    printArray(arr);
    cout << "6 at position " << binarySearch(arr, 0, arr.size() - 1, 6) << "\n";
    assert(binarySearch(arr, 0, arr.size(), 6) == 0);
}

void test8() {
    vector<int> arr {6};
    sort(arr.begin(), arr.end());
    printArray(arr);
    cout << "5 at position " << binarySearch(arr, 0, arr.size() - 1, 5) << "\n";
    assert(binarySearch(arr, 0, arr.size(), 5) == -1);
}

int main() {
    TestSuites tests;
    tests.push_back(test1);
    tests.push_back(test2);
    tests.push_back(test3);
    tests.push_back(test4);
    tests.push_back(test5);
    tests.push_back(test6);
    tests.push_back(test7);
    tests.push_back(test8);
    for (int i = 0; i < tests.size(); ++i) {
        tests[i]();
        cout <<"-------------------------------\n";
    }
    cout << "Successful\n";
    return 0;
}
