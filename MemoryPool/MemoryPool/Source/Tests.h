#ifndef __SPEED_TESTS_H__
#define __SPEED_TESTS_H__

#include "Defs.h"

extern class MemoryPool* memoryPool;

enum class TestType
{
	Default				= 0,
	Verbose				= 1,
	Bruteforce			= 2,
	Individual_Speed	= 3,
	Individual_Random	= 4,
	Unknown				= 5
};

struct TestSettings
{
	uint blockSize = DEFAULT_BLOCK_SIZE;
	uint blocks = DEFAULT_BLOCKS;

	uint testType = 0u;

	uint memoryAllocatedPerCycle = 0u;
	uint testCycles = 0u;
	uint memoryRatioAllocated = 100u; //Range from 0 to 100
};

class Tests
{
public:
	// Used to call any test type with different parameters.
	static void GenericTest(TestSettings& settings);

	// Creates a new pool and runs several tests to check the performance under different circumstances
	// It will run several 'SimpleSpeedTest' and 'SpeedTest_RandomAfterAllocation'
	//   param 'blockSize'						- The size of each block in the pool
	//   param 'blocks'							- The amount of blocks in the pool
	static void FullTestsOnSinglePool(size_t blockSize, uint blocks);

	// This test will allocate a bunch of memory from the pool and measure it against doing the same process with malloc.
	// WARNING: The test will be applied to the current memory pool. No safety size-measures are checked
	//	 param 'memorySize'					- The amount of memory that should be allocated each time
	//	 param 'cycles'						- The amount of times it will be allocating memory
	//	 param 'freeMemoryAfterAllocation'	- Flag to free the memory right after it has been allocated.
	//										- If set to false, the memory will be freed after the test
	//	 param 'testsToPerform'				- The amount of tests that will be performed, displaying averaged results from all the tests
	static bool SimpleSpeedTest(size_t memorySize, uint cycles, bool freeMemoryAfterAllocation, uint testsToPerform = 100);

	// The test will create a new pool, fill it with random data and its state in 'Pool State Dump.txt' and the memory state in 'Pool Memory Dump.txt'
	// It will perform two verbosity tests, one filling allocating 50% of the block size and another one allocating 350%
	// All allocated data will be filled with ordered chars for easier debugging. It is suggested to execute this test in debug, as the memory
	// is not cleaned when executing Release, so it will be harder to read.
	//   param 'blockSize'						- The size of each block in the pool
	//   param 'blocks'							- The amount of blocks in the pool
	static void SimpleVerbosityTest(size_t blockSize, uint blocks);

	// A more complex tests that will initially allocate a specific amount of memory from the pool
	//		*This process will not be taken into account for the test measurements
	// WARNING: The test will be applied to the current memory pool. No safety size-measures are checked
	// After this initial allocation has been done, the test will start firing random allocations / deallocations
	//   param 'memorySize'					- The amount of memory that should be allocated / freed in each operation
	//   param 'cycles'						- The amount of operations that will be performed during the test
	//   param 'initialMemoryRatio'			- The percentage ratio [0 to 1] in which the pool will be filled before beginning the test
	//   param 'testsToPerform'				- The amount of tests that will be performed, displaying averaged results from all tests
	static bool SpeedTest_RandomAfterAllocation(size_t memorySize, uint cycles, float initialMemoryRatio, uint testsToPerform = 100);

	// A test to stress the pool system under extreme and random situations. It will fire several runs of the program creating pools with
	// different blocks and block sizes and run some speed tests on each pool.
	//   param 'testsPerformed'				- The amount of tests that will be done. Each test will use a different pool with randomized parameters
	static void BruteForceTest(uint testsPerformed = 100u);

	
private:
	// Proceeds with the speed test by the given parameters (check definitions in 'SimpleSpeedTest')
	// This test will not apply safety measures of pool size
	//   param 'usePool'					- Flag to set wether we use the pool or malloc for the test 
	static uint PerformSimpleSpeedTest(size_t memorySize, uint cycles, bool freeAfterAllocation, uint testsToPerform, bool usePool);

	// This test will execute a something similar to 'SimpleSpeedTest' on the pool with the goal of logging all the pool data in output files
	// It will fill the pool with random data and output the pool state in 'Pool State Dump.txt' and the memory state in 'Pool Memory Dump.txt'
	// All allocated data will be filled with ordered chars for easier debugging. It is suggested to execute this test in debug, as the memory
	// Is not cleaned when executing Release, so it will be harder to follow.
	// WARNING: The test will be applied to the current memory pool.
	//	 param 'memorySize'					- The amount of memory that should be allocated each time
	//	 param 'cycles'						- The amount of times it will be allocating memory
	//	 param 'testName'					- The output file name to save the data (Memory State Dump_<testName> and Pool State Dump_<testName)
	static bool PerformSimpleVerbosityTest(size_t memorySize, uint cycles, const char* testName);

	// Proceeds with the random after allocation speed test by the given parameters (check definitions in 'SpeedTest_RandomAfterAllocation')
	// It will prepare all the random calculations before calling the actual test ('PerformSpeedTest_RandomAfterAllocation')
	// This test will not apply safety measures of pool size
	//   param 'usePool'					- Flag to set wether we use the pool or malloc for the test 
	static uint PrepareSpeedTest_RandomAfterAllocation(uint memorySize, uint cycles, uint testsPerformed, float initialMemoryRatio, bool usePool);

	// Performs the random after allocation speed test. It will receive the random information already calculated (see 'PrepareSpeedTest_RandomAfterAllocation')
	// It will first allocate some memory from the pool and then start with the test, firing the random allocations / deallocations
	//   param 'initialAllocations'			- Amount of memory allocations that should be done before the test
	//   param 'maxAllocations'				- Max number of allocations that the pool can support
	//   param 'usePool'					- Flag to set wether we use the pool or malloc for the test 
	//   param 'randomAllocations'			- Ready-to-use array with random information on wether the test should allocate or free memory at each cycle
	//   param 'operationsStorageIndex'		- Ready-to-use array with (partial) random information on where the test should store (or free) the allocated memory
	static uint PerformSpeedTest_RandomAfterAllocation(size_t memorySize, uint cycles, uint initialAllocations, uint maxAllocations, bool usePool,
														const bool* const& randomAllocations, const uint* const& operationsStorageIndex);
};


#endif //!__SPEED_TESTS_H__
