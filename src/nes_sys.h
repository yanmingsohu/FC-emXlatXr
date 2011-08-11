#ifndef NES_SYS_H_INCLUDED
#define NES_SYS_H_INCLUDED

#include "cpu.h"
#include "mem.h"
#include "rom.h"


struct NesSystem {

private:
    nes_file*   rom;
    cpu_6502*   cpu;
    memory*     ram;
    MMC*        mmc;

public:
    NesSystem();
    ~NesSystem();
};


#endif // NES_SYS_H_INCLUDED
