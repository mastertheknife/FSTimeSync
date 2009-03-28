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
	return 1;	
}


int RegistryWriteSettings(SyncOptions* WriteSettings) {
	return 1;
}


