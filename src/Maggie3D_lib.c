/**
 * maggie3d_lib.c
 *
 * Contains the __UserLibInit() and __UserLibCleanup() routines for
 * the Maggie3D.library shared library.
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.5 June 2024 (updated: 01/06/2024)
 */

#include <exec/types.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>

#include <dos/dos.h>
#include <clib/dos_protos.h>

#include <utility/utility.h>

#include <cybergraphx/cybergraphics.h>

#include <stdio.h>
#include <stdarg.h>

#include "debug.h"

#define CYBERGFXVERSION       41L

/**
 * These prototypes are just to keep the compiler happy since we don't
 * want them in the distributable proto file.
 */

int __saveds __UserLibInit(void);
void __saveds __UserLibCleanup(void);

/**
 * Following global item are created so our library can make calls to other libs.
 */

struct Library *SysBase = NULL;
struct Library *UtilityBase = NULL;
struct Library *CyberGfxBase = NULL;

/**
 * This function is called when library is opened, it should return 0 if all is OK.
 */
int __saveds __UserLibInit(void)
{
  SysBase = (*((void **)4));
  if ((UtilityBase = OpenLibrary(UTILITYNAME, 1)) == NULL) {
    return 1;
  }
  if ((CyberGfxBase = OpenLibrary(CYBERGFXNAME, CYBERGFXVERSION)) == NULL) {
    return 1;
  }
  return 0;
}

/**
 * This function is called when library is closed
 */
void __saveds __UserLibCleanup(void)
{
  if (UtilityBase != NULL) {
    CloseLibrary(UtilityBase);
  }
  if (CyberGfxBase != NULL) {
    CloseLibrary(CyberGfxBase);
  }
}

void _XCEXIT(long lcode) { }
