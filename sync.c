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
int SyncGo(time_t* UTCtime) {
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
	
	if(UTCtime == NULL) {
		debuglog(DEBUG_ERROR,"NULL Pointer!\n");
		return 0;
	}
	
	/* Get the UTC<->Local time time difference, e.g. if we're flying over israel (GMT+2)
	   Time difference would be 120 minutes (from UTC). */
	if(!SyncGetSimUTCLocalDifference(&TimeDifference)) {
		debuglog(DEBUG_ERROR,"Failed getting timezone difference\n");
		SyncDisconnect();	
		return 0;
	}
	
	ptm = localtime(UTCtime);
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
		debuglog(DEBUG_ERROR,"Failed writing time data to FS, FSUIPC returned: %u\n",nResult);
		SyncDisconnect(); /* Disconnect for now */
		return 0;
	}
	
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

/* Gets the simulator UTC time */
int SyncGetTime(time_t* UTCtime) {
	DWORD nResult;
	FSTime_t FSTime;
	FSDate_t FSDate;
	struct tm fstm;
	time_t Temptime;	

	if(!SyncConStatus) {
		debuglog(DEBUG_ERROR,"Called although simulator isn't connected!\n");
		return 0;
	}
	
	if(UTCtime == NULL) {
		debuglog(DEBUG_ERROR,"NULL Pointer!\n");
		return 0;
	}

	if(!FSUIPC_Read(0x238,5,&FSTime,&nResult)) {
		debuglog(DEBUG_ERROR,"Failed reading time data from FS, FSUIPC returned: %u\n",nResult);
		SyncDisconnect(); /* Disconnect for now */
		return 0;
	}
	
	if(!FSUIPC_Read(0x23E,4,&FSDate,&nResult)) {
		debuglog(DEBUG_ERROR,"Failed reading date data from FS, FSUIPC returned: %u\n",nResult);
		SyncDisconnect();		
		return 0;
	}
	
	if(!FSUIPC_Process(&nResult)) {
		debuglog(DEBUG_ERROR,"Failed processing read time and date requests, FSUIPC returned: %u\n",nResult);
		SyncDisconnect();		
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

