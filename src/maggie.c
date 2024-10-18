/**
 * Magie3D shared library
 *
 * Maggie 3D support functions
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.5 June 2024 (updated: 01/06/2024)
 */

#include <exec/execbase.h>
#include <exec/exec.h>
#include <dos/dos.h>
#include <utility/utility.h>
#include <graphics/gfx.h>
#include <cybergraphx/cybergraphics.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include <proto/graphics.h>
#include <proto/cybergraphics.h>

#include "debug.h"
#include "memory.h"
#include "maggie.h"
#include "Maggie3D.h"

VOID __asm __saveds LIBM3D_FreeAllTextures(register __a0 M3D_Context *context);

extern struct ExecBase *SysBase;
extern struct Library *CyberGfxBase;

/**
 * The following global data item becomes part of the near data section
 * for each library client. Since this library is designed to give a
 * different library base to each client, this data item is unique per
 * client.
 */

// ULONG SampleGlobalNearData = 0L;

/**
 * In contrast to the previous global data item, the following goes in
 * the far data section and is global to all library clients.  Read access
 * doesn't need arbitration, but write access needs a semaphor or use of
 * a Forbid()/Permit() pair.
 */

//ULONG __far SampleGlobalFarData = 0L;

/**
 * Now we have library functions
 */

/** Check if Maggie is present */
BOOL __asm __saveds LIBM3D_CheckMaggie(VOID)
{
  volatile UWORD *card_version = (UWORD *) M3D_VCARD;
  UWORD vampire;

  if (SysBase->AttnFlags & AFF_68080) {
    vampire = *card_version;
    Dbug(kprintf("[MAGGIE3D] Vampire card version is 0x%lx \n", vampire);)
    vampire >>= 8;
    vampire &= 0xff;
    if (vampire == M3D_V4500 || vampire == M3D_V4600 || vampire == M3D_V41200 || vampire == M3D_V4SA) {
      Dbug(kprintf("[MAGGIE3D] Vampire V4 card detected\n");)
      return TRUE;
    }
    Dbug(kprintf("[MAGGIE3D] Vampire card without Maggie support\n");)
    return FALSE;
  }
#if _USE_MAGGIE_ == 0
  Dbug(kprintf("[MAGGIE3D] Not a Vampire card but use emulation\n");)
  return TRUE;
#else
  Dbug(kprintf("[MAGGIE3D] Not a Vampire card\n");)
  return FALSE;
#endif
}

/** Find the best screen mode */
ULONG __asm __saveds LIBM3D_BestModeID(register __d0 ULONG width, register __d1 ULONG height, register __d2 ULONG depth)
{
  ULONG display_id;

  if (depth > 28) {
    depth = 32;
  } else if (depth > 20) {
    depth = 24;
  } else {
    depth = 16;
  }
  Dbug(kprintf("[MAGGIE3D] Looking for a best mode id for %ld x %ld x %ld \n", width, height, depth);)
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
M3D_Context __asm __saveds *LIBM3D_CreateContext(register __a0 LONG *error, register __a1 struct BitMap *bitmap)
{
  M3D_Context *context;
  
  Dbug(kprintf("[MAGGIE3D] Creating context (Maggie3D v1.5)\n");)
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
      Dbug(kprintf(
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
      context->maggie_available = LIBM3D_CheckMaggie();
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
VOID __asm __saveds LIBM3D_DestroyContext(register __a0 M3D_Context *context)
{
  Dbug(kprintf("[MAGGIE3D] Destroying context\n");)
  if (context != NULL) {
    LIBM3D_FreeAllTextures(context);
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
LONG __asm __saveds LIBM3D_SetDrawRegion(register __a0 M3D_Context *context , register __a1 struct BitMap *bitmap, register __a2 M3D_Scissor *scissor)
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
LONG __asm __saveds LIBM3D_SetScissor(register __a0 M3D_Context *context , register __a1 M3D_Scissor *scissor)
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
LONG __asm __saveds LIBM3D_SetState(register __a0 M3D_Context *context, register __d0 UWORD state, register __d1 BOOL enable)
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
    //Dbug(kprintf("[MAGGIE3D] New state is 0x%lx \n", context->states);)
    return M3D_SUCCESS;
  }
  return M3D_NOCONTEXT;
}

/** Get the rendering state */
BOOL __asm __saveds LIBM3D_GetState(register __a0 M3D_Context *context, register __d0 UWORD state)
{
  if (context != NULL && (context->states & state)) {
    return M3D_ENABLE;
  }
  return M3D_DISABLE;
}
