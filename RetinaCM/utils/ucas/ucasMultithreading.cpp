#include "ucasMultithreading.h"

namespace ucas
{
	int THREADS_CONCURRENCY = std::thread::hardware_concurrency();			//number of concurrent threads when multithread mode is enabled
}