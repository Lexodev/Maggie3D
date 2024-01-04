/**
 * draw.c
 *
 * Maggie3D library
 * Drawing functions
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.0 November 2023
 */

#include <stdio.h>
#include <string.h>

#include <proto/graphics.h>
#include <cybergraphx/cybergraphics.h>

#include <proto/graphics.h>
#include <proto/cybergraphics.h>

#include "debug.h"
#include "draw.h"

#if _USE_MAGGIE_ == 1
M3D_MaggieRegs *maggie = (M3D_MaggieRegs *) M3D_MAGGIEBASE;
#else
M3D_MaggieRegs reg_maggie;
M3D_MaggieRegs *maggie = &reg_maggie;
#endif

/*****************************************************************************/
//            DEBUG ONLY
/*****************************************************************************/

// Global drawing debug flag
BOOL draw_debug = FALSE;

#if _ACTIVATE_DEBUG_ == 1

#define SFIXEDTOFLOAT(x)      ((FLOAT)(x) / 32768.0)
#define UFIXEDTOFLOAT(x)      ((FLOAT)(x) / 65535.0)
#define LFIXEDTOFLOAT(x)      ((FLOAT)(x) / 65536.0)

VOID M3D_DumpTriangle(M3D_Triangle *triangle)
{
  printf("[DEBUG] Dump triangle\n");
  printf("[DEBUG] => x1=%f  y1=%f  z1=%f  u1=%f  v1=%f  light=%f\n",
      triangle->v1.x, triangle->v1.y, triangle->v1.z, triangle->v1.u, triangle->v1.v, triangle->v1.light);
  printf("[DEBUG] => x2=%f  y2=%f  z2=%f  u2=%f  v2=%f  light=%f\n",
      triangle->v2.x, triangle->v2.y, triangle->v2.z, triangle->v2.u, triangle->v2.v, triangle->v2.light);
  printf("[DEBUG] => x3=%f  y3=%f  z3=%f  u3=%f  v3=%f  light=%f\n",
      triangle->v3.x, triangle->v3.y, triangle->v3.z, triangle->v3.u, triangle->v3.v, triangle->v3.light);
  printf("[DEBUG] => texw=%d  texh=%d  texs=%d  texdata=0x%X  color=0x%X\n",
      triangle->texture->width, triangle->texture->height, triangle->texture->mipsize, triangle->texture->data, triangle->color);
}

VOID M3D_DumpDrawData(M3D_DrawData *draw_data)
{
  printf("[DEBUG] Dump draw data\n");
  printf("[DEBUG] => Clipping : left=%f  top=%f  right=%f  bottom=%f\n",
      draw_data->left_clip, draw_data->top_clip, draw_data->right_clip, draw_data->bottom_clip);
  printf("[DEBUG] => Left delta : dxdyl=%f  dzdyl=%f  dudyl=%f  dvdyl=%f  dldyl=%f\n", 
      draw_data->delta_dxdyl, draw_data->delta_dzdyl, draw_data->delta_dudyl, draw_data->delta_dvdyl, draw_data->delta_dldyl);
  printf("[DEBUG] => Right delta : dxdyr=%f  dzdyr=%f  dudyr=%f  dvdyr=%f  dldyr=%f\n", 
      draw_data->delta_dxdyr, draw_data->delta_dzdyr, draw_data->delta_dudyr, draw_data->delta_dvdyr, draw_data->delta_dldyr);
  printf("[DEBUG] => X & Z coords : xl=%f  xr=%f  zl=%f  zr=%f\n",
      draw_data->crd_xl, draw_data->crd_xr, draw_data->crd_zl, draw_data->crd_zr);
  printf("[DEBUG] => U & V coords : ul=%f  ur=%f  vl=%f  vr=%f\n",
      draw_data->crd_ul, draw_data->crd_ur, draw_data->crd_vl, draw_data->crd_vr);
  printf("[DEBUG] => Light intensity : ll=%f  lr=%f\n",
      draw_data->int_ll, draw_data->int_lr);
  printf("[DEBUG] => Draw : adr=0x%X  bpr=%d  bpp=%d\n",
      draw_data->dest_adr, draw_data->dest_bpr, draw_data->dest_bpp);
  printf("[DEBUG] => ZBuffer : adr=0x%X  bpr=%d  bpp=%d\n",
      draw_data->zbuf_adr, draw_data->zbuf_bpr, draw_data->zbuf_bpp);
}

