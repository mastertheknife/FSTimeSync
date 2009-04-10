#include "globalinc.h"
#include "debug.h"
#include "sync.h"
#include "FSUIPC_User.h"

static DWORD SyncConStatus;

/*** Global functions ***/
/* Attempts to connect to the simulator */
int SyncConnect(void) {
	DWORD nResult;
	
	if(FSUIPC_Open(SIM_ANY,&nResult))
		SyncConStatus = 1;
	else
		SyncConStatus = 0;	
	
	debuglog(DEBUG_CAPTURE,"SyncConnect return value: %d!\n",SyncConStatus);	
	return SyncConStatus;
}

/* Closes the connection to the simulator */
int SyncDisconnect(void) {
	if(SyncConStatus)
		FSUIPC_Close();
	
	SyncConStatus = 0;
	
	debuglog(DEBUG_CAPTURE,"Simulator disconnected!\n");
	return 1;
}

/* Gets the connection status (1 connected, 0 disconnected) */
int SyncGetConStatus(void) {
	debuglog(DEBUG_CAPTURE,"Status: %d",SyncConStatus);
	return SyncConStatus;	
}

/* Syncs the simulator UTC time from the specified UTC time,
  takes care of updating local time as well by getting the timezone difference */
int SyncGo(time_t* UTCtime) {
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
		return 0;
	}
	
	if(!FSUIPC_Read(0x23E,4,&FSDate,&nResult)) {
		debuglog(DEBUG_ERROR,"Failed reading date data from FS, FSUIPC returned: %u\n",nResult);
		return 0;
	}
	
	if(!FSUIPC_Process(&nResult)) {
		debuglog(DEBUG_ERROR,"Failed processing read time and date requests, FSUIPC returned: %u\n",nResult);
		return 0;
	}
	
	fstm.tm_hour = FSTime.UTCHour;
	fstm.tm_min = FSTime.UTCMinute;
	fstm.tm_sec = FSTime.Second;
	fstm.tm_year = FSDate.Year - 1900;
	fstm.tm_yday = FSDate.Day - 1;
	fstm.tm_isdst = 0;
	
	if((Temptime = mktime(&fstm)) == (time_t)-1) {
		debuglog(DEBUG_ERROR,"mktime failed\n");
		return 0;
	}
	
	/* All went well, save the result */
	*UTCtime = Temptime;
	
	return 1;
}

/*** Internal functions ***/
/* Gets the timezone difference of the local time
   against the UTC time in the area we're flying in */
static int SyncGetSimUTCLocalDifference(int* ZoneDifference) {
	DWORD nResult;
	
	if(!SyncConStatus) {
		debuglog(DEBUG_ERROR,"Called although simulator isn't connected!\n");
		return 0;
	}	
	
	if(ZoneDifference == NULL) {
		debuglog(DEBUG_ERROR,"NULL Pointer!\n");
		return 0;
	}
	
	if(!FSUIPC_Read(0x246,2,ZoneDifference,&nResult)) {
		debuglog(DEBUG_ERROR,"Failed reading flying zone UTC<->Local time difference, FUSIPC returned: %u\n",nResult);
		return 0;
	}
	
	if(!FSUIPC_Process(&nResult)) {
		debuglog(DEBUG_ERROR,"Failed processing request for flying zone UTC<->Local time difference, FSUIPC returned: %u\n",nResult);
		return 0;
	}
	
	return 1;
}

