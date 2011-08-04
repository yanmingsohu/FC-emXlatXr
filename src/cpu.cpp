#include "mem.h"
#include "cpu.h"
#include "stdio.h"
#include "stdlib.h"

cpu_6502::cpu_6502(memory* ram)
        : ram(ram)
{
}

void cpu_6502::reset() {
    A      = 0;
    Y      = 0;
    X      = 0;

    PC     = 0;
    SP     = 0x01FF;
    FLAGS  = 0x20;

    PCH    = ram->read(0xFFFD);
    PCL    = ram->read(0xFFFC);
}

void cpu_6502::push(byte d) {
    ram->write(SP | 0x0100, d);
    SP--;
}

byte cpu_6502::pop() {
    SP++;
    return ram->read(SP);
}

void cpu_6502::process() {
    byte data = ram->readPro(PC);
    byte h = data & 0xF0;
    byte l = data & 0x0F;

    switch (l) {
    case 0x00:
    case 0x01:
    case 0x02:
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x08:
    case 0x09:
    case 0x0A:
    case 0x0C:
    case 0x0D:
    case 0x0E:
    break;
    };
}

void cpu_6502::debug() {
    char buf[9];
    printf("CPU >\t A: %04x\t X: %04x\t Y: %04x\n"
           "    >\tPC: %04x\tSP: %04x\tFG: %08s\n",
           A, X, Y, PC, SP, itoa(FLAGS, buf, 2));
}
