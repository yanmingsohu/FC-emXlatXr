#include <stdio.h>
#include "cpu.h"
#include "mem.h"
#include "rom.h"


void startNes() {
    string filename = "rom/tstd2.nes";
    nes_file rom;


    if (load_rom(&rom, &filename)) {
        printf("cannot load rom, Exit.\n\n");
        return;
    }
    printf("load '%s' over, start..\n", filename.c_str());

    memory ram(&rom);
    ram.reset();
    cpu_6502 cpu(&ram);
    cpu.reset();

    cpu.debug();
}
