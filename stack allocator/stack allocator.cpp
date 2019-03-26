
#include <iostream>
#include "allocator.h"

#include <chrono>
std::chrono::time_point<std::chrono::high_resolution_clock> start, stop;
#define BENCHMARK_START start = std::chrono::high_resolution_clock::now();
#define BENCHMARK_STOP  stop = std::chrono::high_resolution_clock::now();std::cout << std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count()<< " microseconds" << std::endl;

#define SIZE 10*1024*1024

int main()
{
	alc::allocator stack_alloc;
	byte* stack_alloc_ptr = (byte*)stack_alloc.init(SIZE);

	byte* malloc_ptr = (byte*)malloc(SIZE);

	BENCHMARK_START;
	for (size_t i = 0; i < SIZE; i++)
	{
		stack_alloc_ptr[i] = 0;
	}
	BENCHMARK_STOP;
	std::cout << "  thread stack, write bench" << std::endl << std::endl;

	BENCHMARK_START;
	for (size_t i = 0; i < SIZE; i++)
	{
		malloc_ptr[i] = 0;
	}
	BENCHMARK_STOP;
	std::cout << "  malloc, write bench" << std::endl << std::endl;

	stack_alloc.destroy();
	free(malloc_ptr);
    
	system("pause");
}

