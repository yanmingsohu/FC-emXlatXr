#ifndef NES_SYS_H_INCLUDED
#define NES_SYS_H_INCLUDED

#include "cpu.h"
#include "mem.h"
#include "rom.h"
#include "video.h"

struct NesSystem {

private:
    nes_file*   rom;
    cpu_6502*   cpu;
    memory*     ram;
    MMC*        mmc;
    PPU*        ppu;
    int         state;

public:
    NesSystem(Video* video);
    ~NesSystem();

    /* 读取rom文件,成功返回0,失败返回错误代码 */
    int load_rom(string filename);

    cpu_6502* getCpu();
    PPU *getPPU();
};


#endif // NES_SYS_H_INCLUDED
