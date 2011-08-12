#include "ppu.h"

PPU::PPU(MMC *_mmc)
    : mmc(_mmc)
{
}

void PPU::controlWrite(word addr, byte data) {
    switch (addr % 0x08) {
        case 0:
        break;
    }
}

byte PPU::readState(word addr) {
    switch (addr % 0x08) {
    }
    return 0xFF;
}

void PPU::swithMirror(byte type) {
    switch (type) {

    case PPU_VMIRROR_VERTICAL:
        bg0 = bg2 = &bg[0];
        bg1 = bg3 = &bg[1];
    break;

    case PPU_VMIRROR_HORIZONTAL:
        bg0 = bg1 = &bg[0];
        bg2 = bg3 = &bg[1];
    break;

    case PPU_VMIRROR_SINGLE:
        bg0 = bg1 = bg2 = bg3 = &bg[0];
    break;

    case PPU_VMIRROR_4_LAYOUT:
        bg0 = &bg[0];
        bg1 = &bg[1];
        bg2 = &bg[2];
        bg3 = &bg[3];
    break;
    }
}
