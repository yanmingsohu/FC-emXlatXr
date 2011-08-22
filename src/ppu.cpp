#include "ppu.h"
#include <stdio.h>
#include <string.h>

#define H(a) 0x##a
/* 颜色映射, 索引为FC系统颜色, 值为RGB颜色 */
T_COLOR ppu_color_table[0x40] = {
     H(808080), H(003da6), H(0012b0), H(440096)
    ,H(a1005e), H(c70028), H(ba0600), H(8c1700)
    ,H(5c2f00), H(104500), H(054a00), H(00472e)
    ,H(004166), H(000000), H(050505), H(050505)

    ,H(c7c7c7), H(0077ff), H(2155ff), H(8237fa)
    ,H(eb2fb5), H(ff2950), H(ff2200), H(d63200)
    ,H(c46200), H(358000), H(058f00), H(008a55)
    ,H(0099cc), H(212121), H(090909), H(090909)

    ,H(ffffff), H(0fd7ff), H(69a2ff), H(d480ff)
    ,H(ff45f3), H(ff618b), H(ff8833), H(ff9c12)
    ,H(fabc20), H(9fe30e), H(2bf035), H(0cf0a4)
    ,H(05fbff), H(5e5e5e), H(0d0d0d), H(0d0d0d)

    ,H(ffffff), H(a6fcff), H(b3ecff), H(daabeb)
    ,H(ffa8f9), H(ffabb3), H(ffd2b0), H(ffefa6)
    ,H(fff79c), H(d7e895), H(a6edaf), H(a2f2da)
    ,H(99fffc), H(dddddd), H(111111), H(111111)
};
#undef H

PPU::PPU(MMC *_mmc, Video *_video)
    : addr_add(1), ppuSW(pH), w2005(wX), mmc(_mmc)
    , video(_video)
{
}

void PPU::controlWrite(word addr, byte data) {
    switch (addr % 0x08) {

    case 0: /* 0x2000                      */
        control_2000(data);
#ifdef SHOW_PPU_REGISTER
        printf("PPU::write addr 2000:%X\n", data);
#endif
        break;
    case 1: /* 0x2001                      */
        control_2001(data);
#ifdef SHOW_PPU_REGISTER
        printf("PPU::write addr 2001:%X\n", data);
#endif
        break;
    case 3: /* 0x2003 修改卡通工作内存指针 */
        spWorkOffset = data;
#ifdef SHOW_PPU_REGISTER
        printf("PPU::修改精灵指针:%x\n", data);
#endif
        break;
    case 4: /* 0x2004 写入卡通工作内存     */
#ifdef SHOW_PPU_REGISTER
        printf("PPU::写入精灵数据: %x = %x\n", spWorkOffset, data);
#endif
        spWorkRam[spWorkOffset++] = data;
        break;

    case 5: /* 0x2005 设置窗口坐标         */
        if (w2005==wX) {
            winX = (0x100 & winX) | data;
            w2005 = wY;
#ifdef SHOW_PPU_REGISTER
        printf("PPU::窗口坐标...");
#endif
        } else {
            winY = (0x100 & winY) | data;
            w2005 = wX;
#ifdef SHOW_PPU_REGISTER
        printf("PPU::设置窗口坐标 x:%d, y:%d\n", winX, winY);
#endif
        }
        break;

    case 6: /* 0x2006 PPU地址指针           */
        if (ppuSW==pH) {
            ppu_ram_p = (ppu_ram_p & 0x00FF) | (data<<8);
            //ppu_ram_p %= 0x4000;
            ppuSW = pL;
        } else {
            ppu_ram_p = (ppu_ram_p & 0xFF00) | data;
            ppuSW = pH;
        }
#ifdef SHOW_PPU_REGISTER
        printf("PPU::修改PPU指针:%04x\n", ppu_ram_p);
#endif
        break;

    case 7: /* 0x2007 写数据寄存器          */
#ifdef SHOW_PPU_REGISTER
        printf("PPU::向PPU写数据:%04X = %04X\n", ppu_ram_p, data);
#endif
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
    sendNMI     = _BIT(7) ? 1      : 0;
    // D6位 PPU 主/从模式, 没有在NES里使用
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
        if (preheating) {
            preheating >>= 1;
            vblankTime = 1;
        }
        if ( spOverflow ) r |= ( 1<<5 );
        if ( spClash    ) r |= ( 1<<6 );
        if ( vblankTime ) r |= ( 1<<7 );
        vblankTime = 0;
        ppuSW      = pH;
        w2005      = wX;
        break;

    case 7: /* 0x2007 读数据寄存器          */
        r = read();
        break;
    }
#ifdef SHOW_PPU_REGISTER
    printf("PPU::read:%X, return:%X\n", addr, r);
#endif
    return r;
}

