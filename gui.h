#ifndef _FSTS_GUI_H_INC_
#define _FSTS_GUI_H_INC_
#include "globalinc.h"
#include "main.h"
#include "debug.h"

BOOL CALLBACK OptionsDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK MainDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);

static DWORD WINAPI GUIThreadProc(LPVOID lpParameter);
int GUIStartup(void);
int GUIShutdown(void);
int GUIStartThread(BOOL bStartMinimized);


#endif
