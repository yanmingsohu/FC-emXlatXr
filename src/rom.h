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
#ifndef ROM_H_INCLUDED
#define ROM_H_INCLUDED

#include "type.h"
#include "string"

using std::string;

struct nes_file {

#define NES_FILE_MAGIC      0x1A53454E // 4E 45 53 1A
#define NES_FILE_HEAD_LEN   16
#define NES_FILE_HAS_TRA(x) (x->t1 & 0x04)
#define NES_FILE_TRA_SIZE   512

    union {
	byte magic[4];	/* 字符串“NES^Z”用来识别.NES文件                    */
	int  magic_i;
    };

	byte rom_size;	/* 16kB ROM的数目                                   */
	byte vrom_size;	/* 8kB VROM的数目                                   */
	byte t1;		/* D0：1＝垂直镜像，0＝水平镜像
				　   * D1：1＝有电池记忆，SRAM地址$6000-$7FFF
				　   * D2：1＝在$7000-$71FF有一个512字节的trainer
                     * D3：1＝4屏幕VRAM布局
				　   * D4－D7：ROM Mapper的低4位                        */
	byte t2;		/* D0－D3：保留，必须是0（准备作为副Mapper号^_^）   *
				　   * D4－D7：ROM Mapper的高4位                        */
	byte zero[8];	/* 保留，必须是0                                    */

	byte trainer[NES_FILE_TRA_SIZE];  /* 7000~71ff共512个字节， Mapper补*
                                       * 丁就放在该地方，ROM的代码来调用*
                                       * 它们。 游戏Trainer代码由菜单程 *
                                       * 序加载                         */

	byte *rom;		/* 16KxM  ROM段升序排列，如果存在trainer，
					 * 它的512字节摆在ROM段之前                         */
	byte *vrom;		/* 8KxN  VROM段, 升序排列                           */

    /* 打印ROM的内容,offset是起始地址,len为打印长度                     */
    void printRom(int offset, int len);
    /* 打印ROM的信息                                                    */
    void romInfo();
    /* 返回Mapper的类型值                                               */
    word mapperId();

    ~nes_file();
};

#define LOAD_ROM_SUCCESS         0
#define ER_LOAD_ROM_PARM        -1
#define ER_LOAD_ROM_OPEN        -2
#define ER_LOAD_ROM_HEAD        -3
#define ER_LOAD_ROM_TRAINER     -4
#define ER_LOAD_ROM_SIZE        -5
#define ER_LOAD_ROM_VSIZE       -6
#define ER_LOAD_ROM_BADMAP      -7
#define ER_LOAD_ROM_UNKNOW      -99

/**
 * 读取filename文件到rom结构中,成功返回0并设置rom参数
 * rom参数必须是有效的, 失败返回上面定义的错误码
 */
int load_rom(nes_file* rom, const string* filename);

#endif // ROM_H_INCLUDED
