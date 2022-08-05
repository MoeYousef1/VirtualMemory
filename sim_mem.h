
#ifndef MAIN_SIM_MEM_H
#define MAIN_SIM_MEM_H
#define MEMORY_SIZE 200

#include <fcntl.h>
#include <istream>
#include <fstream>
#include <csignal>
#include <queue>

using namespace std;
extern char main_memory[MEMORY_SIZE];

typedef struct page_descriptor {
    int V; // valid
    int D; // dirty
    int P; // permission
    int frame; //the number of a frame if in case it is page-mapped
    int swap_index; // where the page is located in the swap file.
} page_descriptor;

class sim_mem {
    int swapfile_fd{}; //swap file fd
    int program_fd[2]{}; //executable file fd
    int text_size;
    int data_size;
    int bss_size;
    int heap_stack_size;
    int num_of_pages;
    int page_size;
    int num_of_proc;
    page_descriptor **page_table{}; //pointer to page table
    queue<int> memQueue;
    char *fillSwap;


public:
    sim_mem(char exe_file_name1[],char exe_file_name2[], char swap_file_name2[], int text_size,
            int data_size, int bss_size, int heap_stack_size,
            int num_of_pages, int page_size, int num_of_process);
    ~sim_mem();
    char load(int process_id, int address);
    void store(int process_id, int address, char value);
    static void print_memory();
    void print_swap() const;
    void print_page_table();
    int find_Frame();
    int swaper(int newFrame , int page, int process_id, int address);
    char virSwap(int process_id, int page, int newFrame, int arrFrame, int offset, int phyAddress, const char *buf, int address);
    void virSwap1(int process_id, int page, int newFrame, int arrFrame, int offset, int phyAddress, char value, const char *buf, int address);

};
#endif //MAIN_SIM_MEM_H