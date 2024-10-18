/**
 * draw.c
 *
 * Maggie3D shared library
 * Drawing functions
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.6 June 2024 (updated: 24/06/2024)
 */

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <cybergraphx/cybergraphics.h>

#include <proto/exec.h>
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

VOID M3D_DrawDebug(BOOL flag)
{
  draw_debug = flag;
}

#if _ACTIVATE_DEBUG_ == 1

#define SFIXEDTOFLOAT(x)      ((FLOAT)(x) / 32768.0)
#define UFIXEDTOFLOAT(x)      ((FLOAT)(x) / 65535.0)
#define LFIXEDTOFLOAT(x)      ((FLOAT)(x) / 65536.0)

VOID M3D_DumpTriangle(M3D_Triangle *triangle)
{
  kprintf("[MAGGIE3D] Dump triangle\n");
  /*printf("[MAGGIE3D] => x1=%f  y1=%f  z1=%f  u1=%f  v1=%f  light=%f\n",
      triangle->v1.x, triangle->v1.y, triangle->v1.z, triangle->v1.u, triangle->v1.v, triangle->v1.light);
  printf("[MAGGIE3D] => x2=%f  y2=%f  z2=%f  u2=%f  v2=%f  light=%f\n",
      triangle->v2.x, triangle->v2.y, triangle->v2.z, triangle->v2.u, triangle->v2.v, triangle->v2.light);
  printf("[MAGGIE3D] => x3=%f  y3=%f  z3=%f  u3=%f  v3=%f  light=%f\n",
      triangle->v3.x, triangle->v3.y, triangle->v3.z, triangle->v3.u, triangle->v3.v, triangle->v3.light);
  printf("[MAGGIE3D] => texw=%d  texh=%d  texs=%d  texdata=0x%X  color=0x%X\n",
      triangle->texture->width, triangle->texture->height, triangle->texture->mipsize, triangle->texture->data, triangle->color);*/
  kprintf("[MAGGIE3D] !!! NOT AVAILABLE !!!\n");
}

VOID M3D_DumpQuad(M3D_Quad *quad)
{
  kprintf("[MAGGIE3D] Dump quad\n");
  /*printf("[MAGGIE3D] => x1=%f  y1=%f  z1=%f  u1=%f  v1=%f  light=%f\n",
      quad->v1.x, quad->v1.y, quad->v1.z, quad->v1.u, quad->v1.v, quad->v1.light);
  printf("[MAGGIE3D] => x2=%f  y2=%f  z2=%f  u2=%f  v2=%f  light=%f\n",
      quad->v2.x, quad->v2.y, quad->v2.z, quad->v2.u, quad->v2.v, quad->v2.light);
  printf("[MAGGIE3D] => x3=%f  y3=%f  z3=%f  u3=%f  v3=%f  light=%f\n",
      quad->v3.x, quad->v3.y, quad->v3.z, quad->v3.u, quad->v3.v, quad->v3.light);
  printf("[MAGGIE3D] => x4=%f  y4=%f  z4=%f  u4=%f  v4=%f  light=%f\n",
      quad->v4.x, quad->v4.y, quad->v4.z, quad->v4.u, quad->v4.v, quad->v4.light);
  printf("[MAGGIE3D] => texw=%d  texh=%d  texs=%d  texdata=0x%X  color=0x%X\n",
      quad->texture->width, quad->texture->height, quad->texture->mipsize, quad->texture->data, quad->color);*/
  kprintf("[MAGGIE3D] !!! NOT AVAILABLE !!!\n");
}

VOID M3D_DumpSprite(M3D_Sprite *sprite)
{
  kprintf("[MAGGIE3D] Dump sprite\n");
  /*printf("[MAGGIE3D] => left=%d  top=%d  width=%d  height=%d\n",
      sprite->left, sprite->top, sprite->width, sprite->height);
  printf("[MAGGIE3D] => x_zoom=%f  y_zoom=%f  angle=%f\n",
      sprite->x_zoom, sprite->y_zoom, sprite->angle);
  printf("[MAGGIE3D] => x_flip=%d  y_flip=%d  color=0x%X  light=%f\n",
      sprite->x_flip, sprite->y_flip, sprite->color, sprite->light);
  printf("[MAGGIE3D] => texw=%d  texh=%d  texs=%d  texdata=0x%X\n",
      sprite->texture->width, sprite->texture->height, sprite->texture->mipsize, sprite->texture->data, sprite->color);*/
  kprintf("[MAGGIE3D] !!! NOT AVAILABLE !!!\n");
}

VOID M3D_DumpDrawData(M3D_DrawData *draw_data)
{
  kprintf("[MAGGIE3D] Dump draw data\n");
  /*printf("[MAGGIE3D] => Clipping : left=%f  top=%f  right=%f  bottom=%f\n",
      draw_data->left_clip, draw_data->top_clip, draw_data->right_clip, draw_data->bottom_clip);
  printf("[MAGGIE3D] => Left delta : dxdyl=%f  dzdyl=%f  dudyl=%f  dvdyl=%f  dldyl=%f\n", 
      draw_data->delta_dxdyl, draw_data->delta_dzdyl, draw_data->delta_dudyl, draw_data->delta_dvdyl, draw_data->delta_dldyl);
  printf("[MAGGIE3D] => Right delta : dxdyr=%f  dzdyr=%f  dudyr=%f  dvdyr=%f  dldyr=%f\n", 
      draw_data->delta_dxdyr, draw_data->delta_dzdyr, draw_data->delta_dudyr, draw_data->delta_dvdyr, draw_data->delta_dldyr);
  printf("[MAGGIE3D] => X & Z coords : xl=%f  xr=%f  zl=%f  zr=%f\n",
      draw_data->crd_xl, draw_data->crd_xr, draw_data->crd_zl, draw_data->crd_zr);
  printf("[MAGGIE3D] => U & V coords : ul=%f  ur=%f  vl=%f  vr=%f\n",
      draw_data->crd_ul, draw_data->crd_ur, draw_data->crd_vl, draw_data->crd_vr);
  printf("[MAGGIE3D] => Light intensity : ll=%f  lr=%f\n",
      draw_data->int_ll, draw_data->int_lr);
  printf("[MAGGIE3D] => Draw : adr=0x%X  bpr=%d  bpp=%d\n",
      draw_data->dest_adr, draw_data->dest_bpr, draw_data->dest_bpp);
  printf("[MAGGIE3D] => ZBuffer : adr=0x%X  bpr=%d  bpp=%d\n",
      draw_data->zbuf_adr, draw_data->zbuf_bpr, draw_data->zbuf_bpp);*/
  kprintf("[MAGGIE3D] !!! NOT AVAILABLE !!!\n");
}

