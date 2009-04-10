#define SLEEP_DURATION 200
#include "globalinc.h"
#include "debug.h"
#include "main.h"
#include "gui.h"
#include "sync.h"
#include "registry.h"
#include <windows.h>
#include <commctrl.h>

/* Globals */
SyncOptions_t Settings = {0,0,0,0,0,0,MAKEWORD(VK_F6,(HOTKEYF_CONTROL | HOTKEYF_SHIFT)),MAKEWORD(VK_F7,(HOTKEYF_CONTROL | HOTKEYF_SHIFT))}; /* The options! */
SyncOptions_t Defaults = {0,0,0,1,1,10}; /* Default options */
SyncStats_t Stats;
CRITICAL_SECTION ProgramDataCS;
static RuntimeVals_t RuntimeVals;
static CRITICAL_SECTION ProgramControlCS;

int WINAPI WinMain (HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow) {
	/* Mutex to prevent multiple instances of this application */
  	if (CreateMutex(NULL,FALSE,"_FSTimeSyncMutex_") && GetLastError() == ERROR_ALREADY_EXISTS) {
		MessageBox(NULL,"An instance of FS Time Sync is already running.","FS Time Sync Error",MB_OK | MB_ICONERROR);
		return 100;
	}

	/* Initialize the critical sections and perform startup */
	InitializeCriticalSection(&ProgramDataCS);
	InitializeCriticalSection(&ProgramControlCS);
	DebugStartup();
	RegistryStartup();
 	GUIStartup();

	/* Load the settings from the registry */
	RegistryReadSettings(&Settings);
	/* Set the operating mode (manual or auto) based on the setting */
 	SetRTVal(FST_AUTOMODE,Settings.AutoOnStart);
	Stats.SyncNext = time(NULL)+Settings.AutoSyncInterval;
	
	/* Start the GUI Thread, based on the Start Minimized setting */
	if(Settings.StartMinimized == 1)
		GUIStartThread(SW_MINIMIZE);
	else	
		GUIStartThread(nCmdShow);	
			 		
	
	/* Main program loop */
	while(!GetRTVal(FST_QUIT)) {
		time_t CurrentTime;
		
		EnterCriticalSection(&ProgramDataCS);
		
		CurrentTime = time(NULL); /* Saves multiple expensive calls to time() */
		Stats.SimStatus = SyncGetConStatus(); /* Get updated state of the connection */
			
		/* System UTC time */
		Stats.SysLocalTime = CurrentTime;
		if(Settings.SystemUTCOffsetState)
			Stats.SysLocalTime += Settings.SystemUTCOffset*60;

		/* Only proceed if sim is connected */
		if(Stats.SimStatus) {
		
			/* Get the simulator's UTC time */
			SyncGetTime(&Stats.SimUTCTime);				
					
			if(GetRTVal(FST_AUTOMODE)) {
				/* Automatic mode */		
				/* Check if switched to faster synch interval and there's too long to wait */
				if((Stats.SyncNext-CurrentTime) > Settings.AutoSyncInterval) {
					Stats.SyncNext = CurrentTime+Settings.AutoSyncInterval;
					Stats.SyncInterval = Settings.AutoSyncInterval; /* Progress bar is drawn based on current synch, not next one */
				}
				/* Check if switched to slower interval */
				if(Stats.SyncInterval < Settings.AutoSyncInterval) {
					Stats.SyncNext = CurrentTime+Settings.AutoSyncInterval;
					Stats.SyncInterval = Settings.AutoSyncInterval; /* Progress bar is drawn based on current synch, not next one */
				}	
				if(Stats.SyncNext <= CurrentTime) {
					/* Sync code here */
					Stats.SyncLast = time(&CurrentTime); /* Sync takes time, update the time on the way */
					Stats.SyncNext = CurrentTime+Settings.AutoSyncInterval;
					Stats.SyncInterval = Settings.AutoSyncInterval;
					Stats.SyncLastModified = 1;
				}
			} else {
				/* Manual mode */
				Stats.SyncInterval = 0;
				if(GetRTVal(FST_SYNCNOW)) {
					/* Sync code goes here */
					SetRTVal(FST_SYNCNOW, 0); /* Clear the event */
					Stats.SyncLast = time(&CurrentTime);
					Stats.SyncLastModified = 1;
				}
			} /* Mode */
		} else {
			/* Connect to the sim */
			Stats.SimStatus = SyncConnect();		
			/* Signal GUI thread to call GUIUpdate - needed for tray icon */				
			PostMessage(hDummyWindow,WM_USER+1,10,0);

		} /* SimStatus */
			
		LeaveCriticalSection(&ProgramDataCS);		
		Sleep(SLEEP_DURATION);
	} /* Loop end */ 

	/* Stop the GUI thread, or at least wait for it to close */
	GUIStopThread();
	
	/* Disconnect from the simulator if it's still connected */
	if(SyncGetConStatus())
		SyncDisconnect();
	
	/* Write settings to the registry */
	RegistryWriteSettings(&Settings);

	/* Delete the critical sections and perform complete shutdown */	
	DeleteCriticalSection(&ProgramDataCS);
	DeleteCriticalSection(&ProgramControlCS);
	GUIShutdown();
    RegistryShutdown();
    DebugShutdown();

    return 0;
}