VOID M3D_DumpMaggieRegs(VOID)
{
#if _TRACE_MAGGIE_ == 1
  printf("[DEBUG] Dump Maggie registers\n");
  printf("[DEBUG] => texture=0x%X  destination=0x%X  zbuffer=0x%X\n", maggie->texture, maggie->destination, maggie->zbuffer);
  printf("[DEBUG] => start_length=%d  tex_size=%d  mode=%d  modulo=%d\n", maggie->start_length, maggie->tex_size, maggie->mode, maggie->modulo);
  printf("[DEBUG] => u_start=0x%X (%f)  v_start=0x%X (%f)  u_delta=0x%X (%f)  v_delta=0x%X (%f)\n",
    maggie->u_start, LFIXEDTOFLOAT(maggie->u_start), maggie->v_start, LFIXEDTOFLOAT(maggie->v_start),
    maggie->u_delta, LFIXEDTOFLOAT(maggie->u_delta), maggie->v_delta, LFIXEDTOFLOAT(maggie->v_delta));
  printf("[DEBUG] => light_start=0x%X (%f)  light_delta=0x%X (%f)  color=0x%X\n",
    maggie->light_start, UFIXEDTOFLOAT(maggie->light_start), maggie->light_delta, SFIXEDTOFLOAT(maggie->light_delta), maggie->color);
  printf("[DEBUG] => z_start=0x%X (%f)  z_delta=0x%X (%f)\n",
    maggie->z_start, LFIXEDTOFLOAT(maggie->z_start), maggie->z_delta, LFIXEDTOFLOAT(maggie->z_delta));
#endif
}

#endif

/*****************************************************************************/

/** Copy triangle data */
VOID M3D_CopyTriangle(M3D_Triangle *dst, M3D_Triangle *src)
{
  DDbug(printf("[DEBUG] M3D_CopyTriangle\n");)
  memcpy(dst, src, sizeof(M3D_Triangle));
}

/** Swap two vertices */
VOID M3D_SwapVertext(M3D_Vertex *va, M3D_Vertex *vb)
{
  M3D_Vertex vt;
  
  DDbug(printf("[DEBUG] M3D_SwapVertext\n");)
  memcpy(&vt, va, sizeof(M3D_Vertex));
  memcpy(va, vb, sizeof(M3D_Vertex));
  memcpy(vb, &vt, sizeof(M3D_Vertex));
}

/** Check the triangle type & order the vertices */
ULONG M3D_CheckTriangleType(M3D_Context *context, M3D_Triangle *triangle, M3D_DrawData *draw_data)
{
  LONG round;

  // Let's start with integer values
  round = (LONG) triangle->v1.x; triangle->v1.x = (FLOAT) round;
  round = (LONG) triangle->v2.x; triangle->v2.x = (FLOAT) round;
  round = (LONG) triangle->v3.x; triangle->v3.x = (FLOAT) round;
  round = (LONG) triangle->v1.y; triangle->v1.y = (FLOAT) round;
  round = (LONG) triangle->v2.y; triangle->v2.y = (FLOAT) round;
  round = (LONG) triangle->v3.y; triangle->v3.y = (FLOAT) round;
  // Degenerated triangle elimination
  if (triangle->v1.x == triangle->v2.x && triangle->v2.x == triangle->v3.x) {
    DDbug(printf("[DEBUG] - Rejected because x1=x2=x3\n");)
    return TRI_REJECTED;
  }
  if (triangle->v1.y == triangle->v2.y && triangle->v2.y == triangle->v3.y) {
    DDbug(printf("[DEBUG] - Rejected because y1=y2=y3\n");)
    return TRI_REJECTED;
  }
  // Order triangle vertices from top to bottom
  if (triangle->v1.y > triangle->v3.y) {
    M3D_SwapVertext(&(triangle->v1), &(triangle->v3));
  }
  if (triangle->v1.y > triangle->v2.y) {
    M3D_SwapVertext(&(triangle->v1), &(triangle->v2));
  }
  if (triangle->v2.y > triangle->v3.y) {
    M3D_SwapVertext(&(triangle->v2), &(triangle->v3));
  }
  DDbug(printf("[DEBUG] - Triangle ordered\n");)
  DDbug(M3D_DumpTriangle(triangle);)
  // Trivial rejection
  if (triangle->v3.y < draw_data->top_clip || triangle->v1.y > draw_data->bottom_clip) {
    DDbug(printf("[DEBUG] - Rejected because y3 < top_clip or y1 > bottom_clip\n");)
    return TRI_REJECTED;
  }
  if (triangle->v1.x < draw_data->left_clip && triangle->v2.x < draw_data->left_clip && triangle->v3.x < draw_data->left_clip) {
    DDbug(printf("[DEBUG] - Rejected because x1,x2 & x3 < left_clip\n");)
    return TRI_REJECTED;
  }
  if (triangle->v1.x > draw_data->right_clip && triangle->v2.x > draw_data->right_clip && triangle->v3.x > draw_data->right_clip) {
    DDbug(printf("[DEBUG] - Rejected because x1,x2 & x3 > right_clip\n");)
    return TRI_REJECTED;
  }
  // Check the type & order from left to right
  if (triangle->v1.y == triangle->v2.y) {
    if (triangle->v1.x > triangle->v2.x) {
      M3D_SwapVertext(&(triangle->v1), &(triangle->v2));
    }
    return TRI_FLATTOP;
  } else if (triangle->v2.y == triangle->v3.y) {
    if (triangle->v2.x > triangle->v3.x) {
      M3D_SwapVertext(&(triangle->v2), &(triangle->v3));
    }
    return TRI_FLATBOTTOM;
  }
  return TRI_GENERIC;
}

