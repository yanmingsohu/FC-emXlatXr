#ifndef WINSYS_H_INCLUDED
#define WINSYS_H_INCLUDED

#include <windows.h>
#include   <stdio.h>
#include   "video.h"
#include     "ppu.h"
#include   "ddraw.h"
#include     "pad.h"

void initHdcColor(HDC hdc);

/* 使用GUI绘图 */
class WindowsVideo : public Video {

private:
    HWND    m_hwnd;
    HDC     hdc;
    HDC     hMemDC;
    HBITMAP hBitmap;
    int     x_off;
    int     y_off;
    int     width;
    int     height;

public:
    WindowsVideo(HWND hwnd) {
        WindowsVideo(NULL, PPU_DISPLAY_P_WIDTH, PPU_DISPLAY_P_HEIGHT);
    }

    WindowsVideo(HWND hwnd, int w, int h)
            : x_off(0), y_off(0), width(w), height(h)
    {
        m_hwnd = hwnd;
        hdc = GetDC(hwnd);
        hMemDC = CreateCompatibleDC(hdc);

        hBitmap = CreateCompatibleBitmap(hdc, width, height);

        SelectObject(hMemDC, hBitmap);
        initHdcColor(hdc);
    }

    void setOffset(int x, int y) {
        x_off = x;
        y_off = y;
    }

    void drawPixel(int x, int y, T_COLOR color) {
        SetPixel(hMemDC, x, y, color);
    }

    void refresh() {
        BitBlt(hdc, x_off, y_off, width, height, hMemDC, 0, 0, SRCCOPY);
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

    inline void drawPixel(int x, int y, T_COLOR color) {
        pixel[x + (y<<8)] = color;
    }

    inline void refresh() {
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

        for (int x=0, y=0;;) {
            buffer[y*nPitch + x] = pixel[x + (y<<8)];
            if (++x>=PPU_DISPLAY_P_WIDTH) {
                x = 0;
                if (++y>=PPU_DISPLAY_P_HEIGHT) break;
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

/* 0 : 0x30, 9 : 0x39
 * A : 0x41, W : 0x57, S : 0x53, D : 0x44
 * F : 0x46, H : 0x48, J : 0x4A, K : 0x4B */
class WinPad : public PlayPad {

private:
    static const byte K_A = 0x41;
    static const byte K_W = 0x57;
    static const byte K_S = 0x53;
    static const byte K_D = 0x44;
    static const byte K_F = 0x46;
    static const byte K_H = 0x48;
    static const byte K_J = 0x4A;
    static const byte K_K = 0x48;

    byte p1_key_map[8];

public:
    WinPad() {
        p1_key_map[FC_PAD_BU_A     ] = K_J;
        p1_key_map[FC_PAD_BU_B     ] = K_K;
        p1_key_map[FC_PAD_BU_START ] = K_H;
        p1_key_map[FC_PAD_BU_SELECT] = K_F;
        p1_key_map[FC_PAD_BU_UP    ] = K_W;
        p1_key_map[FC_PAD_BU_DOWN  ] = K_S;
        p1_key_map[FC_PAD_BU_LEFT  ] = K_A;
        p1_key_map[FC_PAD_BU_RIGHT ] = K_D;
    }

    inline byte keyPushed(FC_PAD_KEY key, byte id) {
        static int a=0;
        if (key==FC_PAD_BU_START && a++>2) {
            a=0;
            printf("send start button.\n");
          //  return 1;
        }
        return (GetAsyncKeyState( p1_key_map[key] ) & 0x8000) ? 1 : 0;
    }
};

#endif
