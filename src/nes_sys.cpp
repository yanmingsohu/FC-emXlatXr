/*----------------------------------------------------------------------------*|
|*                                                                            *|
|* FC 模拟器 (Famicom是Nintendo公司在1983年7月15日于日本发售的8位游戏机)      *|
|*                                                                            *|
|* $ C++语言的第一个项目,就用它练手吧                                         *|
|* $ 猫饭写作, 如引用本程序代码需注明出处                                     *|
|* $ 作者对使用本程序造成的后果不负任何责任                                   *|
|* $ 亦不会对代码的工作原理做进一步解释,如有重大问题请拨打119 & 911           *|
|*                                                                            *|
|* > 使用 [Code::Block 10.05] 开发环境                                        *|
|* > 编译器使用 [MinGW 3.81] [gcc 4.4.1]                                      *|
|* > 参考了来自 [http://nesdev.parodius.com] 网站的资料                       *|
|* > 感谢 [Flubba] 设计的测试程序, 有了它开发效率成指数提升                   *|
|*                                                                            *|
|* ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ | CatfoOD |^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ *|
|*                                           | yanming-sohu@sohu.com          *|
|* [ TXT CHARSET WINDOWS-936 / GBK ]         | https://github.com/yanmingsohu *|
|*                                           | qQ:412475540                   *|
|*----------------------------------------------------------------------------*/
#include "nes_sys.h"
#include "debug.h"
#include <stdio.h>

NesSystem::NesSystem(Video* video, PlayPad *_pad) : pad(_pad) {
    mmc = new MMC();
    ppu = new PPU(mmc, video);
    ram = new memory(mmc, ppu, pad);
    cpu = new cpu_6502(ram);
    rom = new nes_file();
    ppu->setNMI(&cpu->NMI);
    state = 0;
}

NesSystem::~NesSystem() {
    delete cpu;
    delete ram;
    delete ppu;
    delete mmc;
    delete rom;
    delete pad;
}

void NesSystem::drawFrame() {
    static int cpu_cyc = 0;

    ppu->startNewFrame();
    ppu->drawSprite(PPU::bpBehind);

    int x = 0, y = 0;
    double ppu_cyc = 0, cyc = 0;

    while (y<PPU_DISPLAY_P_HEIGHT) {
        /* 绘制一行 */
        while (cpu_cyc<P_HLINE_CPU_CYC) {
            cyc = cpu->process();
            cpu_cyc += cyc;
            ppu_cyc += cyc;

            while (ppu_cyc>=P_PIXEL_CPU_CYC) {
                ppu->drawPixel(x++, y);
                ppu_cyc -= P_PIXEL_CPU_CYC;
            }
        }
        cpu_cyc -= P_HLINE_CPU_CYC;
        x = 0; y++;
#ifdef SHOW_PPU_DRAW_INFO
        printf("一行完成,水平消隐 %X,%X,,", x, y);
#endif
        /* 水平消隐周期 */
        while (cpu_cyc<P_HBLANK_CPU_CYC) {
            cpu_cyc += cpu->process();
        }
        cpu_cyc -= P_HBLANK_CPU_CYC;
#ifdef SHOW_PPU_DRAW_INFO
        printf("消隐结束\n");
#endif
    }
    ppu->drawSprite(PPU::bpFront);
    ppu->oneFrameOver();

#ifdef SHOW_PPU_DRAW_INFO
    printf("绘制一帧结束,垂直消隐\n");
#endif
    /* 垂直消隐周期 */
    while (cpu_cyc<P_VBLANK_CPU_CYC) {
        cpu_cyc += cpu->process();
    }
    cpu_cyc -= P_VBLANK_CPU_CYC;
#ifdef SHOW_PPU_DRAW_INFO
    printf("drawFrame 返回\n");
#endif
}

int NesSystem::load_rom(string filename) {
    int res = ::load_rom(rom, &filename);

    if (!res) {
        rom->romInfo();

        if (mmc->loadNes(rom)) {
            printf("INT vector(0xFFFA-0xFFFF): ");
            rom->printRom(0xFFFA - 0x8000, 6);
            ram->reset();
            ppu->switchMirror(rom->t1 & 0x0B);
            ppu->reset();
            cpu->RES = 1;
        } else {
            res = ER_LOAD_ROM_BADMAP;
        }
        state = res;
    }

    return res;
}

cpu_6502* NesSystem::getCpu() {
    if (state) {
        printf("> NesSystem::尚未初始化rom.\n");
    }
    return cpu;
}

PPU* NesSystem::getPPU() {
    return ppu;
}

memory* NesSystem::getMem() {
    return ram;
}

void NesSystem::debug() {
    static int debuging = 0;

    if (!debuging) {
        debuging = 1;
        printf("NES::start step debug.\n");
        debugCpu(this);
        debuging = 0;
    }
}
