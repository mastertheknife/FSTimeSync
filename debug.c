#include "globalinc.h"
#include "debug.h"
#include "main.h"

static FILE* debugfile;				 			/* The debug file handle. */
static unsigned long debugfilter = DEBUG_ALL;	/* Debug mask for filtering messages */
static char* debugstrings[] = {"DEBUG_TRACE","DEBUG_CAPTURE","DEBUG_INFO","DEBUG_NOTICE","DEBUG_WARNING","DEBUG_ERROR","DEBUG_CRITICAL","DEBUG_SHOWALWAYS","DEBUG_MIXED","DEBUG_UNKNOWN"};
static CRITICAL_SECTION debuglogCS;

const char* DebugGetMaskString(unsigned long nmask) {
	switch(nmask) {
		case 0x4UL:
			return debugstrings[0];
		case 0x10UL:
			return debugstrings[1];
		case 0x40UL:
			return debugstrings[2];
		case 0x100UL:
			return debugstrings[3];
		case 0x400UL:
			return debugstrings[4];
		case 0x1000UL:
			return debugstrings[5];
		case 0x4000UL:
			return debugstrings[6];
		case 0xFFFFFFFFUL:
			return debugstrings[7];
		default: 
			return debugstrings[8];
		}
}										
									
int DebugStartup(void) {
	InitializeCriticalSection(&debuglogCS);
	debugfile = fopen("fstimesync.log","w");
	setvbuf (debugfile, NULL, _IONBF,0);
	debuglog(DEBUG_ALL,"Debug log opened.\n");
}		

int DebugShutdown(void) {
	debuglog(DEBUG_ALL,"Debug log closed.\n"); 
	fclose(debugfile);
	DeleteCriticalSection(&debuglogCS); 
}		

unsigned long DebugGetMask(void) {
	return debugfilter;
}

void DebugSetMask(unsigned long newmask) {
	debuglog(DEBUG_ALL,"Debug mask being changed to: %s (%u) from %s (%u).\n",DebugGetMaskString(newmask),newmask,DebugGetMaskString(debugfilter),debugfilter); 	
	InterlockedExchange(&debugfilter,newmask);
}

void DebugWrite(const char* strfile, const char* strfunc, unsigned int nTID, unsigned long nmask, const char* format, ...) {
#ifndef _DEBUG
	/* Empty body, so a good compiler will optimise calls. */
#else
	char debugmsg[1024] = {0};
	va_list args;

	if(!(nmask & debugfilter) && nmask != DEBUG_ALL)
		return;

	EnterCriticalSection(&debuglogCS);
	va_start(args, format);
	vsprintf(debugmsg, format, args);
	fprintf(debugfile, "[%s:%u]: %s: %s: %s", DebugGetMaskString(nmask), nTID, strfile, strfunc, debugmsg);
	fflush(debugfile);
	va_end(args);
	LeaveCriticalSection(&debuglogCS);
#endif
}
