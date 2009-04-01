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
	DWORD dwSize = sizeof(DWORD);
	DWORD dwTemp; /* Used for convert the DWORDs to WORDs of the hotkeys */
	
	if(RegOpenKeyEx(HKEY_CURRENT_USER,"Software\\mastertheknife\\FS Time Sync",0,KEY_READ,&hRegKey) != ERROR_SUCCESS) {
		debuglog(DEBUG_WARNING,"Failed opening registry key for reading.\n");
		ReadSettings->StartMinimized = Defaults.StartMinimized;
		ReadSettings->SystemUTCOffsetState = Defaults.SystemUTCOffsetState;
		ReadSettings->SystemUTCOffset = Defaults.SystemUTCOffset;
		ReadSettings->DaylightSaving = Defaults.DaylightSaving;
		ReadSettings->AutoOnStart = Defaults.AutoOnStart;	
		ReadSettings->AutoSyncInterval = Defaults.AutoSyncInterval;
		return 0;
	}				
	
	if((nRegResult = RegQueryValueEx(hRegKey,"StartMinimized",0,NULL,(LPVOID)&(ReadSettings->StartMinimized),&dwSize)) != ERROR_SUCCESS) {
		if(nRegResult != ERROR_FILE_NOT_FOUND)
			debuglog(DEBUG_WARNING,"Failed reading StartMinimized registry value.\n");
			
		ReadSettings->StartMinimized = Defaults.StartMinimized;
	}
	
	if((nRegResult = RegQueryValueEx(hRegKey,"SystemUTCOffsetState",0,NULL,(LPVOID)&(ReadSettings->SystemUTCOffsetState),&dwSize)) != ERROR_SUCCESS) {
		if(nRegResult != ERROR_FILE_NOT_FOUND)
			debuglog(DEBUG_WARNING,"Failed reading SystemUTCOffsetState registry value.\n");
			
		ReadSettings->SystemUTCOffsetState = Defaults.SystemUTCOffsetState;
	}
	
	if((nRegResult = RegQueryValueEx(hRegKey,"SystemUTCOffset",0,NULL,(LPVOID)&(ReadSettings->SystemUTCOffset),&dwSize)) != ERROR_SUCCESS) {
		if(nRegResult != ERROR_FILE_NOT_FOUND)
			debuglog(DEBUG_WARNING,"Failed reading SystemUTCOffset registry value.\n");
			
		ReadSettings->SystemUTCOffset = Defaults.SystemUTCOffset;
	}
	
	if((nRegResult = RegQueryValueEx(hRegKey,"DaylightSaving",0,NULL,(LPVOID)&(ReadSettings->DaylightSaving),&dwSize)) != ERROR_SUCCESS) {
		if(nRegResult != ERROR_FILE_NOT_FOUND)
			debuglog(DEBUG_WARNING,"Failed reading DaylightSaving registry value.\n");
			
		ReadSettings->DaylightSaving = Defaults.DaylightSaving;
	}	
	
	if((nRegResult = RegQueryValueEx(hRegKey,"AutoOnStart",0,NULL,(LPVOID)&(ReadSettings->AutoOnStart),&dwSize)) != ERROR_SUCCESS) {
		if(nRegResult != ERROR_FILE_NOT_FOUND)
			debuglog(DEBUG_WARNING,"Failed reading AutoOnStart registry value.\n");
			
		ReadSettings->AutoOnStart = Defaults.AutoOnStart;
	}
	
	if((nRegResult = RegQueryValueEx(hRegKey,"AutoSyncInterval",0,NULL,(LPVOID)&(ReadSettings->AutoSyncInterval),&dwSize)) != ERROR_SUCCESS) {
		if(nRegResult != ERROR_FILE_NOT_FOUND)
			debuglog(DEBUG_WARNING,"Failed reading AutoSyncInterval registry value.\n");
			
		ReadSettings->AutoSyncInterval = Defaults.AutoSyncInterval;
	}
	
	if((nRegResult = RegQueryValueEx(hRegKey,"ManSyncHotkey",0,NULL,(LPVOID)&dwTemp,&dwSize)) != ERROR_SUCCESS) {
		if(nRegResult != ERROR_FILE_NOT_FOUND)
			debuglog(DEBUG_WARNING,"Failed reading ManSyncHotkey registry value.\n");
			
		ReadSettings->ManSyncHotkey = Defaults.ManSyncHotkey;
	} else {
		ReadSettings->ManSyncHotkey = dwTemp;
	}
	
	if((nRegResult = RegQueryValueEx(hRegKey,"ModeSwitchHotkey",0,NULL,(LPVOID)&dwTemp,&dwSize)) != ERROR_SUCCESS) {
		if(nRegResult != ERROR_FILE_NOT_FOUND)
			debuglog(DEBUG_WARNING,"Failed reading ModeSwitchHotkey registry value.\n");
			
		ReadSettings->ModeSwitchHotkey = Defaults.ModeSwitchHotkey;
	} else {
		ReadSettings->ModeSwitchHotkey = dwTemp;
	}
	
	if(RegCloseKey(hRegKey) != ERROR_SUCCESS)
		debuglog(DEBUG_WARNING,"Failed to close the registry key.\n");
		
	return 1;	
}


