#ifndef _FSTS_DEBUG_H_INC_
#define _FSTS_DEBUG_H_INC_
#include "globalinc.h"
#include "stdarg.h"

/* Debug and log levels, The bit spacing is for future use.*/
#define DEBUG_TRACE 0x4UL
#define DEBUG_CAPTURE 0x10UL
#define DEBUG_INFO 0x40UL
#define DEBUG_NOTICE 0x100UL
#define DEBUG_WARNING 0x400UL
#define DEBUG_ERROR 0x1000UL
#define DEBUG_CRITICAL 0x4000UL

/* Show always\print all messages. */
#define DEBUG_ALL 0xFFFFFFFFUL

/* Declarations and debug printing macro */
#ifdef __cplusplus
extern "C" {
#endif /* C++ */

/* __func__ is a C99 feature */
/* and VA ARGS doesnt work with every compiler. */
/* Thread safe - doesn't care about the lock */	
#define debuglog(level, format, ...) DebugWrite(__FILE__, __func__ , GetCurrentThreadId(), level, format, ## __VA_ARGS__)

int DebugStartup(void); 			/* Initalizes the debug module.		*/
int DebugShutdown(void); 			/* Shuts down the debug module.		*/
unsigned long DebugGetMask(void);   /* Gets the debugging output mask. */
void DebugSetMask(unsigned long newmask); /* Sets the debugging output mask. */
const char* DebugGetMaskString(unsigned long nmask); /* Gets a friendly text description of the mask integer */

void DebugWrite(const char* strfile, const char* strfunc, unsigned int nTID, unsigned long nmask, const char* format, ...);


#ifdef __cplusplus
}
#endif /* C++ */

#endif

