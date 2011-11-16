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
#ifndef VIDEO_H_INCLUDED
#define VIDEO_H_INCLUDED

#include "type.h"

/* 绘制像素的接口, 每个系统实现的不同, 高宽要多8像素用来绘制超出去的部分 */
class Video {
public:
    /* 绘制一个像素 */
    virtual void drawPixel(int x, int y, T_COLOR color) = 0;
    /* 刷新缓存到页面 */
    virtual void refresh() {}
    /* 清除缓冲区，使用指定的颜色  */
    virtual void clear(T_COLOR color) = 0;
    /* 准备完成则返回0, 否则返回错误码 */
    virtual int prepareSuccess() { return 0; }
    /* 修改窗口大小后, 调用该方法, 重设比例 */
    virtual void resize(int w, int h) {}

    virtual ~Video() {};
};

#endif // VIDEO_H_INCLUDED
