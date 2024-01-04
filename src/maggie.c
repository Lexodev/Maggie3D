/**
 * Magie3D library
 *
 * Maggie 3D support functions
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.0 November 2023
 */

#include <exec/execbase.h>
#include <exec/exec.h>
#include <graphics/gfx.h>
#include <cybergraphx/cybergraphics.h>

#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/cybergraphics.h>

#include <stdio.h>

#include "debug.h"
#include "memory.h"
#include "maggie.h"
#include "Maggie3D.h"

/** @var Intuition library */
extern struct ExecBase *SysBase;

/** @var CybergraphX library */
extern struct Library *CyberGfxBase;

/** @var Program version */
char M3D_Version[] = "$VER: Maggie3D V1.1, January 2024";

/** Check if Maggie is present */
BOOL M3D_CheckMaggie(VOID)
{
  volatile UWORD *card_version = (UWORD *) M3D_VCARD;
  UWORD vampire;

  if (SysBase->AttnFlags & AFF_68080) {
    vampire = *card_version;
    Dbug(printf("[DEBUG] Vampire card version is 0x%X\n", vampire);)
    vampire >>= 8;
    vampire &= 0xff;
    if (vampire == M3D_V4500 || vampire == M3D_V4600 || vampire == M3D_V41200 || vampire == M3D_V4SA) {
      Dbug(printf("[DEBUG] Vampire V4 card detected\n");)
      return TRUE;
    }
    Dbug(printf("[DEBUG] Vampire card without Maggie support\n");)
    return FALSE;
  }
  Dbug(printf("[DEBUG] Not a Vampire card\n");)
  return FALSE;
}

/** Create a new context */
M3D_Context *M3D_CreateContext(LONG *error, struct BitMap *bitmap)
{
  M3D_Context *context;
  
  if (bitmap == NULL) {
    *error = M3D_NOBITMAP;
    return NULL;
  }
  context = M3D_AllocMem(sizeof(M3D_Context));
  if (context != NULL) {
    if (GetCyberMapAttr(bitmap, CYBRMATTR_ISCYBERGFX)) {
      context->drawregion.bitmap = bitmap;
      context->drawregion.width = GetCyberMapAttr(bitmap, CYBRMATTR_WIDTH);
      context->drawregion.height = GetCyberMapAttr(bitmap, CYBRMATTR_HEIGHT);
      context->drawregion.depth = GetCyberMapAttr(bitmap, CYBRMATTR_DEPTH);
      context->drawregion.bpr = GetCyberMapAttr(bitmap, CYBRMATTR_XMOD);
      context->drawregion.bpp = GetCyberMapAttr(bitmap, CYBRMATTR_BPPIX);
      Dbug(printf("[DEBUG] RTG screen size informations %dx%dx%d (%dBPR, %dBPP)\n", context->drawregion.width, context->drawregion.height, context->drawregion.depth, context->drawregion.bpr, context->drawregion.bpp);)
      context->clipping.top = 0;
      context->clipping.left = 0;
      context->clipping.width = context->drawregion.width;
      context->clipping.height = context->drawregion.height;
      context->flat_shading = M3D_AllocAlignMem(8, 8);      // Fake texture for flat shading
      if (context->flat_shading == NULL) {
        M3D_FreeMem(context);
        *error = M3D_NOMEMORY;
        return NULL;
      }
      context->flat_shading[0] = 0xffffffff;                // This is a small DXT1 white texture
      context->flat_shading[1] = 0xaaaaaaaa;                // of 4x4 pixels used for flat shading
      context->maggie_available = M3D_CheckMaggie();
      *error = M3D_SUCCESS;
      return context;
    }
    *error = M3D_BITMAPTYPE;
    return NULL;
  }
  *error = M3D_NOMEMORY;
  return NULL;
}

/** Destroy context and free all resources */
VOID M3D_DestroyContext(M3D_Context *context)
{
  if (context != NULL) {
    if (context->zbuffer.data != NULL) {
      M3D_FreeMem(context->zbuffer.data);
    }
    if (context->flat_shading != NULL) {
      M3D_FreeMem(context->flat_shading);
    }
    M3D_FreeMem(context);
  }
  // By security release all memory blocks
  M3D_ReleaseMem();
}

/** Set the drawing region */
LONG M3D_SetDrawRegion(M3D_Context *context , struct BitMap *bitmap, M3D_Scissor *scissor)
{
  if (context == NULL) {
    return M3D_NOCONTEXT;
  }
  if (bitmap == NULL) {
    return M3D_NOBITMAP;
  }
  // Check for CGX bitmap
  if (GetCyberMapAttr(bitmap, CYBRMATTR_ISCYBERGFX)) {
    context->drawregion.bitmap = bitmap;
    if (scissor != NULL) {
      context->clipping.top = scissor->top;
      context->clipping.left = scissor->left;
      context->clipping.width = scissor->width;
      context->clipping.height = scissor->height;
    } else {
      context->clipping.top = 0;
      context->clipping.left = 0;
      context->clipping.width = context->drawregion.width;
      context->clipping.height = context->drawregion.height;
    }
    return M3D_SUCCESS;
  }
  return M3D_BITMAPTYPE;
}

/** Set the rendering state */
LONG M3D_SetState(M3D_Context *context, UWORD state, BOOL enable)
{
  if (context != NULL) {
    if (enable) {
      context->states |= state;
    } else {
      context->states &= ~state;
    }
    Dbug(printf("[DEBUG] New state is 0x%X\n", context->states);)
    return M3D_SUCCESS;
  }
  return M3D_NOCONTEXT;
}
