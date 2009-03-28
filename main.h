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
extern unsigned int AutoSync; /* Runtime setting controlling manual or auto mode */
extern CRITICAL_SECTION SettingsCS; /* Critical section to protect the options structure */

#endif
