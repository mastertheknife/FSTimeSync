#include "globalinc.h"
#include "debug.h"
#include "registry.h"
#include "main.h"

/* Thread safe but one warning */
/* If src or dest is Settings, then it should be locked first */	
void CopySettings(SyncOptions_t* dest, const SyncOptions_t* src) {
	memcpy(dest,src,sizeof(SyncOptions_t));
}

/* Thread safe - doesn't care about the lock */	
int RegistryStartup() {
	return 1;
}

/* Thread safe - doesn't care about the lock */	
int RegistryShutdown() {
	return 1;
}

/* Thread safe - doesn't care about the lock */	
int RegistryReadSettings(SyncOptions_t* ReadSettings) {
	HKEY hRegKey = NULL;
	LONG nRegResult;
	DWORD dwSize = sizeof(DWORD);
	DWORD dwTemp; /* Used for convert the DWORDs to WORDs of the hotkeys */
	
	if(RegOpenKeyEx(HKEY_CURRENT_USER,"Software\\mastertheknife\\FS Time Sync",0,KEY_READ,&hRegKey) != ERROR_SUCCESS) {
		debuglog(DEBUG_WARNING,"Failed opening registry key for reading.\n");
		ReadSettings->StartMinimized = Defaults.StartMinimized;
		ReadSettings->SystemUTCCorrectionState = Defaults.SystemUTCCorrectionState;
		ReadSettings->SystemUTCCorrection = Defaults.SystemUTCCorrection;
		ReadSettings->NoSyncPaused = Defaults.NoSyncPaused;
		ReadSettings->NoSyncSimRate = Defaults.NoSyncSimRate;
		ReadSettings->AutoOnStart = Defaults.AutoOnStart;	
		ReadSettings->AutoSyncInterval = Defaults.AutoSyncInterval;
		ReadSettings->DisableAffinityFix = Defaults.DisableAffinityFix;
		ReadSettings->DisablePriorityFix = Defaults.DisablePriorityFix;
		ReadSettings->ManSyncHotkey = Defaults.ManSyncHotkey;
		ReadSettings->ModeSwitchHotkey = Defaults.ModeSwitchHotkey;
		return 0;
	}				
	
	if((nRegResult = RegQueryValueEx(hRegKey,"StartMinimized",0,NULL,(LPVOID)&(ReadSettings->StartMinimized),&dwSize)) != ERROR_SUCCESS) {
		if(nRegResult != ERROR_FILE_NOT_FOUND)
			debuglog(DEBUG_WARNING,"Failed reading StartMinimized registry value.\n");
			
		ReadSettings->StartMinimized = Defaults.StartMinimized;
	}
	
	if((nRegResult = RegQueryValueEx(hRegKey,"SystemUTCCorrectionState",0,NULL,(LPVOID)&(ReadSettings->SystemUTCCorrectionState),&dwSize)) != ERROR_SUCCESS) {
		if(nRegResult != ERROR_FILE_NOT_FOUND)
			debuglog(DEBUG_WARNING,"Failed reading SystemUTCCorrectionState registry value.\n");
			
		ReadSettings->SystemUTCCorrectionState = Defaults.SystemUTCCorrectionState;
	}
	
	if((nRegResult = RegQueryValueEx(hRegKey,"SystemUTCCorrection",0,NULL,(LPVOID)&(ReadSettings->SystemUTCCorrection),&dwSize)) != ERROR_SUCCESS) {
		if(nRegResult != ERROR_FILE_NOT_FOUND)
			debuglog(DEBUG_WARNING,"Failed reading SystemUTCCorrection registry value.\n");
			
		ReadSettings->SystemUTCCorrection = Defaults.SystemUTCCorrection;
	}
	
	if((nRegResult = RegQueryValueEx(hRegKey,"NoSyncPaused",0,NULL,(LPVOID)&(ReadSettings->NoSyncPaused),&dwSize)) != ERROR_SUCCESS) {
		if(nRegResult != ERROR_FILE_NOT_FOUND)
			debuglog(DEBUG_WARNING,"Failed reading NoSyncPaused registry value.\n");

		ReadSettings->NoSyncPaused = Defaults.NoSyncPaused;
	}

	if((nRegResult = RegQueryValueEx(hRegKey,"NoSyncSimRate",0,NULL,(LPVOID)&(ReadSettings->NoSyncSimRate),&dwSize)) != ERROR_SUCCESS) {
		if(nRegResult != ERROR_FILE_NOT_FOUND)
			debuglog(DEBUG_WARNING,"Failed reading NoSyncSimRate registry value.\n");

		ReadSettings->NoSyncSimRate = Defaults.NoSyncSimRate;
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
	
	if((nRegResult = RegQueryValueEx(hRegKey,"DisableAffinityFix",0,NULL,(LPVOID)&(ReadSettings->DisableAffinityFix),&dwSize)) != ERROR_SUCCESS) {
		ReadSettings->DisableAffinityFix = Defaults.DisableAffinityFix;
	}
	
	if((nRegResult = RegQueryValueEx(hRegKey,"DisablePriorityFix",0,NULL,(LPVOID)&(ReadSettings->DisablePriorityFix),&dwSize)) != ERROR_SUCCESS) {
		ReadSettings->DisablePriorityFix = Defaults.DisablePriorityFix;
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

/* Thread safe - doesn't care about the lock */	
int RegistryWriteSettings(SyncOptions_t* WriteSettings) {
	HKEY hRegKey = NULL;	
	DWORD dwTemp; /* Used for convert the WORDs to DWORDs of the hotkeys */
		
	if(RegCreateKeyEx(HKEY_CURRENT_USER,"Software\\mastertheknife\\FS Time Sync",0,NULL,0,KEY_WRITE,NULL,&hRegKey,NULL) != ERROR_SUCCESS) {
		debuglog(DEBUG_WARNING,"Failed opening or creating registry key for writing\n");
		return 0;
	}
	
	if(RegSetValueEx(hRegKey,"StartMinimized",0,REG_DWORD,(LPVOID)&(WriteSettings->StartMinimized),sizeof(DWORD)) != ERROR_SUCCESS) {
		debuglog(DEBUG_WARNING,"Failed writing StartMinimized registry value.\n");		
	}
	
	if(RegSetValueEx(hRegKey,"SystemUTCCorrectionState",0,REG_DWORD,(LPVOID)&(WriteSettings->SystemUTCCorrectionState),sizeof(DWORD)) != ERROR_SUCCESS) {
		debuglog(DEBUG_WARNING,"Failed writing SystemUTCCorrectionState registry value.\n");		
	}
	
	if(RegSetValueEx(hRegKey,"SystemUTCCorrection",0,REG_DWORD,(LPVOID)&(WriteSettings->SystemUTCCorrection),sizeof(DWORD)) != ERROR_SUCCESS) {
		debuglog(DEBUG_WARNING,"Failed writingSystemUTCCorrection registry value.\n");		
	}

	if(RegSetValueEx(hRegKey,"NoSyncPaused",0,REG_DWORD,(LPVOID)&(WriteSettings->NoSyncPaused),sizeof(DWORD)) != ERROR_SUCCESS) {
		debuglog(DEBUG_WARNING,"Failed writing NoSyncPaused registry value.\n");		
	}

	if(RegSetValueEx(hRegKey,"NoSyncSimRate",0,REG_DWORD,(LPVOID)&(WriteSettings->NoSyncSimRate),sizeof(DWORD)) != ERROR_SUCCESS) {
		debuglog(DEBUG_WARNING,"Failed writing NoSyncSimRate registry value.\n");		
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


