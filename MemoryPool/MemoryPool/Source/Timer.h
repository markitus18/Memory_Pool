#ifndef __TIMER_H__
#define __TIMER_H__

#include <chrono>

class Timer
{
public:
	//Reads the current time and stores it in point_begin
	static void start();

	//Reads the current time and stores it in point_end. Returns the difference
	static unsigned int end();

private:
	static std::chrono::high_resolution_clock::time_point point_begin, point_end;
};

#endif