VOID M3D_DumpMaggieRegs(VOID)
{
  kprintf("[MAGGIE3D] Dump Maggie registers\n");
  /*printf("[MAGGIE3D] => texture=0x%X  destination=0x%X  zbuffer=0x%X\n", maggie->texture, maggie->destination, maggie->zbuffer);
  printf("[MAGGIE3D] => start_length=%d  tex_size=%d  mode=%d  modulo=%d\n", maggie->start_length, maggie->tex_size, maggie->mode, maggie->modulo);
  printf("[MAGGIE3D] => u_start=0x%X (%f)  v_start=0x%X (%f)  u_delta=0x%X (%f)  v_delta=0x%X (%f)\n",
    maggie->u_start, LFIXEDTOFLOAT(maggie->u_start), maggie->v_start, LFIXEDTOFLOAT(maggie->v_start),
    maggie->u_delta, LFIXEDTOFLOAT(maggie->u_delta), maggie->v_delta, LFIXEDTOFLOAT(maggie->v_delta));
  printf("[MAGGIE3D] => light_start=0x%X (%f)  light_delta=0x%X (%f)  color=0x%X\n",
    maggie->light_start, UFIXEDTOFLOAT(maggie->light_start), maggie->light_delta, SFIXEDTOFLOAT(maggie->light_delta), maggie->color);
  printf("[MAGGIE3D] => z_start=0x%X (%f)  z_delta=0x%X (%f)\n",
    maggie->z_start, LFIXEDTOFLOAT(maggie->z_start), maggie->z_delta, LFIXEDTOFLOAT(maggie->z_delta));*/
  kprintf("[MAGGIE3D] !!! NOT AVAILABLE !!!\n");
}

#endif

/*****************************************************************************/

#if _USE_MAGGIE_ == 0
/** Output 16bits texel */
VOID M3D_Maggie16Bits(UWORD *dest, ULONG texel)
{
  *dest = (UWORD) (((texel & 0xf80000) >> 8) | ((texel & 0xfc00) >> 5) | ((texel & 0xf8) >> 3));
}

/** Output 24bits texel */
VOID M3D_Maggie24Bits(UBYTE *dest, ULONG texel)
{
  *dest++ = (UBYTE) ((texel >> 16) & 0xff);
  *dest++ = (UBYTE) ((texel >> 8) & 0xff);
  *dest = (UBYTE) (texel & 0xff);
}

/** Output 32bits texel */
VOID M3D_Maggie32Bits(ULONG *dest, ULONG texel)
{
  *dest = texel;
}

/** Apply color blend */
ULONG M3D_MaggieBlend(ULONG texel, UBYTE red, UBYTE green, UBYTE blue)
{
  ULONG tr, tg, tb;

  tr = (texel >> 16) & 0xff;
  tg = (texel >> 8) & 0xff;
  tb = texel & 0xff;
  tr = tr * red / 0xff;
  tg = tg * green / 0xff;
  tb = tb * blue / 0xff;
  return ((tr << 16) | (tg << 8) | tb);
}

/** Apply color shade */
ULONG M3D_MaggieShade(ULONG texel, UBYTE shade)
{
  ULONG tr, tg, tb;

  tr = (texel >> 16) & 0xff;
  tg = (texel >> 8) & 0xff;
  tb = texel & 0xff;
  tr = tr * shade / 0xff;
  tg = tg * shade / 0xff;
  tb = tb * shade / 0xff;
  return ((tr << 16) | (tg << 8) | tb);
}

/** Simple emulation of Maggie feature, assume texture is RGB */
VOID M3D_EmulateMaggie(VOID)
{
  UBYTE *destination, *texture, red, green ,blue;
  LFIXED ui, vi, zi;
  UFIXED li;
  ULONG tex_bpr, tex_scale, index, texel;
  UWORD *zbuffer, count;
  BOOL draw_texel;

  ui = maggie->u_start;
  vi = maggie->v_start;
  zi = maggie->z_start;
  li = maggie->light_start;
  red = (UBYTE) ((maggie->color >> 16) & 0xff);
  green = (UBYTE) ((maggie->color >> 8) & 0xff);
  blue = (UBYTE) (maggie->color & 0xff);
  destination = (UBYTE *) maggie->destination;
  zbuffer = (UWORD *) maggie->zbuffer;
  texture = (UBYTE *) maggie->texture;
  tex_bpr = 4 << maggie->tex_size;
  tex_scale = 16 + (8 - maggie->tex_size);
  count = maggie->start_length;
  while (count--) {
    if (texture == NULL) {
      texel = 0xffffffff;
    } else {
      index = ((vi >> tex_scale) * tex_bpr) + ((ui >> tex_scale) * 4);
      texel = (texture[index] << 24) | (texture[index+1] << 16) | (texture[index+2] << 8) | texture[index+3];
    }
    if ((texel & 0xff) >= 0x80) {
      draw_texel = TRUE;
      if (maggie->mode & M3D_M_ZBUFFER) {
        if ((zi >> 16) > *zbuffer) {
          draw_texel = FALSE;
        } else {
          *zbuffer = (zi >> 16);
        }
      }
      if (draw_texel) {
        texel >>= 8;
        texel = M3D_MaggieBlend(texel, red, green ,blue);
        texel = M3D_MaggieShade(texel, (UBYTE) (li >> 8));
        if (maggie->mode & M3D_M_16BITS) {
          M3D_Maggie16Bits((UWORD *) destination, texel);
        } else if (maggie->mode & M3D_M_24BITS) {
          M3D_Maggie24Bits(destination, texel);
        } else {
          M3D_Maggie32Bits((ULONG *) destination, texel);
        }
      }
    }
    destination += maggie->modulo;
    zbuffer++;
    ui += maggie->u_delta;
    vi += maggie->v_delta;
    zi += maggie->z_delta;
    li += maggie->light_delta;
  }
}
#endif

