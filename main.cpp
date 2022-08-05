#include <iostream>
#include "sim_mem.h"

using namespace std;

int main() {
    char val;
    char a;
    char b;
    char c;
    char d;
    char e;
    char f;
    char g;
    char h;
    char i;

    sim_mem mem_sm((char *) "exec_file1", (char *) "", (char *) "swap_file", 25, 50, 25, 25, 25, 5, 1);

    mem_sm.store(1, 97, 'X');
    val = mem_sm.load(1, 97);

    a = mem_sm.load(1, 0);
    b = mem_sm.load(1, 5);
    c = mem_sm.load(1, 10);
    d = mem_sm.load(1, 15);
    e = mem_sm.load(1, 20);
    f = mem_sm.load(1, 25);
    g = mem_sm.load(1, 30);
    h = mem_sm.load(1, 35);
    i = mem_sm.load(1, 40);

    sim_mem::print_memory();
    mem_sm.print_swap();
    mem_sm.print_page_table();

    printf("%c\n", val);
    printf("%c\n", a);
    printf("%c\n", b);
    printf("%c\n", c);
    printf("%c\n", d);
    printf("%c\n", e);
    printf("%c\n", f);
    printf("%c\n", g);
    printf("%c\n", h);
    printf("%c\n", i);
}