#include "mmc.h"
#include "type.h"
#include "ppu.h"
#include <stdio.h>
#include <memory.h>

/* ------------------------------------------------------------------ */
/* 前缀:
 * rom_mapper_  读取程序代码
 * vrom_mapper_ 读取显示代码
 * switch_page_ 切换页面代码
 * reset_       复位代码
 * ------------------------------------------------------------------ */

#define MMC_PRG_SIZE        rom->rom_size   /* 程序页的数量(x16 K) */
#define MMC_PPU_SIZE        rom->vrom_size  /* 字库页的数量(x8  K) */

//-- 00 --/////////////////////////////////////////////////////////////

class Mapper_0 : public MapperImpl {
public:
    uint r_prom(word off) {
        if (MMC_PRG_SIZE==1) {
            if (off>=0xC000) {
                return (off - 0xC000);
            }
        }
        return off - 0x8000;
    }
};

//-- 03 --/////////////////////////////////////////////////////////////

class Mapper_3 : public Mapper_0 {
};

//-- 04 --/////////////////////////////////////////////////////////////

class Mapper_4 : public MapperImpl {

private:
    static const uint vramSize    = 8 * 1024;
    static const uint prgBankSize = 8 * 1024;
    static const uint ppuBankSize = 1 * 1024;

    uint _prg_page_off;  /* 如果isC000fix==true,则映射
                         /* 到8000 ~ 9FFF否则C000 ~ DFFF */
    uint _prg_A000_off;  /* A000 ~ BFFF */
    uint _prg_fixp_off;  /* 倒数第二固定映射页面         */
    uint _prg_E000_off;  /* E000 ~ FFFF 固定最后一个页面 */
    bool isC000fix;      /* 如果0xC000地址为固定==true   */

    uint *modify;
    uint bankSize;           /* 每次切换页面的大小,1K|8K */

    bool chr_xor;            /* vram地址^0x1000          */
    bool isVRAM;             /* 卡带提供显存开关         */
    byte ex_vram[vramSize];  /* 卡带提供显存             */

    bool enableIrq;
    uint irqLatch;
    uint irqConter;

    /* 0:[0000 ~ 07FF]  1:[0800 ~ 0FFF]  2:[1000 ~ 13FF] *
     * 3:[1400 ~ 17FF]  4:[1800 ~ 1BFF]  5:[1C00 ~ 1FFF] */
    uint _vrom_off[6];
    /* 0:[0000 ~ 03FF]  1:[0400 ~ 07FF]  2:[0800 ~ 0BFF] *
     * 3:[0C00 ~ 0FFF]  4:[1000 ~ 17FF]  5:[1800 ~ 1FFF] */
    uint _vrom_xoff[6];


    uint prg_bank2page(uint prgBank) {
        return prgBank * prgBankSize;
    }

public:
    uint r_prom(word off) {
        if (off<0x8000) {
            if (!isC000fix) {
                return off + _prg_fixp_off - 0x8000;
            } else {
                return off + _prg_page_off - 0x8000;
            }
        }
        if (off<0xA000) {
            return off + _prg_A000_off - 0xA000;
        }
        if (off<0xC000) {
            if (isC000fix) {
                return off + _prg_fixp_off - 0xC000;
            } else {
                return off + _prg_page_off - 0xC000;
            }
        }
        /* ELSE <0xE000 */
        return off + _prg_E000_off - 0xE000;
    }

    uint r_vrom(word off) {
        if (isVRAM) {
            return ex_vram[off];
        }

        int idx;

        if (chr_xor) {
                 if (off<0x0400) idx = 0;
            else if (off<0x0800) idx = 1;
            else if (off<0x0C00) idx = 2;
            else if (off<0x1000) idx = 3;
            else if (off<0x1800) idx = 4;
            else                 idx = 5;

            return off + _vrom_xoff[idx];
        }
        else {
                 if (off<0x0800) idx = 0;
            else if (off<0x1000) idx = 1;
            else if (off<0x1400) idx = 2;
            else if (off<0x1800) idx = 3;
            else if (off<0x1C00) idx = 4;
            else                 idx = 5;

            return off + _vrom_off[idx];
        }
    }

    void w_vrom(word off, byte value) {
        ex_vram[off] = value;
    }

    void sw_page(word off, byte value) {
        if (off==0x8000) {
            chr_xor   = value & (1<<7);
            isC000fix = !(value & (1<<6));
            byte low  = value & 0x7;

            if (low<6) {
                bankSize = ppuBankSize;
                modify   = chr_xor
                         ? &_vrom_xoff[low]
                         : &_vrom_off[low];
            }
            else if (low==6) {
                bankSize = prgBankSize;
                modify   = &_prg_page_off;
            }
            else if (low==7) {
                bankSize = prgBankSize;
                modify   = &_prg_A000_off;
            }
        }

        else if (off==0x8001) {
            *modify = value * bankSize;
        }

        else if (off==0xA000) {
            ppu->switchMirror(value & 1);
        }

        else if (off==0xC000) {
            irqLatch = value;
        }
        else if (off==0xC001) {
            irqConter = 0;
        }
        else if (off==0xE000) {
            enableIrq = false;
        }
        else if (off==0xE001) {
            enableIrq = true;
        }
    }

    void draw_line() {
        if (enableIrq) {
            if (!irqConter) {
                irqConter = irqLatch;
            }
            else if (--irqConter==0) {
                *IRQ = 1;
            }
        }
    }

    void reset() {
        uint psize = MMC_PRG_SIZE * 2 - 1;

        _prg_page_off = prg_bank2page(0);
        _prg_A000_off = prg_bank2page(1);
        _prg_fixp_off = prg_bank2page(psize - 1);
        _prg_E000_off = prg_bank2page(psize);

        isC000fix = true;
        chr_xor   = false;
        isVRAM    = MMC_PPU_SIZE==0;

        memset(ex_vram, 0, sizeof(ex_vram));
    }

    uint capability() {
        return MMC_CAPABILITY_WRITE_VROM
             | MMC_CAPABILITY_CHECK_LINE
             | MMC_CAPABILITY_CHECK_SWITCH;
    }
};

//-- 19 --/////////////////////////////////////////////////////////////

/* is bad */
class Mapper_19 : public MapperImpl {
public:
    dword r_prom(word off) {
        if (off<0xC000) {
            return off;
        } else {
            return off + ( MMC_PRG_SIZE-1 ) * 16 * 1024;
        }
    }
};

/* ------------------------------------------------------------------ */

#define MMC_MAP(x)  case x : return new Mapper_##x

static MapperImpl* createMapper(int mapper_id) {
    switch (mapper_id) {
        MMC_MAP(  0);
        MMC_MAP(  3);
        MMC_MAP(  4);
    //  MMC_MAP( 19);
    }
    return NULL;
}

#undef MMC_MAP, MMC_PRG_SIZE, MMC_PPU_SIZE

/* ------------------------------------------------------------------ */

bool MMC::loadNes(nes_file* _rom) {
    if (sw) {
        delete sw;
        sw = NULL;
    }

    if (_rom) {
        word id = _rom->mapperId();
        sw = createMapper(id);

        if (sw) {
            sw->rom = this->rom = _rom;
            sw->ppu = this->ppu;
            sw->IRQ = this->IRQ;
            capability = sw->capability();
            resetMapper();
            return true;
        }
    }

    /* 失败的情况 */
    this->rom = NULL;
    return false;
}
