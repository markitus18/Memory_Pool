#include "Log.h"

#include <iostream>
#include <stdio.h>
#include <stdarg.h>

#include <Windows.h>

void Log(LogColor color, const char* format, ...)
{
	char buffer[256];
	va_list args;
	va_start(args, format);
	vsprintf_s(buffer, format, args);
	va_end(args);

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, (int)color);
	std::cout << buffer;
	SetConsoleTextAttribute(hConsole, (int)LogColor::White);

	logOutput.append(buffer);
}