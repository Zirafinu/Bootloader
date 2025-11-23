//
// Created by sebastian on 23/11/2025.
//
#include <string.h>

extern void _start();

// the addresses to constant data
extern size_t data_start;
extern size_t data_load_start;
extern size_t data_load_size;
// the addresses to fast_text
extern size_t fast_text_start;
extern size_t fast_text_load_start;
extern size_t fast_text_load_size;
// the addresses to fast_data
extern size_t fast_data_start;
extern size_t fast_data_load_start;
extern size_t fast_data_load_size;

void Reset_Handler() {
    memcpy(&data_start, &data_load_start, (size_t)&data_load_size);
    memcpy(&fast_text_start, &fast_text_load_start, (size_t)&fast_text_load_size);
    memcpy(&fast_data_start, &fast_data_load_start, (size_t)&fast_data_load_size);

    // .bss is correctly handled by _start
    _start();
}