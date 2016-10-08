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


static const word noise_freq [] = {
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


static const byte Duty_cycle[][8] = {
    { 0,1,0,0, 0,0,0,0 },
    { 0,1,1,0, 0,0,0,0 },
    { 0,1,1,1, 1,0,0,0 },
    { 1,0,0,1, 1,1,1,1 },
};


void NesSquare::operator()(CreateSample & cs) {
    double volumn = cs.maxval * (float(r.volume) / 0x0F);
    const byte *dc = Duty_cycle[r.duty_type];
    cs[0] = dc[0] ? volumn : 0;
    cs[1] = dc[1] ? volumn : 0;
    cs[2] = dc[2] ? volumn : 0;
    cs[3] = dc[3] ? volumn : 0;
    cs[4] = dc[4] ? volumn : 0;
    cs[5] = dc[5] ? volumn : 0;
    cs[6] = dc[6] ? volumn : 0;
    cs[7] = dc[7] ? volumn : 0;

    float wave = (dword(r.wavelength_h) << 8) + r.wavelength_l + 1;
    // ? why /16
    float freq = main_freq /2/ wave;
    // printf("S:%f, 0:%d 1:%d 2:%d v:%f rv:%d,%f \n", freq, cs[0], cs[1], cs[2], volumn, r.volume, wave);
    ch.setFrequency(freq);
}


static const byte tri_cycle[] = {
    0xF, 0xE, 0xD, 0xC, 0xB, 0xA, 0x9, 0x8, 
    0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0,
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 
    0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
};


void NesTriangle::operator()(CreateSample & cs) {
    static bool isset = false;
    if (!isset) {
        for (int i=0; i<sizeof(tri_cycle); ++i){
            cs[i] = tri_cycle[i] * 0x0F;
        }
        isset = true;
    }

    float wave = (dword(r.wavelength_h) << 8) + r.wavelength_l + 1;
    // ? why /32
    float freq = main_freq / wave;
    // printf("T:%f, 0:%d 1:%d 2:%d rv:%f \n", freq, cs[0], cs[1], cs[2], wave);
    ch.setFrequency(freq);
}


void NesNoise::operator()(CreateSample & cs) {
    double volumn = cs.maxval * (float(r.volume) / 0x0F);

    for (int i=0; i<32; ++i){
        cs[i] = (rand() % 2) * volumn;
    }

    float freq = main_freq / 8 / (r.nfreq + 1);
    ch.setFrequency(freq);
    // printf("N:%f, v:%f  %d \n", freq, volumn, cs[0]);
}
