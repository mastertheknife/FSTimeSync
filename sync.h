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

#ifndef _FSTS_SYNC_H_INC_
#define _FSTS_SYNC_H_INC_
#include "globalinc.h"
#include "debug.h"
#include "main.h"

/* Internal time and date structures */
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

int SyncConnect(DWORD Sim);
int SyncDisconnect();
int SyncGetConStatus();
int SyncGo(time_t UTCtime, DWORD FSXNoSyncLocalTime);
int SyncGetFSTimestamp(time_t* UTCtime);
int SyncGetFSTimeDate(FSTime_t* TimeDest, FSDate_t* DateDest);
int SyncGetPause(DWORD* bPaused);
int SyncGetSimRate(DWORD* SimRate);
int SyncStartup();
int SyncShutdown();
int SyncGetSimVersion();
static int SyncGetSimUTCLocalDifference(int* ZoneDifference);

#endif
