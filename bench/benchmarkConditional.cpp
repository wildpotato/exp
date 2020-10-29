#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <numeric>
#include <iomanip>

using std::vector;
using std::string;

const double zero_threshold = 1e-9;
const int NUM_OF_COMPARISONS =   2;
const int INT_NORM_METHOD =      0;
const int INT_BIT_METHOD =       1;


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
            std::cout << "Results are equal!\n";
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
        //std::cout << __func__ << ": " << elapsed_seconds.count() << std::endl;
        time[INT_NORM_METHOD][iter] = elapsed_seconds.count();
    }

    void bitComp(int iter)
    {
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < intArr.size(); ++i) {
            intRet2[i] = intArr[i] - 1;
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end-start;
        //std::cout << __func__ << ": " << elapsed_seconds.count() << std::endl;
        time[INT_BIT_METHOD][iter] = elapsed_seconds.count();
    }

    void computeAverageTime()
    {
        for (int i = 0; i < NUM_OF_COMPARISONS; ++i) {
            double sum = std::accumulate(time[i].begin(), time[i].end(), zero_threshold);
            //std::cout << __func__ << ": sum = " << sum << std::endl;
            avg_time[i] = sum / time[i].size();
            std::cout << "Average time using " << method_type[i] << " is " <<
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
            std::cout << "Percent Saving in Time = " << std::fixed <<
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
    vector<string>method_type = {"Integer Normal Conditional", "Integer Bit Computation"};
};

int main()
{
    const int num_iter = 1e2;
    const int size = 1e6;

    Benchmark compare(num_iter, size);

    compare.generateReport();

    return 0;
}
