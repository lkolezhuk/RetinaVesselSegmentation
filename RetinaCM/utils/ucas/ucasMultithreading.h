#ifndef _UNICAS_MULTITHREADING_H
#define _UNICAS_MULTITHREADING_H

#include <thread>
#include <mutex>
#include <condition_variable>

namespace ucas
{
	extern int THREADS_CONCURRENCY;				//number of concurrent threads when multithread mode is enabled
}

#endif