/*****************************************************************************/

/** Swap two vertices */
VOID M3D_SwapVertex(M3D_Vertex *va, M3D_Vertex *vb)
{
  M3D_Vertex vt;
  
  DDbug(kprintf("[MAGGIE3D] M3D_SwapVertex\n");)
  CopyMem(va, &vt, sizeof(M3D_Vertex));
  CopyMem(vb, va, sizeof(M3D_Vertex));
  CopyMem(&vt, vb, sizeof(M3D_Vertex));
}

/** Check the triangle type & order the vertices */
ULONG M3D_CheckTriangleType(M3D_Context *context, M3D_Triangle *triangle, M3D_DrawData *draw_data)
{
  // Let's start with rounded values
  M3D_FastRoundTriangle(triangle);
  // Degenerated triangle elimination
  if (triangle->v1.x == triangle->v2.x && triangle->v2.x == triangle->v3.x) {
    DDbug(kprintf("[MAGGIE3D] - Rejected because x1=x2=x3\n");)
    return TRI_REJECTED;
  }
  if (triangle->v1.y == triangle->v2.y && triangle->v2.y == triangle->v3.y) {
    DDbug(kprintf("[MAGGIE3D] - Rejected because y1=y2=y3\n");)
    return TRI_REJECTED;
  }
  // Order triangle vertices from top to bottom
  if (triangle->v1.y > triangle->v3.y) {
    M3D_SwapVertex(&(triangle->v1), &(triangle->v3));
  }
  if (triangle->v1.y > triangle->v2.y) {
    M3D_SwapVertex(&(triangle->v1), &(triangle->v2));
  }
  if (triangle->v2.y > triangle->v3.y) {
    M3D_SwapVertex(&(triangle->v2), &(triangle->v3));
  }
  DDbug(kprintf("[MAGGIE3D] - Triangle ordered\n");)
  DDbug(M3D_DumpTriangle(triangle);)
  // Trivial rejection
  if (triangle->v3.y < draw_data->top_clip || triangle->v1.y > draw_data->bottom_clip) {
    DDbug(kprintf("[MAGGIE3D] - Rejected because y3 < top_clip or y1 > bottom_clip\n");)
    return TRI_REJECTED;
  }
  if (triangle->v1.x < draw_data->left_clip && triangle->v2.x < draw_data->left_clip && triangle->v3.x < draw_data->left_clip) {
    DDbug(kprintf("[MAGGIE3D] - Rejected because x1,x2 & x3 < left_clip\n");)
    return TRI_REJECTED;
  }
  if (triangle->v1.x > draw_data->right_clip && triangle->v2.x > draw_data->right_clip && triangle->v3.x > draw_data->right_clip) {
    DDbug(kprintf("[MAGGIE3D] - Rejected because x1,x2 & x3 > right_clip\n");)
    return TRI_REJECTED;
  }
  // Check the type & order from left to right
  if (triangle->v1.y == triangle->v2.y) {
    if (triangle->v1.x > triangle->v2.x) {
      M3D_SwapVertex(&(triangle->v1), &(triangle->v2));
    }
    return TRI_FLATTOP;
  } else if (triangle->v2.y == triangle->v3.y) {
    if (triangle->v2.x > triangle->v3.x) {
      M3D_SwapVertex(&(triangle->v2), &(triangle->v3));
    }
    return TRI_FLATBOTTOM;
  }
  return TRI_GENERIC;
}

