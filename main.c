#include "main.h"

my_chip chip;

const uint8_t fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void cycle(void);
bool read_rom(char* filename);

int main(void)
{

    chip.run = 1;

    int fontset_size = sizeof(fontset);
    for (int i = 0; i < fontset_size; i++)
    {
        chip.memory[0x50 + i] = fontset[i];
    }


    while(chip.run)
    {
        if(!read_rom("test.ch8"))
        {
            printf("AN ERROR HAS OCCURED");
            break;
        }
        cycle();
    }

    printf("CPU shutting down");

    return 0;
}

void cycle(void)
{
    uint16_t opcode = (chip.memory[chip.pc] << 8) | chip.memory[chip.pc + 1];
    chip.pc += 2;

    uint8_t n = (opcode & 0x000F);
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t kk = opcode & 0x00FF;
    uint16_t nnn = opcode & 0x0FFF;

    switch ((opcode & 0xF000) >> 12)
    {

        case 0x0:
            switch (kk)
            {
                case 0xE0:
                    memset(chip.gfx, 0, sizeof(chip.gfx));
                    break;

                case 0xEE:
                    if (chip.sp > 0)
                    {
                        chip.sp--;
                        chip.pc = chip.stack[chip.sp];
                    }
                    break;
            }
            break;

        case 0x1:
            chip.pc = nnn;
            break;
        
        case 0x2:
            chip.stack[chip.sp] = chip.pc;
            chip.sp++;
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
                {
                    uint16_t sum = chip.v_reg[x] + chip.v_reg[y];
                    chip.v_reg[x] = sum & 0xFF;
                    chip.v_reg[0xF] = (sum > 0xFF) ? 1 : 0;
                    break;
                }

                case 0x5:
                {
                    uint8_t flag = (chip.v_reg[x] >= chip.v_reg[y]) ? 1 : 0;
                    chip.v_reg[x] -= chip.v_reg[y];
                    chip.v_reg[0xF] = flag;
                    break;
                }

                case 0x6:
                {
                    uint8_t lsb = chip.v_reg[x] & 0x01;
                    chip.v_reg[x] >>= 1;
                    chip.v_reg[0xF] = lsb;
                    break;
                }

                case 0x7:
                {
                    uint8_t flag = (chip.v_reg[x] <= chip.v_reg[y]) ? 1 : 0;
                    chip.v_reg[x] = chip.v_reg[y] - chip.v_reg[x];
                    chip.v_reg[0xF] = flag;
                    break;
                }

                case 0xE:
                {
                    uint8_t msb = (chip.v_reg[x] & 0x80) >> 7;
                    chip.v_reg[x] <<= 1;
                    chip.v_reg[0xF] = msb;
                    break;
                }

            }

            break;

        case 0x9:
            if (chip.v_reg[x] != chip.v_reg[y])
            {
                chip.pc += 2;
            }
            break;

        case 0xA:
            chip.index_reg = nnn;
            break;

        case 0xB:
            chip.pc = nnn + chip.v_reg[0x0];
            break;

        case 0xC:
            chip.v_reg[x] = (rand() % 256) & kk;
            break;

        case 0xD:
        {
            uint8_t x_pos = chip.v_reg[x] % 64;
            uint8_t y_pos = chip.v_reg[y] % 32;
    
            chip.v_reg[0xF] = 0;

            for (int row = 0; row < n; row++) 
            {
                uint8_t sprite_byte = chip.memory[chip.index_reg + row];

                for (int col = 0; col < 8; col++)  
                {
                    if ((sprite_byte & (0x80 >> col)) != 0) 
                    {
                        uint8_t px = (x_pos + col) % 64;
                        uint8_t py = (y_pos + row) % 32;

                        if (chip.gfx[px][py] == 1) 
                        {
                            chip.v_reg[0xF] = 1;
                        }   
                        chip.gfx[px][py] ^= 1;
                    }
                }
            }
            break;
        }

        case 0xE:
            switch(kk)
            {
                case 0x9E:
                    if (chip.keypad[chip.v_reg[x]])
                    {
                        chip.pc += 2;
                    }
                    break;

                case 0xA1:
                    if (!chip.keypad[chip.v_reg[x]])
                    {
                        chip.pc += 2;
                    }
                    break;
            }
            break;


        case 0xF:
            switch(kk)
            {
                case 0x07:
                    chip.v_reg[x] = chip.delay_timer;
                    break;

                case 0x0A:
                {
                    bool key_pressed = false;

                    for (int i = 0; i < 16; i++)
                    {
                        if (chip.keypad[i])
                        {
                            chip.v_reg[x] = i;
                            key_pressed = true;
                            break;
                        }
                    }


                    if (!key_pressed)
                    {
                        chip.pc -= 2;
                    }

                    break;
                }

                case 0x15:
                    chip.delay_timer = chip.v_reg[x];
                    break;

                case 0x18:
                    chip.sound_timer = chip.v_reg[x];
                    break;

                case 0x1E:
                    chip.index_reg += chip.v_reg[x];
                    break;

                case 0x29:
                    chip.index_reg = 5 * chip.v_reg[x];
                    break;

                case 0x33:
                    chip.memory[chip.index_reg] = chip.v_reg[x] / 100;
                    chip.memory[chip.index_reg + 1] = (chip.v_reg[x] / 10) % 10;
                    chip.memory[chip.index_reg + 2] = chip.v_reg[x] % 10;
                    break;

                case 0x55:
                    for (int i = 0; i <= x; i++)
                    {   
                        chip.memory[chip.index_reg + i] = chip.v_reg[i];
                    }
                    break;

                case 0x65:
                    for (int i = 0; i <= x; i++)
                    {   
                        chip.v_reg[i] = chip.memory[chip.index_reg + i];
                    }
                    break;
            }

            break;
    }

    printf("loop occured\n");
}

bool read_rom(char* filename)
{

    FILE* file = fopen(filename, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "ERROR file %s\n couldnt be openes", filename);
        return false;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    long max_size = sizeof(chip.memory) - 0x200;
    if (file_size > max_size)
    {
        fprintf(stderr, "ERROR file is too large! (%ld bytes, max. %ld bytes)\n", file_size, max_size);
        fclose(file);
        return false;
    }
    
    size_t bytes_read = fread(&chip.memory[0x200], sizeof(uint8_t), file_size, file);
    if (bytes_read != (size_t)file_size)
    {
        fprintf(stderr, "ERROR readig the ROM\n");
        fclose(file);
        return false;
    }

    fclose(file);
    return true;
}