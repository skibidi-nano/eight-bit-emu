#include "main.h"

my_chip chip;

int main(void)
{

    int run = 1;

    while(run)
    {
        



        switch (opcode)
        {
            
        }
    }

    printf("CPU shutting down");

    return 0;
}

void cycle(void)
{
    uint16_t opcode = (chip.memory[chip.pc] << 8) | chip.memory[chip.pc + 1];
    chip.pc += 2;

    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t kk = opcode & 0x00FF;
    uint16_t nnn = opcode & 0x0FFF;

    switch (opcode & 0xF000)
    {
        case 0x1:
            chip.pc = nnn;
            break;
        
        case 0x2:
            chip.stack[chip.st] = chip.pc;
            chip.st++;
            chip.pc = nnn; 
            break;

        case 0x3:
            if (chip.v_reg[x] == kk)
            {
                chip.pc += 2;
            }
            break;

        case 0x4:
            if (chip.v_reg[x] != kk)
            {
                chip.pc += 2;
            }
            break;

        case 0x5:
            if (chip.v_reg[x] == chip.v_reg[y])
            {
                chip.pc += 2;
            }
            break;

        case 0x6:
            chip.v_reg[x] = kk;
            break;

        case 0x7:
            chip.v_reg[x] += kk;
            break;
        
        case 0x8:
            switch(opcode & 0x000F)
            {
                case 0x0:
                    chip.v_reg[x] = chip.v_reg[y];
                    break;

                case 0x1:
                    chip.v_reg[x] |= chip.v_reg[y];
                    break;

                case 0x2:
                    chip.v_reg[x] &= chip.v_reg[y];
                    break;

                case 0x3:
                    chip.v_reg[x] ^= chip.v_reg[y];
                    break;

                case 0x4:
                    chip.v_reg[x] += chip.v_reg[y];
                    if ((chip.v_reg[x] + chip.v_reg[y]) > 0x00F0)
                    {
                        chip.vf = true;
                    }
                    break;

                case 0x5:
                    chip.vf = false;
                    if (chip.v_reg[x] > chip.v_reg[y])
                    {
                        chip.vf = true;
                    }
                    chip.v_reg[x] -= chip.v_reg[y];
                    break;

                case 0x6:
                    if ((chip.v_reg[x] & 1) == 1)
                    {
                        chip.vf = 1;
                    }
                    else
                    {
                        chip.vf = 0;
                    }
                    chip.v_reg[x] >>= 1;
                    break;


            }
            
            break;

        case 0x9:
            
            break;


    }
}