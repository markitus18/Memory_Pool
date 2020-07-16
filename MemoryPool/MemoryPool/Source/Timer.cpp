#include "Timer.h"

std::chrono::high_resolution_clock::time_point Timer::point_begin, Timer::point_end;

void Timer::start()
{
	point_begin = std::chrono::high_resolution_clock::now();
}

unsigned int Timer::end()
{
	point_end = std::chrono::high_resolution_clock::now();
	return (unsigned int)std::chrono::duration_cast<std::chrono::microseconds>(point_end - point_begin).count();
}