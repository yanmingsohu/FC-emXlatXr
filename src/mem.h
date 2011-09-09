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
#ifndef MEM_H_INCLUDED
#define MEM_H_INCLUDED

#include "type.h"
#include "rom.h"
#include "mmc.h"
#include "ppu.h"
#include "pad.h"

/*--------------------------------------------------*-
 * $0000-$00FF 系统零页                             *
 * $0100-$01FF 系统堆栈                             *
 * $0200-$03FF 卡通图形定义                         *
 * $0400-$07FF CPU数据暂存                          *
 * $0800-$1FFF 空区                                 *
 * $2000-$7FFF i/o区和用户工作区                    *
 * $6000-$7FFF 为电池存档(卡带中)                   *
 * $8000-$FFFF 游戏程序当程序>32KB则在              *
 * $8000-$BFFF 的16KB间进行存储切换(不能直接使用)   *
 * $C000-$FFFF 可以直接访问                         *
-*--------------------------------------------------*/
struct memory {

private:
    MMC     *mmc;
    PPU     *ppu;
    PlayPad *pad;
    byte    ram[0x07FF];

public:
    memory(MMC *mmc, PPU *_ppu, PlayPad* _pad);
    /** 重置内存状态全部清0                        */
    void hard_reset();
    /** 软重置,不会清除内存                        */
    void soft_reset();
    /** 可以读取全部地址                           */
    byte read(const word offset);
    /** 可以写入全部地址                           */
    void write(const word offset, const byte data);
    /** 专门用来读取程序 offset=(0x8000,0xFFFF)    */
    byte readPro(const word offset);
};

#endif // MEM_H_INCLUDED
