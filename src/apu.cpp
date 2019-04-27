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
#include "apu.h"

#define SUD_FREQ     100
#define SUD_CHC      1
#define SUD_WID      8
#define SUD_SQU_BUF  8
#define SUD_TRI_BUF  32
#define SUD_NOS_BUF  320000
#define VOL_OFF      0.15

static const word noise_freq[] = {
    0x002,0x004,0x008,0x010,
    0x020,0x030,0x040,0x050,
    0x065,0x07F,0x0BE,0x0FE,
    0x17D,0x1FC,0x3F9,0x7F2
    };

static const float main_freq = 1.79 * 1000 * 1000;


void setr0(Register &r, byte v) {
    r.volume    = (0x0F & v);
    r.type      = (0x10 & v);
    r.looping   = (0x20 & v);
    r.duty_type = (0xC0 & v) >> 6;
}


void setr0tri(Register &r, byte v) {
    r.use_linear = (0x80 & v);
    r.linear     = (0x7F & v);
}


void setr1(Register &r, byte v) {
    r.sw_rs   = (0x07 & v);
    r.sw_dir  = (0x08 & v);
    r.sw_rete = (0x70 & v) >> 4;
    r.sw_en   = (0x80 & v);
}


void setr2n(Register &r, byte v) {
    r.nfreq = noise_freq[0x0F & v];
    r.rand_type = 0x80 & v;
    // printf("CHANGE rand type:0x%x freq:%d\n", 
    // r.rand_type, r.nfreq);
}


void setr2(Register &r, byte v) {
    r.wavelength_l = v;
}


void setr3(Register &r, byte v) {
    r.wavelength_h   = (0x07 & v);
    r.length_counter = (0xF8 & v) >> 3;
}


byte Apu::read() {
    byte ret = 0;
    if (*irq) {
        ret |= 0x40;
    }
    return ret;
}


void Apu::write(const word offset, const byte data) {
    // printf("APU [%x]:%x\n", offset, data);

    switch (offset) {
        
    case 0x4000:
        setr0(r1.r, data); r1.change(); break;
    case 0x4004:
        setr0(r2.r, data); r2.change(); break;
    case 0x4008:
        setr0tri(tr.r, data); tr.change(); break;
    case 0x400C:
        setr0(ns.r, data); ns.change(); break;
    
    case 0x4001:
        setr1(r1.r, data); r1.change(); break;
    case 0x4005:
        setr1(r2.r, data); r2.change(); break;
    
    case 0x4002:
        setr2(r1.r, data); r1.change(); break;
    case 0x4006:
        setr2(r2.r, data); r2.change(); break;
    case 0x400A:
        setr2(tr.r, data); tr.change(); break;
    case 0x400E:
        setr2n(ns.r, data); ns.change(); break;
    
    case 0x4003:
        setr3(r1.r, data); r1.change(); break;
    case 0x4007:
        setr3(r2.r, data); r2.change(); break;
    case 0x400B:
        setr3(tr.r, data); tr.change(); break;
    case 0x400F:
        setr3(ns.r, data); ns.change(); break;

    case 0x4015:
        if (data & 0x01 == 0) r1.stop();
        if (data & 0x02 == 0) r2.stop();
        if (data & 0x04 == 0) tr.stop();
        if (data & 0x08 == 0) ns.stop();
        // printf("r 15 0x%x\n", data);
        break;
        
    case 0x4017:
        if (data & 0x40) {
            *irq = 0;
        } else {
            *irq = 1;
        }
        break;
    }
}


void Apu::setIrq(byte* i) {
    irq = i;
}


Apu::Apu(HWND hwnd) 
        : irq(&tmp), dxs(hwnd), 
          r1(dxs), r2(dxs), tr(dxs), ns(dxs) {
}


void Apu::stop() {
    r1.stop();
    r2.stop();
    tr.stop();
    ns.stop();
}


static const byte Duty_cycle[][8] = {
    { 0,1,0,0, 0,0,0,0 },
    { 0,1,1,0, 0,0,0,0 },
    { 0,1,1,1, 1,0,0,0 },
    { 1,0,0,1, 1,1,1,1 },
};


static void initSquare(CreateSample cs) {
    int duty_type = (int) cs.xdata;
    const byte *dc = Duty_cycle[duty_type];
    for (int i=0; i<cs.dwLength; ++i) {
        cs[i] = dc[i%8] ? cs.maxval : 0;
    }
}


