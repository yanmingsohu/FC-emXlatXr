#include "mem.h"
#include "string.h"


memory::memory(MMC *mmc, PPU *_ppu)
      : mmc(mmc), ppu(_ppu)
{
}

void memory::reset() {
    memset(&ram, 0, sizeof(ram));
}

byte memory::read(const word offset) {
    if (offset<0x2000) {
        return ram[offset%0x0800];
    }
    if (offset<0x4000) {    /* PPU ¼Ä´æÆ÷                      */
        return ppu->readState(offset);
    }
    if (offset<0x4014) {    /* pAPU ¼Ä´æÆ÷                     */
        return 0;
    }
    if (offset==0x4014) {   /* OAM DMA ¼Ä´æÆ÷                  */
        return 0;
    }
    if (offset==0x4015) {   /* pAPU ×´Ì¬¼Ä´æÆ÷                 */
        return 0;
    }
    if (offset<0x4018) {    /* ÊäÈëÉè±¸×´Ì¬¼Ä´æÆ÷(ÊÖ±úµÈ)      */
        return 0;
    }
    if (offset<0x401F) {    /* Î´ÓÃ                            */
        return 0;
    }
    if (offset<0x6000) {    /* À©Õ¹ ROM                        */
        return 0;
    }
    if (offset<0x8000) {    /* SRAM£¨µç³Ø´¢´æ RAM£©            */
        return 0;
    }
    return readPro(offset); /* 32K ³ÌÐò´úÂë ROM                */
}

void memory::write(const word offset, const byte data) {
    if (offset<0x2000) {
        ram[offset%0x0800] = data;
        return;
    }
    if (offset<0x2008) {    /* PPU ¼Ä´æÆ÷                      */
        ppu->controlWrite(offset, data);
        return;
    }
    mmc->checkSwitch(offset, data);
}

byte memory::readPro(const word offset) {
    return mmc->readRom(offset);
}
