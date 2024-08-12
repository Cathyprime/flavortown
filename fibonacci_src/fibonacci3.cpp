#include "fibonacci3.hh"
#include "fib.hh"
#include <iostream>

int fib40()
{
    std::cout << "Fibonacci(40) = " << Fibonacci<40>::value << std::endl;
    return 0;
}
