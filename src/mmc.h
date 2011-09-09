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

class MapperImpl {

public:
    const nes_file* rom;

    /* 切换mapper时释放当前使用的资源 */
    virtual ~MapperImpl() { rom = NULL; }

    /* 切换页面的接口, 可以切换ROM或VROM, off=(0x8000-0xFFFF)
       off为地址, value为写入的数据 */
    virtual void sw_page(word off, byte value) {}

    /* 转换ROM地址的接口, off=(0x8000-0xFFFF), 返回转换后的地址
       转换后的地址映射到rom中(0~n) */
    virtual uint r_prom(word off) = 0;

    /* 与ROM_READER类似 off=(0x0-0x1FFF) */
    virtual uint r_vrom(word off) {
        return off;
    }

    /* 初始化,cpu复位时需要执行 */
    virtual void reset() {}
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
    /* 切换rom, 该操作会改变Mapper, 如果不支持MMC返回false
     * 同时前一个Mapper会被释放                            */
    bool loadNes(nes_file* rom);
    /* 读取程序段 addr=(0x8000-0xFFFF)                     */
    byte readRom(const word addr);
    /* 读取vrom程序段, 暂时未实现 addr=(0x0000-0x1FFF)     */
    byte readVRom(const word addr);
    /* 在向内存写数据时执行换页操作                        */
    void checkSwitch(const word addr, const byte value);
    /* 初始化mapper组件                                    */
    void resetMapper();
    /* 返回vrom的长度(字节)                                */
    int  vromSize();
};

#endif // MMC_H_INCLUDED