#if _USE_MAGGIE_ == 0
/** Simple emulation of Maggie feature, support only 16bits output */
VOID M3D_EmulateMaggie(VOID)
{
#if _USE_FASTASM_ == 0
  UBYTE *texture;
  LONG ui, vi, zi, tex_bpr, tex_scale, texel;
  UWORD *dest, *zbuffer, color, count;
  BOOL draw_texel;
#endif

  DDbug(M3D_DumpMaggieRegs();)
#if _USE_FASTASM_ == 0
  ui = maggie->u_start;
  vi = maggie->v_start;
  zi = maggie->z_start;
  if (maggie->mode & M3D_16BITS) {
    dest = (UWORD *) maggie->destination;
    zbuffer = (UWORD *) maggie->zbuffer;
    count = maggie->start_length;
    if (maggie->texture == NULL) {
      color = (UWORD) (((maggie->color & 0xf80000) >> 8) | ((maggie->color & 0xfc00) >> 5) | ((maggie->color & 0xf8) >> 3));
      while (count--) {
        draw_texel = TRUE;
        if (maggie->mode & M3D_ZBUFFER) {
          if ((zi >> 16) > *zbuffer) {
            draw_texel = FALSE;
          } else {
            *zbuffer = (zi >> 16);
          }
        }
        if (draw_texel) {
          *dest = color;
        }
        dest++;
        zbuffer++;
        zi += maggie->z_delta;
      }
    } else {
      texture = (UBYTE *) maggie->texture;
      tex_bpr = 3 << maggie->tex_size;
      tex_scale = 16 + (8 - maggie->tex_size);
      while (count--) {
        draw_texel = TRUE;
        if (maggie->mode & M3D_ZBUFFER) {
          if ((zi >> 16) > *zbuffer) {
            draw_texel = FALSE;
          } else {
            *zbuffer = (zi >> 16);
          }
        }
        if (draw_texel) {
          texel = ((vi >> tex_scale) * tex_bpr) + ((ui >> tex_scale) * 3);
          color = ((texture[texel] & 0xf8) << 8) | ((texture[texel+1] & 0xfc) << 3) | ((texture[texel+2] & 0xf8) >> 3);
          *dest = color;
        }
        dest++;
        zbuffer++;
        ui += maggie->u_delta;
        vi += maggie->v_delta;
        zi += maggie->z_delta;
      }
    }
  }
#else
  if (maggie->mode & M3D_16BITS) {
    M3D_EmulateMaggie16Bits(maggie);
  }
#endif
}
#endif

