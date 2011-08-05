#ifndef ROM_H_INCLUDED
#define ROM_H_INCLUDED

#include "type.h"
#include "string"
#include "stdio.h"

using std::string;

struct nes_file {

#define NES_FILE_MAGIC      0x1A53454E //4E 45 53 1A
#define NES_FILE_HEAD_LEN   16
#define NES_FILE_HAS_TRA(x) (x->t1 & 0x04)
#define NES_FILE_TRA_SIZE   512

    union {
	byte magic[4];	// 字符串“NES^Z”用来识别.NES文件
	int  magic_i;
    };
	byte rom_size;	// 16kB ROM的数目
	byte vrom_size;	// 8kB VROM的数目
	byte t1;		/*  D0：1＝垂直镜像，0＝水平镜像
				　  　  D1：1＝有电池记忆，SRAM地址$6000-$7FFF
				　  　  D2：1＝在$7000-$71FF有一个512字节的trainer
				　  　  D3：1＝4屏幕VRAM布局
				　  　  D4－D7：ROM Mapper的低4位 */
	byte t2;		/*	D0－D3：保留，必须是0（准备作为副Mapper号^_^）
				　  　  D4－D7：ROM Mapper的高4位  */
	byte zero[8];	// 保留，必须是0

	byte trainer[NES_FILE_TRA_SIZE];  /* 7000~71ff共512个字节，
                     * Mapper补丁就放在该地方，ROM的代码来调用它们。
                     * 游戏Trainer代码由菜单程序加载 */

	byte *rom;		// 16KxM  ROM段升序排列，如果存在trainer，
					// 它的512字节摆在ROM段之前
	byte *vrom;		// 8KxN  VROM段, 升序排列

    void printRom(int offset, int len) {
        if (offset%16!=0) printf("\n0x%x: ", offset);

        for (int i=offset; i<len+offset; ++i) {
            if (i%16==0) printf("\n0x%x: ", i);
            printf(" %02x", rom[i]);
        }

        printf("\n");
    }
};


/**
 * 读取filename文件到rom结构中,成功返回0并设置rom参数
 * rom参数必须是有效的, 失败返回-1
 */
int load_rom(nes_file* rom, const string* filename);

#endif // ROM_H_INCLUDED
