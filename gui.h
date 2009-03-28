#ifndef _FSTS_GUI_H_INC_
#define _FSTS_GUI_H_INC_
#include "globalinc.h"
#include "main.h"
#include "debug.h"

BOOL CALLBACK OptionsDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK MainDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);

int GUIStartup(void);
int GUIShutdown(void);
int GUIStartThread(int nCmdShow);
int GUIStopThread();

static void GUIOperModeDraw(HWND hwnd, unsigned int AutoMode);
static void GUIElementsDraw(HWND hwnd);
static void GUIOptionsDraw(HWND hwnd,SyncOptions* Sets);
static void GUISetDialogIcon(HWND hwnd);
static DWORD WINAPI GUIThreadProc(LPVOID lpParameter);

#endif
