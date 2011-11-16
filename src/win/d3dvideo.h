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
#ifndef D3DVIDEO_H_INCLUDED
#define D3DVIDEO_H_INCLUDED

/* ---------------------------- fix d3dx9.h ---- */
#ifndef DECLSPEC_DEPRECATED
#if (_MSC_VER >= 1300) && !defined(MIDL_PASS)
#define DECLSPEC_DEPRECATED __declspec(deprecated)
#define DEPRECATE_SUPPORTED
#else
#define DECLSPEC_DEPRECATED
#undef DEPRECATE_SUPPORTED
#endif
#endif
/* --------------------------------------------- */

#include "../video.h"
#include <windows.h>
#include <d3d9.h>

class DirectX3DVideo : public Video {

struct PANELVERTEX {
    FLOAT x, y, z;
    DWORD color;
    FLOAT u, v;
};

private:
    static const DWORD D3DFVF_PANELVERTEX
        = (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);

    LPDIRECT3D9             pD3D;
    LPDIRECT3DDEVICE9       pd3dDevice;
    LPDIRECT3DVERTEXBUFFER9 pVertices;
    LPDIRECT3DTEXTURE9      pTexture;
    T_COLOR*                pixel;
    UINT                    width, height;

    void _initVertices(int w, int h);
    void _createTexture(int w, int h);
    void _initMatrix(int w, int h);

public:
    DirectX3DVideo(HWND hwnd, int w, int h);
    ~DirectX3DVideo();

    void drawPixel(int x, int y, T_COLOR color);
    void refresh();
    void clear(T_COLOR color);
    int  prepareSuccess();
    void resize(int w, int h);
};

#endif // D3DVIDEO_H_INCLUDED
