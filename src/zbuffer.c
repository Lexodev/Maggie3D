/**
 * zbuffer.c
 *
 * Maggie3D shared library
 * Z buffer management functions
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.2 March 2024 (updated: 10/03/2024)
 */

#include <stdio.h>

#include "debug.h"
#include "memory.h"
#include "zbuffer.h"
#include "Maggie3D.h"

/** Allocate the Z buffer */
LONG __asm __saveds LIBM3D_AllocZBuffer(register __a0 M3D_Context *context)
{
  if (context != NULL) {
    context->zbuffer.width = context->drawregion.width;
    context->zbuffer.height = context->drawregion.height;
    context->zbuffer.bpr = context->drawregion.width * ZBUF_BPP;
    context->zbuffer.bpp = ZBUF_BPP;
    context->zbuffer.data = M3D_AllocAlignMem(context->zbuffer.width * context->zbuffer.height * ZBUF_BPP, ZBUF_ALIGN);
    if (context->zbuffer.data == NULL) {
      return M3D_NOMEMORY;
    }
    Dbug(kprintf("[MAGGIE3D] Z buffer allocated (%ld x %ld) at 0x%lx \n", context->zbuffer.width, context->zbuffer.height, context->zbuffer.data);)
    return M3D_SUCCESS;
  }
  return M3D_NOCONTEXT;
}

/** Free the Z buffer resources */
VOID __asm __saveds LIBM3D_FreeZBuffer(register __a0 M3D_Context *context)
{
  if (context != NULL) {
    if (context->zbuffer.data != NULL) {
      M3D_FreeMem(context->zbuffer.data);
    }
    context->zbuffer.width = 0;
    context->zbuffer.height = 0;
    context->zbuffer.bpr = 0;
    context->zbuffer.bpp = 0;
    context->zbuffer.data = NULL;
    Dbug(kprintf("[MAGGIE3D] Z buffer released\n");)
  }
}

/** Clear the Z buffer */
LONG __asm __saveds LIBM3D_ClearZBuffer(register __a0 M3D_Context *context)
{
  if (context != NULL && context->zbuffer.data != NULL) {
    return M3D_FastClearZBuffer((ULONG) context->zbuffer.data, (UWORD) context->zbuffer.height, (UWORD) context->zbuffer.bpr);
  }
  return M3D_NOCONTEXT;
}