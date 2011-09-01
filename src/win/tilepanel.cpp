#include  <windows.h>
#include    <stdio.h>
#include  <Winuser.h>
#include   "winsys.h"
#include   "../ppu.h"

static LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
static WindowsVideo *tPanel;
static PPU *ppu;

/* 成功返回指针失败返回NULL */
win_info* tile_panel(HINSTANCE hThisInstance, PPU *_ppu)
{
    MSG messages;            /* Here messages to the application are saved */
    win_info *wi = new win_info();

    wi->procedure   = WindowProcedure;
    wi->hInstance   = hThisInstance;
    wi->szClassName = "title panel";
    wi->titleName   = "字库";
    wi->height      = 300;
    wi->width       = 150;
  //  wi->nCmdShow    = 0;

    if (!createWindow(wi)) {
        delete wi;
        return NULL;
    }

    ppu    = _ppu;
    tPanel = new WindowsVideo(wi->hwnd, 512, 480);
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
        delete tPanel;
        break;

    case WM_PAINT:
        ppu->drawTileTable(tPanel);
        tPanel->refresh();
        break;

    default:               /* for messages that we don't deal with */
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}
