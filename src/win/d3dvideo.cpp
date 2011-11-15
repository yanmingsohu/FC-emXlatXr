#include "d3dvideo.h"

DirectX3DVideo::DirectX3DVideo(HWND hWnd, int w, int h)
    : pD3D(0), pd3dDevice(0), pVertices(0), pTexture(0), pixel(0)
{
    if( NULL == ( pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
        return;

    width = w;
    height = h;

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof( d3dpp ) );
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

    if( FAILED( pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
                                    D3DCREATE_HARDWARE_VERTEXPROCESSING,
                                    &d3dpp, &pd3dDevice ) ) ) return;

    D3DXMATRIX Ortho2D;
    D3DXMATRIX Identity;

    D3DXMatrixOrthoLH(&Ortho2D, w, h, 0.0f, 1.0f);
    D3DXMatrixIdentity(&Identity);

    pd3dDevice->SetTransform(D3DTS_PROJECTION, &Ortho2D);
    pd3dDevice->SetTransform(D3DTS_WORLD, &Identity);
    pd3dDevice->SetTransform(D3DTS_VIEW, &Identity);

    _initVertices(w, h);
    _createTexture(w, h);

    pixel = new T_COLOR[(w+8) * (h+8)];
}

void DirectX3DVideo::_createTexture(int w, int h) {

    pd3dDevice->CreateTexture(w, h, 1, 0, D3DFMT_A8R8G8B8,
                              D3DPOOL_MANAGED, &pTexture, 0);
    if (pTexture) {
        pd3dDevice->SetTexture(0, pTexture);
    } else {
        printf("D3D::create texture fail.\n");
    }
}

void DirectX3DVideo::_initVertices(int w, int h) {

    static PANELVERTEX VERTICES_DATA[] = {
        {-w / 2.0f, h / 2.0f, 1.0f, 0xFFFFFF, 0.0f, 0.0f},
        { w / 2.0f, h / 2.0f, 1.0f, 0xFFFFFF, 1.0f, 0.0f},
        { w / 2.0f,-h / 2.0f, 1.0f, 0xFFFFFF, 1.0f, 1.0f},
        {-w / 2.0f,-h / 2.0f, 1.0f, 0xFFFFFF, 0.0f, 1.0f},
    };

    pd3dDevice->CreateVertexBuffer(4 * sizeof(PANELVERTEX), D3DUSAGE_WRITEONLY,
                                   D3DFVF_PANELVERTEX, D3DPOOL_MANAGED, &pVertices, NULL);

    PANELVERTEX* vertices = NULL;
    pVertices->Lock(0, 4 * sizeof(PANELVERTEX), (void**)&vertices, 0);
    memcpy(vertices, VERTICES_DATA, sizeof(VERTICES_DATA));
    pVertices->Unlock();

    //pd3dDevice->SetVertexShader(D3DFVF_PANELVERTEX); // d3d8
    pd3dDevice->SetFVF(D3DFVF_PANELVERTEX);
    pd3dDevice->SetStreamSource(0, pVertices, 0, sizeof(PANELVERTEX));
    pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
}

DirectX3DVideo::~DirectX3DVideo() {
    if ( pd3dDevice != NULL )
        pd3dDevice->Release();

    if ( pD3D != NULL )
        pD3D->Release();

    if ( pVertices != NULL )
        pVertices->Release();

    if ( pTexture != NULL )
        pTexture->Release();

    if ( pixel != NULL )
        delete [] pixel;
}

void DirectX3DVideo::drawPixel(int x, int y, T_COLOR color) {
    pixel[x + (y*width)] = color;
}

void DirectX3DVideo::refresh() {
    if( NULL == pd3dDevice )
        return;

    // Begin the scene
    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
        /* ÐÞ¸ÄÎÆÀí */
        D3DLOCKED_RECT lrect;
        if (D3D_OK == pTexture->LockRect(0, &lrect , 0, 0)) {
            UINT *pdata = (UINT*) lrect.pBits;
            UINT nPitch = lrect.Pitch >> 2;

            for (int x=0, y=0;;) {
                pdata[x + y*nPitch] = pixel[x + y*width];
                if (++x >= width) {
                    if (++y >= height) break;
                    x = 0;
                }
            }

            pTexture->UnlockRect(0);
        } else {
            printf("D3D::modify texture fail.\n");
        }

        pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);

        pd3dDevice->EndScene();
    }

    // Present the backbuffer contents to the display
    pd3dDevice->Present( NULL, NULL, NULL, NULL );
}

void DirectX3DVideo::clear(T_COLOR color) {
    for (int i=width * height - 1; i>=0; --i) {
        pixel[i] = color;
    }
}
