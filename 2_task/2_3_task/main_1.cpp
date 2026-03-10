#include <omp.h>
#include <fstream>
#include <iostream>
#include <chrono>
#include <cmath>
#include <vector>
#include <cstring>
#include <algorithm>
#include <filesystem>

#ifndef N
#define N 70000
#endif

#ifndef MAX_ITER
#define MAX_ITER 60
#endif

#ifndef TOL
#define TOL 1e-5
#endif

struct ResultData {
    double init_time;
    double solve_time;
    double sum;
    double last_error;
    int iterations;
};

static std::vector<int> get_thread_list() {
    std::vector<int> values = {1, 2, 4, 7, 8, 16, 20, 40};
    int max_th = omp_get_num_procs();
    std::vector<int> result;

    for (int v : values) {
        if (v <= max_th) {
            result.push_back(v);
        }
    }

    return result;
}

static ResultData run_calculation() {
    double* rhs = new double[N];
    double* sol = new double[N];
    double* sol_new = new double[N];

    double alpha = 1.0 / (2.0 * (double)(N + 1));

    auto t1 = std::chrono::steady_clock::now();

    #pragma omp parallel
    {
        #pragma omp single
        std::cout << "threads=" << omp_get_num_threads() << std::endl;
    }

    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        rhs[i] = (double)(N + 1);
    }

    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        sol[i] = 0.0;
    }

    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        sol_new[i] = 0.0;
    }

    auto t2 = std::chrono::steady_clock::now();
    auto t3 = std::chrono::steady_clock::now();

    double final_error = 0.0;
    int iter_count = 0;

    for (int it = 0; it < MAX_ITER; it++) {
        double error = 0.0;

        #pragma omp parallel for reduction(max:error)
        for (int i = 0; i < N; i++) {
            double product = 0.0;

            for (int j = 0; j < N; j++) {
                if (j == i) product += 2.0 * sol[j];
                else        product += sol[j];
            }

            double new_val = sol[i] - alpha * (product - rhs[i]);
            double diff = std::fabs(new_val - sol[i]);

            sol_new[i] = new_val;
            if (diff > error) error = diff;
        }

        #pragma omp parallel for
        for (int i = 0; i < N; i++) {
            sol[i] = sol_new[i];
        }

        final_error = error;
        iter_count = it + 1;

        if (error < TOL) break;
    }

    auto t4 = std::chrono::steady_clock::now();

    double total_sum = 0.0;

    #pragma omp parallel for reduction(+:total_sum)
    for (int i = 0; i < N; i++) {
        total_sum += sol[i];
    }

    delete[] rhs;
    delete[] sol;
    delete[] sol_new;

    const std::chrono::duration<double> init_elapsed{t2 - t1};
    const std::chrono::duration<double> solve_elapsed{t4 - t3};

    return {init_elapsed.count(), solve_elapsed.count(), total_sum, final_error, iter_count};
}

int main(int argc, char** argv) {
    omp_set_dynamic(0);

    std::vector<int> thread_counts = get_thread_list();

    std::ofstream out("result1.csv", std::ios::out | std::ios::trunc);
    out << "threads,work_time_s\n";

    for (int t : thread_counts) {
        omp_set_num_threads(t);
        ResultData res = run_calculation();

        std::cout << "threads=" << t << std::endl;
        std::cout << "init_time=" << res.init_time << std::endl;
        std::cout << "work_time=" << res.solve_time << std::endl;
        std::cout << "checksum=" << res.sum << std::endl;
        std::cout << "iterations=" << res.iterations << std::endl;
        std::cout << "final_error=" << res.last_error << std::endl;
        std::cout << std::endl;

        out << t << "," << res.solve_time << "\n";
    }

    std::cout << "Saved: result1.csv" << std::endl;
    return 0;
}