**-- MEMORY POOL CODE TEST -- Marc Garrigo (markitus18 on GitHub)**


**--- EXPLANATION OF THE PROGRAM ---**

The will test my approach to the memory pool against malloc in different situations and display a speed comparison.<br/>
Several tests will be run by default, trying to test the pool in different situations in order to evaluate its potential.<br/>
The main tests run by default are the following:

   - <ins>Speed Test</ins>: It will allocate memory from the pool until it is (almost) full. This test will be run 4 times,<br/>
		  testing big and small memory allocations, and releasing or not the memory after each allocation<br/>
		  (mallocs happens to get speed up when freeing the memory right after allocation).<br/>
		  
   - <ins>Random Allocation Test</ins>: Before starting any measurement, the test will allocate a variable percentage of the pool.<br/>
		  Once this is done, it will start firing random allocations and deallocations at random places of the pool.<br/>
	          This test will be run 4 times, testing big and small memory allocations, and different initial pool states.<br/>
Once all tests have been completed (4 speed tests and 4 random allocation speed tests) the program will display a "TEST OK"
message, indicating the end of it.

There are 2 other modes in which the program can be run:

   - <ins>Verbose mode</ins>: 	The program will allocate the memory of the pool in 2 different tests<br/>
   (one for big allocations, another one for small ones) and fill it with meaningful data.<br/>
   The test will then save the state of the pool in two files:
       - Pool State Dump: list of all the blocks and the amount of memory used by each block
       - Memory State Dump: a typical memory dump into a file. Each allocation done will be marked by "Start Alloc" and "End Alloc".<br/>
		  *It is desirable to execute this test in Debug, as the memory dump will be easier to read
		  
   - <ins>Bruteforce mode</ins>: This test is designed to stress the pool under extreme circumstances.<br/>
   It will generate 100 pools of randomized blocks and block sizes, and run the default tests (explained above) on each pool.<br/>
   Even though the speed results can be read, the goal of this test is to find any possible errors in the pool during development.


**--- INSTRUCTIONS ---**

There are a few command line options that can be sent to the program in order to execute its different modes.<br/>
If no argument is specified, the program will run the default test (4 speed tests + 4 random allocation speed tests)<br/>
with default pool paramaters (1024 blocks of 128 bytes). All arguments must include a value.<br/>
Command line arguments:
   - t (int) Specifies the test mode to perform<br/>
   		0 - Default: 		Executes 4 speed tests + 4 random allocation speed test<br/>
   		1 - Verbose: 		Executes 'Verbose mode' (see explanation above). If can also work with -b and -s parameters</br>
   		2 - Bruteforce: 	Executes 'Bruteforce mode' (see explanation above).<br/>
	    	It will ignore all the other parameters and execute tests at random.<br/>
   	    	This mode will take a while to execute (5 to 10 minutes) as it will perform ~10^5 tests in total.<br/>
   	    	The test results can be seen during the process.<br/>
   		3 - Individual Speed 	Executes 1 speed test. <br/>
   		4 - Individual Random	Executes 1 random allocation speed test.<br/>

   - b (int) Amount of blocks for the pool. Works with all test modes except 'Bruteforce'<br/>
   Valid range from 1 to 10.000. Defaults to 1024.

   - s (int) Size of each block in bytes. Works with all test modes except 'Bruteforce'.<br/>
   Valid range from 1 to 2.048. Defaults to 128.

   - m (int) Amount of memory allocated in each iteration of the test. Only used if an 'Individual Test' was specified. <br/>
   Valid range from 1 to 'block size * block count'. Defaults to 'block size'.

   - c (int) Amount of iterations (cycles) of the test.<br/>
   Valid range from 1 to 10.000. The amount of cycles will be reduced if the test would overrun the pool's capacity.
			
   - r (int) Initial memory ratio allocated from the pool. Only used if test mode (t) is specified to 4 - Individual Random. <br/>
   Valid range from 0 to 100. Defaults to 100.


**--- DEVELOPMENT APPROACH ---**

My main goal in this test was to create a memory pool that was (almost) as easy to use as malloc. This can be seen in the project as the
only functions necessary to run the pool are Reserve and Free. The user will be fully in charge of managing the memory as he will
simply receive and use void* when requesting / freeing memory.

