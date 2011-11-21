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

#define MMC_PRG_SIZE    rom->rom_size   /* 程序页的数量(x16 K) */
#define MMC_PPU_SIZE    rom->vrom_size  /* 字库页的数量(x8  K) */

//-- 00 --/////////////////////////////////////////////////////////////

class Mapper_0 : public MapperImpl {

private:
    byte ram[8 * 1024];

public:
    byte r_prom(word off) {
        if (MMC_PRG_SIZE==1 && off>=0xC000) {
            off -= 0xC000;
        } else {
            off -= 0x8000;
        }
        return rom->rom[off];
    }

    byte r_vrom(word off) {
        if (MMC_PPU_SIZE) {
            return rom->vrom[off];
        } else {
            return ram[off];
        }
    }

    void w_vrom(word off, byte value) {
        ram[off] = value;
    }

    uint capability() {
        return MMC_CAPABILITY_WRITE_VROM;
    }

    void reset() {
        memset(ram, 0, sizeof(ram));
    }
};

//-- 02 --/////////////////////////////////////////////////////////////

class Mapper_2 : public Mapper_0 {
private:
    static const uint ONE_PAGE = 16 * 1024;
    uint p8000off;
    uint pC000off;

public:

    byte r_prom(word off) {
        if (off < 0xC000) {
            return rom->rom[p8000off + off - 0x8000];
        } else {
            return rom->rom[pC000off + off - 0xC000];
        }
    }

    void sw_page(word off, byte value) {
        p8000off = value * ONE_PAGE;
    }

    uint capability() {
        return MMC_CAPABILITY_CHECK_SWITCH | MMC_CAPABILITY_WRITE_VROM;
    }

    void reset() {
        Mapper_0::reset();

        uint max_prg_size = MMC_PRG_SIZE;
        pC000off = (max_prg_size-1) * ONE_PAGE;
        p8000off = 0;
    }
};

//-- 03 --/////////////////////////////////////////////////////////////

class Mapper_3 : public Mapper_0 {
private:
    uint vram_offset;

public:
    void sw_page(word off, byte value) {
        vram_offset = value * 0x2000;
    }

    byte r_vrom(word off) {
        return rom->vrom[off + vram_offset];
    }

    uint capability() {
        return MMC_CAPABILITY_WRITE_VROM | MMC_CAPABILITY_CHECK_SWITCH;
    }

    void reset() {
        Mapper_0::reset();
        vram_offset = 0;
    }
};

//-- 04 --/////////////////////////////////////////////////////////////

class Mapper_4 : public MapperImpl {

private:
    static const uint vramSize    = 8 * 1024;
    static const uint prgBankSize = 8 * 1024;
    static const uint ppuBankSize = 1 * 1024;

    uint max_prg_size;
    uint max_chr_size;

    uint _prg_8000_off;
    uint _prg_A000_off;
    uint _prg_C000_off;
    uint _prg_E000_off;  /* E000 ~ FFFF 固定最后一个页面 */

    uint *modify;
    uint bankSize;           /* 每次切换页面的大小,1K|8K */

    bool chr_xor;            /* vram地址^0x1000          */
    bool isVRAM;             /* 卡带提供显存开关         */
    byte ex_vram[vramSize];  /* 卡带提供显存             */

    bool enableIrq;
    byte irqLatch;
    byte irqConter;

    /* 0:[0000 ~ 07FF]  1:[0800 ~ 0FFF]  2:[1000 ~ 13FF] *
     * 3:[1400 ~ 17FF]  4:[1800 ~ 1BFF]  5:[1C00 ~ 1FFF] */
    uint _vrom_off[6];


