#include "Archive.h"

#include <string>
#include <fstream>

void Archive::Write(const char* path, const char* buffer, unsigned int size)
{
	std::ofstream file(path, std::ios::binary);
	if (file.is_open())
	{
		file.write(buffer, size);
		file.close();
	}
}

void Archive::Append(const char* path, const char* buffer)
{
	std::fstream file(path, std::ios::app);
	std::string content = "\n\n -----------------------------------------------------\n\n";
	content.append(buffer);

	if (file.is_open())
	{
		file << content.c_str();
		file.close();
	}
}

