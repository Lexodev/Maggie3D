/**
 * Magie3D static library
 *
 * Maggie 3D support functions
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.5 May 2024 (updated: 27/05/2024)
 */

#include <exec/execbase.h>
#include <exec/exec.h>
#include <dos/dos.h>
#include <graphics/gfx.h>
#include <utility/utility.h>
#include <cybergraphx/cybergraphics.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>
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
char M3D_Version[] = "$VER: Maggie3D V1.5, May 2024";

/** Check if Maggie is present */
BOOL M3D_CheckMaggie(VOID)
{
  volatile UWORD *card_version = (UWORD *) M3D_VCARD;
  UWORD vampire;

  if (SysBase->AttnFlags & AFF_68080) {
    vampire = *card_version;
    Dbug(printf("[MAGGIE3D] Vampire card version is 0x%x \n", vampire);)
    vampire >>= 8;
    vampire &= 0xff;
    if (vampire == M3D_V4500 || vampire == M3D_V4600 || vampire == M3D_V41200 || vampire == M3D_V4SA) {
      Dbug(printf("[MAGGIE3D] Vampire V4 card detected\n");)
      return TRUE;
    }
    Dbug(printf("[MAGGIE3D] Vampire card without Maggie support\n");)
    return FALSE;
  }
#if _USE_MAGGIE_ == 0
  Dbug(printf("[MAGGIE3D] Not a Vampire card but use emulation\n");)
  return TRUE;
#else
  Dbug(printf("[MAGGIE3D] Not a Vampire card\n");)
  return FALSE;
#endif
}

/** Find the best screen mode */
ULONG M3D_BestModeID(ULONG width, ULONG height, ULONG depth)
{
  ULONG display_id;

  if (depth > 28) {
    depth = 32;
  } else if (depth > 20) {
    depth = 24;
  } else {
    depth = 16;
  }
  Dbug(printf("[MAGGIE3D] Looking for a best mode id for %d x %d x %d \n", width, height, depth);)
  display_id = BestCModeIDTags(
      CYBRBIDTG_NominalWidth, width,
      CYBRBIDTG_NominalHeight, height,
      CYBRBIDTG_Depth, depth,
      TAG_DONE
  );
  if (display_id != INVALID_ID && IsCyberModeID(display_id)) {
    return display_id;
  }
  return INVALID_ID;
}

/** Create a new context */
M3D_Context *M3D_CreateContext(LONG *error, struct BitMap *bitmap)
{
  M3D_Context *context;
  
  Dbug(printf("[MAGGIE3D] Creating context (Maggie3D v1.5)\n");)
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
      Dbug(printf(
          "[MAGGIE3D] RTG screen size informations %ld x %ld x %ld (%ld BPR, %ld BPP)\n",
          context->drawregion.width,
          context->drawregion.height,
          context->drawregion.depth,
          context->drawregion.bpr,
          context->drawregion.bpp
      );)
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
      context->states = M3D_TEXMAPPING | M3D_GOURAUD | M3D_ZBUFFERUPDATE;
      if (context->drawregion.depth == 16) {
        context->mode = M3D_M_16BITS;
      } else if (context->drawregion.depth == 24) {
        context->mode = M3D_M_24BITS;
      }
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
  Dbug(printf("[MAGGIE3D] Destroying context\n");)
  if (context != NULL) {
    M3D_FreeAllTextures(context);
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
      if (scissor->width < 8) {
        scissor->width = 8;
      }
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

/** Set the clipping scissor */
LONG M3D_SetScissor(M3D_Context *context , M3D_Scissor *scissor)
{
  if (context == NULL) {
    return M3D_NOCONTEXT;
  }
  if (scissor->width < 8) {
    scissor->width = 8;
  }
  context->clipping.top = scissor->top;
  context->clipping.left = scissor->left;
  context->clipping.width = scissor->width;
  context->clipping.height = scissor->height;
  return M3D_SUCCESS;
}

/** Set the rendering state */
LONG M3D_SetState(M3D_Context *context, UWORD state, BOOL enable)
{
#if _ACTIVATE_DEBUG_ == 1
  if (state == F_DRAW_DEBUG) {
    M3D_DrawDebug(enable);
    return M3D_SUCCESS;
  }
#endif
  if (context != NULL) {
    switch (state) {
      case M3D_FILTERING:
        if (enable) {
          context->mode |= M3D_M_BILINEAR;
        } else {
          context->mode &= ~M3D_M_BILINEAR;
        }
        break;
      case M3D_ZBUFFER:
        if (context->zbuffer.data != NULL && enable) {
          context->mode |= M3D_M_ZBUFFER;
        } else {
          context->mode &= ~M3D_M_ZBUFFER;
        }
        break;
      case M3D_ZBUFFERUPDATE:
        if (enable) {
          context->mode &= ~M3D_M_INHIBZBUF;
        } else {
          context->mode |= M3D_M_INHIBZBUF;
        }
        break;
    }
    if (enable) {
      context->states |= state;
    } else {
      context->states &= ~state;
    }
    return M3D_SUCCESS;
  }
  return M3D_NOCONTEXT;
}

/** Get the rendering state */
BOOL M3D_GetState(M3D_Context *context, UWORD state)
{
  if (context != NULL && (context->states & state)) {
    return M3D_ENABLE;
  }
  return M3D_DISABLE;
}