/** Check the quad type & order the vertices */
ULONG M3D_CheckQuadType(M3D_Context *context, M3D_Quad *quad, M3D_DrawData *draw_data)
{
  ULONG type, minv, vertex_size;
  FLOAT miny, vec1, vec2;
  M3D_Vertex tmp_vx;

  // Let's start with rounded values
  M3D_FastRoundQuad(quad);
  // Degenerated quad elimination
  if (quad->v1.x == quad->v2.x && quad->v2.x == quad->v3.x && quad->v3.x == quad->v4.x) {
    DDbug(kprintf("[MAGGIE3D] - Rejected because x1=x2=x3=x4\n");)
    return QUAD_REJECTED;
  }
  if (quad->v1.y == quad->v2.y && quad->v2.y == quad->v3.y && quad->v3.y == quad->v4.y) {
    DDbug(kprintf("[MAGGIE3D] - Rejected because y1=y2=y3=y4\n");)
    return QUAD_REJECTED;
  }
  // Trivial rejection
  if (quad->v1.x < draw_data->left_clip && quad->v2.x < draw_data->left_clip && quad->v3.x < draw_data->left_clip && quad->v4.x < draw_data->left_clip) {
    DDbug(kprintf("[MAGGIE3D] - Rejected because x1,x2,x3 & x4 < left_clip\n");)
    return QUAD_REJECTED;
  }
  if (quad->v1.x >= draw_data->right_clip && quad->v2.x >= draw_data->right_clip && quad->v3.x >= draw_data->right_clip && quad->v4.x >= draw_data->right_clip) {
    DDbug(kprintf("[MAGGIE3D] - Rejected because x1,x2,x3 & x4 >= right_clip\n");)
    return QUAD_REJECTED;
  }
  if (quad->v1.y < draw_data->top_clip && quad->v2.y < draw_data->top_clip && quad->v3.y < draw_data->top_clip && quad->v4.y < draw_data->top_clip) {
    DDbug(kprintf("[MAGGIE3D] - Rejected because y1,y2,y3 & y4 < top_clip\n");)
    return QUAD_REJECTED;
  }
  // Order in clock wise
  vec1 = (quad->v1.y - quad->v2.y) * (quad->v2.x - quad->v4.x);
  vec2 = (quad->v2.y - quad->v4.y) * (quad->v1.x - quad->v2.x);
  if (vec1 > vec2) {
    M3D_SwapVertex(&(quad->v2), &(quad->v4));
  }
  DDbug(kprintf("[MAGGIE3D] - Quad in clock wise\n");)
  DDbug(M3D_DumpQuad(quad);)
  // Order quad vertices from top to bottom
  minv = 1;
  miny = quad->v1.y;
  if (quad->v2.y < miny) { minv = 2; miny = quad->v2.y; }
  if (quad->v3.y < miny) { minv = 3; miny = quad->v3.y; }
  if (quad->v4.y < miny) { minv = 4; miny = quad->v4.y; }
  DDbug(kprintf("[MAGGIE3D] - Min Y=%f (vertex %d)\n", miny, minv);)
  if (miny >= draw_data->bottom_clip) {
    DDbug(kprintf("[MAGGIE3D] - Rejected because y1,y2,y3 & y4 >= bottom_clip\n");)
    return QUAD_REJECTED;
  }
  vertex_size = sizeof(M3D_Vertex);
  if (minv == 2) {
    CopyMem(&(quad->v1), &tmp_vx, vertex_size);
    CopyMem(&(quad->v2), &(quad->v1), vertex_size);
    CopyMem(&(quad->v3), &(quad->v2), vertex_size);
    CopyMem(&(quad->v4), &(quad->v3), vertex_size);
    CopyMem(&tmp_vx, &(quad->v4), vertex_size);
  } else if (minv == 3) {
    CopyMem(&(quad->v1), &tmp_vx, vertex_size);
    CopyMem(&(quad->v3), &(quad->v1), vertex_size);
    CopyMem(&tmp_vx, &(quad->v3), vertex_size);
    CopyMem(&(quad->v2), &tmp_vx, vertex_size);
    CopyMem(&(quad->v4), &(quad->v2), vertex_size);
    CopyMem(&tmp_vx, &(quad->v4), vertex_size);
  } else if (minv == 4) {
    CopyMem(&(quad->v4), &tmp_vx, vertex_size);
    CopyMem(&(quad->v3), &(quad->v4), vertex_size);
    CopyMem(&(quad->v2), &(quad->v3), vertex_size);
    CopyMem(&(quad->v1), &(quad->v2), vertex_size);
    CopyMem(&tmp_vx, &(quad->v1), vertex_size);
  }
  DDbug(kprintf("[MAGGIE3D] - Quad ordered\n");)
  DDbug(M3D_DumpQuad(quad);)
  type = QUAD_GENERIC;
  // Check the type & order vertices
  if (quad->v1.y == quad->v2.y) {
    type |= QUAD_FLATTOP;
  } else if (quad->v1.y == quad->v4.y) {
    CopyMem(&(quad->v4), &tmp_vx, vertex_size);
    CopyMem(&(quad->v3), &(quad->v4), vertex_size);
    CopyMem(&(quad->v2), &(quad->v3), vertex_size);
    CopyMem(&(quad->v1), &(quad->v2), vertex_size);
    CopyMem(&tmp_vx, &(quad->v1), vertex_size);
    type |= QUAD_FLATTOP;
  }
  if (quad->v2.y == quad->v3.y) {
    type |= QUAD_FLATBOTTOM;
  } else if (quad->v3.y == quad->v4.y) {
    type |= QUAD_FLATBOTTOM;
  }
  DDbug(M3D_DumpQuad(quad);)
  return type;
}

/*****************************************************************************/
/**                  DRAW SHADED TRIANGLE                                    */
/*****************************************************************************/

/** Draw a shaded triangle */
VOID M3D_DrawShadedTriangle(M3D_Context *context, M3D_Triangle *triangle, M3D_DrawData *draw_data, ULONG type)
{
  // Setup Maggie registers
  maggie->texture = (APTR) context->flat_shading;;
  maggie->tex_size = M3D_TEX64;
  maggie->color = triangle->color;
  // Render the triangle depending on his type
  if (type == TRI_FLATTOP) {
    if (context->states & M3D_GOURAUD) {
      M3D_DrawGouraudShadedTop(context, triangle, draw_data);
    } else {
      M3D_DrawFlatShadedTop(context, triangle, draw_data);
    }
  } else if (type == TRI_FLATBOTTOM) {
    if (context->states & M3D_GOURAUD) {
      M3D_DrawGouraudShadedBottom(context, triangle, draw_data);
    } else {
      M3D_DrawFlatShadedBottom(context, triangle, draw_data);
    }
  } else if (type == TRI_GENERIC) {
    if (context->states & M3D_GOURAUD) {
      M3D_DrawGouraudShadedGeneric(context, triangle, draw_data);
    } else {
      M3D_DrawFlatShadedGeneric(context, triangle, draw_data);
    }
  }
}

/*****************************************************************************/
/**                      DRAW SHADED QUAD                                    */
/*****************************************************************************/

/** Draw a shaded quad */
VOID M3D_DrawShadedQuad(M3D_Context *context, M3D_Quad *quad, M3D_DrawData *draw_data, ULONG type)
{
  // Setup Maggie registers
  maggie->texture = (APTR) context->flat_shading;;
  maggie->tex_size = M3D_TEX64;
  maggie->color = quad->color;
  // Render the quad depending on his type
  if (type == QUAD_FLATTOP) {
    DDbug(kprintf("[MAGGIE3D] M3D_DrawQuadFlatTop\n");)
    if (context->states & M3D_GOURAUD) {
      M3D_DrawQuadGouraudShadedTop(context, quad, draw_data);
    } else {
      M3D_DrawQuadFlatShadedTop(context, quad, draw_data);
    }
  } else if (type == QUAD_FLATBOTTOM) {
    DDbug(kprintf("[MAGGIE3D] M3D_DrawQuadFlatBottom\n");)
    if (context->states & M3D_GOURAUD) {
      M3D_DrawQuadGouraudShadedBottom(context, quad, draw_data);
    } else {
      M3D_DrawQuadFlatShadedBottom(context, quad, draw_data);
    }
  } else if (type == QUAD_FLATBOTH) {
    DDbug(kprintf("[MAGGIE3D] M3D_DrawQuadFlatBoth\n");)
    if (context->states & M3D_GOURAUD) {
      M3D_DrawQuadGouraudShadedBoth(context, quad, draw_data);
    } else {
      M3D_DrawQuadFlatShadedBoth(context, quad, draw_data);
    }
  } else if (type == QUAD_GENERIC) {
    DDbug(kprintf("[MAGGIE3D] M3D_DrawQuadGeneric\n");)
    if (context->states & M3D_GOURAUD) {
      M3D_DrawQuadGouraudShadedGeneric(context, quad, draw_data);
    } else {
      M3D_DrawQuadFlatShadedGeneric(context, quad, draw_data);
    }
  }
}