    uint prg_bank2page(uint prgBank) {
        return prgBank * prgBankSize;
    }

public:
    byte r_prom(word off) {
        uint _off;
        if (off<0xA000) {
            _off = off + _prg_8000_off - 0x8000;
        }
        else if (off<0xC000) {
            _off = off + _prg_A000_off - 0xA000;
        }
        else if (off<0xE000) {
            _off = off + _prg_C000_off - 0xC000;
        }
        else { /* <0x10000 */
            _off = off + _prg_E000_off - 0xE000;
        }
        return rom->rom[_off];
    }

    byte r_vrom(word off) {
        if (isVRAM) {
            return ex_vram[off];
        }

        uint _off;

        if (chr_xor) {
            off ^= 0x1000;
        }
#define _DO(i,b)            _off = off + _vrom_off[i] - b
#define IF_DO(a,i,b)        if (off<a) _DO(i,b)
#define ELSE_IF_DO(a,i,b)   else IF_DO(a,i,b)
#define ELSE_DO(i,b)        else _DO(i,b)
             IF_DO(0x0400, 0,      0);
        ELSE_IF_DO(0x0800, 1, 0x0400);
        ELSE_IF_DO(0x0C00, 2, 0x0800);
        ELSE_IF_DO(0x1000, 3, 0x0C00);
        ELSE_IF_DO(0x1800, 4, 0x1000);
           ELSE_DO(        5, 0x1800);
#undef IF_DO
#undef _DO
#undef ELSE_DO
#undef ELSE_IF_DO
        return rom->vrom[_off];
    }

    void sw_page(word off, byte value) {
        PRINT("sw page: %4X %2X\n", off, value);
        /* 过滤off,使之支持镜像端口 */
        switch (off & 0xE001) {

        case 0x8000: {
            byte comm = value & 0x7;

            if (comm < 0x06) {
                chr_xor  = value & (1<<7);
                bankSize = ppuBankSize;
                modify   = _vrom_off + comm;
            }
            else { /* comm >= 6 */
                bankSize = prgBankSize;

                if (comm & 0x01) { /* comm==7 */
                    modify = &_prg_A000_off;
                } else {
                    if (value & (1<<6)) { /* value is 0x46 */
                        _prg_8000_off = prg_bank2page(max_prg_size - 2);
                        modify = &_prg_C000_off;
                    } else { /* value is 0x06 */
                        _prg_E000_off = prg_bank2page(max_prg_size - 2);
                        modify = &_prg_8000_off;
                    }
                }
            }
        }   break;

        case 0x8001: {
            if (bankSize==prgBankSize) {
                if (!max_prg_size) break;
                value = value % max_prg_size;
            } else {
                if (!max_chr_size) break;
                value = value % max_chr_size;
            }
            *modify = value * bankSize;
        }   break;

        case 0xA000:
            ppu->switchMirror(value & 1);
            break;

        case 0xA001:
        /*  D7 ：用来控制SRAM的使能， 0 禁止使用SRAM（Disable）；1 可以使用SRAM（Enable）
            D6 ：用来控制SRAM的写保护 0 SRAM可写；1 SRAM只读 */
            break;

        case 0xC000:
            irqLatch = value;
            break;

        case 0xC001:
            irqConter = 0;
            break;

        case 0xE000:
            enableIrq = false;
            break;

        case 0xE001:
            enableIrq = true;
            break;
        }
    }

    void draw_line() {
        if (!irqConter) {
            irqConter = irqLatch;
        } else {
            --irqConter;
            if ((!irqConter) && enableIrq) {
                *IRQ = 1;
            }
        }
    }

    void reset() {
        max_prg_size = MMC_PRG_SIZE * 2;
        max_chr_size = MMC_PPU_SIZE * 8;

        _prg_8000_off = prg_bank2page(0);
        _prg_A000_off = prg_bank2page(1);
        _prg_C000_off = prg_bank2page(max_prg_size - 2);
        _prg_E000_off = prg_bank2page(max_prg_size - 1);

        chr_xor   = false;
        isVRAM    = MMC_PPU_SIZE==0;
        memset(ex_vram,   0, sizeof(ex_vram)  );
        memset(_vrom_off, 0, sizeof(_vrom_off));
    }