inline BackGround* PPU::swBg() {
    word offs = (ppu_ram_p % 0x3000 - 0x2000) / 0x400;
#ifdef SHOW_ERR_MEM_OPERATE
    if (offs>=4) {
        printf("PPU::无效的显存访问%X.\n", ppu_ram_p);
        return NULL;
    }
#endif
    return pbg[offs];
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
#ifdef SHOW_ERR_MEM_OPERATE
       printf("PPU::can't write VROM $%04x=%02x.\n", ppu_ram_p, data);
#endif
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

void PPU::switchMirror(byte type) {
    switch (type) {

    case PPU_VMIRROR_VERTICAL:
        pbg[0] = pbg[2] = &bg[0];
        pbg[1] = pbg[3] = &bg[1];
    break;

    case PPU_VMIRROR_HORIZONTAL:
        pbg[0] = pbg[1] = &bg[0];
        pbg[2] = pbg[3] = &bg[1];
    break;

    case PPU_VMIRROR_SINGLE:
        pbg[0] = pbg[1] = pbg[2] = pbg[3] = &bg[0];
    break;

    case PPU_VMIRROR_4_LAYOUT:
        pbg[0] = &bg[0];
        pbg[1] = &bg[1];
        pbg[2] = &bg[2];
        pbg[3] = &bg[3];
    break;
    }
}

void PPU::reset() {
    spOverflow = 0;
    spClash    = 0;
    *NMI       = 0;
    preheating = 3;
}

void PPU::setNMI(byte* cpu_nmi) {
    NMI = cpu_nmi;
}

void PPU::drawTileTable() {
    byte dataH, dataL;
    byte colorIdx;
    int x=0, y=0;

    for (int p=0; p<512; ++p) {
        for (int i=0; i<8; ++i) {
            dataL = mmc->readVRom(p * 16 + i);
            dataH = mmc->readVRom(p * 16 + i + 8);

            for (int d=7; d>=0; --d) {
                colorIdx = 0;
                if (dataL>>d & 1) {
                    colorIdx |= 1;
                }
                if (dataH>>d & 1) {
                    colorIdx |= 2;
                }
                video->drawPixel(x++, y, ppu_color_table[colorIdx]);
            }
            x-= 8;
            y++;
        }
        x+= 8;
        y-= 8;
        if (p && p%32==0) {
            x = 0;
            y+= 8;
        }
    }
}

inline byte PPU::gtLBit(int x, int y, byte tileIdx, word vromOffset) {
    byte paletteIdx = 0;
    word tileAddr = vromOffset + (tileIdx << 4) + y % 8;

    byte tile0 = mmc->readVRom(tileAddr    );
    byte tile1 = mmc->readVRom(tileAddr + 8);
    byte tileX = 1<<(7 - x % 8);

    if (tile0 & tileX) paletteIdx |= 0x01;
    if (tile1 & tileX) paletteIdx |= 0x02;

    return paletteIdx;
}

inline byte PPU::bgHBit(int x, int y, byte *attrTable) {
    byte attrIdx = DIV32(x)    + (DIV32(y)    << 3);
    byte attrBit = DIV16(x%32) | (DIV16(y%32) << 1);
    byte attr    = attrTable[attrIdx] >> (attrBit<<1);
    return (attr & 0x3) << 2;
}

void PPU::drawBackGround(Video *panel) {
    int x  = 0, y  = 0;
    int bx = 0, by = 0;
    int bgIdx = 0;

    for (;;) {
        word nameIdx = (x/8) + (y/8*32);
        word tileIdx = pbg[bgIdx]->name[nameIdx];
        byte paletteIdx = gtLBit(x, y, tileIdx, bgRomOffset);

        if (paletteIdx) {
            paletteIdx |= bgHBit(x, y, pbg[bgIdx]->attribute);
            panel->drawPixel(bx + x, by + y, ppu_color_table[ bkPalette[paletteIdx] ]);
        }

        if (++x>=256) {
            x = 0;
            if (++y>=240) {
                y = 0;
                ++bgIdx;

                if (bgIdx==1) {
                    bx = 256;
                    by = 0;
                }
                else if (bgIdx==2) {
                    bx = 0;
                    by = 240;
                }
                else if (bgIdx==3) {
                    bx = 256;
                    by = 240;
                }
                else {
                    break;
                }
            }
        }
    }
}

void PPU::drawSprite() {
    byte paletteIdx;

    for (int i=63<<2; i>=0; i-=4) {
        byte by   = spWorkRam[i  ];
        byte tile = spWorkRam[i+1];
        byte ctrl = spWorkRam[i+2];
        byte bx   = spWorkRam[i+3];

        for (int y=by; y<by+8; ++y) {
            for (int x=bx; x<bx+8; ++x) {
                paletteIdx = gtLBit(x, y, tile, spRomOffset);

                if (paletteIdx) {
                    paletteIdx |= ((ctrl & 0x03) << 2);
                    video->drawPixel(x, y, ppu_color_table[ spPalette[paletteIdx] ]);
                }
            }
        }
    }
}

void PPU::drawPixel(int X, int Y) {
    int x = (winX + X) % 512;
    int y = (winY + Y) % 480;
    BackGround *bgs;

    if (x<256) {
        if (y<240) {
            bgs = pbg[0];
        } else {
            bgs = pbg[2];
            y = Y;
        }
    } else {
        if (y<240) {
            bgs = pbg[1];
            x = X;
        } else {
            bgs = pbg[3];
            x = X;
            y = Y;
        }
    }

    word nameIdx = DIV8(x) + (DIV8(y)<<5);
    word tileIdx = bgs->name[nameIdx];

    byte paletteIdx = gtLBit(x, y, tileIdx, bgRomOffset);

    if (paletteIdx) {
        paletteIdx |= bgHBit(x, y, bgs->attribute);
        byte colorIdx = bkPalette[paletteIdx];
        video->drawPixel(X, Y, ppu_color_table[colorIdx]);
    }
}

void PPU::oneFrameOver() {
    if (sendNMI) {
        *NMI = 1;
#ifdef NMI_DEBUG
        printf("PPU::发送中断到CPU\n");
#endif
    }
    vblankTime = 1;
}

void PPU::startNewFrame() {
    vblankTime = 0;
    ppu_ram_p = 0x2000;
}

void PPU::copySprite(byte *data) {
    memcpy(spWorkRam, data, 256);
}
