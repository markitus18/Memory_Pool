#include "Tests.h"

#include "Log.h"
#include "Defs.h"
#include "Timer.h"
#include "Archive.h"
#include "MemoryPool.h"

void Tests::FullTestsOnSinglePool(size_t blockSize, uint blocks)
{
	if (memoryPool != nullptr)
		delete memoryPool;

	LOG(LogColor::Cyan, "Creating new pool of %i blocks of %i bytes\n\n", blocks, blockSize)

	memoryPool = new MemoryPool(blockSize, blocks);

	//The following tests are done taking into consideration that pool has, generally,
	//a bigger amount of blocks than the size of each block (for example, 2048 blocks of 32 byes, 5000 blocks of 100 byes,...)
	//The results comparison will be valid in any situation, but will make more sense in tests that share a similar proportion

	uint blockFactor = blocks * 0.9f;
	if (blockFactor < 1) blockFactor = 1; //<- Just in case we receive a really small pool

	//Speed tests of small chunks (allocating the same size as one block) until the pool is filled
	//This test will use one block in each allocation, but will run more cycles
	Tests::SimpleSpeedTest(blockFactor, blockSize, false);
	Tests::SimpleSpeedTest(blockFactor, blockSize, true);

	//Speed tests of small allocation chunks until the pool is ~90% filled
	//This test will use a lot of blocks in a single allocation, but will run shorter cycles
	Tests::SimpleSpeedTest(blockSize, blockFactor, false);
	Tests::SimpleSpeedTest(blockSize, blockFactor, true);

	//Random speed tests of big allocation chunks
	//This will test the pool in its best state (memory totally empty) and
	//its worst state (memory totally filled)
	Tests::SpeedTest_RandomAfterAllocation(blockFactor, blockSize, 0.0f);
	Tests::SpeedTest_RandomAfterAllocation(blockFactor, blockSize, 1.0f);

	//Random speed tests of small allocation chunks
	//This will test the pool in its best state (memory totally empty) and
	//its worst state (memory totally filled)
	Tests::SpeedTest_RandomAfterAllocation(blockSize, blockFactor, 0.0f);
	Tests::SpeedTest_RandomAfterAllocation(blockSize, blockFactor, 1.0f);

	LOG(LogColor::Green, "TEST OK --------------------------------------------\n\n");

	delete memoryPool;
	memoryPool = nullptr;
}

bool Tests::SimpleSpeedTest(size_t memorySize, uint cycles, bool freeMemoryAfterAllocation, uint testsToPerform)
{
	//Test LOG ----------------------------------------
	LOG(LogColor::Cyan, "SPEED TEST - Allocating %i bytes x %i cycles -\n", memorySize, cycles);

	if (!freeMemoryAfterAllocation)
		LOG(LogColor::White, "--- not freeing memory after each allocation ---\n")
	else
		LOG(LogColor::White, "  --- freeing memory after each allocation ---\n")

	//Making sure the test does not try to allocate more memory in a single cycle than the pool can support
	if (memorySize > memoryPool->GetTotalMemory())
	{
		LOG(LogColor::Red, " *Error: The test was trying to allocate more memory size than the total memory of the pool. Test Cancelled\n");
		return false;
	}

	//Making sure the test does not try to allocate more total memory (memorySize times cycles) than the pool can support
	uint neededBlocksPerCycle = memoryPool->GetNeededBlocksForMemory(memorySize);
	if (!freeMemoryAfterAllocation && neededBlocksPerCycle * cycles > memoryPool->GetTotalBlocks())
	{
		cycles = memoryPool->GetTotalBlocks() / neededBlocksPerCycle;
		LOG(LogColor::Yellow, " *Warning: The test was trying to allocate more memory than the pool can support. Reducing cycles to %i\n", cycles);
	}

	uint averagedResults_Pool = PerformSimpleSpeedTest(memorySize, cycles, freeMemoryAfterAllocation, testsToPerform, true);
	uint averagedResults_Malloc = PerformSimpleSpeedTest(memorySize, cycles, freeMemoryAfterAllocation, testsToPerform, false);

	//Displaying test results ------------------------
	if (testsToPerform > 1)
		LOG(LogColor::White, "  Displaying averaged results from %i tests: \n", testsToPerform)
		LOG(LogColor::White, "    Pool took:   %i ms\n", averagedResults_Pool)
		LOG(LogColor::White, "    Malloc took: %i ms\n", averagedResults_Malloc);
	LOG(LogColor::White, "----------------------------------------------------\n\n");

	return true;
}

