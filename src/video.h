#ifndef VIDEO_H_INCLUDED
#define VIDEO_H_INCLUDED

#include "type.h"

/* 绘制像素的接口, 每个系统实现的不同 */
class Video {
public:
    /* 绘制一个像素 */
    virtual void drawPixel(int x, int y, T_COLOR color) = 0;
    /* 刷新缓存到页面 */
    virtual void refresh() {}
};

#endif // VIDEO_H_INCLUDED
