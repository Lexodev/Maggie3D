/**
 * zbuffer.c
 *
 * Maggie3D library
 * Z buffer management functions
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.0 November 2023
 */

#include <stdio.h>

#include "debug.h"
#include "memory.h"
#include "zbuffer.h"
#include "Maggie3D.h"

/** Allocate the Z buffer */
LONG M3D_AllocZBuffer(M3D_Context *context)
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
    Dbug(printf("[DEBUG] Z buffer allocated (%dx%d) at 0x%X\n", context->zbuffer.width, context->zbuffer.height, context->zbuffer.data);)
    return M3D_SUCCESS;
  }
  return M3D_NOCONTEXT;
}

/** Free the Z buffer resources */
VOID M3D_FreeZBuffer(M3D_Context *context)
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
    Dbug(printf("[DEBUG] Z buffer released\n");)
  }
}

/** Clear the Z buffer */
LONG M3D_ClearZBuffer(M3D_Context *context)
{
  if (context != NULL && context->zbuffer.data != NULL) {
    return M3D_FastClearZBuffer((ULONG) context->zbuffer.data, (UWORD) context->zbuffer.height, (UWORD) context->zbuffer.bpr);
  }
  return M3D_NOCONTEXT;
}