void Tests::SimpleVerbosityTest(size_t blockSize, uint blocks)
{
	if (memoryPool != nullptr)
		delete memoryPool;

	LOG(LogColor::Cyan, "Creating new pool of %i blocks of %i bytes\n", blocks, blockSize)
	LOG(LogColor::White, "Printing memory and pool results in output files...\n\n")

	memoryPool = new MemoryPool(blockSize, blocks);

	uint memorySize = blockSize * 0.5f;
	if (memorySize < 1) memorySize = 1;  //<- Just in case we receive a really small pool

	uint cycles = blocks / 2; //We will be filling half the pool, so we can check that non-allocated memory is also ok.
	if (cycles < 1) cycles = 1;
	PerformSimpleVerbosityTest(memorySize, cycles, "SmallChunks");
	
	//Making sure we have enough room to perform the 'bigChunks' test
	if (blocks < 4)
	{
		LOG(LogColor::Yellow, "Warning: Could not complete verbosity check on 'big' chunks as only %i block(s) are available\n", blocks)
		LOG(LogColor::Yellow, "Test exits without errors\n\n", blocks)
	}
	else
	{
		memorySize = blockSize * 3.5f;
		cycles = (blocks / 4) / 2; //We will be filling half the pool, so we can check that non-allocated memory is also ok.
		if (cycles < 1) cycles = 1;
		PerformSimpleVerbosityTest(memorySize, cycles, "BigChunks");

		LOG(LogColor::Green, "TEST OK --------------------------------------------\n\n");
	}


	delete memoryPool;
	memoryPool = nullptr;
}

bool Tests::PerformSimpleVerbosityTest(size_t memorySize, uint cycles, const char* outputName)
{
	//Test LOG ----------------------------------------
	LOG(LogColor::Cyan, "VERBOSITY TEST - Allocating %i bytes x %i cycles -\n", memorySize, cycles);

	if (memorySize > memoryPool->GetTotalMemory())
	{
		LOG(LogColor::Red, " *Error: The test was trying to allocate more memory size than the total memory of the pool. Test Cancelled\n");
		return false;
	}

	//Making sure the test does not try to allocate more total memory (memorySize times cycles) than the pool can support
	uint neededBlocksPerCycle = memoryPool->GetNeededBlocksForMemory(memorySize);
	if (neededBlocksPerCycle * cycles > memoryPool->GetTotalBlocks())
	{
		cycles = memoryPool->GetTotalBlocks() / neededBlocksPerCycle;
		LOG(LogColor::Yellow, " *Warning: The test was trying to allocate more memory than the pool can support. Reducing cycles to %i\n", cycles);
	}

	//We will try to fill the beginning and the end of the block with meaningful data for easier read
	char blockStartStr[] = "<Block Start>";
	char blockEndStr[] = "<Block End>";
	uint blockStartSize = strlen(blockStartStr);
	uint blockEndSize = strlen(blockEndStr);

	//We will be storing all allocations in this array.
	char** arr = new char* [cycles];
	for (uint i = 0; i < cycles; ++i)
	{
		arr[i] = (char*)memoryPool->Reserve(memorySize);

		//Filling the start of the block
		if (memorySize >= blockStartSize)
			memcpy(arr[i], blockStartStr, blockStartSize);
		//Filling the rest of the memory with ordered chars
		if (memorySize > blockStartSize + blockEndSize)
		{
			for (uint m = blockStartSize; m < memorySize - blockEndSize; ++m)
				arr[i][m] = m % (255 + 1);
		}
		//Filling the end of the block
		if (memorySize >= blockStartSize + blockEndSize)
			memcpy(arr[i] + (memorySize - blockEndSize), blockEndStr, blockEndSize);
	}

	std::string output = memoryPool->DumpMemoryState();
	std::string fileName("Memory State Dump_");
	fileName.append(outputName).append(".txt");
	Archive::Write(fileName.c_str(), output.c_str(), output.size());

	output = memoryPool->DumpPoolState();
	fileName = "Pool State Dump_";
	fileName.append(outputName).append(".txt");
	Archive::Write(fileName.c_str(), output.c_str(), output.size());

	memoryPool->Clear();
	return true;
}

