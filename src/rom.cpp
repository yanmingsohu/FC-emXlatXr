#include "rom.h"
#include <stdio.h>
#include <stdlib.h>


int load_rom(nes_file* rom, const string* filename) {
    FILE* file = NULL;
    int res = FAILED;

    do {
        if (!rom) {
            printf("%s: parm rom not null.\n", __FILE__);
            break;
        }
        if (!filename) {
            printf("%s: parm filename not null.\n", __FILE__);
            break;
        }

        const char* filen = filename->c_str();
        file = fopen(filen, "rb");

        if (!file) {
			printf("\ncannot open '%s' file.\n", filen);
			break;
		}

        int readlen = fread(rom, 1, NES_FILE_HEAD_LEN, file);
        if (readlen<NES_FILE_HEAD_LEN) {
            printf("The file '%s' not enough size. \n", filen);
            break;
        }

        if (rom->magic_i!=NES_FILE_MAGIC) {
            printf("The file '%s' not nes rom. \n", filen);
            break;
        }

        if (NES_FILE_HAS_TRA(rom)) {
            readlen = fread(rom->trainer, 1, NES_FILE_TRA_SIZE, file);
            if (readlen!=NES_FILE_TRA_SIZE) {
                printf("The 'trainer' not enough '%s'.\n", filen);
                break;
            }
        }

        int rsize = rom->rom_size * 16 * 1024;
        if (rsize>0) {
            rom->rom = (byte*)malloc(rsize);
            readlen = fread(rom->rom, 1, rsize, file);
            if (rsize!=readlen) {
                printf("cannot read enough rom size.\n");
                break;
            }
        } else {
            printf("the rom is bad zero size");
            break;
        }

        rsize = rom->vrom_size * 8 * 1024;
        if (rsize>0) {
            rom->vrom = (byte*)malloc(rsize);
            readlen = fread(rom->vrom, 1, rsize, file);
            if (rsize!=readlen) {
                printf("cannot read enough vrom size.\n");
                break;
            }
        } else {
            rom->vrom = NULL;
        }

        res = SUCCESS;

    } while(0);

    if (file) {
        fclose(file);
    }

    return res;
}

nes_file::~nes_file() {
    if (rom) {
        free(rom);
    }
    if (vrom) {
        free(vrom);
    }
}

void nes_file::printRom(int offset, int len) {
    if (offset%16!=0) printf("\n0x%x: ", offset);

    for (int i=offset; i<len+offset; ++i) {
        if (i%16==0) printf("\n0x%x: ", i);
        printf(" %02x", rom[i]);
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
