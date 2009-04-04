#ifndef _FSTS_REGISTRY_H_INC_
#define _FSTS_REGISTRY_H_INC_
#include "globalinc.h"
#include "debug.h"
#include "main.h"

typedef struct tagRegSignature {
	char ModulePath[512];
	char Version[32];
} RegSignature_t;	

void CopySettings(SyncOptions_t* dest, const SyncOptions_t* src);
int RegistryStart();
int RegistryShutdown();
int RegistryReadSettings(SyncOptions_t* ReadSettings);
int RegistryWriteSettings(SyncOptions_t* WriteSettings);


#endif

