#include <iostream>
#include <vector>
#include <cmath>

#ifdef USE_FLOAT
    using Temp = float;
#else
    using Temp = double;
#endif

int main(){

    const size_t SIZE = 10000000;
    Temp s = 0.0;

    #ifdef USE_FLOAT
        // float
        const Temp TWO_PI = 2.0f * static_cast<float>(M_PI);
        for (size_t i = 0; i < SIZE/2; i++)
        {
            Temp x = TWO_PI * i / SIZE;
            s += sin(x) + std::sin(TWO_PI - x);
        }
    #else
        // double
        const Temp TWO_PI = 2.0 * M_PI;
        for (size_t i = 0; i < SIZE; i++)
        {
            s += std::sin(TWO_PI * i / SIZE);
        }
    #endif

    std::cout << s << std::endl;
    return 0;
}