#ifndef TYPE_H_INCLUDED
#define TYPE_H_INCLUDED


typedef unsigned char byte;
typedef unsigned short int word;

const byte SUCCESS = 0;
const byte FAILED  = -1;


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
