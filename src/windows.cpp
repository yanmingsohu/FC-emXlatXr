#include <windows.h>
#include <stdio.h>
#include "nes_sys.h"
#include "ppu.h"


/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
void displayCpu(cpu_6502* cpu, HWND hwnd, HDC hdc);
void start_game(HWND hwnd, PMSG messages);


/*  Make the class name into a global variable  */
static char szClassName[ ] = "CodeBlocksWindowsApp";
static char titleName[ ] = "FC Ä£ÄâÆ÷ DEmo. -=CatfoOD=-";


int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
    welcome();

    const int width = PPU_DISPLAY_P_WIDTH;
    const int height = PPU_DISPLAY_P_HEIGHT;

    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
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
    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           titleName,           /* Title Text */
           WS_OVERLAPPEDWINDOW, /* default window */
           CW_USEDEFAULT,       /* Windows decides the position */
           CW_USEDEFAULT,       /* where the window ends up on the screen */
           width + 20,          /* The programs width */
           height + 40,         /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           hThisInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );

    /* Make the window visible on the screen */
    ShowWindow(hwnd, nCmdShow);
    start_game(hwnd, &messages);

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}

void start_game(HWND hwnd, PMSG messages) {
    const int width = PPU_DISPLAY_P_WIDTH;
    const int height = PPU_DISPLAY_P_HEIGHT;

    HDC hdc = GetDC(hwnd);
	HDC hMemDC = CreateCompatibleDC(hdc);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, width, height);
    SelectObject(hMemDC, hBitmap);

    HFONT font = CreateFont(10, 0, 0, 0, 100
				, FALSE, FALSE, FALSE, GB2312_CHARSET
				, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS
				, PROOF_QUALITY, DEFAULT_PITCH, "ËÎÌå" );

    SelectObject(hMemDC, font);
    SetTextColor(hMemDC, RGB(255, 255, 255));
    SetBkColor(hMemDC, RGB(0, 0, 0));

    NesSystem fc;
    if (fc.load_rom("rom/Tennis.nes")) {
        MessageBox(hwnd, "¶ÁÈ¡ROMÊ§°Ü", "´íÎó", 0);
        return;
    }

	cpu_6502* cpu = fc.getCpu();

    /* Run the message loop. It will run until GetMessage() returns 0 */
    for(;;)
    {
    	if (PeekMessage(messages, NULL, 0, 0, PM_REMOVE)) {
    		if (messages->message==WM_QUIT) {
				break;
    		}
			/* Translate virtual-key messages into character messages */
			TranslateMessage(messages);
			/* Send message to WindowProcedure */
			DispatchMessage(messages);
    	}
        cpu->process();
        displayCpu(cpu, hwnd, hMemDC);
        BitBlt(hdc, 0, 0, width, height, hMemDC, 0, 0, SRCCOPY);
    }

    DeleteObject(hBitmap);
    DeleteDC(hMemDC);
    ReleaseDC(hwnd, hdc);
}

void displayCpu(cpu_6502* cpu, HWND hwnd, HDC hdc) {
    static long opCount = 0;
    static int opy = 50;

    int x = 0;
	int y = 0;
	int xIn = 50;
	char buf[128];
    opCount++;

    sprintf(buf, "A: %02X", cpu->A);
    TextOut(hdc, x, y, buf, 5);
    sprintf(buf, "X: %02X", cpu->X);
    TextOut(hdc, x+=xIn, y, buf, 5);
    sprintf(buf, "Y: %02X", cpu->Y);
    TextOut(hdc, x+=xIn, y, buf, 5);
    y = 10;
    x = 0;
    sprintf(buf, "SP: %02X", cpu->SP);
    TextOut(hdc, x, y, buf, 6);
    sprintf(buf, "PC: %04X", cpu->PC);
    TextOut(hdc, x+=xIn, y, buf, 8);
    sprintf(buf, "FG: %02X", cpu->FLAGS);
    TextOut(hdc, x+=xIn, y, buf, 6);

    sprintf(buf, "op count: %09ld", opCount);
    TextOut(hdc, x+=xIn, y, buf, 19);

    TextOut(hdc, 0, opy, cpu->cmdInfo(), 25);
    if ((opy+=10) > PPU_DISPLAY_P_HEIGHT) {
        opy = 30;
    }

    SetPixel(hdc, 200, opy, 0xFFFFFF);
}


/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)                  /* handle the messages */
    {
        case WM_DESTROY:
            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
            break;
        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}
