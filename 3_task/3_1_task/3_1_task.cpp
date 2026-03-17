#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <iomanip>
#include <cstdlib>

#define N 5000

struct ThreadData{
    double* A;
    double* x;
    double* y;
    int thread_id;
    int num_threads;
};

void initialize(ThreadData& data)
{
    long long totalA = (long long)N * N;
    long long sizeA = totalA / data.num_threads;
    long long startA = (long long)data.thread_id * sizeA;
    long long endA = (data.thread_id == data.num_threads - 1) ? totalA : (long long)(data.thread_id + 1) * sizeA;

    for (long long i = startA; i < endA; i++) data.A[i] = 1.0;

    long long sizeX = N / data.num_threads;
    long long startX = (long long)data.thread_id * sizeX;
    long long endX = (data.thread_id == data.num_threads - 1) ? N : (long long)(data.thread_id + 1) * sizeX;

    for (long long i = startX; i < endX; i++) data.x[i] = 1.0;
    for (long long i = startX; i < endX; i++) data.y[i] = 0.0;
}

void multiply(ThreadData& data)
{
    long long rows = N / data.num_threads;
    long long start = (long long)data.thread_id * rows;
    long long end = (data.thread_id == data.num_threads - 1) ? N : (long long)(data.thread_id + 1) * rows;

    for (long long i = start; i < end; i++)
    {
        double sum = 0.0;
        for (long long j = 0; j < N; j++)
        {
            sum += data.A[i * (long long)N + j] * data.x[j];
        }
        data.y[i] = sum;
    }
}

int main(int argc, char* argv[])
{
    int num_threads;
    
    if (argc > 1)
    {
        num_threads = std::atoi(argv[1]);
    }
    else
    {
        num_threads = std::thread::hardware_concurrency();
    }
    
    std::cout << "Используется потоков: " << num_threads << std::endl;

    double* A = new double[(long long)N * N];
    double* x = new double[N];
    double* y = new double[N];

    auto start_init = std::chrono::steady_clock::now();
    
    std::vector<std::thread> init_threads;
    std::vector<ThreadData> init_data(num_threads);
    
    for (int i = 0; i < num_threads; i++)
    {
        init_data[i] = {A, x, y, i, num_threads};
        init_threads.emplace_back(initialize, std::ref(init_data[i]));
    }
    
    for (auto& t : init_threads) t.join();
    
    auto end_init = std::chrono::steady_clock::now();

    auto start_work = std::chrono::steady_clock::now();
    
    std::vector<std::jthread> work_threads;
    std::vector<ThreadData> work_data(num_threads);
    
    for (int i = 0; i < num_threads; i++)
    {
        work_data[i] = {A, x, y, i, num_threads};
        work_threads.emplace_back(multiply, std::ref(work_data[i]));
    }
    
    for (auto& t : work_threads) t.join();
    
    auto end_work = std::chrono::steady_clock::now();

    double res = 0.0;
    for (long long i = 0; i < N; i++) res += y[i];

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Init time " << std::chrono::duration<double>(end_init - start_init).count() << std::endl;
    std::cout << "Work time " << std::chrono::duration<double>(end_work - start_work).count() << std::endl;
    std::cout << res << std::endl;

    delete[] A;
    delete[] x;
    delete[] y;
    return 0;
}