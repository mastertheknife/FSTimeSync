#include "globalinc.h"
#include "debug.h"
#include "main.h"
#include "gui.h"
#include "sync.h"
#include "registry.h"

int WINAPI WinMain (HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow) {

	/* Modules startup */
	DebugStartup();
	RegistryStartup();
 	GUIStartup();

	 		
			 		

    
    GUIShutdown();
    RegistryShutdown();
    DebugShutdown();

    return 0;
}

