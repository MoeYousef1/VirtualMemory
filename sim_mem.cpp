#include <iostream>
#include <csignal>
#include <fcntl.h>
#include "sim_mem.h"

#define MEMORY_SIZE 200
using namespace std;
char main_memory[MEMORY_SIZE];
static int counter = -1;
static int indexOfSwap = -1;// counter to use with swap index
static int save=0; // in order to know if I used store
static int change=0; // in order to change old pages frames and valid

sim_mem::sim_mem(char exe_file_name1[], char exe_file_name2[], char swap_file_name[], int text_size, // instructor
                 int data_size, int bss_size, int heap_stack_size, int num_of_pages,
                 int page_size, int num_of_process) {

    this->text_size = text_size;
    this->data_size = data_size;
    this->bss_size = bss_size;
    this->heap_stack_size = heap_stack_size;
    this->num_of_pages = num_of_pages;
    this->page_size = page_size;
    this->num_of_proc = num_of_process;
    int text_pages = text_size / page_size;
    int arrOfSwapSize = page_size * (num_of_pages - text_pages) * num_of_process;
    char arrOfSwap[arrOfSwapSize];
    int i;

    if (num_of_process == 1) { // filling memory with zeros
        for (i = 0; i < MEMORY_SIZE; i++) {
            main_memory[i] = '0';
        }
        this->program_fd[0] = open(exe_file_name1, O_RDONLY, 0); // creating first file
        if (this->program_fd[0] == -1) {
            perror("File Couldn't Be Opened\n");
            exit(1);
        }
    } else if (num_of_process == 2) {
        this->program_fd[0] = open(exe_file_name1, O_RDONLY, 0); // creating files for 2 process
        this->program_fd[1] = open(exe_file_name2, O_RDONLY, 0);
        if (this->program_fd[0] == -1 || this->program_fd[1] == -1) {
            perror("One or Both The Files Couldn't Be Opened\n");
            exit(1);
        }
    }
    if (this->swapfile_fd != -1) { // creating swap file
        this->swapfile_fd = open(swap_file_name, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    } else {
        perror("swapfile failed");
        exit(1);
    }

    for (i = 0; i < arrOfSwapSize; i++) { // filling array of swap with zeros
        arrOfSwap[i] = '0';
    }

    for (i = 0; i < arrOfSwapSize; i++) {
        if (write(this->swapfile_fd, arrOfSwap, page_size) == -1) {
            perror("Write Failed");
            exit(1);
        }
    }

    this->page_table = (page_descriptor **) calloc(this->num_of_proc, sizeof(page_descriptor *));
    for (i = 0; i < num_of_process; i++) {
        this->page_table[i] = (page_descriptor *) calloc(num_of_pages, sizeof(page_descriptor));
    }
    if (this->page_table == nullptr) {
        printf("Allocation failed\n");
        return;
    }
    for (i = 0; i < num_of_process; i++) { // initializing
        for (int j = 0; j < num_of_pages; j++) {
            this->page_table[i][j].V = 0;
            this->page_table[i][j].D = 0;
            this->page_table[i][j].frame = -1;
            this-> page_table[i][j].swap_index = -1;

            if (j < text_pages) {
                this->page_table[i][j].P = 0;
            } else {
                this->page_table[i][j].P = 1;
            }
        }
    }
    fillSwap = new char[arrOfSwapSize]; // array to fill the swap
    for (int j = 0; j < arrOfSwapSize; j++) {
        fillSwap[j] = '0';

    }
}

sim_mem::~sim_mem() {
    if (this->num_of_proc == 1) { // closing swap
        if (close(this->swapfile_fd) == -1) {
            perror("close Filed");
            exit(1);
        }

        if (close(this->program_fd[0]) == -1) { // closing first exec file
            perror("close Filed");
            exit(1);
        }
    }
    if (this->num_of_proc == 2) { // same thing for 2 process
        if (close(this->swapfile_fd) == -1) {
            perror("close Filed");
            exit(1);
        }
        if (close(this->program_fd[0]) == -1 || close(this->program_fd[1]) == -1) {
            perror("close Filed");
            exit(1);
        }
    }
    delete[] this->fillSwap; // deleting the array used to fill the swap

    for (int i = 0; i < this->num_of_proc; i++) { // freeing page table
        free(this->page_table[i]);
    }
    free(page_table);
}

char sim_mem::load(int process_id, int address) {
    save--;
    int page = address / this->page_size;
    int offset = address % this->page_size;
    int phyAddress;
    char buf[this->page_size];
    int newFrame = 0;
    int arrFrame;
    int iD = process_id - 1;

    if (address < 0) {
        fprintf(stderr, "Address Can't Be Negative\n");
        return '\0';
    }

    if (page >= this->num_of_pages) {
        fprintf(stderr, "Out Of Bound Error\n");
        return '\0';
    }

    if (this->page_table[iD][page].V == 1) {
        newFrame = this->page_table[iD][page].frame;
        phyAddress = (newFrame * this->page_size) + offset;
        return main_memory[phyAddress];
    }

    if (this->page_table[iD][page].P == 0) {// text
        lseek(this->program_fd[iD], this->page_size * page, SEEK_SET);
        if (read(this->program_fd[iD], buf, this->page_size) != this->page_size) {
            perror("exe read failed");
            return '\0';
        }
        return virSwap(process_id, page, newFrame, arrFrame, offset, phyAddress, buf,address);
    }

    if (this->page_table[iD][page].D == 1) { // dirty
        lseek(this->swapfile_fd, this->page_size * page, SEEK_SET);
        if (read(this->swapfile_fd, buf, this->page_size) != page_size) {
            perror("swap read failed");
            return '\0';
        }
        return virSwap(process_id, page, newFrame, arrFrame, offset, phyAddress, buf,address);
    } else if (this->page_table[iD][page].D == 0) { // not dirty
        if (address >= this->text_size && address < this->text_size + this->data_size) {
            lseek(this->program_fd[iD], this->page_size * page, SEEK_SET);
            if (read(this->program_fd[iD], buf, this->page_size) != this->page_size) { // no valid and it's a text
                perror("exe read failed");
                return '\0';
            }
            return virSwap(process_id, page, newFrame, arrFrame, offset, phyAddress, buf,address);
        } else if (page >= this->text_size + this->data_size) { // the page is stack or heap, and it's not dirty , print error
            perror("couldn't load");
            return '\0';
        }else if (page>=(this->text_size/this->page_size)+(this->data_size/this->page_size)&& page < (this->text_size/this->page_size)+(this->data_size/this->page_size) + (this->bss_size/this->page_size)){
            char tmp[this->page_size];   // bss , we fill it with zeros
            for(int i = 0; i < this->page_size; i++){
                tmp[i] = '0';
            }
            return virSwap(process_id, page, newFrame, arrFrame, offset, phyAddress, tmp,address);
        }
    }

    return '\0';
}

int sim_mem::find_Frame() { // find and empty frame
    int i;
    int j;
    int count;
    for (i = 0; i < MEMORY_SIZE; i += this->page_size) {
        count = 0;
        for (j = 0; j < this->page_size; j++) {
            if (main_memory[j + i] == '0') {
                count++;
            }
            if (count == this->page_size) {
                this->memQueue.push((i / this->page_size));
                return 1;
            }
        }
    }
    return -1;
}

int sim_mem::swaper(int newFrame, int page, int process_id, int address) { // swap out pages if the memory is full, and we have no empty frame
    indexOfSwap++;
    if (counter < (MEMORY_SIZE / this->page_size)-1) { // this will keep  giving me numbers to help place the new page in the place of the swapped page
        counter++;
    } else {
        counter = 0;
    }

    if (change < (MEMORY_SIZE / this->page_size)&&save<0) { // this will help change the frame and the valid of old pages that went to the swap
        change++;
    }
    newFrame += counter;
    int index;
    int iD = process_id - 1;
    if(this->page_table[iD][page].D==0) {
        this->page_table[iD][page].frame = newFrame;
    }
    if(this->page_table[iD][page].D==1&&save>=0){ // if its from store
        this->page_table[iD][page - (((address)/this->page_size)-indexOfSwap)].V = 0;
        this->page_table[iD][page - (((address)/this->page_size)-indexOfSwap)].D= 1;
        this->page_table[iD][page - (((address)/this->page_size)-indexOfSwap)].swap_index = indexOfSwap * this->page_size;}

    if(this->page_table[iD][page].D==0&&save<0){ // if store was not used
        this->page_table[iD][page - (((address)/this->page_size)-indexOfSwap)].frame=-1;
        this->page_table[iD][page - (((address)/this->page_size)-indexOfSwap)].V = 0;
        this->page_table[iD][page - (((address)/this->page_size)-indexOfSwap)].swap_index = -1;}

    this->memQueue.pop();
    char buf[this->page_size];
    char buf2[this->page_size];

    int swapLength = (this->num_of_pages - (this->text_size / this->page_size)) * this->num_of_proc * this->page_size;
    for (int i = 0; i < swapLength; i++) { // filling the swap array if its empty index
        if (fillSwap[i] == '0') {
            index = i;
            fillSwap[i] = (char) index;
            break;
        }
    }

    int k = 0;
    for (int i = newFrame * this->page_size; i < this->page_size + newFrame * this->page_size; ++i) {
        buf[k++] = main_memory[i]; // moving from main memory to buffer in place of old page that has been swapped
    }
    for (int i = newFrame * this->page_size; i < this->page_size; ++i) { // filling memory with zeros
        main_memory[i] = '0';
    }
    if(this->page_table[iD][page].D==1){ // writing the buffer to the swap
        lseek(this->swapfile_fd, index * this->page_size, SEEK_SET);
        if (write(this->swapfile_fd, buf, this->page_size) == -1) {
            perror("couldn't write to swap");
            return -1;
        }
    }
    for (int i = 0; i < this->num_of_pages; ++i) { // changing things
        if (this->page_table[iD][i].swap_index != -1) {
            this->page_table[iD][i].V = 0;
            this->page_table[iD][i].frame = -1;
        }}

    int count=0;
    for (int i = 0; i < this->num_of_pages; ++i) { // get number of v =1 ;
        if (this->page_table[iD][i].V==1) {
            count++;
        }}
    for (int i = 0; i < this->num_of_pages; ++i) { // change old pages that been swapped to v =0
        while (count>=change&&this->page_table[iD][i].V ==1) {
            this->page_table[iD][i].V = 0;
            this->page_table[iD][i].frame = -1;
            count--;
        }
    }



    newFrame *= this->page_size;
    lseek(this->program_fd[iD], this->page_size * page, SEEK_SET);
    if (read(this->program_fd[iD], buf2, this->page_size) != this->page_size) {
        perror("exe read failed");
        return '\0';
    }

    int i;
    for (i = 0; i < this->page_size; ++i) {
        main_memory[newFrame++] = buf2[i]; // fill the memory with new page in place of the old one starting from newFrame
    }

    return newFrame;
}

void sim_mem::store(int process_id, int address, char value) {
    save++;
    int page = address / this->page_size;
    int offset = address % this->page_size;
    int phyAddress;
    char buf[this->page_size];
    int newFrame = 0;
    int arrFrame;
    int iD = process_id - 1;

    if (address < 0) {
        fprintf(stderr, "Address Can't Be Negative\n");
        return;
    }

    if (page >= this->num_of_pages) {
        fprintf(stderr, "Out Of Bound Error\n");
        return;
    }

    if (this->page_table[iD][page].V == 1) { // valid
        newFrame = this->page_table[iD][page].frame;
        phyAddress = (newFrame * this->page_size) + offset;
        this->page_table[iD][page].D = 1;
        main_memory[phyAddress] = value;
    }

    if (this->page_table[iD][page].P == 0) { // text , not allowed in store
        printf("U have no permission to write in this page\n");
        return;
    }

    if (this->page_table[iD][page].D == 1) { // if it was in the swap and it's dirty
        lseek(this->swapfile_fd, this->page_size * page, SEEK_SET);
        if (read(this->swapfile_fd, buf, this->page_size) != this->page_size) {
            perror("exe read failed");
            return;
        }
        virSwap1(process_id, page, newFrame, arrFrame, offset, phyAddress, value, buf,address);
    } else if (this->page_table[iD][page].D == 0) { // if it's first time using it
        if (address >= this->text_size && address < this->text_size + this->data_size) {
            lseek(this->program_fd[iD], this->page_size * page, SEEK_SET);
            if (read(this->program_fd[iD], buf, this->page_size) != this->page_size) {
                perror("exe read failed");
                return;
            }
            virSwap1(process_id, page, newFrame, arrFrame, offset, phyAddress, value, buf,address);
        } else if (address >= this->text_size + this->data_size) { // if its stack or heap
            this->page_table[iD][page].D = 1;
            this->page_table[iD][page].swap_index = address;

            for (int i = 0; i < this->page_size; i++) // fill buf with zeros
                buf[i] = '0';

            virSwap1(process_id, page, newFrame, arrFrame, offset, phyAddress, value, buf,address);
        }

    }
}

char sim_mem::virSwap(int process_id, int page, int newFrame, int arrFrame, int offset, int phyAddress, const char *buf, int address) { // will decide if we have a frame or not
    int iD = process_id - 1;
    if (find_Frame() == 1) { // we have an empty frame
        this->page_table[iD][page].frame = this->memQueue.front();
        newFrame = this->page_table[iD][page].frame * this->page_size;
        arrFrame = this->page_table[iD][page].frame * this->page_size;
        this->memQueue.pop();
        for (int i = 0; i < this->page_size; i++) { // move from buf to memory
            main_memory[arrFrame++] = buf[i];
        }
        this->page_table[iD][page].V = 1;
        phyAddress = (newFrame) + offset;
        return main_memory[phyAddress];

    } else if (find_Frame() == -1) { // no frame
        this->memQueue.push(swaper(newFrame, page, process_id,address)); // swap out a page
        this->page_table[iD][page].V = 1;
        phyAddress = (this->page_table[iD][page].frame * this->page_size) + offset;
        return main_memory[phyAddress];
    }
    return '\0';
}

void sim_mem::virSwap1(int process_id, int page, int newFrame, int arrFrame, int offset, int phyAddress, char value, const char *buf, int address) { // same as virSwap but void
    int iD = process_id - 1;
    if (find_Frame() == 1) {
        this->page_table[iD][page].frame = this->memQueue.front();
        newFrame = this->page_table[iD][page].frame * this->page_size;
        arrFrame = this->page_table[iD][page].frame * this->page_size;
        this->memQueue.pop();
        for (int i = 0; i < this->page_size; i++) {
            main_memory[arrFrame++] = buf[i];
        }
        this->page_table[iD][page].V = 1;
        phyAddress = (newFrame) + offset;
        main_memory[phyAddress] = value;

    } else if (find_Frame() == -1) {
        this->memQueue.push(swaper(newFrame, page, process_id,address));
        this->page_table[iD][page].V = 1;
        phyAddress = (this->page_table[iD][page].frame * this->page_size) + offset;
        main_memory[phyAddress] = value;
    }
}


void sim_mem::print_memory() {
    int i;
    printf("\n Physical memory\n");
    for (i = 0; i < MEMORY_SIZE; i++) {
        printf("[%c]\n", main_memory[i]);
    }
}

void sim_mem::print_swap() const {
    char *str = static_cast<char *>(malloc(this->page_size * sizeof(char)));
    int i;
    printf("\n Swap memory\n");
    lseek(this->swapfile_fd, 0, SEEK_SET); // go to the start of the file
    while (read(this->swapfile_fd, str, this->page_size) == this->page_size) {
        for (i = 0; i < this->page_size; i++) {
            printf("%d - [%c]\t", i, str[i]);
        }
        printf("\n");
    }
    free(str);
}

void sim_mem::print_page_table() {
    int i;
    for (int j = 0; j < this->num_of_proc; j++) {
        printf("\n page table of process: %d \n", j);
        printf("Valid\t Dirty\t Permission \t Frame\t Swap index\n");
        for (i = 0; i < this->num_of_pages; i++) {
            printf(" [%d]   \t  [%d]   \t[%d]        \t  [%d]     \t[%d]\n",
                   this->page_table[j][i].V,
                   this->page_table[j][i].D,
                   this->page_table[j][i].P,
                   this->page_table[j][i].frame,
                   this->page_table[j][i].swap_index);
        }
    }
}