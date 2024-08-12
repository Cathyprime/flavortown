#include "fibonacci1.hh"
#include "fib.hh"
#include <iostream>

int fib30()
{
	std::cout << "Fibonacci(30) = " << Fibonacci<30>::value << std::endl;
	return 0;
}
