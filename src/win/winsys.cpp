#include "winsys.h"

void initHdcColor(HDC hdc) {
    HFONT font = CreateFont(10, 0, 0, 0, 100
                , FALSE, FALSE, FALSE, GB2312_CHARSET
                , OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS
                , PROOF_QUALITY, DEFAULT_PITCH, "宋体" );

    SelectObject(hdc, font);
    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkColor(hdc, RGB(0, 0, 0));
}

/****|WindowsVideo|************************************************************/

WindowsVideo::WindowsVideo(HWND hwnd) {
    WindowsVideo(hwnd, PPU_DISPLAY_P_WIDTH, PPU_DISPLAY_P_HEIGHT);
}

WindowsVideo::WindowsVideo(HWND hwnd, int w, int h)
            : x_off(0), y_off(0), width(w+8), height(h+8)
{
    m_hwnd = hwnd;
    hdc = GetDC(hwnd);
    hMemDC = CreateCompatibleDC(hdc);

    hBitmap = CreateCompatibleBitmap(hdc, width, height);

    SelectObject(hMemDC, hBitmap);
    initHdcColor(hdc);
}

void WindowsVideo::setOffset(int x, int y) {
    x_off = x;
    y_off = y;
}

void WindowsVideo::drawPixel(int x, int y, T_COLOR color) {
    SetPixel(hMemDC, x, y, color);
}

void WindowsVideo::refresh() {
    BitBlt(hdc, x_off, y_off, width, height, hMemDC, 0, 0, SRCCOPY);
}

void WindowsVideo::clear(T_COLOR c) {
    HBRUSH hBrush = CreateSolidBrush(c);
    RECT r = {0, 0, width, height};
    FillRect(hMemDC, &r, hBrush);
    DeleteObject(hBrush);
}

WindowsVideo::~WindowsVideo() {
    DeleteDC(hMemDC);
    ReleaseDC(m_hwnd, hdc);
}

/****|DirectXVideo|************************************************************/

DirectXVideo::DirectXVideo(HWND hwnd, int width, int height)
    :lpDD4(0), lpDDSPrimary(0), success(0), pixel(0), m_hwnd(hwnd)
{
    LPDIRECTDRAW lpDD;
    m_height = height;
    m_width  = width;
    pixel    = new T_COLOR[(width+8) * (height+8)];

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

int DirectXVideo::prepareSuccess() {
    return success;
}

void DirectXVideo::drawPixel(int x, int y, T_COLOR color) {
    pixel[x + (y*m_width)] = color;
}

void DirectXVideo::clear(T_COLOR color) {
    for (int i=0; i<m_width * m_height; ++i) {
        pixel[i] = color;
    }
}

void DirectXVideo::refresh() {
    RECT lpDestRect;

    /* 防止绘制到窗口外面 */
    GetClientRect(m_hwnd, &lpDestRect);
    int _w = lpDestRect.right < m_width  ? lpDestRect.right : m_width;
    int _h = lpDestRect.bottom< m_height ? lpDestRect.bottom: m_height;

    /* 转换坐标点 */
    point.x = point.y = 0;
    ClientToScreen(m_hwnd, &point);

    lpDestRect.top    = point.y;
    lpDestRect.left   = point.x;
    lpDestRect.bottom = point.y + _h;
    lpDestRect.right  = point.x + _w;

    if (lpDDSPrimary->Lock(&lpDestRect, &ddsd,
             DDLOCK_SURFACEMEMORYPTR, NULL) != DD_OK) {
        return;
    }

    UINT *buffer = (UINT*)ddsd.lpSurface;
    UINT nPitch = ddsd.lPitch >> 2;

    for (int x=0, y=0;;) {
        buffer[x + y*nPitch] = pixel[x + y*m_width];
        if (++x >= _w) {
            x = 0;
            if (++y >= _h) break;
        }
    }

    lpDDSPrimary->Unlock(&lpDestRect);
}

DirectXVideo::~DirectXVideo() {
    if( lpDDSPrimary != NULL ) {
        lpDDSPrimary->Restore();
        lpDDSPrimary->Release();
        lpDDSPrimary = NULL;
    }
    if( lpDD4 != NULL ) {
        lpDD4->Release();
        lpDD4 = NULL;
    }
    delete [] pixel;
    pixel = NULL;
}

/****|WinPad|******************************************************************/

WinPad::WinPad() {
    p1_key_map[ FC_PAD_BU_A      ] = K_J;
    p1_key_map[ FC_PAD_BU_B      ] = K_K;
    p1_key_map[ FC_PAD_BU_START  ] = K_H;
    p1_key_map[ FC_PAD_BU_SELECT ] = K_F;
    p1_key_map[ FC_PAD_BU_UP     ] = K_W;
    p1_key_map[ FC_PAD_BU_DOWN   ] = K_S;
    p1_key_map[ FC_PAD_BU_LEFT   ] = K_A;
    p1_key_map[ FC_PAD_BU_RIGHT  ] = K_D;
}

byte WinPad::keyPushed(FC_PAD_KEY key, byte id) {
    if (key>=8) return 0;
    return (GetAsyncKeyState( p1_key_map[key] ) & 0x8000) ? 1 : 0;
}
