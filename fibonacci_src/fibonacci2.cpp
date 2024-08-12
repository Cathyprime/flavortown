#include "fibonacci2.hh"
#include "fib.hh"
#include <iostream>

int fib35()
{
	std::cout << "Fibonacci(35) = " << Fibonacci<35>::value << std::endl;
	return 0;
}
