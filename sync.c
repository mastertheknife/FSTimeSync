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

#include "globalinc.h"
#include "debug.h"
#include "sync.h"
#include "FSUIPC_User.h"

static DWORD SyncConStatus;

/*** Global functions ***/
/* Attempts to connect to the simulator */
int SyncConnect(DWORD Sim) {
	DWORD nResult;
	
	if(FSUIPC_Open(Sim,&nResult))
		SyncConStatus = 1;
	else
		SyncConStatus = 0;	
	
	return SyncConStatus;
}

/* Closes the connection to the simulator */
int SyncDisconnect(void) {
	
	FSUIPC_Close();
	SyncConStatus = 0;

	return 1;
}

/* Gets the connection status (1 connected, 0 disconnected) */
int SyncGetConStatus(void) {
	return SyncConStatus;	
}

/* Syncs the simulator UTC time from the specified UTC time,
  takes care of updating local time as well by getting the timezone difference */
int SyncGo(time_t UTCtime, DWORD FSXNoSyncLocalTime) {
	DWORD nResult;
	int TimeDifference = 0;
	FSTime_t FSTime;
	FSDate_t FSDate;
	struct tm* ptm;
	int WrapHour, WrapMinute;
	
	
	if(!SyncConStatus) {
		debuglog(DEBUG_ERROR,"Called although simulator isn't connected!\n");
		return 0;
	}
	
	ptm = localtime(&UTCtime);
	if(ptm == NULL) {
		debuglog(DEBUG_ERROR,"localtime() failed on given time\n");
		return 0;
	}
	
	/* Assign the values */
	FSTime.UTCHour = ptm->tm_hour;
	FSTime.UTCMinute = ptm->tm_min;
	FSTime.Second = ptm->tm_sec;
	FSDate.Day = ptm->tm_yday + 1;
	FSDate.Year = ptm->tm_year + 1900;		
	
	/* For FS2004 and probably earlier versions, updating the UTC time doesn't update the local time
	So what i did was get UTC time difference from local time of the area we'e flying in and calculate local time from that
	But it seems this is not needed for FS X. */ 
	if(SyncGetSimVersion() != SIM_FSX && !FSXNoSyncLocalTime) {
	debuglog(DEBUG_CAPTURE,"Not skipping local time updating for FS X\n");
		
		/* Get the UTC<->Local time time difference, e.g. if we're flying over israel (GMT+2)
	    Time difference would be 120 minutes (from UTC). */
		if(!SyncGetSimUTCLocalDifference(&TimeDifference)) {
			debuglog(DEBUG_ERROR,"Failed getting timezone difference\n");
			SyncDisconnect();	
			return 0;
		}

		/* Need to warp the hour to keep it within 00 to 23 and minutes within 00 to 59 */
		WrapHour = (int)(floor((double)TimeDifference / 60)) + FSTime.UTCHour;
		if(WrapHour < 0)
			WrapHour = 24 - abs(WrapHour);
		if(WrapHour >= 24)
			WrapHour %= 24;
		WrapMinute = TimeDifference%60 + FSTime.UTCMinute;
		if(WrapMinute < 0)
			WrapMinute = 60 - abs(WrapMinute);
		if(WrapMinute >= 60)
			WrapMinute %= 60;

		/* Set up the local time */
		FSTime.LocalHour = WrapHour;		
		FSTime.LocalMinute = WrapMinute;
	
		if(!FSUIPC_Write(0x238,5,&FSTime,&nResult)) {
			debuglog(DEBUG_ERROR,"Failed writing time data including local time to FS, FSUIPC returned: %u\n",nResult);
			SyncDisconnect(); /* Disconnect for now */
			return 0;
		}
	
	/* Skipping local time, this means we only write 3 bytes instead of 5 bytes. */	
	} else {
		debuglog(DEBUG_CAPTURE,"Skipping local time updating for FS X\n");		
		
		if(!FSUIPC_Write(0x238,3,&FSTime,&nResult)) {
			debuglog(DEBUG_ERROR,"Failed writing time data excluding local time to FS, FSUIPC returned: %u\n",nResult);
			SyncDisconnect(); /* Disconnect for now */
			return 0;
		}
	} /* End of local time check */
	
	if(!FSUIPC_Write(0x23E,4,&FSDate,&nResult)) {
		debuglog(DEBUG_ERROR,"Failed writing date data to FS, FSUIPC returned: %u\n",nResult);
		SyncDisconnect();		
		return 0;
	}
	
	if(!FSUIPC_Process(&nResult)) {
		debuglog(DEBUG_ERROR,"Failed processing write time and date requests, FSUIPC returned: %u\n",nResult);
		SyncDisconnect();		
		return 0;
	}	
		
	return 1;
}

/* Gets the simulator UTC time in structures */
int SyncGetFSTimeDate(FSTime_t* TimeDest, FSDate_t* DateDest) {
	DWORD nResult;	

	if(!SyncConStatus) {
		debuglog(DEBUG_ERROR,"Called although simulator isn't connected!\n");
		return 0;
	}
	
	if(TimeDest == NULL || DateDest == NULL) {
		debuglog(DEBUG_ERROR,"NULL Pointer!\n");
		return 0;
	}

	if(!FSUIPC_Read(0x238,5,TimeDest,&nResult)) {
		debuglog(DEBUG_ERROR,"Failed reading time data from FS, FSUIPC returned: %u\n",nResult);
		SyncDisconnect(); /* Disconnect for now */
		return 0;
	}
	
	if(!FSUIPC_Read(0x23E,4,DateDest,&nResult)) {
		debuglog(DEBUG_ERROR,"Failed reading date data from FS, FSUIPC returned: %u\n",nResult);
		SyncDisconnect();		
		return 0;
	}
	
	if(!FSUIPC_Process(&nResult)) {
		debuglog(DEBUG_ERROR,"Failed processing read time and date requests, FSUIPC returned: %u\n",nResult);
		SyncDisconnect();		
		return 0;
	}
	
	return 1;
}


