#include "wit.h"
#include <map>
#include <stdio.h>

using std::map;

static const char szClassName[ ] = "wit_window_class.";
static map<HWND, wWindow*> wins;
static HANDLE messageLoopHandle = 0;
static bool   messageLoopRunning = 0;

/* 每个应用一个事件处理 */
static LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message,
                                        WPARAM wParam, LPARAM lParam)
{
    if (message == WM_CREATE) {
        LPCREATESTRUCT crt = LPCREATESTRUCT(lParam);
        wins[hwnd] = (wWindow*) crt->lpCreateParams;
        return 0;
    }

    wWindow* w = NULL;
    map<HWND, wWindow*>::iterator it = wins.find(hwnd);
    if (it != wins.end()) {
        w = it->second;
    }

    if (message == WM_DESTROY) {
        if (w) {
            wins.erase(hwnd);
            delete w;
        }
        if (!wins.size()) {
            PostQuitMessage(0);
        }
        return 0;
    }

    if (w) {
        return w->_event_procedure(message, wParam, lParam);
    }
    return DefWindowProc (hwnd, message, wParam, lParam);
}

static DWORD messageLoopThread(LPVOID parm) {
    MSG messages;
    while ( GetMessage(&messages, NULL, 0, 0) ) {
        TranslateMessage(&messages);
        DispatchMessage(&messages);
    }

    if (messageLoopHandle) {
        CloseHandle(messageLoopHandle);
        messageLoopHandle = 0;
    }

    messageLoopRunning = false;
    return true;
}

void startMessageLoop(bool inThread) {
    if (!messageLoopRunning) {
        messageLoopRunning = true;

        if (inThread) {
            messageLoopHandle = CreateThread(NULL, 0,
                (LPTHREAD_START_ROUTINE)messageLoopThread, 0, 0, NULL);
        } else {
            messageLoopThread(NULL);
        }
    }
}

bool wPeekAMessage() {
    static MSG messages;

    if (PeekMessage(&messages, NULL, 0, 0, PM_REMOVE)) {
        if (messages.message==WM_QUIT) {
            return false;
        }
        TranslateMessage(&messages);
        DispatchMessage(&messages);
    }

    return true;
}

static void registerWinClass(HINSTANCE hThisInstance) {
    static ATOM reg_class_code = 0;
    if (reg_class_code) return;

    WNDCLASSEX wincl;
    wincl.cbSize        = sizeof (WNDCLASSEX);

    wincl.hInstance     = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc   = WindowProcedure;
    wincl.style         = CS_DBLCLKS;
    wincl.hIcon   = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra   = 0;
    wincl.cbWndExtra   = 0;
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    reg_class_code = RegisterClassEx (&wincl);
}

/* -------------------------------------------------- wMenu class ----------- */

wMenu::wMenu(HMENU _menu)
: menu(_menu), position(0), parent(0), lastsub(0), previous(0)
{
#ifdef __DEBUG__
    printf("::create Menu %X\n", this);
#endif
}

wMenu::wMenu(wMenu *parent, HMENU _menu)
: menu(_menu), position(0), lastsub(0)
{
    this->parent = parent;
    this->previous = parent->lastsub;
    parent->lastsub = this;
#ifdef __DEBUG__
    printf("::create Menu %X\n", this);
#endif
}

UINT wMenu::addItem(UINT id, char* text) {
    InsertMenu(menu, position++, MF_BYPOSITION | MF_STRING, id, text);
    return id;
}

pwMenu wMenu::createSub(UINT id, char* text) {
    MENUITEMINFO minfo;
    minfo.cbSize     = sizeof(MENUITEMINFO);
    minfo.fMask      = MIIM_TYPE | MIIM_ID | MIIM_SUBMENU;
    minfo.fType      = MFT_STRING;
    minfo.dwTypeData = text;
    minfo.wID        = id;
    minfo.hSubMenu   = CreateMenu();

    InsertMenuItem(menu, position++, true, &minfo);

    return new wMenu(this, minfo.hSubMenu);
}

void wMenu::setText(UINT id, char* text) {
    ModifyMenu(menu, id, MF_BYCOMMAND | MF_STRING, id, text);
}

void wMenu::setEnable(UINT id, bool b) {
    EnableMenuItem(menu, id, b ? MF_ENABLED : MF_GRAYED);
}

void wMenu::separator() {
    InsertMenu(menu, position++, MF_BYPOSITION | MF_SEPARATOR, 0, 0);
}

