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
#ifndef PAD_H_INCLUDED
#define PAD_H_INCLUDED

#include <memory.h>

/* 手柄按键 */
enum FC_PAD_KEY {
    FC_PAD_BU_A       = 0,
    FC_PAD_BU_B       = 1,
    FC_PAD_BU_SELECT  = 2,
    FC_PAD_BU_START   = 3,
    FC_PAD_BU_UP      = 4,
    FC_PAD_BU_DOWN    = 5,
    FC_PAD_BU_LEFT    = 6,
    FC_PAD_BU_RIGHT   = 7
};

/**
 * 游戏手柄,1P&2P
 */
class PlayPad {

private:
    byte wcount;
    byte rcount;
    byte keySave[16];

public:
    static const int PLAYER_1 = 0;
    static const int PLAYER_2 = 1;

    PlayPad() : wcount(0), rcount(0) {
        memset(keySave, 0, sizeof(keySave));
    }

    virtual ~PlayPad() {};

    /* 如果该键按下则返回1, id为控制器号码 0==1P */
    virtual byte keyPushed(FC_PAD_KEY key, byte id) = 0;

    /* 向4016/4017端口写数据 */
    void writePort(word port, byte data) {
        if (port==0x4016) {
            /* 发送1,再发送0开始读取键位 */
            if (wcount==0 && data==1) wcount++;
            if (wcount==1 && data==0) {
                rcount = 0;
                wcount = 0;
            }
        }
    }

    /* 从4016/4017端口读数据 */
    byte readPort(word port) {
        if (port==0x4016 && rcount<8) {
            if (keySave[rcount]) {
                keySave[rcount] = 0;
                ++rcount;
                return 1;
            }
            return keyPushed(FC_PAD_KEY(rcount++), PLAYER_1);
        }
        return 0;
    }

    /* 使用编程的方法按下一个键子,调试时使用 */
    void pushKey(FC_PAD_KEY key, byte padid) {
        if (padid==PLAYER_1) {
            keySave[key] = 1;
        } else {
            keySave[key + 8] = 1;
        }
    }
};

#endif // PAD_H_INCLUDED