    void w_vrom(word off, byte value) {
        //printf("write vrom %4X %2X\n", off, value);
        ex_vram[off] = value;
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
    byte r_prom(word off) {
        return 0;
    }
};

//-- 23 --/////////////////////////////////////////////////////////////

class Mapper_23 : public MapperImpl {

private:
    static const uint pSize = 8 * 1024;
    static const uint cSize = 1 * 1024;

    uint max_prg_size; /* 8K代码页的数量 */
    uint max_chr_size; /* 1K字库页的数量 */

    uint p8000off;
    uint pA000off;
    uint pC000off;

    /*    0,  400,  800,  C00
     * 1000, 1400, 1800, 1C00 */
    uint chr_off[8];

    uint off_latch;

public:
    void sw_page(word off, byte value) {
        switch (off) {

        case 0x8000:
            p8000off = (value % max_prg_size) * pSize;
            break;

        case 0x9000:
            if (value == 0) {
                ppu->switchMirror(PPU_VMIRROR_VERTICAL);
            }
            else if (value == 1) {
                ppu->switchMirror(PPU_VMIRROR_HORIZONTAL);
            }
            break;

        case 0xA000:
            pA000off = (value % max_prg_size) * pSize;
            break;

#define O_CASE(v, i) \
        case v:   off_latch  =  0x0F & value;     break; \
        case v+1: off_latch |= (0x0F & value)<<4;        \
                  chr_off[i] = (off_latch % max_chr_size) * cSize; \
                  break

        O_CASE(0xB000, 0);
        O_CASE(0xB002, 1);
        O_CASE(0xC000, 2);
        O_CASE(0xC002, 3);
        O_CASE(0xD000, 4);
        O_CASE(0xD002, 5);
        O_CASE(0xE000, 6);
        O_CASE(0xE002, 7);

#undef O_CASE
        }
    }

    byte r_prom(word off) {
        uint _off;
        if (off < 0xA000) {
            _off = off + p8000off - 0x8000;
        }
        else if (off < 0xC000) {
            _off = off + pA000off - 0xA000;
        }
        else { /* 0xC000 ~ 0xFFFF */
            _off = off + pC000off - 0xC000;
        }
        return rom->rom[_off];
    }

    byte r_vrom(word off) {
        uint _off;
#define IF_DO(o, i)      if (off < o) _off = off + chr_off[i] - o + 0x0400
#define ELS_IF_DO(o,i)   else IF_DO(o, i)

        IF_DO    (0x0400, 0);
        ELS_IF_DO(0x0800, 1);
        ELS_IF_DO(0x0C00, 2);
        ELS_IF_DO(0x1000, 3);
        ELS_IF_DO(0x1400, 4);
        ELS_IF_DO(0x1800, 5);
        ELS_IF_DO(0x1C00, 6);
        ELS_IF_DO(0x2000, 7);

#undef ELS_IF_DO
#undef IF_DO
        return rom->vrom[_off];
    }

    void reset() {
        max_chr_size = MMC_PPU_SIZE * 8;
        max_prg_size = MMC_PRG_SIZE * 2;

        p8000off = pSize * 0;
        pA000off = pSize * 1;
        pC000off = pSize * (max_prg_size-2);
    }

    uint capability() {
        return MMC_CAPABILITY_CHECK_SWITCH;
    }
};

/* ------------------------------------------------------------------ */

#define MMC_MAP(x)  case x : return new Mapper_##x

static MapperImpl* createMapper(int mapper_id) {
    switch (mapper_id) {
        MMC_MAP(  0);
        MMC_MAP(  2);
        MMC_MAP(  3);
        MMC_MAP(  4);
    //  MMC_MAP( 19);
        MMC_MAP( 23);
    }
    return NULL;
}

#undef MMC_MAP
#undef MMC_PRG_SIZE
#undef MMC_PPU_SIZE

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
