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
	DWORD SimStatus;
	DWORD SyncInterval; /* The interval of the current sync operation */
	time_t SyncNext;
	time_t SyncLast;
	DWORD SyncLastModified;
} SyncStats_t;

extern SyncOptions_t Settings; /* The options! */
extern SyncOptions_t Defaults; /* Default options */
extern CRITICAL_SECTION ProgramDataCS; /* Critical section to protect the settings and stats structures */
extern SyncStats_t Stats;

int SetOperMode(unsigned int bAuto);
int GetOperMode();

int HotkeysRegister(HWND hwnd, WORD ManSync, WORD OperModeSwitch);
int HotkeysUnregister(HWND hwnd);

static unsigned int bAutoSync; /* Runtime setting controlling manual or auto mode */
extern volatile unsigned int bQuit;
#endif
