/*
 * Desrciption: Example benchmark program
 *
 * Date: 2020/10/22
 *
 * Author: Johnny Chan 180762 #6389
 *
 * In bestprice.c: _SendQuote_BestPrice() we see
 * [1st case] const int quoteSideVal = (quoteSide == '1') ? 0 : 1
 * [2nd case] const int quoteOpsSideVal = (quoteSide == '1') ? 1 : 0
 *
 *  Since quoteSide can only take on values of '1' and '2', it is possible
 *  that we speed up the first case [1st case] via more direct computation.
 *
 *  This program provides benchmark for exactly that reason.
 *
 *
 */
#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <numeric>

using std::vector;
using std::string;

const double zero_threshold = 1e-8;

void initMyArr(vector<uint8_t> &vec)
{
    for (int i = 0; i < vec.size(); ++i) {
        vec[i] = rand()%2 + 1;
    }
}

void normalComp(const vector<uint8_t> &vec, vector<uint8_t> &ret1, vector<double> &time1,
        int iter)
{
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < vec.size(); ++i) {
        ret1[i] = vec[i] == 1 ? 0 : 1;
    }
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    //std::cout << __func__ << ": " << elapsed_seconds.count() << std::endl;
    time1[iter] = elapsed_seconds.count();
}

void bitComp(const vector<uint8_t> &vec, vector<uint8_t> &ret2, vector<double> &time2,
        int iter)
{
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < vec.size(); ++i) {
        ret2[i] = vec[i] - 1;
    }
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    //std::cout << __func__ << ": " << elapsed_seconds.count() << std::endl;
    time2[iter] = elapsed_seconds.count();
}

bool checkResultEqual(const vector<uint8_t> &ret1, const vector<uint8_t> &ret2)
{
    for (int i = 0; i < ret1.size(); ++i) {
        if (ret1[i] != ret2[i]) {
            return false;
        }
    }
    return true;
}

void computeAverageTime(vector<double> &time, const string &method)
{
    double sum = std::accumulate(time.begin(), time.end(), zero_threshold);
    //std::cout << __func__ << ": sum = " << sum << std::endl;
    double avg_time = sum / time.size();
    std::cout << "Average time using " << method << " is " << avg_time << std::endl;
}

int main()
{
    const int num_iter = 100;
    const int size = 1000000;
    vector<uint8_t> myArr(size);
    vector<uint8_t> ret1(size, 0);
    vector<uint8_t> ret2(size, 0);
    vector<double> time1(num_iter, 0);
    vector<double> time2(num_iter, 0);
    vector<bool> result(num_iter, false);

    for (int i = 0 ; i < num_iter; ++i) {
        initMyArr(myArr);

        normalComp(myArr, ret1, time1, i);

        bitComp(myArr, ret2, time2, i);

        result[i] = checkResultEqual(ret1, ret2);
    }

    bool is_result_correct = true;
    for (int i = 0 ; i < num_iter; ++i) {
        if (result[i] != true) {
            is_result_correct = false;
            std::cout << "Result not equal: " << i << std::endl;
        }
    }
    if (is_result_correct) {
        std::cout << "Results are equal!" << std::endl;
    }
    computeAverageTime(time1, "NORMAL COMP");
    computeAverageTime(time2, "BIT COMP");
    return 0;
}
