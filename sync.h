#ifndef _FSTS_SYNC_H_INC_
#define _FSTS_SYNC_H_INC_
#include "globalinc.h"
#include "debug.h"
#include "main.h"

typedef struct tagFSTime_t {
   BYTE LocalHour; // 0x238
   BYTE LocalMinute; // 0x239
   BYTE Second; // 0x23A
   BYTE UTCHour; // 0x23B
   BYTE UTCMinute; // 0x23C
   BYTE Alignment[3]; // Used to align this structure to 8 bytes.
} FSTime_t;

typedef struct tagFSDate_t {  
   WORD Day; // 0x23E
   WORD Year; // 0x240
} FSDate_t;

int SyncConnect();
int SyncDisconnect();
int SyncConStatus();

#endif
