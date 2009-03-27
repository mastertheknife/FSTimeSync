#include "globalinc.h"
#include "debug.h"
#include "main.h"
#include "gui.h"
#include "sync.h"
#include "registry.h"

unsigned int AutoSync = 1;

int WINAPI WinMain (HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow) {

	/* Modules startup */
	DebugStartup();
	// RegistryStartup();
 	GUIStartup();

	GUIStartThread(1);	
			 		
	while(1)
	Sleep(50);
    
    GUIShutdown();
    // RegistryShutdown();
    DebugShutdown();

    return 0;
}

