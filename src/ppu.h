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
#ifndef PPU_H_INCLUDED
#define PPU_H_INCLUDED

#include "mmc.h"
#include "video.h"

#define PPU_DISPLAY_P_WIDTH      256
#define PPU_DISPLAY_P_HEIGHT     240

#define PPU_DISPLAY_N_WIDTH      256
#define PPU_DISPLAY_N_HEIGHT     240

#define PPU_VMIRROR_VERTICAL     0x01
#define PPU_VMIRROR_HORIZONTAL   0x00
#define PPU_VMIRROR_SINGLE       0x03
#define PPU_VMIRROR_4_LAYOUT     0x08

#define TO_CPU_CC(x)             (x/12.0)
/*---------------------------------------| PAL Info |-------------*/
#define P_VLINE_COUNT                312   /* 每帧扫描线          */
#define P_HLINE_CPU_CYC    TO_CPU_CC(1024) /* 每行绘制周期        */
#define P_HBLANK_CPU_CYC   TO_CPU_CC(338 ) /* 每行水平消隐周期    */
#define P_VBLANK_CPU_CYC   TO_CPU_CC(683 ) /* 垂直消隐周期        */
                                           /* 每帧周期            */
#define P_FRAME_CPU_CYC       \
            ( (P_HLINE_CPU_CYC + P_HBLANK_CPU_CYC) * P_VLINE_COUNT )
                                           /* 每像素周期          */
#define P_PIXEL_CPU_CYC       \
            ( P_HLINE_CPU_CYC / PPU_DISPLAY_P_WIDTH )
#define P_END_CYC             2
/*---------------------------------------| NTSC Info |------------*/
#define N_VLINE_COUNT                262   /* 每帧扫描线          */
#define N_HLINE_CPU_CYC    TO_CPU_CC(1024) /* 每行绘制周期        */
#define N_HBLANK_CPU_CYC   TO_CPU_CC(340 ) /* 每行水平消隐周期    */
#define N_VBLANK_CPU_CYC   TO_CPU_CC(0   ) /* 垂直消隐周期        */
                                           /* 每帧周期            */
#define N_FRAME_CPU_CYC       \
            ( (N_HLINE_CPU_CYC + N_HBLANK_CPU_CYC) * N_VLINE_COUNT )
                                           /* 每像素周期          */
#define N_PIXEL_CPU_CYC       \
            ( N_HLINE_CPU_CYC / PPU_DISPLAY_P_WIDTH )
#define N_END_CYC             4
/*----------------------------------------------------------------*/

struct BackGround {

    byte name      [0x03C0];
    byte attribute [0x0040];

    /* offset=[0 - 0x400] */
    void write(word offset, byte data) {
#ifdef SHOW_ERR_MEM_OPERATE
        _check(offset);
#endif
        if (offset<0x03C0) {
            name[offset] = data;
        } else {
            attribute[offset-0x03C0] = data;
        }
    }

    /* offset=[0 - 0x400] */
    byte read(word offset) {
#ifdef SHOW_ERR_MEM_OPERATE
        _check(offset);
#endif
        if (offset<0x03C0) {
            return name[offset];
        } else {
            return attribute[offset-0x03C0];
        }
    }

    void _check(word i) {
        if (i>=0x400) printf("BackGround::out of offset %x", i);
    }
};

/*----------------------------------------| vrom 映射 |----*-
 * 每屏 32列 x 30行 个图形单元, 可显示960个单元            *
 * 每个图形单元8x8点阵,16字节,每个库保存256个图形单元      *
 * 每屏同显64个卡通单元(一个页0x100字节)                   *
 * 卡通定义在内存中, 4字节: 1.Y 2.字库序号 3.形状 4.X      *
 *                                                         *
 * $0000-$0FFF 卡通图形库                                  *
 * $1000-$1FFF 背景字符图形                                *
-*----------------------------------------| vram 映射 |----*
 * $2000-$23BF 背景第一页映射 960(字节)个图形单元          *
 * $23C0-$23FF 背景第一页配色区 64(字节)个配色单元         *
 *                                            (一个映射1KB)*
 * $2400-$27BF 背景第二页映射                              *
 * $27C0-$27FF 背景第二页配色区 0x7FF 2KB                  *
 *                                                         *
 * $2800-$2BBF 背景第三页映射                              *
 * $2BC0-$2BFF 背景第三页配色区 0xBFF 3KB                  *
 *                                                         *
 * $2C00-$2FBF 背景第四页映射                              *
 * $2FC0-$2FFF 背景第四页配色区 0xFFF 4KB                  *
 *                                                         *
 * $3000-$3EFF $2000 - $2EFF 的镜像                        *
 *                                                         *
 * $3F00-$3F1F 背景卡通配色代码数据 各16字节               *
 * $3F20-$3FFF 为空  $3F00-$3F1F 的7次镜像                 *
 * $4000-$FFFF $0000 - $3FFF 的镜像。                      *
-*----------------------------------------| vram 映射 |----*/
struct PPU {

private:
    BackGround  bg[4];
    BackGround *pbg[4];

