#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <iomanip>

struct TestResult{
    std::string filename;
    int records;
    int errors;
    double max_error;
    bool passed;
};

// Проверка файла с синусами
TestResult test_sin_file(const std::string& filename)
{
    TestResult result{filename, 0, 0, 0.0, false};
    
    std::ifstream file(filename);
    
    std::string line;
    std::getline(file, line); // пропустить заголовок
    
    while (std::getline(file, line))
    {
        if (line.empty()) continue;
        
        std::stringstream ss(line);
        std::string id_str, arg_str, res_str;
        
        std::getline(ss, id_str, ',');
        std::getline(ss, arg_str, ',');
        std::getline(ss, res_str, ',');
        
        double arg = std::stod(arg_str);
        double res = std::stod(res_str);
        double expected = std::sin(arg);
        
        double error = std::abs(expected - res);
        result.records++;
        
        if (error > 1e-5)
        {
            result.errors++;
            if (error > result.max_error) result.max_error = error;
        }
    }
    
    result.passed = (result.errors == 0);
    return result;
}

// Проверка файла с корнями
TestResult test_sqrt_file(const std::string& filename)
{
    TestResult result{filename, 0, 0, 0.0, false};
    
    std::ifstream file(filename);
    
    std::string line;
    std::getline(file, line);
    
    while (std::getline(file, line))
    {
        if (line.empty()) continue;
        
        std::stringstream ss(line);
        std::string id_str, arg_str, res_str;
        
        std::getline(ss, id_str, ',');
        std::getline(ss, arg_str, ',');
        std::getline(ss, res_str, ',');
        
        double arg = std::stod(arg_str);
        double res = std::stod(res_str);
        double expected = std::sqrt(arg);
        
        double error = std::abs(expected - res);
        result.records++;
        
        if (error > 1e-5)
        {
            result.errors++;
            if (error > result.max_error) result.max_error = error;
        }
    }
    
    result.passed = (result.errors == 0);
    return result;
}

// Проверка файла со степенями
TestResult test_pow_file(const std::string& filename)
{
    TestResult result{filename, 0, 0, 0.0, false};
    
    std::ifstream file(filename);
    
    std::string line;
    std::getline(file, line);
    
    while (std::getline(file, line))
    {
        if (line.empty()) continue;
        
        std::stringstream ss(line);
        std::string id_str, base_str, exp_str, res_str;
        
        std::getline(ss, id_str, ',');
        std::getline(ss, base_str, ',');
        std::getline(ss, exp_str, ',');
        std::getline(ss, res_str, ',');
        
        double base = std::stod(base_str);
        double exp = std::stod(exp_str);
        double res = std::stod(res_str);
        double expected = std::pow(base, exp);
        
        double error = std::abs(expected - res);
        result.records++;
        
        if (error > 1e-5)
        {
            result.errors++;
            if (error > result.max_error) result.max_error = error;
        }
    }
    
    result.passed = (result.errors == 0);
    return result;
}

void print_result(const TestResult& r)
{
    std::cout << r.filename << ": " << r.records << " records";
    if (r.errors > 0)
    {
        std::cout << " | ERRORS: " << r.errors 
                  << " (max error: " << std::scientific << r.max_error << ")";
    }
    else
    {
        std::cout << " | ALL CORRECT";
    }
    std::cout << "\n";
}

int main() {
    std::cout << "\n========== TESTING RESULTS ==========\n\n";
    
    bool all_passed = true;
    
    auto r1 = test_sin_file("../sin_results.csv");
    print_result(r1);
    if (!r1.passed) all_passed = false;
    
    auto r2 = test_sqrt_file("../sqrt_results.csv");
    print_result(r2);
    if (!r2.passed) all_passed = false;
    
    auto r3 = test_pow_file("../pow_results.csv");
    print_result(r3);
    if (!r3.passed) all_passed = false;
    
    std::cout << "\n=====================================\n";
    
    if (all_passed)
    {
        std::cout << "ALL TESTS PASSED\n";
        return 0;
    }
    else
    {
        std::cout << "SOME TESTS FAILED\n";
        return 1;
    }
}