/*****************************************************************************/
/**                DRAW TEXTURED TRIANGLE                                    */
/*****************************************************************************/

/** Draw a textured triangle */
VOID M3D_DrawTexturedTriangle(M3D_Context *context, M3D_Triangle *triangle, M3D_DrawData *draw_data, ULONG type)
{
  // Setup Maggie registers
  if (triangle->texture->filtering == M3D_LINEAR) {
    maggie->mode = context->mode | M3D_M_BILINEAR;
  }
  maggie->texture = triangle->texture->data;
  maggie->tex_size = triangle->texture->mipsize;
  if (context->states & M3D_BLENDING) {
    maggie->color = triangle->color;
  } else {
    maggie->color = 0xffffff;
  }
  // Setup texture scale
  if (context->states & M3D_TEXCRDNORM) {
    draw_data->scale = 65536.0 * 256.0;
  } else {
    draw_data->scale = 65536.0 * 256.0 / triangle->texture->width;
  }
  // Render the triangle depending on his type
  if (type == TRI_FLATTOP) {
    if (context->states & M3D_GOURAUD) {
      M3D_DrawGouraudTexturedTop(context, triangle, draw_data);
    } else {
      M3D_DrawFlatTexturedTop(context, triangle, draw_data);
    }
  } else if (type == TRI_FLATBOTTOM) {
    if (context->states & M3D_GOURAUD) {
      M3D_DrawGouraudTexturedBottom(context, triangle, draw_data);
    } else {
      M3D_DrawFlatTexturedBottom(context, triangle, draw_data);
    }
  } else if (type == TRI_GENERIC) {
    if (context->states & M3D_GOURAUD) {
      M3D_DrawGouraudTexturedGeneric(context, triangle, draw_data);
    } else {
      M3D_DrawFlatTexturedGeneric(context, triangle, draw_data);
    }
  }
}

/*****************************************************************************/
/**                    DRAW TEXTURED QUAD                                    */
/*****************************************************************************/

/** Draw a textured quad */
VOID M3D_DrawTexturedQuad(M3D_Context *context, M3D_Quad *quad, M3D_DrawData *draw_data, ULONG type)
{
  // Setup Maggie registers
  if (quad->texture->filtering == M3D_LINEAR) {
    maggie->mode = context->mode | M3D_M_BILINEAR;
  }
  maggie->texture = quad->texture->data;
  maggie->tex_size = quad->texture->mipsize;
  if (context->states & M3D_BLENDING) {
    maggie->color = quad->color;
  } else {
    maggie->color = 0xffffff;
  }
  // Setup texture scale
  if (context->states & M3D_TEXCRDNORM) {
    draw_data->scale = 65536.0 * 256.0;
  } else {
    draw_data->scale = 65536.0 * 256.0 / quad->texture->width;
  }
  // Render the quad depending on his type
  if (type == QUAD_FLATTOP) {
    DDbug(kprintf("[MAGGIE3D] M3D_DrawTexturedFlatTopQuad\n");)
    if (context->states & M3D_GOURAUD) {
      M3D_DrawQuadGouraudTexturedTop(context, quad, draw_data);
    } else {
      M3D_DrawQuadFlatTexturedTop(context, quad, draw_data);
    }
  } else if (type == QUAD_FLATBOTTOM) {
    DDbug(kprintf("[MAGGIE3D] M3D_DrawTexturedFlatBottomQuad\n");)
    if (context->states & M3D_GOURAUD) {
      M3D_DrawQuadGouraudTexturedBottom(context, quad, draw_data);
    } else {
      M3D_DrawQuadFlatTexturedBottom(context, quad, draw_data);
    }
  } else if (type == QUAD_FLATBOTH) {
    DDbug(kprintf("[MAGGIE3D] M3D_DrawTexturedFlatBothQuad\n");)
    if (context->states & M3D_GOURAUD) {
      M3D_DrawQuadGouraudTexturedBoth(context, quad, draw_data);
    } else {
      M3D_DrawQuadFlatTexturedBoth(context, quad, draw_data);
    }
  } else if (type == QUAD_GENERIC) {
    DDbug(kprintf("[MAGGIE3D] M3D_DrawTexturedGenericQuad\n");)
    if (context->states & M3D_GOURAUD) {
      M3D_DrawQuadGouraudTexturedGeneric(context, quad, draw_data);
    } else {
      M3D_DrawQuadFlatTexturedGeneric(context, quad, draw_data);
    }
  }
}

/*****************************************************************************/
/**                      DRAW A TRIANGLE                                     */
/*****************************************************************************/

