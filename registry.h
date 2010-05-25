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

