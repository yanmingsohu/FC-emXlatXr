#include "mem.h"
#include "string.h"
#include <stdio.h>
#include <memory.h>

memory::memory(MMC *mmc, PPU *_ppu, PlayPad *_pad)
      : mmc(mmc), ppu(_ppu), pad(_pad)
{
}

void memory::hard_reset() {
    memset(&ram, 0, sizeof(ram));
    mmc->resetMapper();
}

void memory::soft_reset() {
    mmc->resetMapper();
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
        return pad->readPort(offset);
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
                            /* 32K ³ÌÐò´úÂë ROM                */
    return mmc->readRom(offset);
}

void memory::write(const word offset, const byte data) {
    if (offset<0x2000) {
        ram[offset%0x0800] = data;
        return;
    }
    if (offset<0x4000) {    /* PPU ¼Ä´æÆ÷                      */
        ppu->controlWrite(offset, data);
        return;
    }
    if (offset==0x4014) {   /* OAM DMA ¼Ä´æÆ÷                  */
#ifdef SHOW_PPU_REGISTER
        printf("MEM::ÏòPPU´«ËÍOAM,ram:%04X\n", data<<8);
#endif
        ppu->copySprite(ram + (data<<8));
        return;
    }
    if (offset==0x4016 || offset==0x4017) {
        pad->writePort(offset, data);
        return;
    }
    if (offset>=0x8000) {
        mmc->checkSwitch(offset, data);
    }
}
