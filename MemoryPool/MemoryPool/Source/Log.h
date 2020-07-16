#ifndef __LOG_H__
#define __LOG_H__

#include <string>

#define LOG(color, format, ...) Log(color, format, __VA_ARGS__);

enum class LogColor
{
	Red = 4,
	Yellow = 6,
	White = 7,
	Green = 10,
	Cyan = 11,
};

extern std::string logOutput;
void Log(LogColor color, const char* format, ...);

#endif //!__LOG_H__
