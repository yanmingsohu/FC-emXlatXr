#ifndef PAD_H_INCLUDED
#define PAD_H_INCLUDED

/* 手柄按键 */
enum FC_PAD_KEY {
    FC_PAD_BU_A       = 0,
    FC_PAD_BU_B       = 1,
    FC_PAD_BU_SELECT  = 2,
    FC_PAD_BU_START   = 3,
    FC_PAD_BU_DOWN    = 4,
    FC_PAD_BU_UP      = 5,
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

public:
    PlayPad() : wcount(0), rcount(0)
    {
    }

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
        if (port==0x4016) {
            return keyPushed(FC_PAD_KEY(rcount++), 0);
        }
        return 0;
    }
};

#endif // PAD_H_INCLUDED
