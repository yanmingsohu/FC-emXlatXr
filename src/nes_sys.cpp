#include <stdio.h>
#include "cpu.h"
#include "mem.h"
#include "rom.h"


void startNes() {
    string filename = "rom/els.nes";
    nes_file rom;

    if (load_rom(&rom, &filename)) {
        printf("cannot load rom, Exit.\n\n");
        return;
    }

    printf("load '%s' over, start..\n", filename.c_str());

    rom.printRom(0xfff0-0x8000, 0x10);

    memory ram(&rom);
    ram.reset();

    cpu_6502 cpu(&ram);
    cpu.reset();

    cpu.debug();

    int i = 200;
    while(cpu.process() && i--);

    cpu.debug();
}