DWORD GetRTVal(int RTVal) {
	DWORD dwRet;
	EnterCriticalSection(&ProgramControlCS);	
	
	switch(RTVal) {
		case FST_QUIT:
			dwRet = RuntimeVals.bQuit;
			break;
		case FST_SYNCNOW:
			dwRet = RuntimeVals.bSyncNow;
			break;
		case FST_AUTOMODE:
			dwRet = RuntimeVals.bAutoMode;
			break;
		default:
			dwRet = 0;
	}
	
	LeaveCriticalSection(&ProgramControlCS);
	return dwRet;
}	
	
void SetRTVal(int RTVal, int NewValue) {
	EnterCriticalSection(&ProgramControlCS);	
	
	switch(RTVal) {
		case FST_QUIT:
			RuntimeVals.bQuit = NewValue;
			break;
		case FST_SYNCNOW:
			RuntimeVals.bSyncNow = NewValue;
			break;
		case FST_AUTOMODE:
			RuntimeVals.bAutoMode = NewValue;
			break;
	}
	
	LeaveCriticalSection(&ProgramControlCS);
}
				
int HotkeysRegister(HWND hwnd, WORD ManSync, WORD OperModeSwitch) {
	UINT ManSyncModifiers = 0;
	UINT OperModeSwitchModifiers = 0;
	UINT ManSyncVk;
	UINT OperModeSwitchVk;

	/* Perform conversion of the format we got from HKM_GETHOTKEY
	   To the format needed by RegisterHotkey() */
	if (HOTKEYF_ALT & HIBYTE(ManSync))
		ManSyncModifiers |= MOD_ALT;
	if (HOTKEYF_CONTROL & HIBYTE(ManSync))
		ManSyncModifiers |= MOD_CONTROL;
	if (HOTKEYF_SHIFT & HIBYTE(ManSync))
		ManSyncModifiers |= MOD_SHIFT;
	ManSyncVk = LOBYTE(ManSync);
	
	if (HOTKEYF_ALT & HIBYTE(OperModeSwitch))
		OperModeSwitchModifiers |= MOD_ALT;
	if (HOTKEYF_CONTROL & HIBYTE(OperModeSwitch))
		OperModeSwitchModifiers |= MOD_CONTROL;
	if (HOTKEYF_SHIFT & HIBYTE(OperModeSwitch))
		OperModeSwitchModifiers |= MOD_SHIFT;
	OperModeSwitchVk = LOBYTE(OperModeSwitch);
	
	if(!(RegisterHotKey(hwnd, 443, ManSyncModifiers, ManSyncVk)))
		debuglog(DEBUG_ERROR,"Failed registering manual sync hotkey\n");
		
	if(!(RegisterHotKey(hwnd, 444, OperModeSwitchModifiers, OperModeSwitchVk)))
		debuglog(DEBUG_ERROR,"Failed registering mode switch hotkey\n");	
	
	return 1;
}

int HotkeysUnregister(HWND hwnd) {
	if(!(UnregisterHotKey(hwnd, 443)))
		debuglog(DEBUG_ERROR,"Failed unregistering manual sync hotkey\n");
		
	if(!(UnregisterHotKey(hwnd, 444)))
		debuglog(DEBUG_ERROR,"Failed unregistering mode switch hotkey\n");	
	
	return 1;
}	