The approach in developing the pool was to create an initial system as simplistic as possible, but compact and ready to measure.
I started with a system that would allocate a big amount of memory and an array of blocks each block would be assigned a portion of
that memory of fixed size. When the pool got a memory request, it would search through the blocks until if found a block (or a set of
blocks) holding enough memory for the request. The pool would iterate from the beginning of the array of blocks when new memory
was requested. I expected this logic to be quite slower than malloc, but it was a solid base to work on. I added test measurements at
this point to be able to see clearly the impact any change in the pool would have.

From that base I added the cursor, a ptr that would point to the first available block in the pool. As more memory was being requested,
the cursor would advance to the next available block (i.e: when the pool is empty, and 3 blocks are reserved, the cursor will point to the 4th
block). This removed the need to iterate the entire array of blocks from the beginning. I also changed the way the memory usage
was stored in the blocks: when reserving new memory, each block would store the amount of memory from the block to the end of the
reserved memory. For example, if we have a pool of blocks of 100 bytes, and we reserved 300 bytes, the memory usage data from each
block will look as so:

		 	|      Block 0     ||      Block 1     ||      Block 2     ||      Block 3     |
	        	| Used Memory: 300 || Used Memory: 200 || Used Memory: 100 || Used Memory: 000 | <- block 3 is free

This allowed to skip some unnecessary iterations when finding blocks that were unavailable

At this point, after performing some extensive tests, it was quite easy to see that once the pool got filled a first time, if random
allocations / deallocations happened, the amount of time taken to find free memory would be greaty increased, as the cursor would
lose part of its purpose (we could be pontentially iterating the whole array of blocks). To solve this issue, I tried adding a 'freedPtr' that
would point to the last freed block of memory.<br/>
Once the pool reached the end of the block array on time, the pool would start looking for new memory from this ptr. To my surprise,
this approach got things slower. The reason behind it is that this pointer would work only in a very specific situation: when allocating
the pool 100% of its capacity, then deallocating and allocating *only once* and repeating the process. This approach got discarded in the end.


**--- CONCLUSIONS AND FURTHER IMPROVEMENTS ---**

As I started this project I realized that a memory pool can be used in a very wide variety of situations. My goal then, was to create a set
of tests that could stress the pool in very different situations and then compare the results with malloc. I ended up with some quite
good tests, but I spent a lot more time than I intended developing them, specially the random test. In order to avoid contaminating the
performance test, I needed to do all random calculations before starting the test, and this was not an easy task. This situation reduced
significantly the amount of time I could spend developing the pool, as I had some cool ideas I wanted to try but had to discard in the end.
I am satisfied with the end result as the tests complexity allowed me to see all implications a small change in the pool could have,
even though I would've liked exploring other functionallities of the pool.<br/><br/>
Here is a list of further improvements and topics I am interested in exploring:
   - I believe the last approach (the freedPtr), even if did not work as expected, was in the right direction. The next step would be
   storing the blocks indices in a secondary array, placing the available blocks at the beginning (it is a similar approach at how Unreal
   manages the Particle pool). Storing the amount of available space in the first free block in a set of blocks seems a good way to go,
   as blocks could be easily skpipped when iterating.
   - Finding the smallest spot to allocate new memory. In an environment where different memory sizes were requested, allocating
   new memory on the smallest possible space would reduce memory overheads. This option will probably generate an expensive
   time cost, but it could be combined with the previous improvement.
   - Blocks that could be subdivided into smaller blocks. At this moment the pool has fixed size for all blocks and the amount of
   memory overhead for different memory sizes can be really big (3 blocks of 100 bytes storing 250 bytes of memory are wasting 50
   bytes). Dividing a block in smaller sizes would produce less overheads.
   - Hierarchial pool: The fact that releasing all the memory from a scene could be done in a single 'Free' call fascinates me. Storing the
   blocks in a tree hierarchy, where all the smaller objects in a scene could be holding from a 'root' block and could be freed by
   releasing this root.
   - Saving and loading the game state through the memory dump: If all memory allocations are done in the exact order everytime,
   the state of a game could be saved just by storing the memory from the pool into a file.

