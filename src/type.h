#ifndef TYPE_H_INCLUDED
#define TYPE_H_INCLUDED


typedef unsigned char byte;
typedef unsigned short int word;

const byte SUCCESS = 0;
const byte FAILED  = -1;

struct nes_file {

#define NES_FILE_MAGIC 0x1A53454E //4E 45 53 1A

	byte magic[4];	// 字符串“NES^Z”用来识别.NES文件
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
	byte trainer[512];
	byte *rom;		// 16KxM  ROM段升序排列，如果存在trainer，
					// 它的512字节摆在ROM段之前
	byte *vrom;		// 8KxN  VROM段, 升序排列
};

struct cpu_6502 {
	/* NTSC制式机型运行频率为1.7897725 MHz
	 * PAL制式机型运行频率为1.773447 MHz */

    byte A; 	    // 累加器
    byte Y;	    	// 索引暂存器
    byte X;	    	// 索引暂存器

    union {
    struct {
        byte PCL;   // intel高低位交换
        byte PCH;
    };
    word PC;
    };  			// 程序计数器,指向下一个要执行的指令

    word SP;		/* 堆叠指示器 0x0100-0x01FF
                       递减的, 指向'空', 每次2字节 */

#define CPU_FLAGS_NEGATIVE    (1<<7)  // 负值
#define CPU_FLAGS_OVERFLOW    (1<<6)  // 有溢出
#define CPU_FLAGS_BREAK       (1<<4)  // 有中断
#define CPU_FLAGS_DECIMAL     (1<<3)  // 十进制
#define CPU_FLAGS_INTERDICT   (1<<2)  // 禁止中断
#define CPU_FLAGS_ZERO        (1<<1)  // 结果为零
#define CPU_FLAGS_CARRY       1       // 有进位

    byte FLAGS;	    /* 状态暂存器  N V 1 B D I Z C
					   N负值 	V溢出		B中断命令
					   D十进制	I插断禁能	Z零值
					   C进位 */
};

union memory {
    struct {
	byte start[0x0100];	// $0000-$00FF 系统零页
	byte stact[0x0100];	// $0100-$01FF 系统堆栈
	byte graph[0x0200];	// $0200-$03FF 卡通图形定义
	byte data [0x0400];	// $0400-$07FF CPU数据暂存
	byte null [0x1800];	// $0800-$1FFF 空区
	byte user [0x6000];	// $2000-$7FFF i/o区和用户工作区
	byte rom  [0x8000];	/* $8000-$FFFF 游戏程序当程序>32KB则在
							$8000-$BFFF的16KB间进行存储切换 */
    };
    byte idx  [0x10000];
};

union vram {
    struct {
	byte part [0x1000];	// $0000-$0FFF 卡通图形库
	byte bgg  [0x1000];	// $1000-$1FFF 背景字符图形
	byte bg1m [0x03C0];	// $2000-$23BF 背景第一页映射
	byte bg1c [0x0040];	// $23C0-$23FF 背景第一页配色区
	byte bg2m [0x03C0];	// $2400-$27BF 背景第二页映射
	byte bg2c [0x0040];	// $27C0-$27FF 背景第二页配色区
	byte bg3m [0x03C0];	// $2800-$2BBF 背景第三页映射
	byte bg3c [0x0040];	// $2BC0-$2BFF 背景第三页配色区
	byte bg4m [0x03C0];	// $2C00-$2FBF 背景第四页映射
	byte bg4c [0x0040];	// $2FC0-$2FFF 背景第四页配色区
	byte nul1 [0x0F00];	// $3000-$3EFF 空
	byte cdat [0x0020];	// $3F00-$3F1F 背景卡通配色代码数据
	byte nul2 [0x00E0];	// $3F20-$3FFF 为空
	};
	byte idx  [0x4000];
};

#endif // TYPE_H_INCLUDED
