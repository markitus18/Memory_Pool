#include "MemoryPool.h"
#include "Archive.h"
#include "Tests.h"
#include "Timer.h"
#include "Defs.h"
#include "Log.h"

#include "getopt/getopt.h"

#include <Windows.h>
#include <random>

extern MemoryPool* memoryPool = nullptr;
std::string logOutput;

bool IsOptionANumber(char* option)
{
	while (*option != '\0')
	{
		if (!std::isdigit(*option))
		{
			return false;
		}
		++option;
	}
	return true;
}

bool AssignArgValue(uint& value, const char* arg, const char* message)
{
	if (optarg != nullptr && IsOptionANumber(optarg))
	{
		value = std::stoi(optarg);
	}
	else
	{
		LOG(LogColor::Red, "Error: %s value was not valid, can't proceed.\n", message)
			return false;
	}
	return true;
}

int ProcessArgs(int argc, char** argv, TestSettings& settings)
{
	char c;
	bool argsFound = false;
	while ((c = getopt_long(argc, argv, "t:b:s:m:c:r:", nullptr, nullptr)) != -1)
	{
		argsFound = true;
		switch (c)
		{
			case 't':
			{
				if (!AssignArgValue(settings.testType, optarg, "Test Type ('t')"))
					return -1;
				break;
			}
			case 'b':
			{
				if (!AssignArgValue(settings.blocks, optarg, "Block count ('b')"))
					return -1;
				break;
			}
			case 's':
			{
				if (!AssignArgValue(settings.blockSize, optarg, "Block size ('s')"))
					return -1;
				break;
			}
			case 'm':
			{
				if (!AssignArgValue(settings.memoryAllocatedPerCycle, optarg, "Memory per cicle ('m')"))
					return -1;
				break;
			}
			case 'c':
			{
				if (!AssignArgValue(settings.testCycles, optarg, "Test cycles ('c')"))
					return -1;
				break;
			}
			case 'r':
			{
				if (!AssignArgValue(settings.memoryRatioAllocated, optarg, "Initial memory ratio ('r')"))
					return -1;
				break;
			}
			case '?':
			{
				LOG(LogColor::Yellow, "Invalid argument found, it will be ignored. Valid arguments are -t(int), -b(int), -s(int), -m(int), -c(int), -r(int)\n")
			}
		}
	}
	return argsFound;
}

bool ValidateArgs(TestSettings& settings)
{
	if (settings.testType >= (uint)TestType::Unknown)
	{
		LOG(LogColor::Red, "The test type ('t') value is out of the scope. Acceptable entries range from %i to %i\n", (uint)TestType::Default, (uint)TestType::Unknown - 1);
		return false;
	}

	if (settings.blocks < 1 || settings.blocks > MAX_BLOCKS)
	{
		LOG(LogColor::Red, "The block count ('b') is out of the scope. Acceptable entries range from %i to %i\n", 1, MAX_BLOCKS);
		return false;
	}

	if (settings.blockSize < 1 || settings.blockSize > MAX_BLOCK_SIZE)
	{
		LOG(LogColor::Red, "The block size ('s') is out of the scope. Acceptable entries range from %i to %i\n", 1, MAX_BLOCK_SIZE);
		return false;
	}

	if (settings.memoryAllocatedPerCycle == 0)
		settings.memoryAllocatedPerCycle = settings.blockSize;

	if (settings.memoryAllocatedPerCycle < 1 || settings.memoryAllocatedPerCycle > settings.blockSize * settings.blocks)
	{
		LOG(LogColor::Red, "The memory per cycle ('m') is out of the scope. Acceptable entries range from %i to 'block size * block count'\n", 1, MAX_BLOCK_SIZE);
		return false;
	}

	if (settings.testCycles == 0)
		settings.testCycles = settings.blocks;

	if (settings.testCycles < 1 || settings.testCycles > MAX_BLOCKS) //No point in doing more cycles than the maximum amount of blocks 
	{
		LOG(LogColor::Red, "The test cycles ('c') is out of the scope. Acceptable entries range from %i to %i\n", 1, MAX_BLOCKS);
		return false;
	}

	if (settings.memoryRatioAllocated < 0 || settings.memoryRatioAllocated > 100)
	{
		LOG(LogColor::Red, "The memory ratio ('r') is out of the scope. Acceptable entries range from %i to %i\n", 0, 100);
		return false;
	}

	return true;
}

int main(int argc, char** argv)
{
	srand((uint)time(nullptr));

	LOG(LogColor::White, "STARTING POOL ALLOCATION SIMULATOR...\n")

	//Processing cmd arguments
	TestSettings settings;
	int argsResult = ProcessArgs(argc, argv, settings);

	if (argsResult == -1 || !ValidateArgs(settings))
	{
		LOG(LogColor::Red, "An error occured while processing the program arguments. The test will now close\n\n")
		system("pause");
		return 1;
	}

	//No arguments were found, proceed with defaults
	if (argsResult == 0)
	{
		LOG(LogColor::White, "*No argument entries were found. Proceeding with default values")
	}
	LOG(LogColor::White, "\n")

	Tests::GenericTest(settings);

	Archive::Write("Output Log.txt", logOutput.c_str(), logOutput.size());

	delete memoryPool;

	system("pause");
	return 0;
}