#include "MemoryPool.h"

#include <iostream>
#include <cassert>
#include <stdlib.h>

#include "Archive.h"

#define ASSERT(condition, statement) { if (!condition) { std::cerr << statement; assert(condition); }}

MemoryPool::MemoryPool(std::size_t blockSize, uint totalBlocks) : totalBlocks(totalBlocks), memoryInUse(0), totalMemory(totalBlocks * blockSize)
{
	//Initialize memory pool
	std::size_t totalMemorySize = blockSize * totalBlocks;
	unsigned char* data = (unsigned char*)malloc(totalMemorySize);
	
#ifdef _DEBUG
	memset(data, 0, totalMemorySize);
#endif

	//Initialize block array
	blockCursor = firstBlock = (MemoryBlock*)malloc(sizeof(MemoryBlock) * totalBlocks);
	MemoryBlock* prevBlock;

	//Initialize all block data
	for (uint i = 0u; i < totalBlocks; ++i)
	{
		unsigned char* blockDataPtr = data + blockSize * i;

		blockCursor->data = blockDataPtr;
		blockCursor->blockSize = blockSize;
		blockCursor->usedSize = 0;
#ifdef _DEBUG
		blockCursor->blockIndex = i;
#endif // _DEBUG

		blockCursor->next = i < totalBlocks - 1 ? blockCursor + 1 : nullptr;
		prevBlock = blockCursor;
		if (blockCursor->next)
		{
			lastBlock = blockCursor = blockCursor->next;
		}
	}

	lastBlock = blockCursor;
	blockCursor = firstBlock;
}


MemoryPool::~MemoryPool()
{
	free(firstBlock->data);
	free(firstBlock);
}

void* MemoryPool::Reserve(std::size_t memSize)
{
	//Finding a block that can hold 'memSize'.
	//We start iterating from the last added memory, as it is more likely to be free
	MemoryBlock* next = blockCursor->next;
	bool blockAvailable = false;
	uint checkedBlocks = 0u;

	while (checkedBlocks < totalBlocks && !blockAvailable)
	{
		//We have reached the end of the array, start over
		if (blockCursor == nullptr)
		{
			blockCursor = firstBlock;
		}
		//The current block is in use. Calculate the amount of blocks it is occupying and skip them
		if (blockCursor->usedSize > 0)
		{
			//Checking that we do not skip outside the array
			uint blocksToSkip = GetNeededBlocksForMemory(blockCursor->usedSize);
			uint dstFromLast = lastBlock - blockCursor;

			if (dstFromLast < blocksToSkip)
			{
				blockCursor = firstBlock;
				checkedBlocks += dstFromLast;
			}
			else
			{
				blockCursor += blocksToSkip;
				checkedBlocks += blocksToSkip;
			}
			continue;
		}

		//Find out if that block holds enough memory behind
		next = blockCursor->next;
		uint availableMemory = blockCursor->blockSize;
		while (availableMemory < memSize && next != nullptr && next->usedSize == 0)
		{
			availableMemory += next->blockSize;
			next = next->next;
			++checkedBlocks;
		}
		if (availableMemory >= memSize)
			blockAvailable = true;

		//At this point, if next == null, we reached the end of the block array
		//Otherwise, we reached a block which is not available, so we continue with the next one
		else
		{
			if (next == nullptr)
				blockCursor = nullptr;
			else
			{
				blockCursor = next->next;
				++checkedBlocks;
			}
		}
	}

#ifdef _DEBUG
	if (!blockAvailable)
	{
		std::string output = DumpPoolState();
		Archive::Write("CrashMemoryStateDump.txt", output.c_str(), output.size());
		ASSERT(blockAvailable, "Program ran out of memory!");
	}
#endif

	unsigned char* reservedData = blockCursor->data; //FillUsedMemory will modify "blockCursor" so we need to store the return ptr
	FillUsedMemory(blockCursor, memSize);
	return reservedData;
}

void MemoryPool::FillUsedMemory(MemoryBlock* block, std::size_t memorySize)
{
	memoryInUse += memorySize;
	
	while (memorySize > 0)
	{
		block->usedSize = memorySize; //<-- this could be avoided for the blocks after the first one, but the insignificant cost is worth the debugging facilitation
		block->blockSize > memorySize ? memorySize = 0 : memorySize -= block->blockSize;
		block = block->next;
	}
	blockCursor = block ? block : firstBlock; //Moving the pointer to the block after the last used one
}

void MemoryPool::Free(void* ptr)
{
	//By calculating the distance from the ptr to the beginning of the memory pool,
	//we can calculate how many blocks away the pointer is from the first block
	uint ptrDistance = ((std::uintptr_t)ptr) - (std::uintptr_t)firstBlock->data;
	uint blockDistance = ptrDistance / firstBlock->blockSize;

	MemoryBlock* block = firstBlock + blockDistance;
	ASSERT((block->data == (unsigned char*)ptr), "Tryinig to free an invalid ptr!"); //ptr does not match the expected block in the pool -> ptr is not valid
	ASSERT(block->usedSize != 0, "Trying to free an already freed memory"); //Attempting to free a memory block that is already free

	memoryInUse -= block->usedSize;

	//We are only clearing the freed memory for debugging purposes, so dumping the memory into a file is easier to read
#ifdef _DEBUG
	memset(block->data, 0, block->usedSize);
#endif

	//Flag to amount of used blocks as available
	uint blocksToFree = GetNeededBlocksForMemory(block->usedSize);
	while (blocksToFree > 0)
	{
		block->usedSize = 0;
		block = block->next;
		--blocksToFree;
	}
}

void MemoryPool::Clear()
{
	//Flag all blocks as available
	blockCursor = firstBlock;
	for (uint i = 0u; i < totalBlocks; ++i)
	{
		blockCursor->usedSize = 0u;
		blockCursor = blockCursor->next;
	}
	blockCursor = firstBlock;
}

std::string MemoryPool::DumpPoolState() const
{
	std::string ret;

	MemoryBlock* blockIt = firstBlock;

	for (uint i = 0u; i < totalBlocks; i += 10u)
	{
		for (uint b = 0u; b < 10u && i+b < totalBlocks; ++b)
		{
			char output[16];
#ifdef _DEBUG
			sprintf_s(output, "Block:  %04i | ", (blockIt+b)->blockIndex);
#else
			sprintf_s(output, "Block:  %04i | ", i + b);
#endif
			ret.append(output);
		}
		ret.append("\n");
		for (uint b = 0u; b < 10u && blockIt; ++b)
		{
			char output[16];
			sprintf_s(output, "Memory: %04i | ",blockIt->usedSize);
			ret.append(output);

			blockIt = blockIt->next;
		}
		ret.append("\n\n");
	}

	return ret;
}

std::string MemoryPool::DumpMemoryState() const
{
	return std::string((char*)firstBlock->data, totalMemory);
}