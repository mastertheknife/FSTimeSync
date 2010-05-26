/****************************************************************************
*	This file is part of FSTimeSync.										*
*																			*
*	FSTimeSync is free software: you can redistribute it and/or modify		*
*	it under the terms of the GNU General Public License as published by	*
*	the Free Software Foundation, either version 3 of the License, or		*
*	(at your option) any later version.										*
*																			*
*	FSTimeSync is distributed in the hope that it will be useful,			*
*	but WITHOUT ANY WARRANTY; without even the implied warranty of			*
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the			*
*	GNU General Public License for more details.							*
*																			*
*	You should have received a copy of the GNU General Public License		*
*	along with FSTimeSync.  If not, see <http://www.gnu.org/licenses/>.		*
****************************************************************************/

#ifndef _FSTS_GUI_H_INC_
#define _FSTS_GUI_H_INC_
#include "globalinc.h"
#include "main.h"
#include "debug.h"

#define FSTSGUI_WM_MAINMSG (WM_USER+1)
#define FSTSGUI_WM_TRAYMSG (WM_USER+2)
#define FSTSGUI_GUIFULLUPDATE 10
#define FSTSGUI_GUIELEMUPDATE 11

static BOOL CALLBACK OptionsDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK MainDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK TraynHotkeysProc(HWND hwnd,UINT Message, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);

/* Globals */
int GUIStartup(void);
int GUIShutdown(void);
int GUIStartThread(int nCmdShow);
int GUIStopThread();

/* Internal functions called by internal functions */
static void GUIUpdate(SyncOptions_t* SafeSets, SyncStats_t* SafeStats);
static void GUIElementsUpdate(SyncOptions_t* SafeSets, SyncStats_t* SafeStats);
static void GUITrayUpdate(SyncOptions_t* SafeSets, SyncStats_t* SafeStats);
static void GUIOptionsDraw(HWND hwnd,SyncOptions_t* Sets);
static void GUIOptionsSave(HWND hwnd,SyncOptions_t* Sets);
static void GUISetDialogIcon(HWND hwnd, DWORD Color);
static void GUIOpenMain(); 
static void GUISyncNowEvent();

/* Thread funciton, used by GUIStartThread() */
static DWORD WINAPI GUIThreadProc(LPVOID lpParameter);

extern HWND hDummyWindow;
#endif
