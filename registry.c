#include "globalinc.h"
#include "debug.h"
#include "registry.h"
#include "main.h"

void CopySettings(SyncOptions* dest, const SyncOptions* src) {
	memcpy(dest,src,sizeof(SyncOptions));
}

int RegistryStartup() {
	return 1;
}
	
int RegistryShutdown() {
	return 1;
}
	
int RegistryReadSettings(SyncOptions* ReadSettings) {
	HKEY hRegKey = NULL;
	LONG nRegResult;
	DWORD dwSize;
	
	if(RegOpenKeyEx(HKEY_CURRENT_USER,"Software\\mastertheknife\\FS Time Sync",0,KEY_READ,&hRegKey) != ERROR_SUCCESS) {
		debuglog(DEBUG_ERROR,"Failed opening registry key for reading.\n");
		ReadSettings->StartMinimized = Defaults.StartMinimized;
		ReadSettings->SystemUTCOffsetState = Defaults.SystemUTCOffsetState;
		ReadSettings->SystemUTCOffset = Defaults.SystemUTCOffset;
		ReadSettings->AutoOnStart = Defaults.AutoOnStart;	
		ReadSettings->AutoSyncInterval = Defaults.AutoSyncInterval;
		return 0;
	}				
	
	if((nRegResult = RegQueryValueEx(hRegKey,"StartMinimized",0,NULL,(LPVOID)&(ReadSettings->StartMinimized),&dwSize)) != ERROR_SUCCESS) {
		if(nRegResult != ERROR_FILE_NOT_FOUND)
			debuglog(DEBUG_ERROR,"Failed reading StartMinimized registry value.\n");
			
		ReadSettings->StartMinimized = Defaults.StartMinimized;
	}
	
	if((nRegResult = RegQueryValueEx(hRegKey,"SystemUTCOffsetState",0,NULL,(LPVOID)&(ReadSettings->SystemUTCOffsetState),&dwSize)) != ERROR_SUCCESS) {
		if(nRegResult != ERROR_FILE_NOT_FOUND)
			debuglog(DEBUG_ERROR,"Failed reading SystemUTCOffsetState registry value.\n");
			
		ReadSettings->SystemUTCOffsetState = Defaults.SystemUTCOffsetState;
	}
	
	if((nRegResult = RegQueryValueEx(hRegKey,"SystemUTCOffset",0,NULL,(LPVOID)&(ReadSettings->SystemUTCOffset),&dwSize)) != ERROR_SUCCESS) {
		if(nRegResult != ERROR_FILE_NOT_FOUND)
			debuglog(DEBUG_ERROR,"Failed reading SystemUTCOffset registry value.\n");
			
		ReadSettings->SystemUTCOffset = Defaults.SystemUTCOffset;
	}
	
	if((nRegResult = RegQueryValueEx(hRegKey,"AutoOnStart",0,NULL,(LPVOID)&(ReadSettings->AutoOnStart),&dwSize)) != ERROR_SUCCESS) {
		if(nRegResult != ERROR_FILE_NOT_FOUND)
			debuglog(DEBUG_ERROR,"Failed reading AutoOnStart registry value.\n");
			
		ReadSettings->AutoOnStart = Defaults.AutoOnStart;
	}
	
	if((nRegResult = RegQueryValueEx(hRegKey,"AutoSyncInterval",0,NULL,(LPVOID)&(ReadSettings->AutoSyncInterval),&dwSize)) != ERROR_SUCCESS) {
		if(nRegResult != ERROR_FILE_NOT_FOUND)
			debuglog(DEBUG_ERROR,"Failed reading AutoSyncInterval registry value.\n");
			
		ReadSettings->AutoSyncInterval = Defaults.AutoSyncInterval;
	}
	
	if(RegCloseKey(hRegKey) != ERROR_SUCCESS)
		debuglog(DEBUG_WARNING,"Failed to close the registry key.\n");
		
	return 1;	
}


int RegistryWriteSettings(SyncOptions* WriteSettings) {
	return 1;
}


