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

/* 所有时钟都是基于cpu*3或ppu/4 */
#define MID_CYC(x)       (x/4)
#define ONE_LINE_CYC     1364
#define START_CYC        MID_CYC(ONE_LINE_CYC*21)
#define END_CYC          MID_CYC(ONE_LINE_CYC)
#define HBLANK_CYC       MID_CYC(340)

void NesSystem::drawFrame() {
    static int _cyc = 0;
    int _t_cyc = 0;

    ppu->oneFrameOver();
    while (_cyc < START_CYC) {
        _cyc += cpu->process() * 3;
    }
    _cyc -= START_CYC;
    _t_cyc += START_CYC;

    ppu->startNewFrame();
    ppu->drawSprite(PPU::bpBehind);

    int x=0, y=0;

    while (y<240) {
        /* 绘制一行 */
        for (;;) {
            _t_cyc++;
            ppu->drawPixel(x++, y);
            if (x>=256) {
                break;
            }

            if (_cyc<=0) {
                _cyc += cpu->process() * 3;
            } else {
                _cyc--;
            }
        }

        x = 0; y++;

        /* 水平消隐周期 */
        while (_cyc < HBLANK_CYC) {
            _cyc += cpu->process() * 3;
        }
        _cyc -= HBLANK_CYC;
        _t_cyc += HBLANK_CYC;
    }

    ppu->drawSprite(PPU::bpFront);

    while (_cyc < END_CYC) {
        _cyc += cpu->process() * 3;
    }
    _cyc -= END_CYC;
    _t_cyc += END_CYC;

    //printf("used time %d, %f\n", _t_cyc*4, 21477270/59.94);
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
