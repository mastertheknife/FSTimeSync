#include "globalinc.h"
#include "debug.h"
#include "main.h"
#include "gui.h"
#include "sync.h"
#include "registry.h"
#include <windows.h>
#include <commctrl.h>

/* Globals */
SyncOptions Settings = {1,0,1,10,0,MAKEWORD(VK_F6,(HOTKEYF_CONTROL | HOTKEYF_SHIFT)),MAKEWORD(VK_F7,(HOTKEYF_CONTROL | HOTKEYF_SHIFT))}; /* The options! */
SyncOptions Defaults = {0,10,1,0,0}; /* Default options for when using the Defaults button */
CRITICAL_SECTION SettingsCS; /* Critical section to protect the options structure */

static unsigned int AutoSync; /* Runtime setting controlling manual or auto mode */
volatile unsigned int bQuit; /* If set to 1, program exists */

int WINAPI WinMain (HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow) {
	
	/* Mutex to prevent multiple instances of this application */
  	if (CreateMutex(NULL,FALSE,"_FSTimeSyncMutex_") && GetLastError() == ERROR_ALREADY_EXISTS) {
		MessageBox(NULL,"An instance of FS Time Sync is already running.","FS Time Sync Error",MB_OK | MB_ICONERROR);
		return 100;
	}

	/* Initialize the critical section and perform startup */
	InitializeCriticalSection(&SettingsCS);
	DebugStartup();
	RegistryStartup();
 	GUIStartup();

 	AutoSync = Settings.AutoOnStartup;
	
	/* Start the GUI Thread */
	if(Settings.StartMinimized == 1)
		GUIStartThread(SW_MINIMIZE);
	else	
		GUIStartThread(nCmdShow);	
			 		
	while(!bQuit) {
		Sleep(50);
	}
	
	/* Stop the GUI thread, or at least wait for it to close */
	GUIStopThread();
	
	/* Delete the critical section and perform complete shutdown */	
	DeleteCriticalSection(&SettingsCS);		
	GUIShutdown();
    RegistryShutdown();
    DebugShutdown();

    return 0;
}

int SetOperMode(unsigned int bAuto) {
	if(bAuto)
		bAutoSync = 1;
	else
		bAutoSync = 0;
	
	return bAutoSync;
}	
				
int GetOperMode() {
	if(bAutoSync)
		return 1;
	else
		return 0;

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
		
	if((UnregisterHotKey(hwnd, 444)))
		debuglog(DEBUG_ERROR,"Failed unregistering mode switch hotkey\n");	
	
	return 1;
}	
	


