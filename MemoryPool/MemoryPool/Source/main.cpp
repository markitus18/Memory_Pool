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


struct TestSettings
{
	uint blockSize = DEFAULT_BLOCK_SIZE;
	uint blocks = DEFAULT_BLOCKS;
	
	bool bruteForce = false;
	bool verboseTest = false;
};

bool IsOptionValid(char* option)
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

int ProcessArgs(int argc, char** argv, TestSettings& settings)
{
	char c;
	bool argsFound = false;
	while ((c = getopt_long(argc, argv, "s:b:fv", nullptr, nullptr)) != -1)
	{
		argsFound = true;
		switch (c)
		{
			case 'b':
			{
				if (optarg != nullptr && IsOptionValid(optarg))
				{
					settings.blocks = std::stoi(optarg);
				}
				else
				{
					LOG(LogColor::Red, "Error: Block count value was not valid, can't proceed. The test will close.\n")
					return -1;
				}
				break;
			}
			case 's':
			{
				if (optarg != nullptr && IsOptionValid(optarg))
				{
					settings.blockSize = std::stoi(optarg);
				}
				else
				{
					LOG(LogColor::Red, "Error: Block size value was not valid, can't proceed. The test will close.\n")
					return -1;
				}
				break;
			}
			case 'f':
			{
				settings.bruteForce = true;
				break;
			}
			case 'v':
			{
				settings.verboseTest = true;
				break;
			}
			case '?':
			{
				LOG(LogColor::Yellow, "Invalid argument found, it will be ignored. Valid arguments are -b(int), -s(int), -f, -v\n")
			}
		}
	}
	return argsFound;
}

int main(int argc, char** argv)
{
	srand((uint)time(nullptr));

	LOG(LogColor::White, "STARTING POOL ALLOCATION SIMULATOR...\n")

	//Processing cmd arguments
	TestSettings settings;
	int argsResult = ProcessArgs(argc, argv, settings);
	if (argsResult != 1)
	{
		//An error ocurred when processing the arguments. Canceling execution
		if (argsResult == -1)
		{
			system("pause");
			return 1;
		}

		//No arguments were found, proceed with defaults
		if (argsResult == 0)
		{
			LOG(LogColor::White, "*No argument entries were found. Proceeding with default values\n\n")
		}
	}

	if (settings.blockSize > MAX_BLOCK_SIZE)
	{
		LOG(LogColor::Yellow, "*Warning: Block Size exceeding the limits. Block size reduced to %i\n", MAX_BLOCK_SIZE)
		settings.blockSize = MAX_BLOCK_SIZE;
	}
	if (settings.blocks > MAX_BLOCKS)
	{
		LOG(LogColor::Yellow, "*Warning: Amount of blocks exceeding the limits. Blocks reduced to %i\n", MAX_BLOCKS)
			settings.blocks = MAX_BLOCKS;
	}

	//Starting tests
	if (settings.bruteForce)
	{
		LOG(LogColor::White, "*Bruteforce mode selected\n\n")
		Tests::BruteForceTest();
	}
	else if (settings.verboseTest)
	{
		LOG(LogColor::White, "*Verbose mode selected\n\n")
		Tests::SimpleVerbosityTest(settings.blockSize, settings.blocks);
	}
	else
	{
		LOG(LogColor::White, "*Performance test mode selected\n\n")
		Tests::FullTestsOnSinglePool(settings.blockSize, settings.blocks);
	}

	Archive::Write("Output Log.txt", logOutput.c_str(), logOutput.size());

	delete memoryPool;

	system("pause");
	return 0;
}