NesSquare::NesSquare(DXSound &dx) 
        : NesSound(dx, {SUD_FREQ,SUD_WID,SUD_CHC,SUD_SQU_BUF})
        , ch2(dx, {SUD_FREQ,SUD_WID,SUD_CHC,SUD_SQU_BUF})
        , ch3(dx, {SUD_FREQ,SUD_WID,SUD_CHC,SUD_SQU_BUF})
        , ch4(dx, {SUD_FREQ,SUD_WID,SUD_CHC,SUD_SQU_BUF}) {
    chs[0] = { &ch,  1, 0 };
    chs[1] = { &ch2, 2, 0 };
    chs[2] = { &ch3, 4, 0 };
    chs[3] = { &ch4, 8, 0 };
    old_type = 0xFF;
}


void NesSquare::change() {
    ch.play( *this, (void*)0);
    ch2.play(*this, (void*)1);
    ch3.play(*this, (void*)2);
    ch4.play(*this, (void*)3);
}


void NesSquare::stop() {
    ch.stop();
    ch2.stop();
    ch3.stop();
    ch4.stop();
}


void NesSquare::operator()(CreateSample & cs) {
    float wave = (dword(r.wavelength_h) << 8) + r.wavelength_l + 1;
    float freq = main_freq / 2 / wave;

    double volumn = 0;
    if (r.volume) {
        volumn = float(r.volume) / (0x0F*7) + (1-float(0xf)/(0xf*7)) - VOL_OFF;
    }

    if (old_type != r.duty_type) {
        const byte *dc = Duty_cycle[r.duty_type];
        for (int i=0; i<cs.dwLength; ++i) {
            cs[i] = dc[i] ? cs.maxval : 0;
        }
        old_type = r.duty_type;
    }

    int i = (int) cs.xdata;
    DXChannel *pch = chs[i].pch;
    pch->setFrequency(freq * chs[i].freq);
    pch->setVolume(max(0, volumn - chs[i].sound));
    // printf("S:%f, v:%f rv:%d,%f \n", freq, volumn, r.volume, wave);
}


static const byte tri_cycle[] = {
    0xF, 0xE, 0xD, 0xC, 0xB, 0xA, 0x9, 0x8, 
    0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0,
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 
    0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
};


static void initTriangle(CreateSample cs) {
    for (int i=0; i<cs.dwLength; ++i){
        cs[i] = tri_cycle[i % sizeof(tri_cycle)] * 0x0F;
    }
} 


NesTriangle::NesTriangle(DXSound &dx) 
        : NesSound(dx, {SUD_FREQ,SUD_WID,SUD_CHC,SUD_TRI_BUF}) {
    ch.play(initTriangle);
    ch.setVolume(1 - VOL_OFF - 0.1);
}


void NesTriangle::operator()(CreateSample & cs) {
    float wave = (dword(r.wavelength_h) << 8) + r.wavelength_l + 1;
    float freq = main_freq / wave;
    ch.setFrequency(freq);
    // printf("T:%f, v:%d rv:%f \n", freq, r.volume, wave);
}


static void initNoise(CreateSample &cs) {
    int32_t rb = 0;
    for (int i=0; i<cs.dwLength; ++i){
        if (rb % 32 == 0) rb = rand();
        byte bit = rb & 0x1;
        cs[i] = bit ? cs.maxval : 0;
        rb >>= 1;
    }
}


NesNoise::NesNoise(DXSound &dx) 
        : NesSound(dx, {SUD_FREQ,SUD_WID,SUD_CHC,SUD_NOS_BUF})
        , ch_low(dx, {SUD_FREQ,SUD_WID,SUD_CHC,32}) {
    ch.play(initNoise, this);
    ch_low.play(initNoise, this);
}


void NesNoise::change() {
    ch.play(*this);
    ch_low.play(*this);
}


void NesNoise::stop() {
    ch.stop();
    ch_low.stop();
}


void NesNoise::operator()(CreateSample & cs) {
    float freq = main_freq / 2 / (r.nfreq + 1);
    double volumn = 0;
    if (r.volume) {
        volumn = float(r.volume) / (0x0F*2) + (1-float(0xf)/(0xf*2)) - VOL_OFF;
    }

    ch.setVolume(volumn);
    ch.setFrequency(freq);

    if (r.nfreq > noise_freq[ sizeof(noise_freq) - 6 ]) {
        ch_low.setVolume(volumn);
        ch_low.setFrequency(freq/4);
    } else {
        ch_low.setVolume(0);
    }
    //printf("N:%f, v:%f  %d \n", freq, volumn, cs[0]);
}
