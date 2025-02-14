/**
 * debug.h
 * 
 * Maggie3D static library
 * Debug macros
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.5 May 2024  (updated: 27/05/2024)
 */

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <exec/exec.h>

#ifndef _ACTIVATE_DEBUG_
#define _ACTIVATE_DEBUG_      1
#endif

#ifndef _TRACE_MAGGIE_
#define _TRACE_MAGGIE_        0
#endif

#ifndef _USE_MAGGIE_
#define _USE_MAGGIE_          0
#endif

#ifndef _USE_FASTASM_
#define _USE_FASTASM_         0
#endif

#if _ACTIVATE_DEBUG_ == 1
#define Dbug(x) x
#define DDbug(x) if (draw_debug) { x }
#else
#define Dbug(x)
#define DDbug(x)
#endif

#define F_DRAW_DEBUG          0xFFFF
VOID M3D_DrawDebug(BOOL);

#endif
