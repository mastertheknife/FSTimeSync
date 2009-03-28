#ifndef _FSTS_MAIN_H_INC_
#define _FSTS_MAIN_H_INC_
#include "globalinc.h"
#include "debug.h"

typedef struct tagSyncOptions {
	unsigned int UTCOffsetState;
	unsigned int AutoSyncInterval;
	unsigned int AutoOnStartup;
	unsigned int StartMinimized;
	int UTCOffset;
	ACCEL Hotkeys[2];
	char Padding[2]; /* Pad this structure to 32 bytes */
} SyncOptions;

extern SyncOptions Settings; /* The options! */
extern SyncOptions Defaults; /* Default options */
extern CRITICAL_SECTION SettingsCS; /* Critical section to protect the Settings structure */

int SetOperMode(unsigned int bAuto);
int GetOperMode();

int RegisterHotkeys(HWND hwnd, WORD ManSync, WORD OperModeSwitch);

static unsigned int bAutoSync; /* Runtime setting controlling manual or auto mode */
extern volatile unsigned int bQuit;
#endif
