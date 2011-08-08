#include <stdio.h>
#include "cpu.h"
#include "mem.h"
#include "rom.h"
#include "time.h"


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

    const int test_command = 300;
    clock_t s = clock();
    for (int i=test_command; cpu.process() && i>0; --i);
    clock_t e = clock();

    printf("执行 %d 条指令使用了 %lf 毫秒\n", test_command,
           (double)(e - s)*(CLOCKS_PER_SEC/1000));

    cpu.debug();
}