/** Draw a single triangle */
LONG __asm __saveds LIBM3D_DrawTriangle(register __a0 M3D_Context *context, register __a1 M3D_Triangle *triangle)
{
  M3D_DrawData draw_data;
  M3D_Triangle tri_copy;
  ULONG type;

  DDbug(kprintf("[MAGGIE3D] M3D_DrawTriangle\n");)
  if (context != NULL) {
    if (context->maggie_available) {
      maggie->mode = context->mode;
      maggie->modulo = context->drawregion.bpp;
      // Setup clip constants
      draw_data.left_clip = (FLOAT) context->clipping.left;
      draw_data.top_clip = (FLOAT) context->clipping.top;
      draw_data.right_clip = (FLOAT) (context->clipping.left + context->clipping.width);
      draw_data.bottom_clip = (FLOAT) (context->clipping.top + context->clipping.height);
      // Setup constants
      draw_data.dest_bpr = context->drawregion.bpr;
      draw_data.dest_bpp = context->drawregion.bpp;
      draw_data.zbuf_bpr = context->zbuffer.bpr;
      draw_data.zbuf_bpp = context->zbuffer.bpp;
      // If not in fast mode
      if (!(context->states & M3D_FAST)) {
        CopyMem(triangle, &tri_copy, sizeof(M3D_Triangle));
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
    }
    return M3D_NOMAGGIE;
  }
  return M3D_NOCONTEXT;
}

/** Draw an array of triangles */
LONG __asm __saveds LIBM3D_DrawTriangleArray(register __a0 M3D_Context *context, register __a1 M3D_Triangle *triangles, register __d0 ULONG count)
{
  M3D_DrawData draw_data;
  M3D_Triangle *triangle, tri_copy;
  ULONG type;
  UWORD index;

  DDbug(kprintf("[MAGGIE3D] M3D_DrawTriangleArray\n");)
  if (context != NULL) {
    if (context->maggie_available) {
      maggie->mode = context->mode;
      maggie->modulo = context->drawregion.bpp;
      // Setup clip constants
      draw_data.left_clip = (FLOAT) context->clipping.left;
      draw_data.top_clip = (FLOAT) context->clipping.top;
      draw_data.right_clip = (FLOAT) (context->clipping.left + context->clipping.width);
      draw_data.bottom_clip = (FLOAT) (context->clipping.top + context->clipping.height);
      // Setup constants
      draw_data.dest_bpr = context->drawregion.bpr;
      draw_data.dest_bpp = context->drawregion.bpp;
      draw_data.zbuf_bpr = context->zbuffer.bpr;
      draw_data.zbuf_bpp = context->zbuffer.bpp;
      for (index = 0;index < count;index++) {
        triangle = &(triangles[index]);
        // If not in fast mode
        if (!(context->states & M3D_FAST)) {
          CopyMem(triangle, &tri_copy, sizeof(M3D_Triangle));
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
    }
    return M3D_NOMAGGIE;
  }
  return M3D_NOCONTEXT;
}

/** Draw a list of triangles */
LONG __asm __saveds LIBM3D_DrawTriangleList(register __a0 M3D_Context *context, register __a1 M3D_Triangle **triangles, register __d0 ULONG count)
{
  M3D_DrawData draw_data;
  M3D_Triangle *triangle, tri_copy;
  ULONG type;
  UWORD index;

  DDbug(kprintf("[MAGGIE3D] M3D_DrawTriangleList\n");)
  if (context != NULL) {
    if (context->maggie_available) {
      maggie->mode = context->mode;
      maggie->modulo = context->drawregion.bpp;
      // Setup clip constants
      draw_data.left_clip = (FLOAT) context->clipping.left;
      draw_data.top_clip = (FLOAT) context->clipping.top;
      draw_data.right_clip = (FLOAT) (context->clipping.left + context->clipping.width);
      draw_data.bottom_clip = (FLOAT) (context->clipping.top + context->clipping.height);
      // Setup constants
      draw_data.dest_bpr = context->drawregion.bpr;
      draw_data.dest_bpp = context->drawregion.bpp;
      draw_data.zbuf_bpr = context->zbuffer.bpr;
      draw_data.zbuf_bpp = context->zbuffer.bpp;
      for (index = 0;index < count;index++) {
        triangle = triangles[index];
        // If not in fast mode
        if (!(context->states & M3D_FAST)) {
          CopyMem(triangle, &tri_copy, sizeof(M3D_Triangle));
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
    }
    return M3D_NOMAGGIE;
  }
  return M3D_NOCONTEXT;
}

/*****************************************************************************/
/**                        DRAW A QUAD                                       */
/*****************************************************************************/

/** Draw a single quad */
LONG __asm __saveds LIBM3D_DrawQuad(register __a0 M3D_Context *context, register __a1 M3D_Quad *quad)
{
  M3D_DrawData draw_data;
  M3D_Quad quad_copy;
  ULONG type;

  DDbug(kprintf("[MAGGIE3D] M3D_DrawQuad\n");)
  if (context != NULL) {
    if (context->maggie_available) {
      maggie->mode = context->mode;
      maggie->modulo = context->drawregion.bpp;
      // Setup clip constants
      draw_data.left_clip = (FLOAT) context->clipping.left;
      draw_data.top_clip = (FLOAT) context->clipping.top;
      draw_data.right_clip = (FLOAT) (context->clipping.left + context->clipping.width);
      draw_data.bottom_clip = (FLOAT) (context->clipping.top + context->clipping.height);
      // Setup constants
      draw_data.dest_bpr = context->drawregion.bpr;
      draw_data.dest_bpp = context->drawregion.bpp;
      draw_data.zbuf_bpr = context->zbuffer.bpr;
      draw_data.zbuf_bpp = context->zbuffer.bpp;
      // If not in fast mode
      if (!(context->states & M3D_FAST)) {
        CopyMem(quad, &quad_copy, sizeof(M3D_Quad));
        quad = &quad_copy;
      }
      // Check for quad type
      type = M3D_CheckQuadType(context, quad, &draw_data);
      if (type != QUAD_REJECTED) {
        if (context->states & M3D_TEXMAPPING && quad->texture != NULL) {
          M3D_DrawTexturedQuad(context, quad, &draw_data, type);
        } else {
          M3D_DrawShadedQuad(context, quad, &draw_data, type);
        }
        return M3D_SUCCESS;
      }
      return M3D_NOQUAD;
    }
    return M3D_NOMAGGIE;
  }
  return M3D_NOCONTEXT;
}

/*****************************************************************************/
/**                       DRAW A SPRITE                                      */
/*****************************************************************************/

/** Draw a normal sprite */
LONG M3D_DrawNormalSprite(M3D_Context *context, M3D_Sprite *sprite, LONG xpos, LONG ypos)
{
  FLOAT scale, ui, vi, du, dv;
  LONG clip_left, clip_top, clip_right, clip_bottom, dx, dy;
  ULONG dest;

  clip_left = context->clipping.left;
  clip_top = context->clipping.top;
  clip_right = context->clipping.left + context->clipping.width;
  clip_bottom = context->clipping.top + context->clipping.height;
  DDbug(kprintf("[MAGGIE3D] Sprite clipping %ld,%ld -> %ld,%ld\n", clip_left, clip_top, clip_right, clip_bottom);)
  // Apply zoom factor
  dx = sprite->width * sprite->x_zoom;
  dy = sprite->height * sprite->y_zoom;
  // Check if sprite is in the clipping region
  if (xpos >= clip_right || ypos >= clip_bottom || (xpos + dx) < clip_left || (ypos + dy) < clip_top) {
    DDbug(kprintf("[MAGGIE3D] Sprite out of clipping region\n");)
    return M3D_SUCCESS;
  }
  // Setup Maggie registers
  if (sprite->texture->filtering == M3D_LINEAR) {
    maggie->mode = (context->mode | M3D_M_BILINEAR) & ~M3D_M_ZBUFFER;
  } else {
    maggie->mode = context->mode & ~M3D_M_ZBUFFER;
  }
  maggie->modulo = context->drawregion.bpp;
  maggie->texture = sprite->texture->data;
  maggie->tex_size = sprite->texture->mipsize;
  maggie->color = sprite->color;
  maggie->light_start = (UFIXED) (sprite->light * 65535.0);
  maggie->light_delta = 0;
  maggie->z_start = 0;
  maggie->z_delta = 0;
  maggie->zbuffer = NULL;
  // Texture scaling
  scale = 65536.0 * 256.0 / sprite->texture->width;
  // Manage sprite flipping
  if (sprite->x_flip) {
    ui = (FLOAT)(sprite->left + sprite->width);
    du = -1.0 * (FLOAT)sprite->width / dx;
  } else {
    ui = (FLOAT)sprite->left;
    du = (FLOAT)sprite->width / dx;
  }
  if (sprite->y_flip) {
    vi = (FLOAT)(sprite->top + sprite->height);
    dv = -1.0 * (FLOAT)sprite->height / dy;
  } else {
    vi = (FLOAT)sprite->top;
    dv = (FLOAT)sprite->height / dy;
  }
  // Check for clipping
  if (xpos < clip_left) {
    ui += (clip_left - xpos) * du;
    dx -= (clip_left - xpos);
    xpos = clip_left;
  }
  if ((xpos + dx) >= clip_right) {
    dx = clip_right - xpos;
  }
  if (ypos < clip_top) {
    vi += (clip_top - ypos) * dv;
    dy -= (clip_top - ypos);
    ypos = clip_top;
  }
  if ((ypos + dy) >= clip_bottom) {
    dy = clip_bottom - ypos;
  }
  // Destination address
  dest = (ULONG)context->drawregion.data + (context->drawregion.bpr * ypos) + (xpos * context->drawregion.bpp);
  // Draw the sprite
  while (dy--) {
    maggie->destination = (APTR) dest;
    maggie->u_start = (LFIXED) (ui * scale);
    maggie->v_start = (LFIXED) (vi * scale);
    maggie->u_delta = (LFIXED) (du * scale);
    maggie->v_delta = 0;
    WaitBlit();
    maggie->start_length = dx;
#if _USE_MAGGIE_ == 0
    M3D_EmulateMaggie();
#endif
    vi += dv;
    dest += context->drawregion.bpr;
  }
  return M3D_SUCCESS;
}

/** Rotate the quad */
VOID M3D_RotateQuad(M3D_Quad *quad, FLOAT angle)
{
  FLOAT center_x, center_y, vertex_x, vertex_y, sinus, cosinus;
  
  sinus = sin(RAD(angle));
  cosinus = cos(RAD(angle));
  center_x = (quad->v2.x - quad->v1.x) / 2.0F + quad->v1.x;
  center_y = (quad->v4.y - quad->v1.y) / 2.0F + quad->v1.y;
  vertex_x = quad->v1.x - center_x;
  vertex_y = quad->v1.y - center_y;
  quad->v1.x = (vertex_x * cosinus - vertex_y * sinus) + center_x;
  quad->v1.y = (vertex_x * sinus + vertex_y * cosinus) + center_y;
  vertex_x = quad->v2.x - center_x;
  vertex_y = quad->v2.y - center_y;
  quad->v2.x = (vertex_x * cosinus - vertex_y * sinus) + center_x;
  quad->v2.y = (vertex_x * sinus + vertex_y * cosinus) + center_y;
  vertex_x = quad->v3.x - center_x;
  vertex_y = quad->v3.y - center_y;
  quad->v3.x = (vertex_x * cosinus - vertex_y * sinus) + center_x;
  quad->v3.y = (vertex_x * sinus + vertex_y * cosinus) + center_y;
  vertex_x = quad->v4.x - center_x;
  vertex_y = quad->v4.y - center_y;
  quad->v4.x = (vertex_x * cosinus - vertex_y * sinus) + center_x;
  quad->v4.y = (vertex_x * sinus + vertex_y * cosinus) + center_y;
}

/** Draw a rotated sprite */
LONG M3D_DrawRotatedSprite(M3D_Context *context, M3D_Sprite *sprite, LONG xpos, LONG ypos)
{
  M3D_DrawData draw_data;
  M3D_Quad quad;
  ULONG type, dx, dy;

  // Setup clip constants
  draw_data.left_clip = (FLOAT) context->clipping.left;
  draw_data.top_clip = (FLOAT) context->clipping.top;
  draw_data.right_clip = (FLOAT) (context->clipping.left + context->clipping.width);
  draw_data.bottom_clip = (FLOAT) (context->clipping.top + context->clipping.height);
  // Apply zoom factor
  dx = sprite->width * sprite->x_zoom;
  dy = sprite->height * sprite->y_zoom;
  // Setup constants
  draw_data.dest_bpr = context->drawregion.bpr;
  draw_data.dest_bpp = context->drawregion.bpp;
  draw_data.zbuf_bpr = context->zbuffer.bpr;
  draw_data.zbuf_bpp = context->zbuffer.bpp;
  // Setup quad with sprite info
  quad.v1.x = xpos;
  quad.v1.y = ypos;
  quad.v1.z = quad.v1.w = 1.0F;
  quad.v1.light = sprite->light;
  quad.v2.x = xpos + dx;
  quad.v2.y = ypos;
  quad.v2.z = quad.v2.w = 1.0F;
  quad.v2.light = sprite->light;
  quad.v3.x = xpos + dx;
  quad.v3.y = ypos + dy;
  quad.v3.z = quad.v3.w = 1.0F;
  quad.v3.light = sprite->light;
  quad.v4.x = xpos;
  quad.v4.y = ypos + dy;
  quad.v4.z = quad.v4.w = 1.0F;
  quad.v4.light = sprite->light;
  quad.texture = sprite->texture;
  quad.color = sprite->color;
  // Rotate the quad
  M3D_RotateQuad(&quad, sprite->angle);
  // Set the texture coords
  if (sprite->x_flip && !sprite->y_flip) {
    quad.v1.u = sprite->left + sprite->width;
    quad.v1.v = sprite->top;
    quad.v2.u = sprite->left;
    quad.v2.v = sprite->top;
    quad.v3.u = sprite->left;
    quad.v3.v = sprite->top + sprite->height;
    quad.v4.u = sprite->left + sprite->width;
    quad.v4.v = sprite->top + sprite->height;
  } else if (!sprite->x_flip && sprite->y_flip) {
    quad.v1.u = sprite->left;
    quad.v1.v = sprite->top + sprite->height;
    quad.v2.u = sprite->left + sprite->width;
    quad.v2.v = sprite->top + sprite->height;
    quad.v3.u = sprite->left + sprite->width;
    quad.v3.v = sprite->top;
    quad.v4.u = sprite->left;
    quad.v4.v = sprite->top;
  } else if (sprite->x_flip && sprite->y_flip) {
    quad.v1.u = sprite->left + sprite->width;
    quad.v1.v = sprite->top + sprite->height;
    quad.v2.u = sprite->left;
    quad.v2.v = sprite->top + sprite->height;
    quad.v3.u = sprite->left;
    quad.v3.v = sprite->top;
    quad.v4.u = sprite->left + sprite->width;
    quad.v4.v = sprite->top;
  } else {
    quad.v1.u = sprite->left;
    quad.v1.v = sprite->top;
    quad.v2.u = sprite->left + sprite->width;
    quad.v2.v = sprite->top;
    quad.v3.u = sprite->left + sprite->width;
    quad.v3.v = sprite->top + sprite->height;
    quad.v4.u = sprite->left;
    quad.v4.v = sprite->top + sprite->height;
  }
  DDbug(M3D_DumpQuad(&quad);)
  // Check for quad type
  type = M3D_CheckQuadType(context, &quad, &draw_data);
  if (type != QUAD_REJECTED) {
    // Setup Maggie registers
    if (sprite->texture->filtering == M3D_LINEAR) {
      maggie->mode = (context->mode | M3D_M_BILINEAR) & ~M3D_M_ZBUFFER;
    } else {
      maggie->mode = context->mode & ~M3D_M_ZBUFFER;
    }
    maggie->modulo = context->drawregion.bpp;
    maggie->texture = sprite->texture->data;
    maggie->tex_size = sprite->texture->mipsize;
    maggie->color = sprite->color;
    maggie->light_start = (UFIXED) (sprite->light * 65535.0);
    maggie->light_delta = 0;
    maggie->z_start = 0;
    maggie->z_delta = 0;
    maggie->zbuffer = NULL;
    // Setup texture scale
    draw_data.scale = 65536.0 * 256.0 / sprite->texture->width;
    // Render the quad depending on his type
    if (type == QUAD_FLATTOP) {
      M3D_DrawQuadFlatTexturedTop(context, &quad, &draw_data);
    } else if (type == QUAD_FLATBOTTOM) {
      M3D_DrawQuadFlatTexturedBottom(context, &quad, &draw_data);
    } else if (type == QUAD_FLATBOTH) {
      M3D_DrawQuadFlatTexturedBoth(context, &quad, &draw_data);
    } else if (type == QUAD_GENERIC) {
      M3D_DrawQuadFlatTexturedGeneric(context, &quad, &draw_data);
    }
    return M3D_SUCCESS;
  }
  return M3D_NOQUAD;
}

/** Draw a sprite */
LONG __asm __saveds LIBM3D_DrawSprite(register __a0 M3D_Context *context, register __a1 M3D_Sprite *sprite, register __d0 LONG xpos, register __d1 LONG ypos)
{
  DDbug(kprintf("[MAGGIE3D] M3D_DrawSprite at %ld , %ld\n", xpos, ypos);)
  DDbug(M3D_DumpSprite(sprite);)
  if (context != NULL) {
    if (context->maggie_available) {
      if (sprite->angle == 0.0) {
        return M3D_DrawNormalSprite(context, sprite, xpos, ypos);
      } else {
        return M3D_DrawRotatedSprite(context, sprite, xpos, ypos);
      }
    }
    return M3D_NOMAGGIE;
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
LONG __asm __saveds LIBM3D_LockHardware(register __a0 M3D_Context *context)
{
  if (context != NULL) {
    if (context->drawregion.bitmap != NULL) {
      context->drawregion.data = (APTR) M3D_GetBitmapAddress(context->drawregion.bitmap);
      DDbug(kprintf("[MAGGIE3D] Hardware locked with buffer address 0x%lx \n", context->drawregion.data);)
      OwnBlitter();
      return M3D_SUCCESS;
    }
    return M3D_NOBITMAP;
  }
  return M3D_NOCONTEXT;
}

/** Unlock the hardware */
VOID __asm __saveds LIBM3D_UnlockHardware(register __a0 M3D_Context *context)
{
  if (context != NULL) {
    DisownBlitter();
    DDbug(kprintf("[MAGGIE3D] Hardware unlocked\n");)
  }
}

/** Clear the draw region with specified color */
LONG __asm __saveds LIBM3D_ClearDrawRegion(register __a0 M3D_Context *context, register __d0 ULONG color)
{
  ULONG region;

  if (context != NULL) {
    if (context->drawregion.data != NULL) {
      region = (ULONG) context->drawregion.data;
      region += (context->clipping.left * context->drawregion.bpp) + (context->clipping.top * context->drawregion.bpr);
      DDbug(kprintf("[MAGGIE3D] Clear region %ld,%ld -> %ld,%ld (%ld)\n",
        context->clipping.left,context->clipping.top,context->clipping.width,context->clipping.height, context->drawregion.bpr
      );)
      if (context->drawregion.depth == 16) {
        M3D_FastClearRegion16((APTR)region, context->clipping.width, context->clipping.height, context->drawregion.bpr, color);
      } else if (context->drawregion.depth == 24) {
        M3D_FastClearRegion24((APTR)region, context->clipping.width, context->clipping.height, context->drawregion.bpr, color);
      } else if (context->drawregion.depth == 32) {
        M3D_FastClearRegion32((APTR)region, context->clipping.width, context->clipping.height, context->drawregion.bpr, color);
      }
    }
    return M3D_SUCCESS;
  }
  return M3D_NOCONTEXT;
}