int RegistryWriteSettings(SyncOptions* WriteSettings) {
	HKEY hRegKey = NULL;	
	DWORD dwTemp; /* Used for convert the WORDs to DWORDs of the hotkeys */
		
	if(RegCreateKeyEx(HKEY_CURRENT_USER,"Software\\mastertheknife\\FS Time Sync",0,NULL,0,KEY_WRITE,NULL,&hRegKey,NULL) != ERROR_SUCCESS) {
		debuglog(DEBUG_WARNING,"Failed opening or creating registry key for writing\n");
		return 0;
	}
	
	if(RegSetValueEx(hRegKey,"StartMinimized",0,REG_DWORD,(LPVOID)&(WriteSettings->StartMinimized),sizeof(DWORD)) != ERROR_SUCCESS) {
		debuglog(DEBUG_WARNING,"Failed writing StartMinimized registry value.\n");		
	}
	
	if(RegSetValueEx(hRegKey,"SystemUTCOffsetState",0,REG_DWORD,(LPVOID)&(WriteSettings->SystemUTCOffsetState),sizeof(DWORD)) != ERROR_SUCCESS) {
		debuglog(DEBUG_WARNING,"Failed writing SystemUTCOffsetState registry value.\n");		
	}
	
	if(RegSetValueEx(hRegKey,"SystemUTCOffset",0,REG_DWORD,(LPVOID)&(WriteSettings->SystemUTCOffset),sizeof(DWORD)) != ERROR_SUCCESS) {
		debuglog(DEBUG_WARNING,"Failed writing SystemUTCOffset registry value.\n");		
	}
	
	if(RegSetValueEx(hRegKey,"DaylightSaving",0,REG_DWORD,(LPVOID)&(WriteSettings->DaylightSaving),sizeof(DWORD)) != ERROR_SUCCESS) {
		debuglog(DEBUG_WARNING,"Failed writing DaylightSaving registry value.\n");		
	}
	
	if(RegSetValueEx(hRegKey,"AutoOnStart",0,REG_DWORD,(LPVOID)&(WriteSettings->AutoOnStart),sizeof(DWORD)) != ERROR_SUCCESS) {
		debuglog(DEBUG_WARNING,"Failed writing AutoOnStart registry value.\n");		
	}
	
	if(RegSetValueEx(hRegKey,"AutoSyncInterval",0,REG_DWORD,(LPVOID)&(WriteSettings->AutoSyncInterval),sizeof(DWORD)) != ERROR_SUCCESS) {
		debuglog(DEBUG_WARNING,"Failed writing AutoSyncInterval registry value.\n");		
	}
	
	dwTemp = WriteSettings->ManSyncHotkey;	
	if(RegSetValueEx(hRegKey,"ManSyncHotkey",0,REG_DWORD,(LPVOID)&dwTemp,sizeof(DWORD)) != ERROR_SUCCESS) {
		debuglog(DEBUG_WARNING,"Failed writing ManSyncHotkey registry value.\n");		
	}
	
	dwTemp = WriteSettings->ModeSwitchHotkey;
	if(RegSetValueEx(hRegKey,"ModeSwitchHotkey",0,REG_DWORD,(LPVOID)&dwTemp,sizeof(DWORD)) != ERROR_SUCCESS) {
		debuglog(DEBUG_WARNING,"Failed writing ModeSwitchHotkey registry value.\n");		
	}
	
	
	if(RegCloseKey(hRegKey) != ERROR_SUCCESS)
		debuglog(DEBUG_WARNING,"Failed to close the registry key.\n");		
	
	return 1;
}


