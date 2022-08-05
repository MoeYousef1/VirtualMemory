==Description==
ex5
In this program we will be creating sth that works like a RAM, we save pages in the main memory in frames , each frame is page_size length , in this case its 5, we look for empty frames
in the main memory , if we find one , then we fill it with the page , but if we don't have one , we have to swap out a page from the memory , then we enter the new page in the frame of the swapped
page.


=Program DATABASE=
char main_memory[MEMORY_SIZE]; // main memory
static int counter = -1; // using it to help re fill the frame in the needed place
static int indexOfSwap = -1;// counter to use with swap index
static int save=0; // in order to know if I used store
static int change=0; // in order to change old pages frames and valid
 buf , buf2 , tmp // to help move from the memory to the swap or the opposite

Functions:

Using 11 main functions

1)sim_mem(char exe_file_name1[],char exe_file_name2[], char swap_file_name2[], int text_size,
              int data_size, int bss_size, int heap_stack_size,
              int num_of_pages, int page_size, int num_of_process);
it's the constructor , we creat the files here, exec files and swap file , and we initialize every thing , we fill the memory with zeros , and the swap arrays with zeros too
we allocate place for page_table too

2) ~sim_mem();
to close and free every file and array

3) load
get from the logical memory to main memory in order to read;

4) store
same as load , but in order to change the value, not to read

5) find_Frame
check if we have an empty frame

6) swaper
if we have no empty frame , we use it to swap out pages and re fill new pages in their place

7) virSwap
does the work for the load function

8) virSwap1
does the work for the store function

9) print_memory();
print memory

10) print_swap() const;
print the swap


11) print_page_table();
print page table

==Program Files==

main.cpp / contains the main
sim_mem.h / contains the functions , header
sim_mem.cpp / contains all the work and the build of the functions

==How to compile==

g++ sim_mem.cpp main.cpp -o main
./main

==Input==

nothing

==Output==

this is what we get, but it will be different depending on the memory size, and the use of load and store

 Physical memory
[a]
[a]
[a]
[a]
[a]
[b]
[b]
[b]
[b]
[b]
[c]
[c]
[c]
[c]
[c]
[d]
[d]
[d]
[d]
[d]
[e]
[e]
[e]
[e]
[e]
[f]
[f]
[f]
[f]
[f]
[g]
[g]
[g]
[g]
[g]
[h]
[h]
[h]
[h]
[h]
[i]
[i]
[i]
[i]
[i]
[0]
[0]
[0]
[0]
[0]

 Swap memory
0 - [0]	1 - [0]	2 - [0]	3 - [0]	4 - [0]
0 - [0]	1 - [0]	2 - [0]	3 - [0]	4 - [0]
0 - [0]	1 - [0]	2 - [0]	3 - [0]	4 - [0]
0 - [0]	1 - [0]	2 - [0]	3 - [0]	4 - [0]
0 - [0]	1 - [0]	2 - [0]	3 - [0]	4 - [0]
0 - [0]	1 - [0]	2 - [0]	3 - [0]	4 - [0]
0 - [0]	1 - [0]	2 - [0]	3 - [0]	4 - [0]
0 - [0]	1 - [0]	2 - [0]	3 - [0]	4 - [0]
0 - [0]	1 - [0]	2 - [0]	3 - [0]	4 - [0]
0 - [0]	1 - [0]	2 - [0]	3 - [0]	4 - [0]
0 - [0]	1 - [0]	2 - [0]	3 - [0]	4 - [0]
0 - [0]	1 - [0]	2 - [0]	3 - [0]	4 - [0]
0 - [0]	1 - [0]	2 - [0]	3 - [0]	4 - [0]
0 - [0]	1 - [0]	2 - [0]	3 - [0]	4 - [0]
0 - [0]	1 - [0]	2 - [0]	3 - [0]	4 - [0]
0 - [0]	1 - [0]	2 - [0]	3 - [0]	4 - [0]
0 - [0]	1 - [0]	2 - [0]	3 - [0]	4 - [0]
0 - [0]	1 - [0]	2 - [0]	3 - [0]	4 - [0]
0 - [0]	1 - [0]	2 - [0]	3 - [0]	4 - [0]
0 - [0]	1 - [0]	2 - [0]	3 - [0]	4 - [0]

 page table of process: 0
Valid	 Dirty	 Permission 	 Frame	 Swap index
 [1]   	  [0]   	[0]        	  [0]     	[-1]
 [1]   	  [0]   	[0]        	  [1]     	[-1]
 [1]   	  [0]   	[0]        	  [2]     	[-1]
 [1]   	  [0]   	[0]        	  [3]     	[-1]
 [1]   	  [0]   	[0]        	  [4]     	[-1]
 [1]   	  [0]   	[1]        	  [5]     	[-1]
 [1]   	  [0]   	[1]        	  [6]     	[-1]
 [1]   	  [0]   	[1]        	  [7]     	[-1]
 [1]   	  [0]   	[1]        	  [8]     	[-1]
 [0]   	  [0]   	[1]        	  [-1]     	[-1]
