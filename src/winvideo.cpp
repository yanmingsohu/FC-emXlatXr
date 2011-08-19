#include "video.h"
#include "ppu.h"
#include "ddraw.h"
#include <windows.h>
#include <stdio.h>

static void initHdcColor(HDC hdc) {
    HFONT font = CreateFont(10, 0, 0, 0, 100
                , FALSE, FALSE, FALSE, GB2312_CHARSET
                , OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS
                , PROOF_QUALITY, DEFAULT_PITCH, "宋体" );

    SelectObject(hdc, font);
    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkColor(hdc, RGB(0, 0, 0));
}

/* 使用GUI绘图 */
class WindowsVideo : public Video {

private:
    HWND    m_hwnd;
    HDC     hdc;
    HDC     hMemDC;
    HBITMAP hBitmap;

public:
    WindowsVideo(HWND hwnd) {
        m_hwnd = hwnd;
        hdc = GetDC(hwnd);
        hMemDC = CreateCompatibleDC(hdc);

        hBitmap = CreateCompatibleBitmap(
                hdc, PPU_DISPLAY_P_WIDTH, PPU_DISPLAY_P_HEIGHT);

        SelectObject(hMemDC, hBitmap);
        initHdcColor(hdc);
    }

    void drawPixel(int x, int y, T_COLOR color) {
        SetPixel(hMemDC, x, y, color);
    }

    void refresh() {
        BitBlt(hdc, 0, 0, PPU_DISPLAY_P_WIDTH,
               PPU_DISPLAY_P_HEIGHT, hMemDC, 0, 0, SRCCOPY);
    }

    ~WindowsVideo() {
        DeleteDC(hMemDC);
        ReleaseDC(m_hwnd, hdc);
    }
};

/* 使用DirectX绘图 */
class DirectXVideo : public Video {

private:
    LPDIRECTDRAW4         lpDD4;                /* DirectDraw对象 */
    LPDIRECTDRAWSURFACE4  lpDDSPrimary;         /* DirectDraw主页面 */
    int                   success;
    T_COLOR               pixel[256*256];
    HWND                  m_hwnd;
    DDSURFACEDESC2        ddsd;
    POINT                 point;

public:
    DirectXVideo(HWND hwnd)
    :lpDD4(0), lpDDSPrimary(0), success(0), m_hwnd(hwnd)
    {
        LPDIRECTDRAW lpDD;

        //创建DirectCraw对象
        if ( DirectDrawCreate( NULL, &lpDD, NULL ) ) {
            printf("DX::Create DirectDraw fail.\n");
            return;
        }

        if ( lpDD->QueryInterface(IID_IDirectDraw4, (LPVOID *)&lpDD4) ) {
            printf("DX::Create DirectDraw4 fail.\n");
            lpDD->Release();
            return;
        }

        lpDD->Release();

        // 取得独占和全屏模式: DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN
        if ( lpDD4->SetCooperativeLevel( hwnd, DDSCL_NORMAL ) ) {
            printf("DX::change Cooperative level fail.\n");
            return;
        }

        // 设置显示模式, 全屏有效
        //if ( lpDD->SetDisplayMode( 640, 480, 8 ) != DD_OK) return FALSE;

        // 填充主页面信息
        //DDSURFACEDESC2 ddsd;
        ZeroMemory(&ddsd, sizeof(ddsd));
        ddsd.dwSize         = sizeof(ddsd);
        ddsd.dwFlags        = DDSD_CAPS;
        ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

        // 创建主页面对象
        if ( lpDD4->CreateSurface( &ddsd, &lpDDSPrimary, NULL ) ) {
            printf("DX::Create main surface fail.\n");
            return;
        }

        success = 1;
    }

    int isSuccess() {
        return success;
    }

    void drawPixel(int x, int y, T_COLOR color) {
        pixel[x + (y<<8)] = color;
    }

    void refresh() {
        RECT lpDestRect;

        /* 转换坐标点 */
        point.x = point.y = 0;
        ClientToScreen(m_hwnd, &point);

        lpDestRect.top    = point.y;
        lpDestRect.left   = point.x;
        lpDestRect.bottom = point.y + PPU_DISPLAY_P_HEIGHT;
        lpDestRect.right  = point.x + PPU_DISPLAY_P_WIDTH;

        if (lpDDSPrimary->Lock(&lpDestRect, &ddsd,
                 DDLOCK_SURFACEMEMORYPTR, NULL) != DD_OK) {
            return;
        }

        UINT *buffer = (UINT*)ddsd.lpSurface;
        UINT nPitch = ddsd.lPitch >> 2;

        for (int x=0; x<PPU_DISPLAY_P_WIDTH; ++x) {
            for (int y=0; y<PPU_DISPLAY_P_HEIGHT; ++y) {
                buffer[y*nPitch + x] = pixel[x + (y<<8)];
            }
        }

        lpDDSPrimary->Unlock(&lpDestRect);
    }

    ~DirectXVideo() {
        if( lpDDSPrimary != NULL ) {
            lpDDSPrimary->Restore();
            lpDDSPrimary->Release();
            lpDDSPrimary = NULL;
        }
        if( lpDD4 != NULL ) {
            lpDD4->Release();
            lpDD4 = NULL;
        }
    }
};

// GetAsyncKeyState(virtual key code)按下=0x8000
