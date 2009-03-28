#ifndef _FSTS_MAIN_H_INC_
#define _FSTS_MAIN_H_INC_
#include "globalinc.h"
#include "debug.h"

typedef struct tagSyncOptions {
	unsigned int StartMinimized;	
	unsigned int UTCOffsetState;
	unsigned int AutoOnStartup;	
	unsigned int AutoSyncInterval;
	int UTCOffset;
	WORD ManSyncHotkey;
	WORD ModeSwitchHotkey;
} SyncOptions;

extern SyncOptions Settings; /* The options! */
extern SyncOptions Defaults; /* Default options */
extern CRITICAL_SECTION SettingsCS; /* Critical section to protect the Settings structure */

int SetOperMode(unsigned int bAuto);
int GetOperMode();

int HotkeysRegister(HWND hwnd, WORD ManSync, WORD OperModeSwitch);
int HotkeysUnregister(HWND hwnd);

static unsigned int bAutoSync; /* Runtime setting controlling manual or auto mode */
extern volatile unsigned int bQuit;
#endif
