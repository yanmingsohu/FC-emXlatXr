#include "ppu.h"

PPU::PPU(MMC *_mmc)
    : mmc(_mmc)
{
}

void PPU::controlWrite(word addr, byte data) {
    switch (addr % 0x08) {
        case 0:
    }
}

byte PPU::readState(word addr) {
    switch (addr % 0x08) {
    }
    return 0xFF;
}
