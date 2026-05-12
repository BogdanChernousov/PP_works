#include <iostream>
#include <iomanip>
#include <cmath>
#include <chrono>
#include <vector>
#include <cstring>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

// Функция инициализации границ с линейной интерполяцией между углами
void initialize_boundaries(double* A, int m, int n) {
    // Значения углов: 10, 20, 30, 20 (по порядку: левый-нижний, правый-нижний, правый-верхний, левый-верхний)
    const double u00 = 10.0;  // (0,0) нижний левый
    const double u10 = 20.0;  // (m-1,0) нижний правый
    const double u11 = 30.0;  // (m-1,n-1) верхний правый
    const double u01 = 20.0;  // (0,n-1) верхний левый
    
    // Индексация: A[x + y*m]
    
    // Нижняя граница (y = 0)
    #pragma acc parallel loop present(A)
    for (int i = 0; i < m; ++i) {
        double t = static_cast<double>(i) / (m - 1);
        A[i + 0*m] = u00 * (1.0 - t) + u10 * t;
    }
    
    // Верхняя граница (y = n-1)
    #pragma acc parallel loop present(A)
    for (int i = 0; i < m; ++i) {
        double t = static_cast<double>(i) / (m - 1);
        A[i + (n-1)*m] = u01 * (1.0 - t) + u11 * t;
    }
    
    // Левая граница (x = 0) (без учёта углов, уже заданы)
    #pragma acc parallel loop present(A)
    for (int j = 1; j < n-1; ++j) {
        double t = static_cast<double>(j) / (n - 1);
        A[0 + j*m] = u00 * (1.0 - t) + u01 * t;
    }
    
    // Правая граница (x = m-1) (без учёта углов)
    #pragma acc parallel loop present(A)
    for (int j = 1; j < n-1; ++j) {
        double t = static_cast<double>(j) / (n - 1);
        A[(m-1) + j*m] = u10 * (1.0 - t) + u11 * t;
    }
}

// Инициализация всей сетки: внутренность нули, границы по линейной интерполяции
void initialize(double* A, double* Anew, int m, int n) {
    // Заполняем всё нулями
    #pragma acc parallel loop present(A, Anew)
    for (int idx = 0; idx < m*n; ++idx) {
        A[idx] = 0.0;
        Anew[idx] = 0.0;
    }
    
    // Заполняем границы
    initialize_boundaries(A, m, n);
    initialize_boundaries(Anew, m, n);
}

// Один шаг метода Якоби с пятиточечным шаблоном
double calcNext(double* A, double* Anew, int m, int n) {
    double error = 0.0;
    
    #pragma acc parallel loop collapse(2) reduction(max:error) present(A, Anew)
    for (int j = 1; j < n-1; ++j) {
        for (int i = 1; i < m-1; ++i) {
            double new_val = 0.25 * (A[i-1 + j*m] + A[i+1 + j*m] +
                                     A[i + (j-1)*m] + A[i + (j+1)*m]);
            double diff = std::fabs(new_val - A[i + j*m]);
            if (diff > error) error = diff;
            Anew[i + j*m] = new_val;
        }
    }
    return error;
}

// Копирование нового слоя в старый (только внутренние точки)
void swap(double* A, double* Anew, int m, int n) {
    #pragma acc parallel loop collapse(2) present(A, Anew)
    for (int j = 1; j < n-1; ++j) {
        for (int i = 1; i < m-1; ++i) {
            A[i + j*m] = Anew[i + j*m];
        }
    }
}

// Печать сетки (для малых размеров 10x10 и 13x13)
void printGrid(const double* A, int m, int n) {
    std::cout << std::fixed << std::setprecision(4);
    for (int j = n-1; j >= 0; --j) {  // с верхней строки
        for (int i = 0; i < m; ++i) {
            std::cout << std::setw(10) << A[i + j*m] << " ";
        }
        std::cout << std::endl;
    }
}

int main(int argc, char* argv[]) {
    // Параметры по умолчанию
    int size = 256;
    double eps = 1e-6;
    int max_iter = 1000000;
    
    // Разбор параметров командной строки через boost::program_options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Show help message")
        ("size,s", po::value<int>(&size), "Grid size NxN (default: 256)")
        ("eps,e", po::value<double>(&eps), "Precision/tolerance (default: 1e-6)")
        ("iter,i", po::value<int>(&max_iter), "Maximum iterations (default: 1e6)")
    ;
    
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }
    
    const int m = size; // ширина (x)
    const int n = size; // высота (y)
    
    std::cout << "========================================" << std::endl;
    std::cout << "Heat Equation Solver (2D Jacobi)" << std::endl;
    std::cout << "Grid: " << m << " x " << n << std::endl;
    std::cout << "Precision: " << eps << std::endl;
    std::cout << "Max iterations: " << max_iter << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Выделение памяти на хосте
    std::vector<double> A(m * n, 0.0);
    std::vector<double> Anew(m * n, 0.0);
    
    double* A_ptr = A.data();
    double* Anew_ptr = Anew.data();
    
    // Инициализация (границы по линейной интерполяции)
    #pragma acc enter data copyin(A_ptr[0:m*n], Anew_ptr[0:m*n])
    
    // Для малых сеток печатаем начальное состояние
    if (size == 10 || size == 13) {
        #pragma acc update self(A_ptr[0:m*n])
        std::cout << "\nInitial grid (boundary conditions):" << std::endl;
        printGrid(A_ptr, m, n);
    }
    
    initialize(A_ptr, Anew_ptr, m, n);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    double error = 1.0;
    int iter = 0;
    
    // Основной итерационный цикл
    while (error > eps && iter < max_iter) {
        error = calcNext(A_ptr, Anew_ptr, m, n);
        swap(A_ptr, Anew_ptr, m, n);
        ++iter;
        
        // Печать прогресса каждые 10000 итераций
        if (iter % 10000 == 0) {
            std::cout << "Iteration " << iter << ", error = " << error << std::endl;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "RESULT:" << std::endl;
    std::cout << "Iterations: " << iter << std::endl;
    std::cout << "Final error: " << error << std::endl;
    std::cout << "Time: " << elapsed << " seconds" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Для малых сеток печатаем финальное состояние
    if (size == 10 || size == 13) {
        #pragma acc update self(A_ptr[0:m*n])
        std::cout << "\nFinal grid:" << std::endl;
        printGrid(A_ptr, m, n);
    }
    
    #pragma acc exit data delete(A_ptr, Anew_ptr)
    
    return 0;
}