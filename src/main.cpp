#include <stdio.h>
#include <stdlib.h>
#include "time.h"
#include <string>
#include "cpu.h"
#include "mem.h"
#include "rom.h"


const int test_command = 200;


byte test() {

    using std::string;

    string filename = "rom/Tennis.nes"; //"rom/Dr_Mario.nes";
    nes_file* rom = new nes_file();

    if (load_rom(rom, &filename)) {
        printf("cannot load rom, Exit.\n\n");
        return NULL;
    }

    printf("load '%s' over, start..\n", filename.c_str());
    rom->romInfo();
    //rom->printRom(0xFFFA - 0x8000, 6);
    printf("\n");

    MMC mmc;
    if (!mmc.loadNes(rom)) {
        printf("unsupport map id:%d\n", rom->mapperId());
        return NULL;
    }

    memory* ram = new memory(&mmc);
    ram->reset();

    cpu_6502* cpu = new cpu_6502(ram);

    int c=0;

    clock_t s = clock();

    for (;;) {
        if (cpu->process() && c++<test_command) {
            printf(cpu->cmdInfo());
        } else {
            break;
        }
    }

    clock_t e = clock();

    printf("执行 %d 条指令使用了 %lf 毫秒\n", c,
           (double)(e - s)*(CLOCKS_PER_SEC/1000));

    printf(cpu->debug());
}

int main()
{
    test();
    system("pause");
    return 0;
}

