#ifndef PPU_H_INCLUDED
#define PPU_H_INCLUDED

#include "mmc.h"
#include "video.h"


#define PPU_DISPLAY_P_WIDTH     256
#define PPU_DISPLAY_P_HEIGHT    240

#define PPU_DISPLAY_N_WIDTH     256
#define PPU_DISPLAY_N_HEIGHT    224

#define PPU_VMIRROR_VERTICAL    0x01
#define PPU_VMIRROR_HORIZONTAL  0x00
#define PPU_VMIRROR_SINGLE      0x03
#define PPU_VMIRROR_4_LAYOUT    0x08

struct BackGround {

    byte name      [0x03C0];
    byte attribute [0x0040];

    inline void write(word offset, byte data) {
        word off = (offset % 0x0400);
        if (off<0x03C0) {
            name[off] = data;
        } else {
            attribute[off-0x03C0] = data;
        }
    }

    inline byte read(word offset) {
        word off = (offset % 0x0400);
        if (off<0x03C0) {
            return name[off];
        } else {
            return attribute[off-0x03C0];
        }
    }
};

struct PPU {

private:
    BackGround  bg[4];

    BackGround *bg0;
    BackGround *bg1;
    BackGround *bg2;
    BackGround *bg3;

    byte bkPalette[16];     /* 背景调色板                          */
    byte spPalette[16];     /* 卡通调色板                          */

    byte spWorkRam[256];    /* 卡通工作内存                        */
    word spWorkOffset;      /* 卡通工作页面首地址                  */
    word ppu_ram_p;         /* ppu寄存器指针                       */
    byte addr_add;          /* 地址增长累加值                      */
    enum { pH, pL } ppuSW;  /* 写入ppu寄存器的位置 $0000-$3FFF     */

    word winX;
    word winY;
    enum { wX, wY } w2005;  /* 写入哪一个参数                      */

    word spRomOffset;       /* 卡通字库首地址                      */
    word bgRomOffset;       /* 背景字库首地址                      */
    enum { t8x8, t8x16 } spriteType;

    byte *NMI;
    byte bkleftCol;         /* 背景显示左一列                      */
    byte spleftCol;         /* 卡通显示左一列                      */
    byte bkAllDisp;         /* 背景全显示                          */
    byte spAllDisp;         /* 卡通全显示                          */

    byte hasColor;          /* 有无色彩                            */
    byte red;               /* 红色着色                            */
    byte green;             /* 绿色着色                            */
    byte blue;              /* 蓝色着色                            */

    byte spOverflow;        /* 卡通8个溢出                         */
    byte spClash;           /* 卡通冲突?                           */

    MMC   *mmc;
    Video *video;

    void control_2000(byte data);
    void control_2001(byte data);

    void write(byte);       /* 写数据                              */
    byte read();            /* 读数据                              */
    BackGround* swBg();     /* 依据ppu_ram_p的值得到相应的背景指针 */

public:
    PPU(MMC *mmc, Video *video);

    void reset();
    /* cpu通过写0x2000~0x2007(0x3FFF)控制PPU                       */
    void controlWrite(word addr, byte data);
    /* cpu通过读0x2000~0x2007(0x3FFF)得到PPU状态                   */
    byte readState(word addr);
    /* 切换屏幕布局                                                */
    void swithMirror(byte type);
    /* 设置cpu的NMI地址线                                          */
    void setNMI(byte* cpu_nmi);
    /* 绘制一个像素                                                */
    void drawNextPixel();
    /* 立即绘制背景字库                                            */
    void drawTileTable();
};

#endif // PPU_H_INCLUDED
