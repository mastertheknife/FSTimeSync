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

#define SLEEP_DURATION 100
#include "globalinc.h"
#include "debug.h"
#include "main.h"
#include "gui.h"
#include "sync.h"
#include "registry.h"
#include <windows.h>
#include <commctrl.h>
#include "FSUIPC_User.h"

/* Globals */
SyncOptions_t Settings;
SyncOptions_t Defaults = {0,0,0,1,10,0,0,1,1,0,MAKEWORD(0x53,(HOTKEYF_CONTROL | HOTKEYF_SHIFT)),MAKEWORD(0x4D,(HOTKEYF_CONTROL | HOTKEYF_SHIFT))}; /* Default options */
SyncStats_t Stats;
CRITICAL_SECTION ProgramDataCS;
static RuntimeVals_t RuntimeVals;
static CRITICAL_SECTION ProgramControlCS;

/* Version info */
Version_t Ver = {__TIME__,__DATE__,"v1.0"};

/* Locals */
static DWORD ReUpdateGUI;
static DWORD SimStatusPrev;
static DWORD SimPausedPrev;
static DWORD SimRatePrev;
static DWORD CancelSync;

int WINAPI WinMain (HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow) {
	/* Mutex to prevent multiple instances of this application */
  	if (CreateMutex(NULL,FALSE,"_FSTimeSyncMutex_") && GetLastError() == ERROR_ALREADY_EXISTS) {
		MessageBox(NULL,"An instance of FS Time Sync is already running.","FS Time Sync Error",MB_OK | MB_ICONERROR);
		return 100;
	}

	/* Initialize the critical sections and perform indepenedant modules startup */
	InitializeCriticalSection(&ProgramDataCS);
	InitializeCriticalSection(&ProgramControlCS);
	DebugStartup();
	RegistryStartup();
	SyncStartup();
 	GUIStartup();

	/* Load the settings from the registry */
	RegistryReadSettings(&Settings);
	
	/* Set the operating mode (manual or auto) based on the setting and next sync */
 	SetRTVal(FST_AUTOMODE,Settings.AutoOnStart);
	Stats.SyncNext = time(NULL)+Settings.AutoSyncInterval;
	Stats.SyncInterval = Settings.AutoSyncInterval;
	
	/* Start the GUI Thread, based on the Start Minimized setting */
	if(Settings.StartMinimized == 1)
		GUIStartThread(SW_MINIMIZE);
	else	
		GUIStartThread(nCmdShow);	
			 		
	
	/* Main program loop */
	while(!GetRTVal(FST_QUIT)) {
		time_t CurrentTime;
		struct tm* systm;
		int nResult;
		time_t SimUTCDest;

		if(ReUpdateGUI) {
			ReUpdateGUI = 0;
			/* Signal GUI thread to call GUIUpdate */
			PostMessage(hDummyWindow,WM_USER+1,10,0);
		}
		
		if(CancelSync)
			CancelSync = 0;

		EnterCriticalSection(&ProgramDataCS);

		Stats.SimStatus = SyncGetConStatus(); /* Get updated state of the connection */
		CurrentTime = time(NULL); /* Saves multiple expensive calls to time() */
			
		/* System UTC time */
		systm = gmtime(&CurrentTime);
		if((Stats.SysUTCTime = mktime(systm)) == (time_t)-1) {
			debuglog(DEBUG_ERROR,"mktime failed making system UTC time!\n");
			Stats.SysUTCTime = CurrentTime; /* If we can't get UTC, then lets have local */
		}
		if(Settings.SystemUTCCorrectionState)
			Stats.SysUTCTime += Settings.SystemUTCCorrection*60;

		/* Only proceed if sim is connected */
		if(Stats.SimStatus) {
		
			/* Get the simulator's UTC time */
			SyncGetFSTimestamp(&Stats.SimUTCTime);	

			/* Get the simulator's pause state (paused or not) */
			if(Settings.NoSyncPaused) {		
				/* Get the simulator's state (paused or not) */
				SyncGetPause(&Stats.SimPaused);
				if(Stats.SimPaused)
					CancelSync = 1;	
				if(Stats.SimPaused != SimPausedPrev)
						ReUpdateGUI = 1;
			}

			/* Get the simulator's simulation rate (256=1) */			
			if(Settings.NoSyncSimRate) {		
				/* Get the simulator's simulation rate (256=1) */
				SyncGetSimRate(&Stats.SimRate);	
				if(Stats.SimRate != 256)
					CancelSync = 1;
				if(Stats.SimRate != SimRatePrev)
					ReUpdateGUI = 1;						
			}
					
			if(GetRTVal(FST_AUTOMODE)) {
				/* Check for no sync, e.g. paused or sim rate */
				if(CancelSync) {
					Stats.SyncNext = CurrentTime+Settings.AutoSyncInterval;
					Stats.SyncInterval = Settings.AutoSyncInterval;
				}
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
				/* Check if it's time to sync and if time, sync*/
				if(Stats.SyncNext <= CurrentTime) {
					/* Use system UTC time */
					SimUTCDest = Stats.SysUTCTime;
					if(nResult = SyncGo(SimUTCDest,Settings.FSNoSyncLocalTime)) {
						Stats.SyncNext = time(&CurrentTime)+Settings.AutoSyncInterval; /* Sync takes time, update the time on the way */
						Stats.SyncLast = CurrentTime;
						Stats.SyncInterval = Settings.AutoSyncInterval;
						Stats.SyncLastModified = 1;
					} else {
						Stats.SyncNext = CurrentTime+3;
						Stats.SyncInterval = 3;
					}
				}
			} else {
				/* Manual mode */
				Stats.SyncInterval = 0;
				if(GetRTVal(FST_SYNCNOW)) {
					SetRTVal(FST_SYNCNOW, 0); /* Clear the event, regardless of success or failure */
					if(!CancelSync) {
						/* Use system UTC time */
						SimUTCDest = Stats.SysUTCTime;														
						if(SyncGo(SimUTCDest,Settings.FSNoSyncLocalTime)) {
							Stats.SyncLast = time(&CurrentTime);
							Stats.SyncLastModified = 1;
						}
					}
				} /* Sync now */
			} /* Mode */
		} else {
			/* Connect to the sim */
			Stats.SimStatus = SyncConnect(SIM_ANY);

			if(Stats.SimStatus != SimStatusPrev)
				ReUpdateGUI = 1; /* Force updating of the GUI */

			/* Set up the affinity fix and\or the priority fix depending on settings */
			if(Stats.SimStatus)
				AffinityPriorityFix(Settings.EnableAffinityFix,Settings.EnablePriorityFix);
				
		} /* SimStatus */

		if(Settings.NoSyncPaused)
			SimPausedPrev = Stats.SimPaused;

		if(Settings.NoSyncSimRate)
			SimRatePrev = Stats.SimRate;			
		
		SimStatusPrev = Stats.SimStatus;
		
		/* Let GUIUpdateElements() know that it can update */
		Stats.UpdateElements = 1;
			
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
	SyncShutdown();
    RegistryShutdown();
    DebugShutdown();

    return 0;
}

/* Thread safe */
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

/* Thread safe */
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

/* Thread safe - doesn't care about the lock */					
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

/* Thread safe - doesn't care about the lock */	
int HotkeysUnregister(HWND hwnd) {
	if(!(UnregisterHotKey(hwnd, 443)))
		debuglog(DEBUG_ERROR,"Failed unregistering manual sync hotkey\n");
		
	if(!(UnregisterHotKey(hwnd, 444)))
		debuglog(DEBUG_ERROR,"Failed unregistering mode switch hotkey\n");	
	
	return 1;
}

/* Thread safe - doesn't care about the lock */	
static int AffinityPriorityFix(DWORD EnableAffinityFix, DWORD EnablePriorityFix) {
	HWND hFShwnd;
	DWORD nPriority;
	DWORD nProcAffinity;
	DWORD nSysAffinity;
	DWORD nProcId;
	HANDLE hFS;	

	if(EnableAffinityFix || EnablePriorityFix) {

		/* Start by finding the window */
		switch(FSUIPC_FS_Version) {
			case SIM_FSX:
				hFShwnd = FindWindow(NULL,"Microsoft Flight Simulator X");
				break;
			case SIM_FS2K4:
				hFShwnd = FindWindow(NULL,"Microsoft Flight Simulator 2004 - A Century of Flight");
				break;
			case SIM_FS2K2:
				hFShwnd = FindWindow(NULL,"Microsoft Flight Simulator 2002");
				break;
			case SIM_FS2K:
				hFShwnd = FindWindow(NULL,"Microsoft Flight Simulator 2000");
				break;
			case SIM_FS98:
				hFShwnd = FindWindow(NULL,"Microsoft Flight Simulator 98");
				break;
			default:
				/* Couldn't find the window name, giving up on affinity or priority fix */
				debuglog(DEBUG_WARNING,"Unknown FS, don't know what window to search\n");
				return 0;
		}
		
		if(hFShwnd == NULL) {
			debuglog(DEBUG_WARNING,"Failed finding the window for the connected FS\n");
			return 0;
		}
	
		GetWindowThreadProcessId(hFShwnd,&nProcId);
		if(!nProcId) {
			debuglog(DEBUG_WARNING,"Failed getting FS process ID\n");
			return 0;
		}
	
		hFS = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_SET_INFORMATION,FALSE,nProcId);
		if(hFS == NULL) {
			debuglog(DEBUG_WARNING,"Failed opening FS process!\n");
			return 0;
		}
		
		/* Get FS priority class and copy it to our process */		
		if(EnablePriorityFix) {
			if(!(nPriority = GetPriorityClass(hFS))) {
				debuglog(DEBUG_WARNING,"Failed getting FS priority class!\n");
			} else {
				if(!SetPriorityClass(GetCurrentProcess(),nPriority))
					debuglog(DEBUG_WARNING,"Failed setting priority class!\n");
			}
		}		
		
		/* Get FS affinity mask and copy it to our process */
		if(EnableAffinityFix) {
			if(!GetProcessAffinityMask(hFS,&nProcAffinity,&nSysAffinity)) {
				debuglog(DEBUG_WARNING,"Failed getting FS affinity mask!\n");
			} else {
				if(!SetProcessAffinityMask(GetCurrentProcess(),nProcAffinity))
					debuglog(DEBUG_WARNING,"Failed setting affinity mask!\n");
			}
		}		
		
		/* Finished with the FS process, let's close it */		
		CloseHandle(hFS);
				
	}
	return 1;
	
}

