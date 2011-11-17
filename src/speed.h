#ifndef SPEED_H_INCLUDED
#define SPEED_H_INCLUDED

#ifdef __WIN32__
#include <Winbase.h>
#endif

/* 使用匀速时间执行start()与end()之间的代码 */
struct SpeedLimit {

private:
    const double  frametime;
    double        sleeptime;
    unsigned long currtime;

    /* 返回ms */
    unsigned long _current_time() {
    #ifdef __WIN32__
        return GetTickCount();
    #endif
    }

    void _sleep(unsigned long milliseconds) {
    #ifdef __WIN32__
        Sleep(milliseconds);
    #endif
    }

public:
    /* frame_time start()-end()最小时间间隔 */
    SpeedLimit(double _frame_time)
    : frametime(_frame_time), sleeptime(0), currtime(0)
    {
    }

    /* 开始执行 */
    void start() {
        currtime = _current_time();
    }

    /* 结束运行, 并可能休眠使 end()-start() >= frametime */
    void end() {
        sleeptime += frametime - (_current_time() - currtime);
        currtime = _current_time();

        if (sleeptime > 0) {
            _sleep(sleeptime);
            sleeptime -= _current_time() - currtime;
        } else {
            sleeptime = 0;
        }
    }
};

#endif // SPEED_H_INCLUDED
