#include  <windows.h>
#include    <stdio.h>
#include     <time.h>
#include  <Winuser.h>
#include   "winsys.h"
#include "../debug.h"


static LRESULT     CALLBACK WindowProcedure   (HWND, UINT, WPARAM, LPARAM  );
static void        displayCpu                 (cpu_6502*, HWND             );
static void        start_game                 (HWND, PMSG, HINSTANCE       );
static HMENU       createMainMenu             (                            );
static void        openFile                   (HWND hwnd                   );

/*  Make the class name into a global variable  */
static char szClassName[ ] = "CodeBlocksWindowsApp";
static char titleName  [ ] = "FC 模拟器 DEmo. -=CatfoOD=-";
static bool active = 1;
static bool sDebug = 0;
static bool run    = 0;
static win_info*  bgpanel = NULL;
static win_info*  tlpanel = NULL;
static NesSystem* fc;

int WINAPI WinMain ( HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow )
{
    MSG messages;            /* Here messages to the application are saved */
    win_info wi;

    wi.procedure   = WindowProcedure;
    wi.hInstance   = hThisInstance;
    wi.szClassName = szClassName;
    wi.titleName   = titleName;
    wi.nCmdShow    = nCmdShow;
    wi.menu        = createMainMenu();

    if (!createWindow(&wi)) {
        return -1;
    }

    start_game(wi.hwnd, &messages, hThisInstance);

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}

#define ROM "rom/NEStress.nes"
//#define ROM "rom/Tennis.nes"
//#define ROM "rom/F-1.nes"
//#define ROM "rom/dkk.nes"
//#define ROM "rom/fighter_f8000.nes"
//#define ROM "H:\\VROMS\\FC_ROMS\\霸王的大陆.nes"
//#define ROM "H:\\VROMS\\FC_ROMS\\吞噬天地2.nes"
//#define ROM "rom/test/cpu_timing_test/cpu_timing_test.nes"

void start_game(HWND hwnd, PMSG messages, HINSTANCE hInstance) {

    PlayPad *pad = new WinPad();
    Video *video = new DirectXVideo(hwnd); // WindowsVideo | DirectXVideo
    fc           = new NesSystem(video, pad);

#ifdef ROM
    if (int ret = fc->load_rom(ROM)) {
        MessageBox(hwnd, parseOpenError(ret), "错误", 0);
        return;
    }
    run = true;
#endif

	cpu_6502* cpu = fc->getCpu();
    clock_t usetime = clock();

    bgpanel = bg_panel(hInstance, fc->getPPU());
    tlpanel = tile_panel(hInstance, fc->getPPU());

    if (!(bgpanel && tlpanel)) {
        MessageBox(hwnd, "创建调试面板错误", "错误", 0);
        return;
    }

    /* Run the message loop. It will run until GetMessage() returns 0 */
    for(;;)
    {
    	if (PeekMessage(messages, NULL, 0, 0, PM_REMOVE)) {
    		if (messages->message==WM_QUIT) {
				break;
    		}
			TranslateMessage(messages);
			DispatchMessage(messages);
    	}

    	if (!run) continue;

        /* 限速,但是并不准确 */
    	if (clock()-usetime<15) continue;
    	usetime = clock();

        if (sDebug) {
            debugCpu(fc);
            sDebug = 0;
        }
        fc->drawFrame();
        displayCpu(cpu, hwnd);

        if (active) video->refresh();
    }

    delete fc;
    delete video;
}

void openFile(HWND hwnd) {

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

void displayCpu(cpu_6502* cpu, HWND hwnd) {
    static long frameC = 0, f2c = 0;
    static clock_t time = clock();

    ++frameC;
    ++f2c;
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
    TEXT_OUT("rate : %02lf/s", f2c/utime, 11);

#undef TEXT_OUT

    if (f2c>50) {
        time = clock();
        f2c = 0;
    }
}

static const int MENU_FILE   = 1;
static const int MENU_SHOW   = 2;
static const int MENU_DEBUG  = 3;

/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)              /* handle the messages */
    {
    case WM_ACTIVATE:
        switch (LOWORD(wParam))
        {
        case WA_ACTIVE:
        case WA_CLICKACTIVE:
            // 活动, 可以继续向主页面绘画了.
            active = true;
            break;
        case WA_INACTIVE:
            // 不活动, 停止向主页面绘画.
            //active = false;
            break;
        }

        break;

    case WM_DESTROY:
        active = false;
        PostQuitMessage (0);      /* send a WM_QUIT to the message queue */
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case MENU_FILE:
            openFile(hwnd);
            break;

        case MENU_DEBUG:
            sDebug = 1;
            break;

        case MENU_SHOW:
            bgpanel->show();
            tlpanel->show();
            break;
        }
        break;

    default:                      /* for messages that we don't deal with */
        return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}

HMENU createMainMenu() {
    HMENU hmenu = CreateMenu();
	InsertMenu(hmenu, 0, MF_BYPOSITION | MF_STRING, MENU_FILE,  "文件");
	InsertMenu(hmenu, 1, MF_BYPOSITION | MF_STRING, MENU_SHOW,  "显示后台");
	InsertMenu(hmenu, 2, MF_BYPOSITION | MF_STRING, MENU_DEBUG, "中断调试");
	return hmenu;
}
