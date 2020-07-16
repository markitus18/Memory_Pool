Initial Approach: slowest but smallest and fastest to integrate
Iterations:
	1. Save a pointer to the last added memory, to keep checking from there
	2. When finding a used block, skip the amount of blocks it is occupying
	
Realization: malloc is faster when using free

	3. Realized the need for performing averaged tests. As a single malloc test could have a big range in performance
		Actual numbers: 5000 bytes x 100 ranged from 100 to 230ms
				100 bytes * 5000 ranged from 900 to as much as 1800!!
	4. Iterations on "FillUsedMemory". Tried an approach in which I was calculating the amount of blocks to skip and then
		jumping the cursor in one step (original function works by skipping block by block until all memory has been skipped).
		The new approach was faster in tests with fewer allocations of bigger size (5000 bytes x 100 cycles) but was slower in
		a lot of allocations with smaller sizes (100 bytes x 5000 cycles). As, in any case, the second type of tests were slower,
		I stuck to the original approach for filling the memory.

Realization: malloc got faster as more tests were being performed in a single execution. Malloc was taking in average 110ms in the first
test (done 100 times), and blasting the program with tests (a total of 100.000) got malloc down to 27ms. Probably as the OS keeps being asked
for new memory constantly, it prepares it so the next time will be faster to find.