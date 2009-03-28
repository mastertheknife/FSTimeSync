#include "globalinc.h"
#include "debug.h"
#include "main.h"
#include "gui.h"
#include "sync.h"
#include "registry.h"

/* Globals */
SyncOptions Settings; /* The options! */
unsigned int AutoSync; /* Runtime setting controlling manual or auto mode */
CRITICAL_SECTION SettingsCS; /* Critical section to protect the options structure */

int WINAPI WinMain (HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow) {

	/* Modules startup */
	DebugStartup();
	RegistryStartup();
 	GUIStartup();
 	
 	Settings.AutoSyncInterval = 10;
 	Settings.UTCOffsetState = 1;
 	Settings.UTCOffset = -60;
 	Settings.AutoOnStartup = 1;
 	Settings.StartMinimized = 1;

	/* Initialize the critical section protecting the options and settings! */
	InitializeCriticalSection(&SettingsCS);
	
	/* Load settings from registry */
	
	/* Start the GUI Thread */
	if(Settings.StartMinimized == 1)
	GUIStartThread(SW_MINIMIZE);
	else	
	GUIStartThread(nCmdShow);	
			 		
	while(1)
	Sleep(50);
    
    GUIShutdown();
    RegistryShutdown();
    DebugShutdown();

    return 0;
}

