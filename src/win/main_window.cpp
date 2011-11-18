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


static const double FRAME_RATE = 1000/60.0;
static char titleName  [ ] = SF_NAME_JYM SF_VERSION_JYM;


class NesScrollWindow : public wWindow {
private:
    static const UINT TIME_ID = 1;
    static const UINT UPDATE_TIME = 300;

    PPU*    ppu;
    Video*  video;
    int     w, h;

    void _beginUpdate(bool begin) {
        if (begin) {
            setVisible(true);
            startTimer(TIME_ID, UPDATE_TIME);
            on_time(0);
        } else {
            stopTimer(TIME_ID);
            setVisible(false);
        }
    }

public:
    NesScrollWindow(wWindow *p, PPU *_ppu, int _w=512, int _h=512)
    : wWindow(p), ppu(_ppu), w(_w), h(_h)
    {
        setClientSize(w, h);
        setTitle("背景卷轴");
        video = new DirectX3DVideo(getWindowHandle(), w, h);
    }

    ~NesScrollWindow() {
        delete video;
    }

    /* 开始循环绘制内容 */
    void beginUpdate() {
        _beginUpdate(true);
    }

    virtual void paintFunc(PPU *_p, Video *_v) {
        _p->drawBackGround(_v);
    }

    void on_close(WIT_EVENT_PARM* p) {
        _beginUpdate(false);
    }

    void on_time(UINT timer_id) {
        video->clear(0);
        paintFunc(ppu, video);
        video->refresh();
    }
};

class NesTileWindow : public NesScrollWindow {
public:
    NesTileWindow(wWindow *p, PPU *_ppu)
    : NesScrollWindow(p, _ppu, 128, 256)
    {
        setTitle("瓦片卷轴");
    }

    void paintFunc(PPU *_p, Video *_v) {
        _p->drawTileTable(_v);
    }
};

class NesWindow : public wWindow {
private:
    static const int WIDTH  = 256;
    static const int HEIGHT = 256;

    enum MenuIDS {
        M_ID_OPEN=1, M_ID_EXIT,   M_ID_STEP,
        M_DI_TILE,   M_DI_BG,     M_ID_PAUSE,
        MM_ID_GAME,  MM_ID_DEBUG
    };

    NesSystem       *fc;
    Video           *video;
    PlayPad         *pad;
    NesTileWindow   *tile;
    NesScrollWindow *bg;

    bool run;
    bool quit;
    bool sDebug;

    void _create_menu() {
        pwMenu menu = getMenuBar();

        pwMenu file = menu->createSub(MM_ID_GAME, "游戏");
        file->addItem(M_ID_OPEN, "打开游戏");
        file->addItem(M_ID_PAUSE,"暂停/继续");
        file->separator();
        file->addItem(M_ID_EXIT, "退出");

        pwMenu debug = menu->createSub(MM_ID_DEBUG, "调试");
        debug->addItem(M_ID_STEP,"单步执行");
        debug->addItem(M_DI_TILE,"显示瓦片");
        debug->addItem(M_DI_BG,  "显示卷轴");
    }

    void frameRate() {
        static long frameC = 0;
        static int f2c = 0;
        static clock_t time = clock();
        static char txt[20];

        ++frameC; ++f2c;
        if (frameC%42!=0) return;

        double utime = (double)(clock() - time)/CLOCKS_PER_SEC;
        printf("frame: %ld rate : %04lf/s\n", frameC, f2c/utime);
        time = clock(); f2c = 0;
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
            if (!run) {
                Sleep(50);
                continue;
            }

            if (sDebug) {
                debugCpu(fc);
                sDebug = 0;
            }
            fc->drawFrame();
            video->refresh();

            //frameRate();

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

        case M_ID_PAUSE:
            run = !run;
            break;

        case M_DI_TILE:
            tile->beginUpdate();
            break;

        case M_DI_BG:
            bg->beginUpdate();
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
