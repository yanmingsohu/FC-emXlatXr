#include <windows.h>
#include <stdio.h>
#include <time.h>
#include "winsys.h"
#include "debug.h"
#include <Winuser.h>


LRESULT     CALLBACK WindowProcedure   (HWND, UINT, WPARAM, LPARAM  );
void        displayCpu                 (cpu_6502*, HWND             );
void        start_game                 (HWND hwnd, PMSG messages    );
int         initWindow                 (HWND*, HINSTANCE, int       );
HMENU       createMenu                 ();

/*  Make the class name into a global variable  */
static char szClassName[ ] = "CodeBlocksWindowsApp";
static char titleName  [ ] = "FC 模拟器 DEmo. -=CatfoOD=-";
static bool active = 1;

int WINAPI WinMain ( HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow )
{
    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */

    if (!initWindow(&hwnd, hThisInstance, nCmdShow)) {
        return -1;
    }

    start_game(hwnd, &messages);

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}

int initWindow(HWND *hwnd, HINSTANCE hThisInstance, int nCmdShow) {
    const int width = 800;
    const int height = 600;
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;

    /* The class is registered, let's create the program*/
    *hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           titleName,           /* Title Text */
           WS_OVERLAPPEDWINDOW, /* default window */
           CW_USEDEFAULT,       /* Windows decides the position */
           CW_USEDEFAULT,       /* where the window ends up on the screen */
           width,               /* The programs width */
           height,              /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           createMenu(),        /* menu */
           hThisInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );

    /* Make the window visible on the screen */
    ShowWindow(*hwnd, nCmdShow);
    return 1;
}

//#define ROM "rom/Tennis.nes"
//#define ROM "rom/Dr_Mario.nes"
//#define ROM "rom/test.nes"
//#define ROM "rom/F-1.nes"
#define ROM "rom/dkk.nes"
void start_game(HWND hwnd, PMSG messages) {

    PlayPad *pad = new WinPad();
    Video *video = new DirectXVideo(hwnd); // WindowsVideo | DirectXVideo
    NesSystem fc(video, pad);
    int ret = 0;

    if (ret = fc.load_rom(ROM)) {
        MessageBox(hwnd, parseOpenError(ret), "错误", 0);
        return;
    }

    WindowsVideo *bgPanel = new WindowsVideo(hwnd, 512, 480);
    bgPanel->setOffset(270, 40);

	cpu_6502* cpu = fc.getCpu();
    int count = 0;
    clock_t usetime = clock();

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

        /* 限速,但是并不准确 */
    	if (clock()-usetime<20) continue;
    	usetime = clock();

        debugCpu(&fc);
        fc.drawFrame();
        //fc.getPPU()->drawTileTable();

        displayCpu(cpu, hwnd);

        if (active) video->refresh();

        if (++count%10==0) {
            fc.getPPU()->drawBackGround(bgPanel);
            bgPanel->refresh();
        }
    }

    delete video;
    delete bgPanel;
}

void displayCpu(cpu_6502* cpu, HWND hwnd) {
    static long frameC = 0, f2c = 0;
    static clock_t time = clock();

    frameC++; f2c++;
    if (frameC%20!=0) return;

    HDC hdc = GetDC(hwnd);
    int x = 300;
	int y = 0;
	int xIn = 70;
	int yIn = 18;
	char buf[128];

    sprintf(buf, "A: %02X", cpu->A);
    TextOut(hdc, x, y, buf, 5);
    sprintf(buf, "X: %02X", cpu->X);
    TextOut(hdc, x+=xIn, y, buf, 5);
    sprintf(buf, "Y: %02X", cpu->Y);
    TextOut(hdc, x+=xIn, y, buf, 5);
    y += yIn;
    x = 300;
    sprintf(buf, "SP: %02X", cpu->SP);
    TextOut(hdc, x, y, buf, 6);
    sprintf(buf, "PC: %04X", cpu->PC);
    TextOut(hdc, x+=xIn, y, buf, 8);
    sprintf(buf, "FG: %02X", cpu->FLAGS);
    TextOut(hdc, x+=xIn, y, buf, 6);

    double utime = (double)(clock() - time)/CLOCKS_PER_SEC;

    sprintf(buf, "frame: %09ld", frameC);
    TextOut(hdc, x+=xIn, y, buf, 16);
    sprintf(buf, "rate : %02lf/s", f2c/utime);
    TextOut(hdc, x, y-yIn, buf, 11);

    if (f2c>50) {
        time = clock();
        f2c = 0;
    }
}

/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)              /* handle the messages */
    {
    case WM_ACTIVATE:
        {
        switch((LOWORD(wParam)))
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
        }
        break;
    case WM_DESTROY:
        PostQuitMessage (0);      /* send a WM_QUIT to the message queue */
        break;
    default:                      /* for messages that we don't deal with */
        return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}

HMENU createMenu() {
    HMENU hmenu = CreateMenu();
	InsertMenu(hmenu, 0, MF_BYPOSITION | MF_STRING, 1, "One");
	InsertMenu(hmenu, 1, MF_BYPOSITION | MF_STRING, 2, "Two");
	return hmenu;
}
