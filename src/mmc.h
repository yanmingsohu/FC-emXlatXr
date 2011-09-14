/*----------------------------------------------------------------------------*|
|*                                                                            *|
|* FC 模拟器 (Famicom是Nintendo公司在1983年7月15日于日本发售的8位游戏机)      *|
|*                                                                            *|
|* $ C++语言的第一个项目,就用它练手吧                                         *|
|* $ 猫饭写作, 如引用本程序代码需注明出处                                     *|
|* $ 作者对使用本程序造成的后果不负任何责任                                   *|
|* $ 亦不会对代码的工作原理做进一步解释,如有重大问题请拨打119                 *|
|*                                                                            *|
|* > 使用 [Code::Block 10.05] 开发环境                                        *|
|* > 编译器使用 [MinGW 3.81] [gcc 4.4.1]                                      *|
|* > 参考了来自 [http://nesdev.parodius.com] 网站的资料                       *|
|* > 感谢 [Flubba,blargg] 设计的测试程序, 有了它开发效率成指数提升            *|
|*                                                                            *|
|* ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ | CatfoOD |^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ *|
|*                                           | yanming-sohu@sohu.com          *|
|* [ TXT CHARSET WINDOWS-936 / GBK ]         | https://github.com/yanmingsohu *|
|*                                           | qQ:412475540                   *|
|*----------------------------------------------------------------------------*/
#ifndef MMC_H_INCLUDED
#define MMC_H_INCLUDED

#include "rom.h"
#include <stdio.h>

#define MMC_CAPABILITY_CHECK_SWITCH     (   1) /* 需要检查端口写入以切换页面 */
#define MMC_CAPABILITY_CHECK_LINE       (1<<1) /* 每行绘制结束需要通知MMC    */
#define MMC_CAPABILITY_WRITE_VROM       (1<<2) /* 把VROM当作VRAM使用         */

struct PPU;

class MapperImpl {

public:
    const nes_file* rom;
    PPU*            ppu;
    byte*           IRQ;

    /* 切换mapper时释放当前使用的资源 */
    virtual ~MapperImpl() { rom = NULL; }

    /* 切换页面的接口, 可以切换ROM或VROM, off=(0x8000-0xFFFF)
       off为地址, value为写入的数据, 默认打印警告 */
    virtual void sw_page(word off, byte value) {
#ifdef SHOW_ERR_MEM_OPERATE
        printf("MMC::向程序段写入数据: 0x%x = 0x%x \n", off, value);
#endif
    }

    /* 转换ROM地址的接口, off=(0x8000-0xFFFF), 返回转换后的地址的数据 */
    virtual byte r_prom(word off) = 0;

    /* 与ROM_READER类似 off=(0x0-0x1FFF) */
    virtual byte r_vrom(word off) {
        return rom->vrom[off];
    }

    /* 有的卡带使用扩展显存, 而且通常不提供vrom, 默认打印警告 */
    virtual void w_vrom(word off, byte value) {
#ifdef SHOW_ERR_MEM_OPERATE
        printf("MMC::can't write VROM $%04x=%02x.\n", off, value);
#endif
    }

    /* ppu绘制一行结束该方法被调用 */
    virtual void draw_line() {}

    /* 初始化,cpu复位时需要执行 */
    virtual void reset() {}

    /* 返回需要调用的方法，如果方法的号码没有返回则这个方法不会被调用 */
    virtual uint capability() { return MMC_CAPABILITY_WRITE_VROM; }
};

/**
 * 通常只要一个MMC对象即可,通过读取不同的rom
 * 来改变读取rom的方式
 */
struct MMC {

private:
    nes_file*   rom;
    MapperImpl* sw;
    PPU*        ppu;
    byte*       IRQ;
    uint        capability;

public:
    MMC() : rom(0), sw(0)
    {
    }

    /* 切换rom, 该操作会改变Mapper, 如果不支持MMC返回false
     * 同时前一个Mapper会被释放 */
    bool loadNes(nes_file* rom);

    /* 读取程序段 addr=(0x8000-0xFFFF) */
    byte readRom(const word addr) {
#ifdef SHOW_ERR_MEM_OPERATE
        if (addr<0x8000) {
            printf("MMC::读取ROM错误:使用了无效的程序地址 0x%x\n", addr);
        }
#endif
        return sw->r_prom(addr);
    }

    /* 读取vrom字库段 addr=(0x0000-0x1FFF) */
    byte readVRom(const word addr) {
        return sw->r_vrom(addr);
    }

    /* 有的卡带可写 */
    void writeVRom(const word addr, const byte value) {
        if (capability & MMC_CAPABILITY_WRITE_VROM) {
            sw->w_vrom(addr, value);
        }
    }

    /* 在向内存写数据时执行换页操作 */
    void checkSwitch(const word addr, const byte value) {
        if (capability & MMC_CAPABILITY_CHECK_SWITCH) {
            sw->sw_page(addr, value);
        }
    }

    /* 初始化mapper组件 */
    void resetMapper() {
        sw->reset();
    }

    /* 返回vrom的长度(字节) */
    int vromSize() {
        return rom->vrom_size * 8 * 1024;
    }

    /* 一行绘制结束该方法被调用 */
    void drawLineOver() {
        if (capability & MMC_CAPABILITY_CHECK_LINE) {
            sw->draw_line();
        }
    }

    void setPPU(PPU* p) {
        ppu = p;
    }

    void setIRQ(byte* i) {
        IRQ = i;
    }
};

#endif // MMC_H_INCLUDED
