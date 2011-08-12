#include "mmc.h"
#include "type.h"


/* ------------------------------------------------------------------ */
/* ǰ׺:
 * rom_mapper_  ��ȡ�������
 * vrom_mapper_ ��ȡ��ʾ����
 * switch_page_ �л�ҳ�����
 */////////////////////////////////////////////////////////////////////

dword rom_mapper_3(MapperImpl* mi, word offset) {
    if (mi->rom->rom_size==1) {
        if (offset>=0xC000)
        return (offset-0x4000);
    }
    return offset;
}

/* ------------------------------------------------------------------ */

#define MA_PA(x, a,b,c) x.sw_page = a; x.r_prom = b; x.r_vrom = c
#define MA_P1(x, b)     MA_PA(x, NULL, b, NULL)
#define MA_PL(x)        MA_P1(map_list[x], rom_mapper_##x)
#define MA_PD(x, y)     MA_P1(map_list[x], rom_mapper_##y)

MapperImpl map_list[256] = {};

static void init_map_list() {
    MA_PD(0, 3);
    MA_PL(   3);
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