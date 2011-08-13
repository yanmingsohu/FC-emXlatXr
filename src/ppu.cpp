#include "ppu.h"
#include "stdio.h"

#define DC(b, x)    (b), (b+x), (b+x+x), (b+x+x+x)
#define WHI(x)      (x + (x<<8) +(x<<16))
#define BLK(x)      (x), (x+32), (x+64)

/* 颜色映射, 索引为FC系统颜色, 值为RGB颜色 */
T_COLOR ppu_color_index[0x40] = {
     WHI(0x7F)
    ,DC(0x000080, 0x200000)
    ,DC(0x800000, 0x002000)
    ,DC(0x008000, 0x000020)
    ,BLK(0)

    ,WHI(0xA9)
    ,DC(0x0000FF, 0x200000)
    ,DC(0xFF0000, 0x002000)
    ,DC(0x00FF00, 0x000020)
    ,BLK(8)

    ,WHI(0xD3)
    ,DC(0x4040FF, 0x200000)
    ,DC(0xFF4040, 0x002000)
    ,DC(0x40FF40, 0x000020)
    ,BLK(16)

    ,WHI(0xFF)
    ,DC(0x8080FF, 0x200000)
    ,DC(0xFF8080, 0x002000)
    ,DC(0x80FF80, 0x000020)
    ,BLK(24)
};

#undef DC
#undef WHI
#undef BLK

PPU::PPU(MMC *_mmc, Video *_video)
    : addr_add(1), ppuSW(pH), w2005(wX), mmc(_mmc)
    , video(_video)
{
}

void PPU::controlWrite(word addr, byte data) {
    switch (addr % 0x08) {

    case 0: /* 0x2000                      */
        control_2000(data);
        break;
    case 1: /* 0x2001                      */
        control_2001(data);
        break;
    case 3: /* 0x2003 修改卡通工作内存指针 */
        spWorkOffset = data;
        break;
    case 4: /* 0x2004 写入卡通工作内存     */
        spWorkRam[spWorkOffset] = data;
        break;

    case 5: /* 0x2005 设置窗口坐标         */
        if (w2005==wX) {
            winX &= 0xFF00;
            winX |= data;
            w2005 = wY;
        } else {
            winY &= 0xFF00;
            winY |= data;
            w2005 = wX;
        }
        break;

    case 6: /* 0x2006 PPU地址指针           */
        if (ppuSW==pH) {
            ppu_ram_p &= 0x00FF;
            ppu_ram_p |=(data<<8);
            ppu_ram_p %= 0x4000;
            ppuSW = pL;
        } else {
            ppu_ram_p &= 0xFF00;
            ppu_ram_p |= data;
            ppuSW = pH;
        }
        break;

    case 7: /* 0x2007 写数据寄存器          */
        write(data);
        break;
    }
}

#define _BIT(x)  (data & (1<<x))

inline void PPU::control_2000(byte data) {
    if (_BIT(0)) {
        winX |= 0x0100;
    } else {
        winX &= 0x00FF;
    }

    if (_BIT(1)) {
        winY |= 0x0100;
    } else {
        winY &= 0x00FF;
    }

    addr_add    = _BIT(2) ? 0x20   : 0x01;
    spRomOffset = _BIT(3) ? 0x1000 : 0;
    bgRomOffset = _BIT(4) ? 0x1000 : 0;
    spriteType  = _BIT(5) ? t8x16  : t8x8;
    *NMI        = _BIT(7) ? 1      : 0;
    // D6位的作用??
}

inline void PPU::control_2001(byte data) {
    hasColor    = _BIT(0) ? 0 : 1;
    bkleftCol   = _BIT(1) ? 0 : 1;
    spleftCol   = _BIT(2) ? 0 : 1;
    bkAllDisp   = _BIT(3) ? 0 : 1;
    spAllDisp   = _BIT(4) ? 0 : 1;
    red         = _BIT(5) ? 1 : 0;
    green       = _BIT(6) ? 1 : 0;
    blue        = _BIT(7) ? 1 : 0;
}

#undef _BIT

byte PPU::readState(word addr) {
    byte r = 0xFF;

    switch (addr % 0x08) {

    case 2: /* 0x2002                       */
        r = 0;
        if ( spOverflow ) r |= ( 1<<5 );
        if ( spClash    ) r |= ( 1<<6 );
        if ( *NMI       ) r |= ( 1<<7 );
        break;

    case 7: /* 0x2007 写数据寄存器          */
        r = read();
        break;
    }

    return r;
}

BackGround* PPU::swBg() {
    word offs = (ppu_ram_p % 0x3000 - 0x2000) / 0x400;
    switch (offs) {
    case 0:
        return bg0;
    case 1:
        return bg1;
    case 2:
        return bg2;
    case 3:
        return bg3;
    }
    return NULL;
}

inline byte PPU::read() {
    byte res = 0xFF;

    if (ppu_ram_p<0x2000) {
        res = mmc->readVRom(ppu_ram_p);
    } else

    if (ppu_ram_p<0x3EFF) {
        BackGround* pBg = swBg();
        res = pBg->read(ppu_ram_p);
    } else

    if (ppu_ram_p<0x3FFF) {
        word off = ppu_ram_p % 0x20;
        if (off<0x10) {
            res = bkPalette[off     ];
        } else {
            res = spPalette[off-0x10];
        }
    }

    ppu_ram_p += addr_add;
    return res;
}

inline void PPU::write(byte data) {
    if (ppu_ram_p<0x2000) {
        printf("PPU: can't write vrom %x.", ppu_ram_p);
        return;
    } else

    if (ppu_ram_p<0x3EFF) {
        BackGround* pBg = swBg();
        pBg->write(ppu_ram_p, data);
    } else

    if (ppu_ram_p<0x3FFF) {
        word off = ppu_ram_p % 0x20;
        if (off<0x10) {
            bkPalette[off     ] = data;
        } else {
            spPalette[off-0x10] = data;
        }
    }

    ppu_ram_p += addr_add;
}

void PPU::swithMirror(byte type) {
    switch (type) {

    case PPU_VMIRROR_VERTICAL:
        bg0 = bg2 = &bg[0];
        bg1 = bg3 = &bg[1];
    break;

    case PPU_VMIRROR_HORIZONTAL:
        bg0 = bg1 = &bg[0];
        bg2 = bg3 = &bg[1];
    break;

    case PPU_VMIRROR_SINGLE:
        bg0 = bg1 = bg2 = bg3 = &bg[0];
    break;

    case PPU_VMIRROR_4_LAYOUT:
        bg0 = &bg[0];
        bg1 = &bg[1];
        bg2 = &bg[2];
        bg3 = &bg[3];
    break;
    }
}

void PPU::reset() {
    spOverflow = 0;
    spClash    = 0;
    *NMI       = 0;
}

void PPU::setNMI(byte* cpu_nmi) {
    NMI = cpu_nmi;
}

void PPU::drawNextPixel() {
    static word X = 0;
    static word Y = 0;
    static int c = 0xFF0000;

    //video->drawPixel(X, Y, c);

    if (++X>=PPU_DISPLAY_N_WIDTH) {
        X=0;
        if (++Y>=PPU_DISPLAY_N_HEIGHT) {
            Y=0;
            c = c>>1 | 0xA00000;
            *NMI = 1;
            printf("一帧完成\n");
        }
    }
}
