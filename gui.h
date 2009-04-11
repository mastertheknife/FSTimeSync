#ifndef _FSTS_GUI_H_INC_
#define _FSTS_GUI_H_INC_
#include "globalinc.h"
#include "main.h"
#include "debug.h"

static BOOL CALLBACK OptionsDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK MainDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK TraynHotkeysProc(HWND hwnd,UINT Message, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);

/* Globals */
int GUIStartup(void);
int GUIShutdown(void);
int GUIStartThread(int nCmdShow);
int GUIStopThread();
extern HWND hDummyWindow;

/* Internal functions */
static void GUIUpdate();
static void GUIElementsUpdate();
static void GUIOptionsDraw(HWND hwnd,SyncOptions_t* Sets);
static void GUIOptionsSave(HWND hwnd,SyncOptions_t* Sets);
static void GUISetDialogIcon(HWND hwnd, DWORD Color);
static void GUITrayUpdate();
static void GUIOpenMain(); 
static void GUISyncNowEvent();

/* Thread funciton, used by GUIStartThread() */
static DWORD WINAPI GUIThreadProc(LPVOID lpParameter);

#endif
