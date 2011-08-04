#include <stdio.h>
#include "cpu.h"
#include "mem.h"


int main()
{
    reset_ram();
    reset_cpu();

    printf("%x,%x,%s\n", cpu.FLAGS, cpu.SP, "NES^Z");

    printf("cpu:%x\nram:%x\nvram:%x\n",
           sizeof(cpu_6502), sizeof(memory), sizeof(vram));

    return 0;
}
