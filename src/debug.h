/**
 * debug.h
 * 
 * Maggie3D shared library
 * Debug macros
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.2 March 2024 (updated: 10/03/2024)
 */

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <exec/exec.h>

// Activation of debug log
#ifndef _ACTIVATE_DEBUG_
#define _ACTIVATE_DEBUG_      0
#endif

// Use real Maggie or the internal emulation
#ifndef _USE_MAGGIE_
#define _USE_MAGGIE_          1
#endif

// Use fast ASM functions (not really stable)
#ifndef _USE_FASTASM_
#define _USE_FASTASM_         0
#endif

#if _ACTIVATE_DEBUG_ == 1
#define Dbug(x) x
#define DDbug(x) if (draw_debug) { x }
void kprintf(char *, ...);
#else
#define Dbug(x)
#define DDbug(x)
#endif

#define F_DRAW_DEBUG          0xFFFF
VOID M3D_DrawDebug(BOOL);

#endif
