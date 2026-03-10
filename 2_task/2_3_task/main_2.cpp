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

struct Measurement {
    double init_duration;
    double compute_duration;
    double total;
    double residual;
    int steps;
};

static std::vector<int> generate_thread_set() {
    std::vector<int> candidates = {1, 2, 4, 7, 8, 16, 20, 40};
    int max_threads = omp_get_num_procs();
    std::vector<int> valid;

    for (int val : candidates) {
        if (val <= max_threads) {
            valid.push_back(val);
        }
    }

    return valid;
}

static Measurement execute_solver() {
    double* vec_b = new double[N];
    double* vec_x = new double[N];
    double* vec_xn = new double[N];

    double tau = 1.0 / (2.0 * (double)(N + 1));

    auto start_init = std::chrono::steady_clock::now();

    #pragma omp parallel
    {
        #pragma omp single
        std::cout << "threads=" << omp_get_num_threads() << std::endl;
        
        #pragma omp for
        for (int i = 0; i < N; i++) {
            vec_b[i] = (double)(N + 1);
        }

        #pragma omp for
        for (int i = 0; i < N; i++) {
            vec_x[i] = 0.0;
        }

        #pragma omp for
        for (int i = 0; i < N; i++) {
            vec_xn[i] = 0.0;
        }
    }

    auto end_init = std::chrono::steady_clock::now();
    auto start_solve = std::chrono::steady_clock::now();

    double final_norm = 0.0;
    int iterations = 0;
    int converged = 0;

    for (int step = 0; step < MAX_ITER && !converged; step++) {
        double norm = 0.0;

        #pragma omp parallel for reduction(max:norm)
        for (int i = 0; i < N; i++) {
            double sum = 0.0;

            for (int j = 0; j < N; j++) {
                if (j == i) sum += 2.0 * vec_x[j];
                else        sum += vec_x[j];
            }

            double updated = vec_x[i] - tau * (sum - vec_b[i]);
            double change = std::fabs(updated - vec_x[i]);

            vec_xn[i] = updated;
            if (change > norm) norm = change;
        }

        #pragma omp parallel for
        for (int i = 0; i < N; i++) {
            vec_x[i] = vec_xn[i];
        }

        final_norm = norm;
        iterations = step + 1;

        if (norm < TOL) converged = 1;
    }

    auto end_solve = std::chrono::steady_clock::now();

    double sum_elements = 0.0;

    #pragma omp parallel for reduction(+:sum_elements)
    for (int i = 0; i < N; i++) {
        sum_elements += vec_x[i];
    }

    delete[] vec_b;
    delete[] vec_x;
    delete[] vec_xn;

    const std::chrono::duration<double> init_time{end_init - start_init};
    const std::chrono::duration<double> solve_time{end_solve - start_solve};

    return {init_time.count(), solve_time.count(), sum_elements, final_norm, iterations};
}

int main(int argc, char** argv) {
    omp_set_dynamic(0);

    std::vector<int> thread_counts = generate_thread_set();

    std::ofstream outfile("result2.csv", std::ios::out | std::ios::trunc);
    outfile << "threads,work_time_s\n";

    for (int t : thread_counts) {
        omp_set_num_threads(t);
        Measurement m = execute_solver();

        std::cout << "\nthreads=" << t << std::endl;
        std::cout << "init_time=" << m.init_duration << std::endl;
        std::cout << "work_time=" << m.compute_duration << std::endl;
        std::cout << "checksum=" << m.total << std::endl;
        std::cout << "iterations=" << m.steps << std::endl;
        std::cout << "final_error=" << m.residual << std::endl;

        outfile << t << "," << m.compute_duration << "\n";
    }

    std::cout << "\nResults saved: result2.csv" << std::endl;
    return 0;
}