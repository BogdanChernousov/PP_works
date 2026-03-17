#include <iostream>
#include <queue>
#include <future>
#include <thread>
#include <chrono>
#include <cmath>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <random>
#include <fstream>
#include <vector>

// Шаблон класса сервера
template<typename T>
class Server {
private:
    // Очередь задач
    std::queue<std::packaged_task<T()>> tasks;
    // Контейнер результатов
    std::unordered_map<size_t, T> results;
    
    std::mutex mut;
    std::condition_variable cond_var;
    std::jthread server_thread;
    std::atomic<size_t> task_id_counter{0};
    
public:
    void start() {
        server_thread = std::jthread([this](std::stop_token stoken) {
            while (!stoken.stop_requested()) {
                std::unique_lock<std::mutex> lock(mut);
                
                // Ждем задачи
                cond_var.wait(lock, [this, &stoken] { 
                    return !tasks.empty() || stoken.stop_requested(); 
                });
                
                if (stoken.stop_requested()) break;
                
                if (!tasks.empty()) {
                    auto task = std::move(tasks.front());
                    tasks.pop();
                    lock.unlock();
                    
                    task(); // Выполняем задачу
                }
            }
            std::cout << "Server stopped\n";
        });
    }
    
    void stop() {
        if (server_thread.joinable()) {
            server_thread.request_stop();
            cond_var.notify_all();
            server_thread.join();
        }
    }
    
    size_t add_task(std::packaged_task<T()>&& task) {
        size_t id = ++task_id_counter;
        
        {
            std::lock_guard<std::mutex> lock(mut);
            tasks.push(std::move(task));
        }
        cond_var.notify_one();
        
        return id;
    }
    
    T request_result(size_t id) {
        std::unique_lock<std::mutex> lock(mut);
        
        // Ждем результат
        cond_var.wait(lock, [this, id] { 
            return results.find(id) != results.end(); 
        });
        
        T res = results[id];
        results.erase(id);
        return res;
    }
    
    void save_result(size_t id, T value) {
        std::lock_guard<std::mutex> lock(mut);
        results[id] = value;
        cond_var.notify_all();
    }
    
    ~Server() {
        stop();
    }
};

// Функции задач
double sin_func(double x) {
    return std::sin(x);
}

double sqrt_func(double x) {
    return std::sqrt(x);
}

double pow_func(double x, double y) {
    return std::pow(x, y);
}

// Клиент 1: синус
void client_sin(Server<double>& server, int N) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 2 * M_PI);
    
    std::ofstream file("sin_results.txt");
    file << "ID\tArgument\tResult\n";
    
    std::vector<std::pair<size_t, std::future<double>>> futures;
    std::vector<double> args;
    
    for (int i = 0; i < N; i++) {
        double arg = dis(gen);
        args.push_back(arg);
        
        std::packaged_task<double()> task(std::bind(sin_func, arg));
        auto future = task.get_future();
        size_t id = server.add_task(std::move(task));
        
        futures.emplace_back(id, std::move(future));
    }
    
    for (size_t i = 0; i < futures.size(); i++) {
        auto& [id, future] = futures[i];
        double res = future.get();
        server.save_result(id, res);
        file << id << "\t" << args[i] << "\t" << res << "\n";
    }
}

// Клиент 2: корень
void client_sqrt(Server<double>& server, int N) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 100);
    
    std::ofstream file("sqrt_results.txt");
    file << "ID\tArgument\tResult\n";
    
    std::vector<std::tuple<size_t, std::future<double>, double>> futures;
    
    for (int i = 0; i < N; i++) {
        double arg = dis(gen);
        
        std::packaged_task<double()> task(std::bind(sqrt_func, arg));
        auto future = task.get_future();
        size_t id = server.add_task(std::move(task));
        
        futures.emplace_back(id, std::move(future), arg);
    }
    
    for (auto& [id, future, arg] : futures) {
        double res = future.get();
        server.save_result(id, res);
        file << id << "\t" << arg << "\t" << res << "\n";
    }
}

// Клиент 3: степень
void client_pow(Server<double>& server, int N) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis1(1, 10);
    std::uniform_real_distribution<> dis2(0, 5);
    
    std::ofstream file("pow_results.txt");
    file << "ID\tBase\tExponent\tResult\n";
    
    std::vector<std::tuple<size_t, std::future<double>, double, double>> futures;
    
    for (int i = 0; i < N; i++) {
        double base = dis1(gen);
        double exp = dis2(gen);
        
        std::packaged_task<double()> task(std::bind(pow_func, base, exp));
        auto future = task.get_future();
        size_t id = server.add_task(std::move(task));
        
        futures.emplace_back(id, std::move(future), base, exp);
    }
    
    for (auto& [id, future, base, exp] : futures) {
        double res = future.get();
        server.save_result(id, res);
        file << id << "\t" << base << "\t" << exp << "\t" << res << "\n";
    }
}

// Тест
void test() {
    std::cout << "\n=== TEST ===\n";
    
    auto check_file = [](const std::string& name) {
        std::ifstream f(name);
        if (f.is_open()) {
            int lines = 0;
            std::string line;
            while (std::getline(f, line)) lines++;
            std::cout << name << ": " << lines-1 << " записей\n";
            f.close();
        } else {
            std::cout << name << ": не найден\n";
        }
    };
    
    check_file("sin_results.txt");
    check_file("sqrt_results.txt");
    check_file("pow_results.txt");
}

int main() {
    std::cout << "Start\n";
    
    Server<double> server;
    server.start();
    
    int N = 10; // Можно изменить (5 < N < 10000)
    
    std::thread t1(client_sin, std::ref(server), N);
    std::thread t2(client_sqrt, std::ref(server), N);
    std::thread t3(client_pow, std::ref(server), N);
    
    t1.join();
    t2.join();
    t3.join();
    
    server.stop();
    
    test();
    
    std::cout << "End\n";
    return 0;
}