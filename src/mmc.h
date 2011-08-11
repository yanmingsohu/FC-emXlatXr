#ifndef MMC_H_INCLUDED
#define MMC_H_INCLUDED

#include "rom.h"

struct MapperImpl;

/* 切换页面的接口, 可以切换ROM或VROM, off=(0x8000-0xFFFF)  *
 * off为地址, value为写入的数据                            */
typedef void (*SWITCH_PAGE)(MapperImpl*, word off, byte value);
/* 转换ROM地址的接口, off=(0x8000-0xFFFF), 返回转换后的地址*
 * 转换后的地址也必须在(0x8000-0xFFFF)                     */
typedef word (*ROM_READER )(MapperImpl*, word off);
/* 与ROM_READER类似                                        */
typedef word (*VROM_READER)(MapperImpl*, word off);


struct MapperImpl {
    /* 检查内存写入,切换页面,可以空   */
    SWITCH_PAGE     sw;
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
};

#endif // MMC_H_INCLUDED
