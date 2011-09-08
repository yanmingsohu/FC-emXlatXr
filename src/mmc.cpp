#include "mmc.h"
#include "type.h"
#include "stdio.h"

/* ------------------------------------------------------------------ */
/* 前缀:
 * rom_mapper_  读取程序代码
 * vrom_mapper_ 读取显示代码
 * switch_page_ 切换页面代码
 * reset_       复位代码
 */////////////////////////////////////////////////////////////////////

dword rom_mapper_3(MapperImpl* mi, word offset) {
    if (mi->rom->rom_size==1) {
        if (offset>=0xC000)
            return (offset-0x4000);
    }
    return offset;
}

//--///////////////////////////////////////////////////////////////////

dword switch_page_19(MapperImpl* mi, word offset) {
}

dword rom_mapper_19(MapperImpl* mi, word offset) {
    if (offset<0xC000) {
        return offset;
    } else {
        return offset + (mi->rom->rom_size-1) * 16 * 1024;
    }
}

dword vrom_mapper_19(MapperImpl* mi, word offset) {
    return offset;
}

void reset_19(MapperImpl* mi) {
    mi->pr_page = 0;
    mi->vr_page = 0;
}

/* ------------------------------------------------------------------ */

#define MA_PA(x, a,b,c, d) x.sw_page = a; x.r_prom = b; \
                           x.r_vrom = c; x.reset = d

#define MA_P1(x, b)     MA_PA(x, NULL, b, NULL, NULL)
#define MA_PL(x)        MA_P1(map_list[x], rom_mapper_##x)
#define MA_PD(x, y)     MA_P1(map_list[x], rom_mapper_##y)
#define MA_PX(x)        MA_PA(map_list[x], switch_page_##x, \
                              rom_mapper_##x, vrom_mapper_##x, reset_##x)

MapperImpl map_list[256] = {};

static void init_map_list() {
    MA_PD( 0,  3);
    MA_PL(     3);
    MA_PL(    19);
}

#undef MA_PD
#undef MA_PL
#undef MA_P1
#undef MA_PA

/* ------------------------------------------------------------------ */

MMC::MMC() : rom(0), sw(0)
{
    init_map_list();
}

bool MMC::loadNes(nes_file* rom) {
    if (rom) {
        word id = rom->mapperId();
        MapperImpl* mi = &map_list[id];

        if (mi->r_prom) {
            sw = mi;
            sw->rom = rom;
            sw->pr_page = 0;
            sw->vr_page = 0;
            this->rom = rom;
            return true;
        }
    }

    sw = NULL;
    this->rom = NULL;
    return false;
}

byte MMC::readRom(const word addr) {
    if (addr<0x8000) {
        printf("MMC::读取ROM错误:使用了无效的程序地址 0x%x\n", addr);
        return 0;
    }
    if (sw && rom) {
        word n_addr = sw->r_prom(sw, addr);
        return rom->rom[n_addr-0x8000];
    }
    return 0xFF;
}

byte MMC::readVRom(const word addr) {
    if (rom) {
        if (sw->r_vrom) {
            word n_addr = sw->r_vrom(sw, addr);
            return rom->vrom[n_addr];
        } else {
            return rom->vrom[addr];
        }
    }
    return 0xFF;
}

void MMC::checkSwitch(const word addr, const byte value) {
    if (sw->sw_page && rom) {
        sw->sw_page(sw, addr, value);
    }
}

int MMC::vromSize() {
    return rom->vrom_size * 8 * 1024;
}