uint Tests::PerformSimpleSpeedTest(size_t memorySize, uint cycles, bool freeAfterAllocation, uint testsToPerform, bool pool)
{
	//We will be storing all allocations in this array in case we need to free the memory after the test
	char** arr = new char* [cycles];
	uint averagedResults = 0;

	for (uint i = 0; i < testsToPerform; ++i)
	{
		//Starting actual performance test
		Timer::start();
		for (uint i = 0u; i < cycles; ++i)
		{
			arr[i] = (char*)(pool ? memoryPool->Reserve(memorySize) : malloc(memorySize));

			if (freeAfterAllocation)
				pool ? memoryPool->Free(arr[i]) : free(arr[i]);
		}
		averagedResults += Timer::end();

		//Releasing all the remaining memory used in the test
		if (!freeAfterAllocation)
			for (uint i = 0u; i < cycles; ++i)
				pool ? memoryPool->Free(arr[i]) : free(arr[i]);
	}
	delete[] arr;

	averagedResults /= testsToPerform;
	return averagedResults;

}

bool Tests::SpeedTest_RandomAfterAllocation(size_t memorySize, uint cycles, float initialMemoryRatio, uint testsToPerform)
{
	//Test LOG ----------------------------------------
	LOG(LogColor::Cyan, "SPEED TEST (RANDOM) --------------------------------\n");
	LOG(LogColor::White, "- The pool will be filled up to %i%% of its capacity.\n- The test will begin after that, allocating or deallocating %i bytes x %i times\n", (int)(initialMemoryRatio * 100), memorySize, cycles)
	
	//Making sure the test does not try to allocate more memory in a single cycle than the pool can support
	if (memorySize > memoryPool->GetTotalMemory())
	{
		LOG(LogColor::Red, " *Error: The test was trying to allocate more memory size than the total memory of the pool. Test Cancelled.\n");
		return false;
	}

	uint averagedResults_pool = PrepareSpeedTest_RandomAfterAllocation(memorySize, cycles, testsToPerform, initialMemoryRatio, true);
	uint averagedResults_malloc = PrepareSpeedTest_RandomAfterAllocation(memorySize, cycles, testsToPerform, initialMemoryRatio, false);
	
	//Displaying test results ------------------------
	if (testsToPerform > 1)
		LOG(LogColor::White, "  Displaying averaged results from %i tests: \n", testsToPerform)
		LOG(LogColor::White, "    Pool took:   %i ms\n", averagedResults_pool)
		LOG(LogColor::White, "    Malloc took: %i ms\n", averagedResults_malloc);
	LOG(LogColor::White, "----------------------------------------------------\n\n");

	return true;
}

uint Tests::PrepareSpeedTest_RandomAfterAllocation(uint memorySize, uint cycles, uint testsPerformed, float initialMemoryRatio, bool usePool)
{
	uint averagedResults = 0;

	//In order to avoid contaminating the test as much as possible, all random calculations will be done before the test.
	//We will be running the whole random simulation here and storing the random results in two arrays which will be used during the test
	//This will allow us to randomize the test without the cost of calculating the random operations
	uint blocksNeededPerCycle = memoryPool->GetNeededBlocksForMemory(memorySize);
	uint maxAllocations = memoryPool->GetTotalBlocks() / blocksNeededPerCycle;
	uint initialAllocations = maxAllocations * initialMemoryRatio;

	char** arr = new char* [maxAllocations]; //"Fake" array to know wether we have allocated / deallocated something during the simulation
											//We will initialize this array to nullptr and assign slots to a ptr whenever an "allocation" happens

	bool* randomAllocations = new bool[cycles];						//We will use this array to know wether we should allocate or deallocate later on during the test
	uint* operationStorageIndex = new uint[cycles]; //We will store at which place of 'arr' our allocation/deallocation will happen.
																	//For deallocation purposes, the slot will be randomized. This has the implication of randomizing which part of our pool will be freed
																	//For allocation purposes, since all we do is store the allocated memory ptr, we will find the first empty slot

	for (uint testsCount = 0u; testsCount < testsPerformed; ++testsCount)
	{
		char fakePtr;
		memset(arr, 0, maxAllocations * sizeof(char*));
		for (uint i = 0u; i < initialAllocations; ++i)
		{
			arr[i] = &fakePtr; //<- This would not be 100% necessary, but we are doing it like that to keep mental sanity when debugging
		}

		uint allocationsCount = initialAllocations;

		//The process is quite tedious but the goal here is to make the actual test as fast as possible
		//What we are doing is "faking" what would be the normal process of randomizing alloc / frees and storing the steps in arrays that will be read later
		//to avoid all the following process
		for (uint i = 0u; i < cycles; ++i)
		{
			uint allocate;
			//Always deallocate if all allocations are full and allocate when no allocations are done. Otherwise, select at random
			if (allocationsCount == maxAllocations)
				allocate = 0u;
			else if (allocationsCount == 0u)
				allocate = 1u;
			else allocate = (allocationsCount == maxAllocations ? 0u : std::rand() % 2);

			randomAllocations[i] = allocate;
			if (allocate)
			{
				//Find the first available slot to save the ptr to the newly allocated data. Store it in the array so the test does not have to find the slot again
				for (uint j = 0u; j < maxAllocations; ++j)
				{
					if (arr[j] == nullptr)
					{
						arr[j] = &fakePtr;
						operationStorageIndex[i] = j;
						break;
					}
				}
				++allocationsCount;
			}
			else
			{
				//Decide which data should be freed from all existing allocations
				uint deallocationIndex = std::rand() % allocationsCount;

				//Search through all allocations done so far until we reach the one to remove
				for (uint j = 0, allocationIndex = 0; j < maxAllocations; ++j)
				{
					if (arr[j] == &fakePtr)
					{
						if (allocationIndex == deallocationIndex)
						{
							operationStorageIndex[i] = j;
							arr[j] = nullptr;
							break;
						}
						++allocationIndex;
					}
				}
				--allocationsCount;
			}
		}

		//The simulation is ready, start the actual test with the random data already prepared
		averagedResults += PerformSpeedTest_RandomAfterAllocation(memorySize, cycles, initialAllocations, maxAllocations, usePool, randomAllocations, operationStorageIndex);

	}
	delete[] arr;
	delete[] randomAllocations;
	delete[] operationStorageIndex;

	averagedResults /= testsPerformed;
	return averagedResults;

}

