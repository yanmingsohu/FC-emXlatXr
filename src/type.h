#ifndef TYPE_H_INCLUDED
#define TYPE_H_INCLUDED

#define HELP_FNC static inline

typedef unsigned char       byte;
typedef unsigned short int  word;
typedef unsigned int        dword;
typedef dword               T_COLOR; /* 真彩色定义最高字节为0xFF透明0xTTRRGGBB */

const byte SUCCESS = 0;
const byte FAILED  = -1;

void welcome();

/* 无实际作用,仅用来说明vram的结构 */
union vram {
    /* 每屏 32列 x 30行 个图形单元, 可显示960个单元            */
    struct {
    /*----------------------------------------| vrom 映射 |----*/
    /* 每个图形单元8x8点阵,16字节,每个库保存256个图形单元      *
     * 每屏同显64个卡通单元(一个页0x100字节)                   *
     * 卡通定义在内存中, 4字节: 1.Y 2.字库序号 3.形状 4.X      */

	byte part [0x1000];	/* $0000-$0FFF 卡通图形库              */
	byte bgg  [0x1000];	/* $1000-$1FFF 背景字符图形            */

	/*----------------------------------------| vram 映射 |----*/
	byte bg1m [0x03C0];	// $2000-$23BF 背景第一页映射 960(字节)个图形单元
	byte bg1c [0x0040];	// $23C0-$23FF 背景第一页配色区 64(字节)个配色单元 0x3FF 1KB

	byte bg2m [0x03C0];	// $2400-$27BF 背景第二页映射
	byte bg2c [0x0040];	// $27C0-$27FF 背景第二页配色区 0x7FF 2KB

	byte bg3m [0x03C0];	// $2800-$2BBF 背景第三页映射
	byte bg3c [0x0040];	// $2BC0-$2BFF 背景第三页配色区 0xBFF 3KB

	byte bg4m [0x03C0];	// $2C00-$2FBF 背景第四页映射
	byte bg4c [0x0040];	// $2FC0-$2FFF 背景第四页配色区 0xFFF 4KB

	byte nul1 [0x0F00];	// $3000-$3EFF $2000 - $2EFF 的镜像
	byte cdat [0x0020];	// $3F00-$3F1F 背景卡通配色代码数据 各16字节
	byte nul2 [0x00E0];	// $3F20-$3FFF 为空  $3F00-$3F1F 的7次镜像
                        // $4000-$FFFF $0000 - $3FFF 的镜像。
	};
	byte idx  [0x4000];
};

#endif // TYPE_H_INCLUDED
