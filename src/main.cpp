#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include "type.h"

#ifdef __WIN32__
#include <windows.h>
#include "winsys.h"
#endif

#ifdef __linux__
#endif

int main()
{
    welcome();

#ifdef __WIN32__
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WinMain(hInstance, NULL, NULL, SW_SHOW);
#endif

    system("pause");
    return 0;
}