    byte bkPalette[16];     /* 背景调色板                          */
    byte spPalette[16];     /* 卡通调色板                          */

    byte spWorkRam[256];    /* 卡通工作内存                        */
    word spWorkOffset;      /* 卡通工作页面首地址                  */
    word ppu_ram_p;         /* ppu寄存器指针,不会超过 0x3FFF       */
    byte addr_add;          /* 地址增长累加值                      */
    enum { pH, pL } ppuSW;  /* 写入ppu寄存器的位置 $0000-$3FFF     */

    word winX;
    word winY;
    word tmpwinY, tmpwinX;
    enum { wX, wY } w2005;  /* 写入哪一个参数                      */

    word spRomOffset;       /* 卡通字库首地址                      */
    word bgRomOffset;       /* 背景字库首地址                      */
    enum { t8x8, t8x16 } spriteType;

    byte *NMI;              /* cpu的NMI地址线，用来向cpu发送NMI    */
    byte sendNMI;           /* 是否在刷新一帧后发送NMI             */
    byte vblankTime;        /* 如果处于垂直消隐时期则为1           */

    byte bkleftCol;         /* 背景显示左一列                      */
    byte spleftCol;         /* 卡通显示左一列                      */
    byte bkAllDisp;         /* 背景全显示                          */
    byte spAllDisp;         /* 卡通全显示                          */

    byte hasColor;          /* 有无色彩                            */
    T_COLOR bkColor;        /* 背景颜色                            */

    byte spOverflow;        /* 卡通8个溢出                         */
    byte hit;               /* 卡通碰撞                            */

    int  sp0x, sp0y;        /* 记录0号卡通的位置                   */
    byte sp0hit[8][8];      /* 用来做碰撞检测                      */
    byte readBuf;           /* PPU总是返回上一次读取的数据,在每次
                             * 修改指针时<0x3F00则没有预读取导致bug*/
    word tmp_addr;          /* 保存修改地址高位                    */
    int  currentDrawLine;   /* 当前正在渲染的行, 调试时使用        */

    MMC   *mmc;
    Video *video;

    void control_2000(byte data);
    void control_2001(byte data);

    void write(byte);       /* 写数据                              */
    byte read();            /* 读数据                              */
    BackGround* swBg(word); /* 依据word的值得到相应的背景指针      */

    /* 依据x,y的位置从attr属性表中取得颜色的高两位                 */
    byte bgHBit(int x, int y, byte *attr);
    /* 依据x,y的位置从attr属性表中取得颜色的低两位                 */
    byte gtLBit(int x, int y, byte tileIdx, word vromOffset);

    void _drawSprite(byte spriteIdx, byte);
    void _checkHit(int x, int y);
    /* 使用mask(1)清除tmp_addr,并用d非0位设置清除tmp_addr */
    void _setTmpaddr(uint mask, uint d);
    /* 使用tmp_addr设置当前ppuram指针与屏幕偏移 */
    void _resetScreenOffset(bool newFrame);

public:
    enum bgPriority {bpFront, bpBehind};

    PPU(MMC *mmc, Video *video);

    void reset();
    /* cpu通过写0x2000~0x2007(0x3FFF)控制PPU                       */
    void controlWrite(word addr, byte data);
    /* cpu通过读0x2000~0x2007(0x3FFF)得到PPU状态                   */
    byte readState(word addr);
    /* 切换屏幕布局                                                */
    void switchMirror(byte type);
    /* 设置cpu的NMI地址线                                          */
    void setNMI(byte* cpu_nmi);
    /* 当一帧绘制完成时调用以发送中断, 系统预热时也需要调用两次    */
    void oneFrameOver();
    /* 当开始绘制一幅新的帧时,该方法被调用                         */
    void startNewFrame();
    /* 向cpu发送中断信号                                           */
    void sendingNMI();
    /* 2270 cpu 周期后清除VBL                                      */
    void clearVBL();
    /* 复制256字节的数据到精灵Ram,需要512个CPU周期                 */
    void copySprite(byte *data);
    /* 取得窗口坐标                                                */
    void getWindowPos(int *x, int *y);
    /* 返回当前ppu显存指针                                         */
    word getVRamPoint();
    /* 开始绘制新的扫描线                                          */
    void startNewLine();

    /* 绘制一帧中的精灵                                            */
    void drawSprite(bgPriority);
    /* 在video上绘制四个背景 512*480                               */
    void drawBackGround(Video *v);
    /* 立即绘制背景字库                                            */
    void drawTileTable(Video *v);
    /* 绘制指定位置的像素                                          */
    void drawPixel(int x, int y);
};

#endif // PPU_H_INCLUDED
