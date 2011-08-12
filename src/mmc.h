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
    /* 读取vrom程序段, 暂时未实现 addr=(0x0000-0x1FFF)    */
    byte readVRom(const word addr);
    /* 在向内存写数据时执行换页操作                        */
    void checkSwitch(const word addr, const byte value);

};

#endif // MMC_H_INCLUDED