uint Tests::PerformSpeedTest_RandomAfterAllocation(size_t memorySize, uint cycles, uint initialAllocations, uint maxAllocations, bool usePool,
													const bool* const& randomAllocations, const uint* const& operationsStorageIndex)
{
	//We will be storing all the pool / malloc memory allocations in this array. We need to keep track of those for random dealloc.
	//or releasing memory after the test
	char** arr = new char* [maxAllocations];
	memset(arr, 0, maxAllocations * sizeof(char*));
	
	//Performing initial allocations before the test starts. It is very unlikely to affect malloc, but we are keeping it here for consistency
	for (uint i = 0u; i < initialAllocations; ++i)
	{
		arr[i] = (char*)(usePool ? memoryPool->Reserve(memorySize) : malloc(memorySize));
	}

	//Starting the actual test. We will check randomAllocations[cycle] to know wether we should allocate or not.
	//operationsStorageIndex[cycle] will tell us which slot in 'arr' the operation (alloc / dealloc) affects
	Timer::start();
	for (uint i = 0u; i < cycles; ++i)
	{
		if (randomAllocations[i])
		{
			arr[operationsStorageIndex[i]] = (char*)(usePool ? memoryPool->Reserve(memorySize) : malloc(memorySize));
		}
		else
		{
			usePool ? memoryPool->Free(arr[operationsStorageIndex[i]]) : free(arr[operationsStorageIndex[i]]);
			arr[operationsStorageIndex[i]] = nullptr;
		}
	}
	uint testResult = Timer::end();

	//Releasing all the remaining memory used in the test
	for (uint i = 0u; i < maxAllocations; ++i)
	{
		if (arr[i] != nullptr)
			usePool ? memoryPool->Free(arr[i]) : free(arr[i]);
	}

	delete[] arr;
	memoryPool->Clear(); //<- Should be clear at this point, just a safety measure to avoid accidentaly affecting other tests

	return testResult;

}

void Tests::BruteForceTest(uint testsPerformed)
{
	LOG(LogColor::Cyan, "STARTING BRUTEFORCE TEST ----------------------------\n\n");

	for (uint testIndex = 0; testIndex < testsPerformed; ++testIndex)
	{
		uint blockSize = rand() % MAX_BLOCK_SIZE + 1;
		uint blocks = (rand() % MAX_BLOCKS) + 1;

		LOG(LogColor::Cyan, "*STARTING TEST N\370 %i - ", testIndex)

		FullTestsOnSinglePool(blockSize, blocks);
	}

	LOG(LogColor::Green, "BRUTEFORCE TEST FINISHED - ALL OK ------------------\n\n");
}