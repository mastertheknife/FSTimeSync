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

static void GUIOperModeUpdate();
static void GUIElementsUpdate();
static void GUIOptionsDraw(HWND hwnd,SyncOptions_t* Sets);
static void GUIOptionsSave(HWND hwnd,SyncOptions_t* Sets);
static void GUISetDialogIcon(HWND hwnd);
static void GUITrayUpdate();
static void GUIOpenMain(); 
static DWORD WINAPI GUIThreadProc(LPVOID lpParameter);

#endif
