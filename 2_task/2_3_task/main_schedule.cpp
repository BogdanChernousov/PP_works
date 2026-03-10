#include <omp.h>
#include <fstream>
#include <iostream>
#include <chrono>
#include <cmath>
#include <vector>
#include <string>
#include <filesystem>

#ifndef N
#define N 20000
#endif

#ifndef FIXED_THREADS
#define FIXED_THREADS 8
#endif

#ifndef MAX_ITER
#define MAX_ITER 60
#endif

#ifndef TOL
#define TOL 1e-5
#endif

// Инициализация системы Ax = b
void init_system(std::vector<std::vector<double>>& A, std::vector<double>& b, int n) {
    A.resize(n, std::vector<double>(n));
    b.resize(n);
    
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            A[i][j] = (i == j) ? 2.0 : 1.0;
        }
        b[i] = n + 1.0;
    }
}

// Решение с заданным типом schedule и размером чанка
double solve_with_schedule(const std::vector<std::vector<double>>& A, 
                          const std::vector<double>& b,
                          const std::string& schedule_type, 
                          int chunk_size,
                          double tau, double eps, int max_iter) {
    int n = A.size();
    std::vector<double> x(n, 0.0);
    std::vector<double> x_new(n);
    std::vector<double> Ax(n);
    int iter = 0;
    double error;
    
    auto start = std::chrono::steady_clock::now();
    
    do {
        // Вычисляем Ax с заданным schedule
        if (schedule_type == "static") {
            #pragma omp parallel for schedule(static, chunk_size)
            for (int i = 0; i < n; i++) {
                Ax[i] = 0.0;
                for (int j = 0; j < n; j++) {
                    Ax[i] += A[i][j] * x[j];
                }
            }
        }
        else if (schedule_type == "dynamic") {
            #pragma omp parallel for schedule(dynamic, chunk_size)
            for (int i = 0; i < n; i++) {
                Ax[i] = 0.0;
                for (int j = 0; j < n; j++) {
                    Ax[i] += A[i][j] * x[j];
                }
            }
        }
        else if (schedule_type == "guided") {
            #pragma omp parallel for schedule(guided, chunk_size)
            for (int i = 0; i < n; i++) {
                Ax[i] = 0.0;
                for (int j = 0; j < n; j++) {
                    Ax[i] += A[i][j] * x[j];
                }
            }
        }
        
        // Обновление решения
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            x_new[i] = x[i] - tau * (Ax[i] - b[i]);
        }
        
        // Вычисление погрешности
        error = 0.0;
        #pragma omp parallel for reduction(max:error)
        for (int i = 0; i < n; i++) {
            double diff = fabs(x_new[i] - x[i]);
            if (diff > error) error = diff;
        }
        
        x = x_new;
        iter++;
        
    } while (error > eps && iter < max_iter);
    
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    
    return elapsed.count();
}

int main() {
    // Параметры задачи
    int n = N;
    int fixed_threads = FIXED_THREADS;
    double tau = 1.0 / (2.0 * (double)(n + 1));
    double eps = TOL;
    int max_iter = MAX_ITER;
    
    std::cout << "ИССЛЕДОВАНИЕ ПАРАМЕТРОВ SCHEDULE\n";
    std::cout << "================================\n";
    std::cout << "Размер задачи: " << n << "x" << n << std::endl;
    std::cout << "Фиксированное число потоков: " << fixed_threads << std::endl;
    std::cout << "Точность: " << eps << std::endl;
    std::cout << "Макс. итераций: " << max_iter << std::endl;
    
    // Инициализация данных
    std::vector<std::vector<double>> A;
    std::vector<double> b;
    init_system(A, b, n);
    
    // Устанавливаем число потоков
    omp_set_num_threads(fixed_threads);
    omp_set_dynamic(0);
    
    // Параметры для тестирования
    std::vector<std::string> schedule_types = {"static", "dynamic", "guided"};
    std::vector<int> chunk_sizes = {1, 10, 50, 100, 500, 1000, 5000, 10000};
    
    // Создаем директорию для результатов
    std::filesystem::create_directories("results");
    
    // Запись в CSV
    std::ofstream file("schedule_results.csv");
    file << "schedule,chunk,time_s\n";
    
    std::cout << "\nЗапуск тестов...\n";
    std::cout << std::string(50, '-') << std::endl;
    
    for (const auto& sched : schedule_types) {
        for (int chunk : chunk_sizes) {
            std::cout << "Тестирование: " << sched << ", chunk=" << chunk << "... ";
            std::cout.flush();
            
            double time = solve_with_schedule(A, b, sched, chunk, tau, eps, max_iter);
            
            std::cout << time << " с" << std::endl;
            file << sched << "," << chunk << "," << time << "\n";
        }
    }
    
    file.close();
    
    std::cout << std::string(50, '-') << std::endl;
    
    return 0;
}


// Наблюдения:

//     Static лучше всего с chunk=100 (6.31 с)

//     Слишком маленький chunk (1) для dynamic даёт огромное время (31 с) из-за накладных расходов

//     Слишком большой chunk (≥5000) сильно замедляет все типы

//     Guided показывает стабильные результаты, но чуть медленнее static