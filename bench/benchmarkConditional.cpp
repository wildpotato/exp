#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <numeric>
#include <iomanip>

using std::vector;
using std::string;
using std::cout;

constexpr double ZERO_THRESHOLD = 1e-9; // use this as zero in std::accumulate
constexpr int NUM_OF_COMPARISONS =   2; // number of ways to do the same thing
constexpr int INT_COND_METHOD =      0; // method using conditional
constexpr int INT_LESS_METHOD =      1; // method using normal subtraction/less


class Benchmark
{
public:
    Benchmark(int num_iter, int size) : _num_iter(num_iter), _sz(size)
    {
        intArr.resize(_sz, 0);
        intRet1.resize(_sz, 0);
        intRet2.resize(_sz, 0);
        time.resize(NUM_OF_COMPARISONS, vector<double>(_num_iter, 0));
        avg_time.resize(NUM_OF_COMPARISONS);
        intResult.resize(_num_iter, false);
    }

    void generateReport()
    {
        for (int i = 0; i < _num_iter; ++i) {
            initMyArr();
            normalComp(i);
            bitComp(i);
        }
        checkResult();
        if (is_result_correct) {
            cout << "Results are equal!\n";
        } else {
            cout << "Something went wrong..\n";
            return;
        }
        computeAverageTime();
        computeSavingPercent();
    }

    void checkResult() {
        for (int j = 0; j < NUM_OF_COMPARISONS; ++j) {
            bool curr_iter_match = true;
            for (int i = 0; i < intRet1.size(); ++i) {
                if (intRet1[i] != intRet2[i]) {
                    is_result_correct = false;
                    curr_iter_match = false;
                }
            }
            if (curr_iter_match) {
                intResult[j] = true;
            }
        }
    }

    void initMyArr()
    {
        for (int i = 0; i < intArr.size(); ++i) {
            intArr[i] = rand() % 2 + 1;
        }
    }

    void normalComp(int iter)
    {
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < intArr.size(); ++i) {
            intRet1[i] = (intArr[i] == 1) ? 0 : 1;
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end-start;
        //cout << __func__ << ": " << elapsed_seconds.count() << std::endl;
        time[INT_COND_METHOD][iter] = elapsed_seconds.count();
    }

    void bitComp(int iter)
    {
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < intArr.size(); ++i) {
            intRet2[i] = intArr[i] - 1;
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end-start;
        //cout << __func__ << ": " << elapsed_seconds.count() << std::endl;
        time[INT_LESS_METHOD][iter] = elapsed_seconds.count();
    }

    void computeAverageTime()
    {
        for (int i = 0; i < NUM_OF_COMPARISONS; ++i) {
            double sum = std::accumulate(time[i].begin(), time[i].end(), ZERO_THRESHOLD);
            //cout << __func__ << ": sum = " << sum << std::endl;
            avg_time[i] = sum / time[i].size();
            cout << "Average time using " << method_type[i] << " is " <<
                avg_time[i] << std::endl;
        }
    }

    void computeSavingPercent()
    {
        for (int i = 0; i < NUM_OF_COMPARISONS; i+=2) {
            double base = std::max(avg_time[i], avg_time[i+1]);
            double diff = avg_time[i] > avg_time[i+1] ?
            avg_time[i] - avg_time[i+1] : avg_time[i+1] - avg_time[i];
            double percent = diff / base;
            cout << "Percent Saving in Time = " << std::fixed <<
            std::setprecision(2) << percent * 100 << "%" << std::endl;
        }
    }

private:
    const int _num_iter;
    const int _sz;
    bool is_result_correct = true;
    vector<uint8_t> intArr;
    vector<uint8_t> intRet1;
    vector<uint8_t> intRet2;
    vector<vector<double>>time;
    vector<double> avg_time;
    vector<bool> intResult;
    vector<string>method_type = {"Integer Conditional", "Integer Subtraction"};
};

int main()
{
    constexpr int num_iter = 1e3;
    constexpr int size = 1e6;
    Benchmark compare(num_iter, size);
    compare.generateReport();
    return 0;
}
