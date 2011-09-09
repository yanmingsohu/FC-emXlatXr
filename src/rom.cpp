#include "rom.h"
#include <stdio.h>
#include <stdlib.h>


int load_rom(nes_file* rom, const string* filename) {
    FILE* file = NULL;
    int res = ER_LOAD_ROM_UNKNOW;

    do {
        if (!rom) {
            res = ER_LOAD_ROM_PARM;
            break;
        }
        if (!filename) {
            res = ER_LOAD_ROM_PARM;
            break;
        }

        const char* filen = filename->c_str();
        file = fopen(filen, "rb");

        if (!file) {
			res = ER_LOAD_ROM_OPEN;
			break;
		}

        int readlen = fread(rom, 1, NES_FILE_HEAD_LEN, file);
        if (readlen<NES_FILE_HEAD_LEN) {
            res = ER_LOAD_ROM_HEAD;
            break;
        }

        if (rom->magic_i!=NES_FILE_MAGIC) {
            res = ER_LOAD_ROM_HEAD;
            break;
        }

        if (NES_FILE_HAS_TRA(rom)) {
            readlen = fread(rom->trainer, 1, NES_FILE_TRA_SIZE, file);
            if (readlen!=NES_FILE_TRA_SIZE) {
                res = ER_LOAD_ROM_TRAINER;
                break;
            }
        }

        int rsize = rom->rom_size * 16 * 1024;
        if (rsize>0) {
            rom->rom = new byte[rsize];
            readlen = fread(rom->rom, 1, rsize, file);
            if (rsize!=readlen) {
                res = ER_LOAD_ROM_SIZE;
                break;
            }
        } else {
            res = ER_LOAD_ROM_SIZE;
            break;
        }

        rsize = rom->vrom_size * 8 * 1024;
        if (rsize>0) {
            rom->vrom = new byte[rsize];
            readlen = fread(rom->vrom, 1, rsize, file);
            if (rsize!=readlen) {
                res = ER_LOAD_ROM_VSIZE;
                break;
            }
        } else {
            rom->vrom = NULL;
        }

        res = LOAD_ROM_SUCCESS;

    } while(0);

    if (file) {
        fclose(file);
    }

    return res;
}

nes_file::~nes_file() {
    if (rom) {
        delete[] rom;
    }
    if (vrom) {
        delete[] vrom;
    }
}

void nes_file::printRom(int offset, int len) {
    if (offset > rom_size* 16 * 1024) {
        printf("> out of rom index REmapping to %X", offset);
        return;
    }

    if (offset%16!=0) printf("\n0x%X: ", offset);

    for (int i=offset; i<len+offset; ++i) {
        if (i%16==0) printf("\n0x%X: ", i);
        printf(" %02X", rom[i]);
    }

    printf("\n");
}

word nes_file::mapperId() {
    return (t2 & 0xF0)|(t1>>4);
}

void nes_file::romInfo() {
    printf("Rom >> PROM: 16kB x %d, VROM: 8kB x %d, MapperID: %d, Trainer: %d\n",
           rom_size, vrom_size, mapperId(), t1 & 0x04);
}