/** Draw a single triangle */
LONG M3D_DrawTriangle(M3D_Context *context, M3D_Triangle *triangle)
{
  M3D_DrawData draw_data;
  M3D_Triangle tri_copy;
  ULONG type;

  DDbug(printf("[DEBUG] M3D_DrawTriangle\n");)
  if (context != NULL) {
#if _USE_MAGGIE_ == 1
    if (context->maggie_available) {
#endif
      maggie->mode = context->states & 0xff;
      maggie->modulo = context->drawregion.bpp;
      // Setup clip constants
      draw_data.left_clip = (FLOAT) context->clipping.left;
      draw_data.top_clip = (FLOAT) context->clipping.top;
      draw_data.right_clip = (FLOAT) (context->clipping.left + context->clipping.width - 1);
      draw_data.bottom_clip = (FLOAT) (context->clipping.top + context->clipping.height - 1);
      // Setup constants
      draw_data.dest_bpr = context->drawregion.bpr;
      draw_data.dest_bpp = context->drawregion.bpp;
      draw_data.zbuf_bpr = context->zbuffer.bpr;
      draw_data.zbuf_bpp = context->zbuffer.bpp;
      // If not in fast mode
      if (!(context->states & M3D_FAST)) {
        M3D_CopyTriangle(&tri_copy, triangle);
        triangle = &tri_copy;
      }
      // Check for triangle type
      type = M3D_CheckTriangleType(context, triangle, &draw_data);
      if (type != TRI_REJECTED) {
        if (context->states & M3D_TEXMAPPING && triangle->texture != NULL) {
          M3D_DrawTexturedTriangle(context, triangle, &draw_data, type);
        } else {
          M3D_DrawShadedTriangle(context, triangle, &draw_data, type);
        }
        return M3D_SUCCESS;
      }
      return M3D_NOTRIANGLE;
#if _USE_MAGGIE_ == 1
    }
    return M3D_NOMAGGIE;
#endif
  }
  return M3D_NOCONTEXT;
}

/** Draw an array of triangles */
LONG M3D_DrawTriangleArray(M3D_Context *context, M3D_Triangle *triangles, ULONG count)
{
  M3D_DrawData draw_data;
  M3D_Triangle *triangle, tri_copy;
  ULONG type;
  UWORD index;

  DDbug(printf("[DEBUG] M3D_DrawTriangleArray\n");)
  if (context != NULL) {
#if _USE_MAGGIE_ == 1
    if (context->maggie_available) {
#endif
      maggie->mode = context->states & 0xff;
      maggie->modulo = context->drawregion.bpp;
      // Setup clip constants
      draw_data.left_clip = (FLOAT) context->clipping.left;
      draw_data.top_clip = (FLOAT) context->clipping.top;
      draw_data.right_clip = (FLOAT) (context->clipping.left + context->clipping.width - 1);
      draw_data.bottom_clip = (FLOAT) (context->clipping.top + context->clipping.height - 1);
      // Setup constants
      draw_data.dest_bpr = context->drawregion.bpr;
      draw_data.dest_bpp = context->drawregion.bpp;
      draw_data.zbuf_bpr = context->zbuffer.bpr;
      draw_data.zbuf_bpp = context->zbuffer.bpp;
      for (index = 0;index < count;index++) {
        triangle = &(triangles[index]);
        // If not in fast mode
        if (!(context->states & M3D_FAST)) {
          M3D_CopyTriangle(&tri_copy, triangle);
          triangle = &tri_copy;
        }
        // Check for triangle type
        type = M3D_CheckTriangleType(context, triangle, &draw_data);
        if (type != TRI_REJECTED) {
          if (context->states & M3D_TEXMAPPING && triangle->texture != NULL) {
            M3D_DrawTexturedTriangle(context, triangle, &draw_data, type);
          } else {
            M3D_DrawShadedTriangle(context, triangle, &draw_data, type);
          }
        }
      }
      return M3D_SUCCESS;
#if _USE_MAGGIE_ == 1
    }
    return M3D_NOMAGGIE;
#endif
  }
  return M3D_NOCONTEXT;
}

/** Draw a list of triangles */
LONG M3D_DrawTriangleList(M3D_Context *context, M3D_Triangle **triangles, ULONG count)
{
  M3D_DrawData draw_data;
  M3D_Triangle *triangle, tri_copy;
  ULONG type;
  UWORD index;

  DDbug(printf("[DEBUG] M3D_DrawTriangleList\n");)
  if (context != NULL) {
#if _USE_MAGGIE_ == 1
    if (context->maggie_available) {
#endif
      maggie->mode = context->states & 0xff;
      maggie->modulo = context->drawregion.bpp;
      // Setup clip constants
      draw_data.left_clip = (FLOAT) context->clipping.left;
      draw_data.top_clip = (FLOAT) context->clipping.top;
      draw_data.right_clip = (FLOAT) (context->clipping.left + context->clipping.width - 1);
      draw_data.bottom_clip = (FLOAT) (context->clipping.top + context->clipping.height - 1);
      // Setup constants
      draw_data.dest_bpr = context->drawregion.bpr;
      draw_data.dest_bpp = context->drawregion.bpp;
      draw_data.zbuf_bpr = context->zbuffer.bpr;
      draw_data.zbuf_bpp = context->zbuffer.bpp;
      for (index = 0;index < count;index++) {
        triangle = triangles[index];
        // If not in fast mode
        if (!(context->states & M3D_FAST)) {
          M3D_CopyTriangle(&tri_copy, triangle);
          triangle = &tri_copy;
        }
        // Check for triangle type
        type = M3D_CheckTriangleType(context, triangle, &draw_data);
        if (type != TRI_REJECTED) {
          if (context->states & M3D_TEXMAPPING && triangle->texture != NULL) {
            M3D_DrawTexturedTriangle(context, triangle, &draw_data, type);
          } else {
            M3D_DrawShadedTriangle(context, triangle, &draw_data, type);
          }
        }
      }
      return M3D_SUCCESS;
#if _USE_MAGGIE_ == 1
    }
    return M3D_NOMAGGIE;
#endif
  }
  return M3D_NOCONTEXT;
}

