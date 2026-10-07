#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>

typedef struct 
{
    uint8_t  memory[4096];
    uint8_t  v_reg[16];
    uint16_t index_reg;
    uint16_t pc;
    uint8_t  sp;
    uint16_t stack[16];
    uint8_t  delay_timer;
    uint8_t  sound_timer;
    uint8_t  gfx[64][32];
    bool     keypad[16];
    bool     run;
} 
my_chip;

#endif