/*----------------------------------------------------------------------------*\
 * windows UI组件库, 提供简单框架                                             *
 * CatfoOD 2011                                                               *
 * yanming-sohu@sohu.com                                                      *
\*----------------------------------------------------------------------------*/

#ifndef WIT_H_INCLUDED
#define WIT_H_INCLUDED

#include <windows.h>

#define EVENT_FUNC_DEF(_n)  void wWindow::_n
#define EVENT_FUNC_DECL(_n) virtual void _n

/* -------------------------------------------------- *|
 * 启动事件监听,                                   -- *|
 * inThread==true 则启动新的线程, 该函数立即返回   -- *|
 * 否则直到所有的窗口都被销毁后, 该函数才返回      -- *|
 * 多次调用只会启动一个事件监听器                  -- *|
 * -------------------------------------------------- */
void startMessageLoop(bool inThread);

/* -------------------------------------------------- *|
/* 处理一条windows系统事件并立即返回               -- *|
 * 如果返回false说明接收到WM_QUIT消息              -- *|
 * -------------------------------------------------- */
bool wPeekAMessage();


/* 事件处理函数的参数 */
struct WIT_EVENT_PARM {
    HWND    hwnd;
    UINT    message;
    WPARAM  wParam;
    LPARAM  lParam;
    LRESULT ret;     /* 消息的返回值                  */
    bool    defproc; /* 如果true, 则使用默认处理函数  */
};

class wMenu {

friend class wWindow;

private:
    /* 当窗口关闭,系统自动释放已经关联到window的菜单 */
    HMENU menu;
    UINT position;

    wMenu *parent;
    wMenu *lastsub;
    wMenu *previous;

    wMenu(HMENU _menu);
    wMenu(wMenu *parent, HMENU _menu);
    ~wMenu();

    void _free();

public:
    /* 返回菜单项的ID */
    UINT    addItem     (UINT id,  char* text);
    wMenu*  createSub   (UINT id,  char* text);
    void    setText     (UINT item,char* text);
    void    setEnable   (UINT id,  bool  b   );
    void    setChecked  (UINT id,  bool  b   );
    void    separator   ();
};

typedef wMenu* pwMenu;

/* 该对象不需要手工释放内存, 当window被销毁的时候自动释放 */
class wWindow {

private:
    HWND      hwnd;
    HINSTANCE hInstance;
    pwMenu    hmenu;
    UINT      menuCount;
    double    scale;

    void _init(HWND parent);

public:
    /* 立即创建一个窗口, 但不显示 */
    wWindow();
    /* 创建一个子窗口 */
    wWindow(HWND parent);
    wWindow(wWindow *parent);

    virtual ~wWindow();

    /* 立即销毁该窗口, 释放内存, 与之相关的对象都不能使用 */
    void destroy();
    /* 设置窗口是否显示 */
    void setVisible(bool show);
    /* 设置工作区尺寸 */
    void setClientSize(int, int);
    /* 改变窗口的位置 */
    void setPosition(int, int);
    /* 设置标题 */
    void setTitle(char *txt);
    /* 设置文本 */
    void setText(char *txt);
    /* 设置组件是否启用 */
    void setEnable(bool en);
    /* 设置窗口比例, 0则不限制, w/h */
    void setScale(double s);
    /* 创建一个定时器并开始计时,达到uElapse时间(毫秒),on_time方法被调用 */
    void startTimer(UINT timer_id, UINT uElapse_ms);
    /* 立即终止一个定时器 */
    void stopTimer(UINT timer_id);
    /* 取得当前窗口句柄 */
    HWND getWindowHandle();
    /* 返回菜单对象(无需释放), 第一次调用会创建一个 */
    pwMenu getMenuBar();

// *-------------------------------------------------------------------- //--//
//  如果重写了事件处理程序, 则不会再调用DefWindowProc
//  默认实现使用 DEF_EVENT_FUNC_IMPL(...),
//  WIT_EVENT_PARM 参数在事件处理函数返回后失效
// *-------------------------------------------------------------------- //--//
#define DEF_EVENT_FUNC_IMPL(_n) EVENT_FUNC_DECL(_n)(WIT_EVENT_PARM *p) \
                                { p->defproc = true; }

    /* 窗口的事件处理器, 客户不要直接调用 */
    LRESULT _event_procedure(UINT message, WPARAM wParam, LPARAM lParam);

    /* 默认的菜单处理过程, 什么都不做 */
    EVENT_FUNC_DECL     (on_menu)   (WIT_EVENT_PARM*, UINT menu_id);
    /* 默认的控件(按钮等)处理过程, 什么都不做 */
    EVENT_FUNC_DECL     (on_widget) (WIT_EVENT_PARM*, HWND widget);

    /* 默认会销毁该窗口 */
    EVENT_FUNC_DECL     (on_close)  (WIT_EVENT_PARM*);
    /* 默认会打印警告, 如果定义了定时器必须重写该方法 */
    EVENT_FUNC_DECL     (on_time)   (UINT timer_id);

    DEF_EVENT_FUNC_IMPL (on_paint);

#undef DEF_EVENT_FUNC_IMPL
};

typedef wWindow* pwWindow;

#endif // WIT_H_INCLUDED
