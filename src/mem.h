#ifndef MEM_H_INCLUDED
#define MEM_H_INCLUDED

#include "type.h"
#include "rom.h"
#include "mmc.h"

/*
union memory {
    struct {
	byte start[0x0100];	// $0000-$00FF 系统零页
	byte stact[0x0100];	// $0100-$01FF 系统堆栈
	byte graph[0x0200];	// $0200-$03FF 卡通图形定义
	byte data [0x0400];	// $0400-$07FF CPU数据暂存
	byte null [0x1800];	// $0800-$1FFF 空区
	byte user [0x6000];	// $2000-$7FFF i/o区和用户工作区
                        // $6000-$7FFF 为电池存档(卡带中)
	byte rom  [0x8000];	// $8000-$FFFF 游戏程序当程序>32KB则在
                        // $8000-$BFFF 的16KB间进行存储切换(不能直接使用)
                        // $C000-$FFFF 可以直接访问
    };
    byte idx  [0x10000];
};
*/

struct memory {

private:
    MMC *mmc;
    byte ram[0x07FF];

public:
    memory(MMC *mmc);
    /** 重置内存状态全部清0 */
    void reset();
    /** 可以读取全部地址 */
    byte read(const word offset);
    /** 可以写入全部地址 */
    void write(const word offset, const byte data);
    /** 专门用来读取程序 offset=(0x8000,0xFFFF) */
    byte readPro(const word offset);
};

#endif // MEM_H_INCLUDED
