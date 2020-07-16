#ifndef __MEMORY_BLOCK_H__
#define __MEMORY_BLOCK_H__

typedef unsigned int uint;

class MemoryBlock
{
public:
	//Memory blocks function like a linked list.
	//'next' is the pointer to the following memory block in the list
	MemoryBlock* next;

	//The amount of memory this chunk is in charge of.
	//All chunks will have the same size, given at the start of the program (by now)
	uint blockSize;

	//Amount of memory used by this block. 'usedSize' can be bigger than 'blockSize'
	//as one element can occupy multiple blocks. The starting block for that element
	//will have a 'usedSize' equal to the total size of the element.
	uint usedSize;

	//Ptr to the start of the memory handled by this block. All blocks will store a pointer to
	//different parts of the memory pool, each block separated by 'blockSize'
	//i.e: first block in the list points to the start of the pool, second block points to start + usedSize,
	//third block points to start + usedSize * 2
	unsigned char* data;

#ifdef _DEBUG
	//Storing the block index for debugging purposes, it won't be used in any logic of the pool
	uint blockIndex;
#endif
};

#endif //!__MEMORY_BLOCK_H__
