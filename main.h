/********************************************************************************
*	This file is part of FSTimeSync.					*
*										*
*	FSTimeSync is free software: you can redistribute it and/or modify	*
*	it under the terms of the GNU General Public License as published by	*
*	the Free Software Foundation, either version 3 of the License, or	*
*	(at your option) any later version.					*
*										*
*	FSTimeSync is distributed in the hope that it will be useful,		*
*	but WITHOUT ANY WARRANTY; without even the implied warranty of		*
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the		*
*	GNU General Public License for more details.				*
*										*
*	You should have received a copy of the GNU General Public License	*
*	along with FSTimeSync.  If not, see <http://www.gnu.org/licenses/>.	*
********************************************************************************/

#ifndef _FSTS_MAIN_H_INC_
#define _FSTS_MAIN_H_INC_
#include "globalinc.h"
#include "sync.h"
#include "debug.h"

#define FST_QUIT 901
#define FST_AUTOMODE 902
#define FST_SYNCNOW 903

typedef struct tagSyncOptions {
	DWORD StartMinimized;	
	DWORD SystemUTCCorrectionState;
	DWORD FSSyncMethod;
	DWORD FSMenuDetection;
	DWORD AutoOnStart;	
	DWORD AutoSyncInterval;
	DWORD EnableAffinityFix;
	DWORD EnablePriorityFix;
	DWORD NoSyncPaused;
	DWORD NoSyncSimRate;
	LONG SystemUTCCorrection;	
	WORD ManSyncHotkey;
	WORD ModeSwitchHotkey;
} SyncOptions_t;

/* Shared stats */
typedef struct tagSyncStats {
	DWORD SyncInterval; /* The interval of the current sync operation, mostly needed for the progress bar */
	time_t SyncNext; /* Next sync */
	time_t SyncLast; /* Last time synced */
	DWORD SyncLastModified; /* Tells GUI that last sync time was modified, should redraw the text */

	time_t SimUTCTime;
	time_t SysUTCTime;
	
	DWORD SimStatus; /* Is sim connected? */
	FSState_t SimState;
} SyncStats_t;

typedef struct tagRV {
	DWORD bQuit;
	DWORD bSyncNow;
	DWORD bAutoMode;
} RuntimeVals_t;

typedef struct tagVersion {
	char* CompileTime;
	char* CompileDate;
	char* VersionString;
} Version_t;

extern SyncOptions_t Settings; /* The options! */
extern SyncOptions_t Defaults; /* Default options */
extern SyncStats_t Stats;
extern CRITICAL_SECTION ProgramDataCS; /* Critical section to protect the settings and stats structures */
extern Version_t Ver;

void SetRTVal(int RTVal, int NewValue);
DWORD GetRTVal(int RTVal);

int HotkeysRegister(HWND hwnd, WORD ManSync, WORD OperModeSwitch);
int HotkeysUnregister(HWND hwnd);

static int AffinityPriorityFix(DWORD DisableAffinityFix, DWORD DisablePriorityFix);

#endif
