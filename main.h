#ifndef _FSTS_MAIN_H_INC_
#define _FSTS_MAIN_H_INC_
#include "globalinc.h"
#include "debug.h"

#define FST_QUIT 901
#define FST_AUTOMODE 902
#define FST_SYNCNOW 903

typedef struct tagSyncOptions {
	DWORD StartMinimized;	
	DWORD SystemUTCOffsetState;
	LONG SystemUTCOffset;
	DWORD FutureSetting00;
	DWORD AutoOnStart;	
	DWORD AutoSyncInterval;
	WORD ManSyncHotkey;
	WORD ModeSwitchHotkey;
} SyncOptions_t;

typedef struct tagSyncStats {
	DWORD SimStatus;
	DWORD SyncInterval; /* The interval of the current sync operation */
	time_t SyncNext;
	time_t SyncLast;
	DWORD SyncLastModified;
	time_t SimUTCTime;
	time_t SysLocalTime;
} SyncStats_t;

typedef struct tagRV {
	DWORD bQuit;
	DWORD bSyncNow;
	DWORD bAutoMode;
} RuntimeVals_t;

extern SyncOptions_t Settings; /* The options! */
extern SyncOptions_t Defaults; /* Default options */
extern CRITICAL_SECTION ProgramDataCS; /* Critical section to protect the settings and stats structures */
extern SyncStats_t Stats;

void SetRTVal(int RTVal, int NewValue);
DWORD GetRTVal(int RTVal);

int HotkeysRegister(HWND hwnd, WORD ManSync, WORD OperModeSwitch);
int HotkeysUnregister(HWND hwnd);

#endif
