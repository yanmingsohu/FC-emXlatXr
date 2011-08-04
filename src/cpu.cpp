#include "type.h"
#include "mem.h"

cpu_6502 cpu;


void reset_cpu() {
    cpu.A      = 0;
    cpu.Y      = 0;
    cpu.X      = 0;

    cpu.PC     = 0;
    cpu.SP     = 0x01FF;
    cpu.FLAGS  = 0x20;

    cpu.PCH    = ram.idx[0xFFFD];
    cpu.PCL    = ram.idx[0xFFFC];
}

void push(byte d) {
    ram.idx[cpu.SP | 0x0100] = d;
    cpu.SP--;
}

byte pop() {
    cpu.SP++;
    return ram.idx[cpu.SP];
}

