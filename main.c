/*
    CHIP-8 emulator according to http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
    press 0 to exit the emulator cleanly, ctrl+c also works
*/

#define _DEFAULT_SOURCE

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <errno.h>
#include <ncurses.h>



#define ACCEPTABLE_DISTANCE 50
#define PS_PER_CYCLE 1666
#define TIMER_CUTOFF 10
#define PROGRAM_LOCATION_START 0x200
#define FONT_LOCATION_START 0x50


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
    long long keypad[16];
} 
my_chip;


void cycle(void);
void read_rom(char* filename);
void flush_screen(void);
long long get_current_time_ms(void);
void is_pressed(int ch);
bool key_pressed(int k);
void die(const char *fmt, ...);


static bool is_scr_init;
static my_chip chip;

static const char keymap[] = "1234qwerasdfyxcv";

const uint8_t fontset[80] = 
{
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

void cycle(void)
{

    /*
        Function length is due to the CHIP-8 documentation mentioned at the top of the program.
        Follows the docs from top to bottom
    */
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

                default:
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
                    chip.v_reg[x] = chip.v_reg[y] >> 1;
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
                    chip.v_reg[x] = chip.v_reg[y] << 1;
                    chip.v_reg[0xF] = msb;
                    break;
                }

                default:
                    break;

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
                    if (key_pressed(chip.v_reg[x]))
                    {
                        chip.pc += 2;
                    }
                    break;

                case 0xA1:
                    if (!key_pressed(chip.v_reg[x]))
                    {
                        chip.pc += 2;
                    }
                    break;

                default:
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
                    for (int i = 0; i < 16; i++) 
                    {
                        if (key_pressed(i)) 
                        {
                            chip.v_reg[x] = i;
                            return;
                        }
                    }
                    chip.pc -= 2;
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
                    chip.index_reg = (5 * chip.v_reg[x]) + FONT_LOCATION_START;
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
                    chip.index_reg += x + 1;
                    break;

                case 0x65:
                    for (int i = 0; i <= x; i++)
                    {   
                        chip.v_reg[i] = chip.memory[chip.index_reg + i];
                    }
                    chip.index_reg += x + 1;
                    break;

                default:
                    break;
            }

            break;

        default:
            die("unknown opcode %04x at %03x\n", opcode, chip.pc - 2);
    }
}

void read_rom(char* filename)
{

    FILE* file = fopen(filename, "rb");
    if (file == NULL)
    {
        die("Couldn't open file %s: %s\n", filename, strerror(errno));
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    long max_size = sizeof(chip.memory) - PROGRAM_LOCATION_START;
    if (file_size > max_size)
    {
        fclose(file);
        die("error file is too large! (%ld bytes, max. %ld bytes)\n", file_size, max_size);
    }
    
    size_t bytes_read = fread(&chip.memory[PROGRAM_LOCATION_START], sizeof(uint8_t), file_size, file);
    if (bytes_read != (size_t)file_size)
    {
        fclose(file);
        die("error reading the rom\n");
    }

    fclose(file);
}

void flush_screen(void)
{
    for (int col = 0; col < 64; col++)
    {
        for (int row = 0; row < 32; row++)
        {
            if (chip.gfx[col][row] == 1)
            {
                mvaddch(row, col, '#');
            }
            else
            {
                mvaddch(row, col, ' ');
            }
        }
    }
}

long long get_current_time_ms(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    long long return_val = ((long long)tv.tv_sec) * 1000 + (tv.tv_usec / 1000);

    return return_val;
}

void is_pressed(int ch)
{
    const char *p = strchr(keymap, ch);
    if (p == NULL)
    {
        return;
    }
    
    chip.keypad[p - keymap] = get_current_time_ms();
}

bool key_pressed(int k)
{
    long long now = get_current_time_ms();
    return (now - chip.keypad[k]) < ACCEPTABLE_DISTANCE;
}

void die(const char *fmt, ...)
{
    if(is_scr_init)
    {
        endwin();
    }

    va_list ap;
    
    va_start(ap, fmt);
    
    vfprintf(stderr, fmt, ap);

    va_end(ap);
    
    exit(1);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        die("Proper usage: %s FILENAME.ch8\n", argv[0]);
    }
    srand((unsigned int)time(NULL));
    initscr();
    is_scr_init = true;
    cbreak();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);

    int timer_timer = 0;

    chip.pc = PROGRAM_LOCATION_START;

    int fontset_size = sizeof(fontset);
    for (int i = 0; i < fontset_size; i++)
    {
        chip.memory[FONT_LOCATION_START + i] = fontset[i];
    }

    read_rom(argv[1]);

    while(true)
    {
        int ch = getch();
        if (ch == '0')
        {
            break;
        }
        is_pressed(ch);

        cycle();

        timer_timer++;

        if(timer_timer == TIMER_CUTOFF)
        {
            timer_timer = 0;
            if (chip.delay_timer != 0)
            {
                chip.delay_timer--;
            }

            if (chip.sound_timer != 0)
            {
                chip.sound_timer--;
            }
            
            
        }
        

        flush_screen();
        refresh();

        usleep(PS_PER_CYCLE);
    }

    
    fprintf(stderr, "CPU shutting down\n");

    endwin();

    return 0;
}