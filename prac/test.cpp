#include <iostream>
#include <vector>
#include <numeric> // iota

using namespace std;

int main() {
    vector<double> vec(4);
    iota(vec.begin(), vec.end(), 0);
    for (auto &v: vec) {
        v += 0.5;
        cout << v << " ";
    }
    double sum = accumulate(vec.begin(), vec.end(), 0.1);
    cout << endl << "sum = " << sum << endl;
    return 0;
}
