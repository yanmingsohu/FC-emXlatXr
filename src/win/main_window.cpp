#include     <windows.h>
#include       <stdio.h>
#include        <time.h>
#include     <Winuser.h>

#include      "winsys.h"
#include    "../debug.h"
#include "../testRoms.h"
#include    "d3dvideo.h"
#include    "../speed.h"
#include         "wit.h"


static void  displayCpu (cpu_6502*, HWND );
static void  frameRate  (Video*          );

/*  Make the class name into a global variable  */
static const double FRAME_RATE = 1000/60.0;
static char titleName  [ ] = SF_NAME_JYM SF_VERSION_JYM;


class NesScrollWindow : public wWindow {
private:
    PPU   *ppu;
    Video *video;

public:
    NesScrollWindow(wWindow *p, PPU *_ppu)
    : wWindow(p), ppu(_ppu)
    {
        int w, h;
        getVideoSize(&w, &h);
        setClientSize(w, h);
        setTitle("背景卷轴");
        video = new DirectXVideo(getWindowHandle(), w, h);
    }

    ~NesScrollWindow() {
        delete video;
    }

    void on_paint(WIT_EVENT_PARM *p) {
        static int time = 100;

        if (!(--time)) {
            video->clear(0);
            paintFunc(ppu, video);
            video->refresh();
            time = 100;
        }
    }

    virtual void paintFunc(PPU *_p, Video *_v) {
        _p->drawBackGround(_v);
    }

    virtual void getVideoSize(int *w, int *h) {
        *w = 512;
        *h = 480;
    }

    void on_close(WIT_EVENT_PARM* p) {
        setVisible(false);
    }
};

class NesTileWindow : public NesScrollWindow {
public:
    NesTileWindow(wWindow *p, PPU *_ppu)
    : NesScrollWindow(p, _ppu)
    {
        setClientSize(128, 280);
        setTitle("瓦片卷轴");
    }

    void paintFunc(PPU *_p, Video *_v) {
        _p->drawTileTable(_v);
    }

    void getVideoSize(int *w, int *h) {
        *w = 128;
        *h = 280;
    }
};

class NesWindow : public wWindow {
private:
    static const int WIDTH       = 256;
    static const int HEIGHT      = 256;

    enum MenuIDS {
        M_ID_OPEN=1, M_ID_EXIT,  M_ID_STEP,
        M_DI_TILE,   M_DI_BG
    };

    NesSystem  *fc;
    Video      *video;
    PlayPad    *pad;
    bool        run;
    bool        quit;
    bool        sDebug;
    wWindow    *tile;
    wWindow    *bg;

    void _create_menu() {
        pwMenu menu = getMenuBar();

        pwMenu file = menu->createSub(1, "文件");
        file->addItem(M_ID_OPEN, "打开");
        file->addItem(M_ID_EXIT, "退出");

        pwMenu debug = menu->createSub(2, "调试");
        debug->addItem(M_ID_STEP,"单步执行");
        debug->addItem(M_DI_TILE,"显示瓦片");
        debug->addItem(M_DI_BG,  "显示卷轴");
    }

public:
    NesWindow() : run(0), quit(0), sDebug(0) {
        _create_menu();
        setTitle(titleName);
        setClientSize(WIDTH, HEIGHT);

        // WindowsVideo | DirectXVideo | DirectX3DVideo
        video   = new DirectX3DVideo(getWindowHandle(), WIDTH, HEIGHT);
        pad     = new WinPad();
        fc      = new NesSystem(video, pad);

        #ifdef TEST_ROM
        run = !fc->load_rom(TEST_ROM);
        #endif

        bg   = new NesScrollWindow(this, fc->getPPU());
        tile = new NesTileWindow  (this, fc->getPPU());
    }

    ~NesWindow() {
        delete fc;
        delete video;
        delete tile;
        delete bg;
    }

    void gameLoop() {
        SpeedLimit sl(FRAME_RATE);

        for (;;) {
            sl.start();

            if (!wPeekAMessage()) break;
            if (quit) break;
            if (!run) continue;

            if (sDebug) {
                debugCpu(fc);
                sDebug = 0;
            }
            fc->drawFrame();
            video->refresh();

            frameRate(video);
            //displayCpu(cpu, hwnd);

            sl.end();
        }
    }

    void on_menu(WIT_EVENT_PARM *p, UINT menu_id) {
        switch (menu_id) {
        case M_ID_OPEN:
            openFile();
            break;

        case M_ID_STEP:
            sDebug = true;
            break;

        case M_DI_TILE:
            tile->setVisible(true);
            break;

        case M_DI_BG:
            bg->setVisible(true);
            break;

        case M_ID_EXIT:
            quit = true;
            break;
        }
    }

    void on_close(WIT_EVENT_PARM* p) {
        quit = true;
    }

    void openFile() {
        HWND hwnd = getWindowHandle();
        char filename[1024] = ".\\rom\\*.nes";

        OPENFILENAME ofn;
        memset(&ofn, 0, sizeof(ofn));

        ofn.lStructSize    = sizeof(ofn);
        ofn.hwndOwner      = hwnd;
        ofn.lpstrFilter    = "FC Rom (.nes)\0*.nes\0\0";
        ofn.nFilterIndex   = 1;
        ofn.lpstrFile      = filename;
        ofn.nMaxFile       = sizeof(filename);

        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle  = 0;

        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

        if(GetOpenFileName(&ofn))
        {
            if (int ret = fc->load_rom(filename)) {
                MessageBox(hwnd, parseOpenError(ret), "错误", 0);
                run = false;
            } else {
                run = true;
            }
        }
    }
};

int WINAPI WinMain ( HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow )
{
    NesWindow nes;
    nes.setVisible(true);
    nes.gameLoop();

    return 0;
}

void frameRate(Video *video) {
    static long frameC = 0;
    static int f2c = 0;
    static clock_t time = clock();

    ++frameC;
    ++f2c;

    if (frameC%42!=0) return;

    double utime = (double)(clock() - time)/CLOCKS_PER_SEC;

    printf("frame: %ld rate : %04lf/s\n", frameC, f2c/utime);

    time = clock();
    f2c = 0;
}

void displayCpu(cpu_6502* cpu, HWND hwnd) {
    static long frameC = 0, f2c = 0;
    static clock_t time = clock();

    ++frameC;
    ++f2c;
    /* 就是不让它每一帧都更新 */
    if (frameC%42!=0) return;

    HDC hdc = GetDC(hwnd);
    SetBkColor(hdc, 0);
    SetTextColor(hdc, 0xFFFFFF);

    int x   = 270;
	int y   = 0;
	int yIn = 18;
	char buf[128];

    double utime = (double)(clock() - time)/CLOCKS_PER_SEC;

#define TEXT_OUT(fmt, parm, len)    sprintf(buf, fmt, parm);  \
                                    TextOut(hdc, x, y+=yIn, buf, len)

    TEXT_OUT("A: %02X",        cpu->A,     5);
    TEXT_OUT("X: %02X",        cpu->X,     5);
    TEXT_OUT("Y: %02X",        cpu->Y,     5);
    TEXT_OUT("SP: %02X",       cpu->SP,    6);
    TEXT_OUT("PC: %04X",       cpu->PC,    8);
    TEXT_OUT("FG: %02X",       cpu->FLAGS, 6);
    TEXT_OUT("frame: %09ld",   frameC,    16);
    TEXT_OUT("rate : %04lf/s", f2c/utime, 13);

#undef TEXT_OUT

    time = clock();
    f2c = 0;
}
