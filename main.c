#include "globalinc.h"
#include "debug.h"
#include "main.h"
#include "gui.h"
#include "sync.h"
#include "registry.h"

/* Globals */
SyncOptions Settings; /* The options! */
SyncOptions Defaults = {0,10,1,0,0}; /* Default options for when using the Defaults button */
unsigned int AutoSync; /* Runtime setting controlling manual or auto mode */
CRITICAL_SECTION SettingsCS; /* Critical section to protect the options structure */
volatile unsigned int bQuit; /* If set to 1, program exists */

int WINAPI WinMain (HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow) {

	/* Initialize the critical section and perform startup */
	InitializeCriticalSection(&SettingsCS);
	DebugStartup();
	RegistryStartup();
 	GUIStartup();

	/* Load settings from registry */
	Settings.AutoSyncInterval = 10;
 	Settings.UTCOffsetState = 1;
 	Settings.UTCOffset = -60;
 	Settings.AutoOnStartup = 1;
 	Settings.StartMinimized = 1;
 	
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

