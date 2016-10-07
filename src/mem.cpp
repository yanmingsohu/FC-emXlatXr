#include "mem.h"
#include "string.h"
#include <stdio.h>
#include <memory.h>

memory::memory(MMC *mmc, PPU *_ppu, PlayPad *_pad, Apu *a)
      : mmc(mmc), ppu(_ppu), pad(_pad), apu(a)
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
    if (offset<0x4000) {    /* PPU 寄存器                      */
        return ppu->readState(offset);
    }
    if (offset<0x4014) {    /* pAPU 寄存器                     */
        return 0;
    }
    if (offset==0x4014) {   /* OAM DMA 寄存器                  */
        return 0;
    }
    if (offset==0x4015) {   /* pAPU 状态寄存器                 */
        return apu->read();
    }
    if (offset<0x4018) {    /* 输入设备状态寄存器(手柄等)      */
        return pad->readPort(offset);
    }
    if (offset<0x401F) {    /* 未用                            */
        return 0;
    }
    if (offset<0x6000) {    /* 扩展 ROM                        */
        return 0xFF;
    }
    if (offset<0x8000) {    /* SRAM（电池储存 RAM）6000-8000   */
        return batteryRam[offset - 0x6000];
    }
                            /* 32K 程序代码 ROM                */
    return mmc->readRom(offset);
}

void memory::write(const word offset, const byte data) {
    if (offset<0x2000) {
        ram[offset%0x0800] = data;
        return;
    }
    if (offset<0x4000) {    /* PPU 寄存器                      */
        ppu->controlWrite(offset, data);
        return;
    }
    if (offset==0x4014) {   /* OAM DMA 寄存器                  */
#ifdef SHOW_PPU_REGISTER
        printf("MEM::向PPU传送OAM,ram:%04X\n", data<<8);
#endif
        ppu->copySprite(ram + (data<<8));
        return;
    }
    if (offset<=0x4015) {   /* pApu 寄存器                     */
        apu->write(offset, data);
        //printf("[%4X=%2X]\t", offset, data);
        return;
    }
    if (offset==0x4016) {
        pad->writePort(offset, data);
        return;
    }
    if (offset==0x4017) {   /* pApu */
        apu->write(offset, data);
        return;
    }
    if (offset>=0x6000 && offset<0x8000) {
        batteryRam[offset - 0x6000] = data;
    }
    if (offset>=0x8000) {
        mmc->checkSwitch(offset, data);
    }
}