/** Draw a texture for test purpose */
LONG M3D_DrawTest(M3D_Context *context, M3D_Texture *texture)
{
  ULONG y, dest, zbuf;
  FLOAT ui, vi, du, dv, scale;
  
  DDbug(printf("[DEBUG] M3D_DrawTest\n");)
  if (context != NULL) {
#if _USE_MAGGIE_ == 1
    if (!context->maggie_available) {
      return M3D_NOMAGGIE;
    }
#endif
    // Setup Maggie
    DDbug(printf(
        "[DEBUG] Setup for texture %dx%d (%d)\n",
         texture->width, texture->height, texture->mipsize
    );)
    maggie->texture = texture->data;
    maggie->tex_size = texture->mipsize;
    maggie->mode = context->states & 0xff;
    maggie->modulo = context->drawregion.bpp;
    maggie->color = 0xffffff;
    maggie->light_start = (UFIXED) (1.0 * 65535.0);
    maggie->light_delta = (SFIXED) (0.0 * 32768.0);
    maggie->z_start = (LFIXED) (100.0 * 65536.0);
    maggie->z_delta = (LFIXED) (0.0 * 65536.0);
    dest = (ULONG) context->drawregion.data;
    zbuf = (ULONG) context->zbuffer.data;
    ui = (FLOAT) 0.0;
    vi = (FLOAT) 0.0;
    du = (FLOAT) 1.0;
    dv = (FLOAT) 0.0;
    if (context->states & M3D_TEXNORMCRD) {
      scale = 65536.0 * 256.0;
    } else {
      scale = 65536.0 * 256.0 / texture->width;
    }
    for (y = 0;y < texture->height;y++) {
      // Start drawing
      maggie->destination = (APTR) dest;
      maggie->zbuffer = (APTR) zbuf;
      maggie->u_start = (LFIXED) (ui * scale);
      maggie->v_start = (LFIXED) (vi * scale);
      maggie->u_delta = (LFIXED) (du * scale);
      maggie->v_delta = (LFIXED) (dv * scale);
      WaitBlit();
      maggie->start_length = (UWORD) texture->width;
#if _USE_MAGGIE_ == 0
      M3D_EmulateMaggie();
#endif
      vi += 1.0;
      dest += context->drawregion.bpr;
      zbuf += context->zbuffer.bpr;
    }
    return M3D_SUCCESS;
  }
  return M3D_NOCONTEXT;
}

/** Return the physical bitmap address */
ULONG M3D_GetBitmapAddress(struct BitMap *bitmap)
{
  APTR cgx_handle = NULL;
  ULONG memory;

  memory = 0;
  if (GetCyberMapAttr(bitmap, CYBRMATTR_ISCYBERGFX)) {
    cgx_handle = LockBitMapTags(bitmap, LBMI_BASEADDRESS, &memory, TAG_DONE);
    if (cgx_handle != NULL) {
      UnLockBitMap(cgx_handle);
    }
  }
  return memory;
}

/** Lock the hardware before drawing */
LONG M3D_LockHardware(M3D_Context *context)
{
  if (context != NULL) {
    if (context->drawregion.bitmap != NULL) {
      context->drawregion.data = (APTR) M3D_GetBitmapAddress(context->drawregion.bitmap);
      DDbug(printf("[DEBUG] Hardware locked with buffer address 0x%X\n", context->drawregion.data);)
      OwnBlitter();
      return M3D_SUCCESS;
    }
    return M3D_NOBITMAP;
  }
  return M3D_NOCONTEXT;
}

/** Unlock the hardware */
VOID M3D_UnlockHardware(M3D_Context *context)
{
  if (context != NULL) {
    DisownBlitter();
    DDbug(printf("[DEBUG] Hardware unlocked\n");)
  }
}
