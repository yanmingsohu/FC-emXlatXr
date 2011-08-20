#ifndef NES_SYS_H_INCLUDED
#define NES_SYS_H_INCLUDED

#include "cpu.h"
#include "mem.h"
#include "rom.h"
#include "video.h"
#include "pad.h"

struct NesSystem {

private:
    nes_file*   rom;
    cpu_6502*   cpu;
    memory*     ram;
    MMC*        mmc;
    PPU*        ppu;
    PlayPad*    pad;
    int         state;

public:
    /* PlayPad 在该类销毁时销毁 */
    NesSystem(Video* video, PlayPad*);
    ~NesSystem();

    /* 读取rom文件,成功返回0,失败返回错误代码, *
     * 通过parseOpenError()得到错误原因        */
    int load_rom(string filename);

    cpu_6502    *getCpu();
    PPU         *getPPU();
    memory      *getMem();

    /* 绘制一帧 */
    void drawFrame();

    /* 开始cpu单步执行,同时需要打开编译开关 */
    void debug();
};


#endif // NES_SYS_H_INCLUDED
