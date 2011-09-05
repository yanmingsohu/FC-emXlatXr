#include  <windows.h>
#include    <stdio.h>
#include  <Winuser.h>
#include   "winsys.h"
#include   "../ppu.h"

static LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
static Video *bgPanel;
static PPU *ppu;

/* 成功返回指针失败返回NULL */
win_info* bg_panel(HINSTANCE hThisInstance, PPU *_ppu)
{
    win_info *wi = new win_info();

    wi->procedure   = WindowProcedure;
    wi->hInstance   = hThisInstance;
    wi->szClassName = _CSTR("bg panel");
    wi->titleName   = _CSTR("背景");
    wi->height      = 480 + 50;
    wi->width       = 512 + 10;
    wi->nCmdShow    = 0;

    if (!createWindow(wi)) {
        delete wi;
        return NULL;
    }

    ppu     = _ppu;
    bgPanel = new DirectXVideo(wi->hwnd, 512, 480);
    UpdateWindow(wi->hwnd);
    return wi;
}

/*  This function is called by the Windows function DispatchMessage()  */
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)       /* handle the messages */
    {
    case WM_CLOSE:
        ShowWindow(hwnd, SW_HIDE);
        break;

    case WM_DESTROY:
        delete bgPanel;
        break;

    case WM_PAINT:
        bgPanel->clear(0);
        ppu->drawBackGround(bgPanel);
        bgPanel->refresh();
        break;

    default:               /* for messages that we don't deal with */
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}
