#ifndef SAMPLE_TASKS_H
#define SAMPLE_TASKS_H

#include <iostream>
#include <vector>
#include <chrono>
#include <stdio.h>
#include <string>
#include <thread>

std::string factorial(int64_t N) {
    int64_t mod = 1e9 + 7;

    int64_t res = 1;
    for (int64_t i = 2; i <= N; i++) {
        res *= i;
        res %= mod;
    }

    //printf("factorial = %d\n", res);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    return std::to_string(res);
}


std::string double_factorial(int64_t N) {
    if (N == 0) {
        return std::to_string(1);
    }

    int64_t mod = 1e9 + 7;
    int64_t res = 1;

    int64_t start = (N % 2 == 0 ? 2 : 1);
    for (int64_t i = start; i <= N; i += 2) {
        res *= (i % mod);
        res %= mod;
    }

    //printf("factorial_2 = %d\n", res);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    return std::to_string(res);
}


std::string fibbonacci(int64_t N) {
    int64_t mod = 1e9 + 7;
    std::vector<int64_t> last_values = { 0, 1 };

    for (int64_t i = 1; i <= N; i++) {
        int64_t last = (last_values[0] + last_values[1] > mod ? last_values[0] + last_values[1] - mod : last_values[0] + last_values[1]);

        last_values[0] = last_values[1];
        last_values[1] = last;
    }

    //printf("fibbonacci = %d\n", last_values[0]);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    return std::to_string(last_values[0]);
}


#endif // SAMPLE_TASKS_H
