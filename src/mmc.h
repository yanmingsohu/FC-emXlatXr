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
#ifndef MMC_H_INCLUDED
#define MMC_H_INCLUDED

#include "rom.h"

struct MapperImpl;

/* 切换页面的接口, 可以切换ROM或VROM, off=(0x8000-0xFFFF)  *
 * off为地址, value为写入的数据                            */
typedef void  (*SWITCH_PAGE)(MapperImpl*, word off, byte value);
/* 转换ROM地址的接口, off=(0x8000-0xFFFF), 返回转换后的地址*
 * 转换后的地址可以在任意范围                              */
typedef dword (*ROM_READER )(MapperImpl*, word off);
/* 与ROM_READER类似 off=(0x0-0x1FFF)                       */
typedef dword (*VROM_READER)(MapperImpl*, word off);


struct MapperImpl {
    /* 检查内存写入,切换页面,可以空   */
    SWITCH_PAGE     sw_page;
    /* 读取程序ROM,不能为空           */
    ROM_READER      r_prom;
    /* 读取显存ROM,暂时为空(未实现)   */
    VROM_READER     r_vrom;

    const nes_file* rom;
    /* 当前程序页号                   */
    byte            pr_page;
    /* 当前显存页号                   */
    byte            vr_page;
};

/**
 * 通常只要一个MMC对象即可,通过读取不同的rom
 * 来改变读取rom的方式
 */
struct MMC {

private:
    nes_file* rom;
    MapperImpl *sw;

public:
    MMC();
    /* 切换rom, 该操作会改变Mapper, 如果不支持MMC返回false */
    bool loadNes(nes_file* rom);
    /* 读取程序段 addr=(0x8000-0xFFFF)                     */
    byte readRom(const word addr);
    /* 读取vrom程序段, 暂时未实现 addr=(0x0000-0x1FFF)     */
    byte readVRom(const word addr);
    /* 在向内存写数据时执行换页操作                        */
    void checkSwitch(const word addr, const byte value);
    /* 返回vrom的长度(字节)                                */
    int  vromSize();
};

#endif // MMC_H_INCLUDED