/* Gets the simulator UTC time as a unix timestamp */
int SyncGetFSTimestamp(time_t* UTCtime) {
	FSTime_t FSTime;
	FSDate_t FSDate;
	struct tm fstm;
	time_t Temptime;	

	if(UTCtime == NULL) {
		debuglog(DEBUG_ERROR,"NULL Pointer!\n");
		return 0;
	}

	if(!SyncConStatus) {
		debuglog(DEBUG_ERROR,"Called although simulator isn't connected!\n");
		return 0;
	}
	
	if(!SyncGetFSTimeDate(&FSTime,&FSDate)) {
		debuglog(DEBUG_ERROR,"SyncGetFSTimeDate returned 0\n");
		return 0;
	}
	
	fstm.tm_hour = FSTime.UTCHour;
	fstm.tm_min = FSTime.UTCMinute;
	fstm.tm_sec = FSTime.Second;
	fstm.tm_wday = 0; /* Ignored */
	fstm.tm_mday = 0;
	fstm.tm_mon = 0; 
	fstm.tm_yday = 0; /* Ignored as well */
	fstm.tm_year = FSDate.Year - 1900;	
	fstm.tm_isdst = 0;
	
	if((Temptime = mktime(&fstm)) == (time_t)-1) {
		debuglog(DEBUG_ERROR,"mktime failed\n");
		return 0;
	}
	
	/* mktime ignores the yday and wday fields, so we add the yday field manually */
	Temptime += (FSDate.Day)*86400;
	
	/* All went well, save the result */
	*UTCtime = Temptime;
	
	return 1;
}

int SyncGetPause(DWORD* bPaused) {
	DWORD nResult;
	short bPausedTemp;
		
	if(!SyncConStatus) {
		debuglog(DEBUG_ERROR,"Called although simulator isn't connected!\n");
		return 0;
	}
	
	if(bPaused == NULL) {
		debuglog(DEBUG_ERROR,"NULL Pointer!\n");
		return 0;
	}
	
	if(!FSUIPC_Read(0x264,2,&bPausedTemp,&nResult)) {
		debuglog(DEBUG_ERROR,"Failed reading pause indicator from FS, FSUIPC returned: %u\n",nResult);
		SyncDisconnect(); /* Disconnect for now */
		return 0;
	}
		
	if(!FSUIPC_Process(&nResult)) {
		debuglog(DEBUG_ERROR,"Failed processing pause indicator read request, FSUIPC returned: %u\n",nResult);
		SyncDisconnect();		
		return 0;
	}
	
	*bPaused = bPausedTemp;
	
	return 1;
}

int SyncGetSimRate(DWORD* SimRate) {
	DWORD nResult;
	short SimRateTemp;
		
	if(!SyncConStatus) {
		debuglog(DEBUG_ERROR,"Called although simulator isn't connected!\n");
		return 0;
	}
	
	if(SimRate == NULL) {
		debuglog(DEBUG_ERROR,"NULL Pointer!\n");
		return 0;
	}
	
	if(!FSUIPC_Read(0xC1A,2,&SimRateTemp,&nResult)) {
		debuglog(DEBUG_ERROR,"Failed reading simulation rate from FS, FSUIPC returned: %u\n",nResult);
		SyncDisconnect(); /* Disconnect for now */
		return 0;
	}
		
	if(!FSUIPC_Process(&nResult)) {
		debuglog(DEBUG_ERROR,"Failed processing simulation rate read request, FSUIPC returned: %u\n",nResult);
		SyncDisconnect();		
		return 0;
	}
	
	*SimRate = SimRateTemp;
	
	return 1;
}

int SyncStartup() {
	SyncConStatus = 0;
	return 1;
}

int SyncShutdown() {
	SyncConStatus = 0;
	return 1;
}

/*** Internal functions ***/
/* Gets the timezone difference of the local time
   against the UTC time in the area we're flying in */
static int SyncGetSimUTCLocalDifference(int* ZoneDifference) {
	DWORD nResult;
	short Tempdiff = 0;
	
	if(!SyncConStatus) {
		debuglog(DEBUG_ERROR,"Called although simulator isn't connected!\n");
		return 0;
	}	
	
	if(ZoneDifference == NULL) {
		debuglog(DEBUG_ERROR,"NULL Pointer!\n");
		return 0;
	}
	
	if(!FSUIPC_Read(0x246,2,&Tempdiff,&nResult)) {
		debuglog(DEBUG_ERROR,"Failed reading flying zone UTC<->Local time difference, FUSIPC returned: %u\n",nResult);
		SyncDisconnect();
		return 0;
	}
	
	if(!FSUIPC_Process(&nResult)) {
		debuglog(DEBUG_ERROR,"Failed processing request for flying zone UTC<->Local time difference, FSUIPC returned: %u\n",nResult);
		SyncDisconnect();
		return 0;
	}
	
	*ZoneDifference = Tempdiff*(-1);
	
	return 1;
}

int SyncGetSimVersion() {
	if(!SyncConStatus) {
		debuglog(DEBUG_ERROR,"Called although simulator isn't connected!\n");
		return 0;
	}
	
	return FSUIPC_FS_Version;
}
