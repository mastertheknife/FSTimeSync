#ifndef _FSTS_MAIN_H_INC_
#define _FSTS_MAIN_H_INC_
#include "globalinc.h"
#include "debug.h"

typedef struct tagSyncOptions {
	DWORD StartMinimized;	
	DWORD SystemUTCOffsetState;
	LONG SystemUTCOffset;
	DWORD DaylightSaving;
	DWORD AutoOnStart;	
	DWORD AutoSyncInterval;
	WORD ManSyncHotkey;
	WORD ModeSwitchHotkey;
} SyncOptions_t;

typedef struct tagSyncStats {
	DWORD SyncNext;
	DWORD SyncInterval;
	DWORD SimStatus;
	time_t SysUTCTime;
	time_t SimUTCTime;
	time_t LastSync;
	DWORD LastSyncChanged;
	DWORD NextSyncChanged;
} SyncStats_t;

extern SyncOptions_t Settings; /* The options! */
extern SyncOptions_t Defaults; /* Default options */
extern CRITICAL_SECTION SettingsCS; /* Critical section to protect the Settings structure */
extern CRITICAL_SECTION StatsCS; /* This one to protect the stats */
extern SyncStats_t Stats;

int SetOperMode(unsigned int bAuto);
int GetOperMode();

int HotkeysRegister(HWND hwnd, WORD ManSync, WORD OperModeSwitch);
int HotkeysUnregister(HWND hwnd);

static unsigned int bAutoSync; /* Runtime setting controlling manual or auto mode */
extern volatile unsigned int bQuit;
#endif
