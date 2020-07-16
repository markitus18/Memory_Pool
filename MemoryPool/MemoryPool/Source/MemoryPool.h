#ifndef __MEMORY_POOL_H__
#define __MEMORY_POOL_H__

#include "MemoryBlock.h"

#include <cstddef>
#include <string>

typedef unsigned int uint;

class MemoryPool
{
public:
	// Allocates the memory for all the pool and sets up the block array to each segment of the memory
	MemoryPool(std::size_t blockSize, uint totalBlocks);

	// Releases all the memory, including the block array
	~MemoryPool();

	// Finds a suitable space in the pool which contains enough blocks to hold 'memSize'
	// It will start the search from 'blockCursor' which is the block next to the last added memory
	void* Reserve(std::size_t memSize);

	// Finds the block holding 'memory' ptr and flags it as available for new usage
	void Free(void* memory);

	// Flags all blocks as available for new usage
	void Clear();
	

	// Returns the amount of blocks needed for a specific amount of memory
	// As all blocks have the same size, adding 'blockSize - 1' makes the effect of rounding up (250/100 = 2, but it would use 3 blocks)
	// and we avoid some operations in the process.
	inline uint GetNeededBlocksForMemory(std::size_t memorySize) const { return (memorySize + firstBlock->blockSize - 1) / firstBlock->blockSize; };

	// Dumps the block information into a string
	std::string DumpPoolState() const;

	// Dumps the memory into a string
	std::string DumpMemoryState() const;

	inline uint GetTotalBlocks() const { return totalBlocks; };
	inline uint GetTotalMemory() const { return totalMemory; };
	inline uint GetUsedMemory() const { return memoryInUse; };

private:
	void FillUsedMemory(MemoryBlock* block, std::size_t memorySize);

private:
	//First block in the array
	MemoryBlock* firstBlock;

	//Last block in the array
	MemoryBlock* lastBlock;

	//Pointing to the last added memory
	MemoryBlock* blockCursor;

	//Total amount of blocks stored in the pool
	uint totalBlocks;

	//Total amount of memory that the pool can use (equals to totalBlocks * blockSize)
	uint totalMemory;

	//Amount of memory currently in use
	uint memoryInUse;
};

#endif //!__MEMORY_POOL_H__