void wMenu::_free() {
    pwMenu main = this;
    while (main->parent) main = main->parent;

    pwMenu dnode = main;
    pwMenu tmp, tmp2;

    for (;;) {
        tmp = dnode->lastsub;
        if (tmp) {
            dnode->lastsub = NULL;
            dnode = tmp;
        } else {
            tmp = dnode->previous ? dnode->previous : dnode->parent;
            dnode->previous = dnode->parent = NULL;
            if (this!=dnode) delete dnode;

            if (tmp) dnode = tmp;
            else break;
        }
    }

    if (this!=main) delete main;
    delete this;
}

wMenu::~wMenu() {
#ifdef __DEBUG__
    printf("::delete Menu %X\n", this);
#endif
}

/* -------------------------------------------------- wWindow class --------- */

wWindow::wWindow() {
    _init(HWND_DESKTOP);
}

wWindow::wWindow(HWND parent) {
    _init(parent);
}

wWindow::wWindow(wWindow *parent) {
    _init(parent->hwnd);
}

void wWindow::_init(HWND parent) {
    hmenu       = NULL;
    menuCount   = 0;
    hInstance   = GetModuleHandle(NULL);
    scale       = 0;
    registerWinClass(hInstance);

    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           NULL,                /* Title Text */
           WS_OVERLAPPEDWINDOW, /* default window */
           CW_USEDEFAULT,       /* Windows decides the position */
           CW_USEDEFAULT,       /* where the window ends up on the screen */
           400,                 /* The programs width */
           300,                 /* and height in pixels */
           parent,              /* The window is a child-window to parent */
           NULL,                /* No menu */
           hInstance,           /* Program Instance handler */
           this
           );
}

void wWindow::setVisible(bool show) {
    ShowWindow(hwnd, show ? SW_SHOW : SW_HIDE);
    if (show) {
        DrawMenuBar(hwnd);
    }
}

void wWindow::setClientSize(int w, int h) {
	RECT r;

	GetClientRect(hwnd, &r);
	w -= r.right;
	h -= r.bottom;

	GetWindowRect(hwnd, &r);
	w += (r.right - r.left);
	h += (r.bottom - r.top);

	SetWindowPos(hwnd, NULL, 0, 0, w, h, SWP_NOMOVE);
}

void wWindow::setPosition(int x, int y) {
    SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void wWindow::setTitle(char *txt) {
    SendMessage(hwnd, WM_SETTEXT, (LPARAM)NULL, (LPARAM)txt);
}

void wWindow::setText(char *txt) {
    setTitle(txt);
}

void wWindow::setEnable(bool en) {
    EnableWindow(hwnd, en);
}

void wWindow::setScale(double s) {
    scale = s;
}

HWND wWindow::getWindowHandle() {
    return hwnd;
}

pwMenu wWindow::getMenuBar() {
    if (!hmenu) {
        HMENU menu = CreateMenu();
        SetMenu(hwnd, menu);
        hmenu = new wMenu(menu);
    }
    return hmenu;
}

void wWindow::destroy() {
    DestroyWindow(hwnd);
}

wWindow::~wWindow() {
    /* 防止WindowProcedure二次删除该对象 */
    wins.erase(hwnd);
    DestroyWindow(hwnd);
    if (hmenu) hmenu->_free();

    hmenu     = NULL;
    hwnd      = NULL;
    hInstance = NULL;
}

/* ----------------------------------------- default event process ---------- */

LRESULT wWindow::_event_procedure(UINT message, WPARAM wParam, LPARAM lParam) {
    WIT_EVENT_PARM parm = {hwnd, message, wParam, lParam, 0, false};

    switch (message)
    {
    case WM_COMMAND:
        if (lParam) {
            on_widget(&parm, (HWND)lParam);
        } else if (HIWORD(wParam)) {
            // key
        } else {
            on_menu(&parm, LOWORD(wParam));
        }
        break;

#define CASE(_a,_b) case _a: _b(&parm); break
    CASE(WM_CLOSE,      on_close);
    CASE(WM_PAINT,      on_paint);
#undef CASE

    case WM_SIZE:
        if (scale) {
            int w = LOWORD(lParam);
            int h = w / scale;
            setClientSize(w, h);;
        }

    default: parm.defproc = true;
    }

    if (parm.defproc) {
        return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return parm.ret;
}

EVENT_FUNC_DEF(on_menu)(WIT_EVENT_PARM *p, UINT menu_id) {
#ifdef __DEBUG__
    printf("menu pushed %X\n", menu_id);
#endif
}

EVENT_FUNC_DEF(on_widget)(WIT_EVENT_PARM *p, HWND widget) {
#ifdef __DEBUG__
    printf("widget event %X\n", widget);
#endif
}

EVENT_FUNC_DEF(on_close)(WIT_EVENT_PARM *p) {
    destroy();
}
