# SUCKLESS CHIP-8 EMULATOR

## KEY FEATURES

- All basic instructions named by http://devernay.free.fr/hacks/chip8/C8TECH10.HTM (the go-to CHIP-8 docs)
- Terminal output according to the docs using ncurses for graphics
- Mostly suckless
- Error handling with output to stderr
- Loading roms using terminal input

## How build

### Build tutorial

You can build using either the make file or not (it is up to you if you want to use it)

- USING THE MAKEFILE
    You will need the following packages:
    - clang
    - gnumake

    NOTE: 
        - If on nixos you can use my shell.nix also in the project files
        - Build was tested on fedora and nixos

    The Makefile is included. Move into the project folder:

        make main

    To clean up build artifacts:

        make clean

- NOT USING THE MAKEFILE
    You will need the following packages:
    - clang/gcc/..
    - ncurses

    build by using: 
        cc -O2 -o main main.c -lncurses (replace cc by your compiler of choice)

## How to use

The program needs roms to load to do anything. Currently there is the countr.ch8 file in the repo which needs to be downloaded and put into the project folder. 
ONLY .ch8 FILES ARE SUPPORTED
You can also download more ROMS on https://github.com/kripod/chip8-roms (i dont have any relations to the repo or creator, i just found their repo)

To run the program enter:

    ./main FILENAME.ch8

## NOTE
    Project source consists only of main.c  

## STATUS

Standard CHIP-8 is implemented and tested against tetris and a counter ROM. S-CHIP extensions and a few opcode-edge-case behaviours are not implemented

## LICENSE

MIT (see license file)