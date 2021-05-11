#include <iostream>
#include <omp.h>
using namespace std;

int main()
{
  omp_set_num_threads(4);
  #pragma omp parallel
	{
		std::cout << "Hello" << ", I am Thread " << omp_get_thread_num() << std::endl;
	}
  return 0;
}
