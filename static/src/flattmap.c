/**
 * flattmap.c
 *
 * Maggie3D static library
 * Texture mapping functions
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.6 June 2024 (updated: 15/06/2024)
 */

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <proto/graphics.h>

#include "debug.h"
#include "draw.h"

/** Maggie registers */
extern M3D_MaggieRegs *maggie;

#if _ACTIVATE_DEBUG_ == 1
extern BOOL draw_debug;
#endif

/*****************************************************************************/
/**                     FLAT MAPPING                                         */
/*****************************************************************************/

/** Map a flat shaded & textured figure with Maggie */
VOID M3D_FlatTextureMapping(UWORD nblines, M3D_DrawData *draw_data)
{
  FLOAT xs, xe, dx;
  FLOAT du, dv, dz;
  FLOAT ui, vi, zi;
  ULONG dest, zbuf;

  DDbug(printf("[MAGGIE3D] - Go flat shade mapping for %d lines\n", nblines);)
  DDbug(M3D_DumpDrawData(draw_data);)
  // Light will not change for flat shading
  maggie->light_start = (UFIXED) (draw_data->int_ll * 65535.0);
  maggie->light_delta = (SFIXED) 0;
  while (nblines--) {
    DDbug(printf("[MAGGIE3D] Render line %d\n", nblines);)
    // Calcul edge coords
    xs = floor(draw_data->crd_xl);
    xe = floor(draw_data->crd_xr);
    DDbug(printf("[MAGGIE3D] => xs=%f  xe=%f\n", xs, xe);)
    // Draw if line is not outside of clipping region
    if (xs < draw_data->right_clip && xe >= draw_data->left_clip && xs < xe) {
      // Calcul interpolations
      du = draw_data->crd_ur - draw_data->crd_ul;
      dv = draw_data->crd_vr - draw_data->crd_vl;
      dz = draw_data->crd_zr - draw_data->crd_zl;
      dx = xe - xs;
      du /= dx;
      dv /= dx;
      dz /= dx;
      // Calcul texture coords
      ui = draw_data->crd_ul;
      vi = draw_data->crd_vl;
      // Calcul Z value
      zi = draw_data->crd_zl;
      // Horizontal clipping
      if (xs < draw_data->left_clip) {
        DDbug(printf("[MAGGIE3D] => left clipping\n");)
        dx = draw_data->left_clip - xs;
        ui += dx * du;
        vi += dx * dv;
        zi += dx * dz;
        xs = draw_data->left_clip;
        dx = xe - xs;
      }
      if (xe >= draw_data->right_clip) {
        DDbug(printf("[MAGGIE3D] => right clipping\n");)
        dx = draw_data->right_clip - xs;
      }
      DDbug(printf("[MAGGIE3D] => xs=%f  ui=%f  vi=%f  zi=%f\n", xs, ui, vi, zi);)
      // Draw the line
      DDbug(printf("[MAGGIE3D] => dx=%f  du=%f  dv=%f  dz=%f\n", dx, du, dv, dz);)
      // Destination address
      dest = draw_data->dest_adr + ((LONG)xs * draw_data->dest_bpp);
      // Z buffer address
      zbuf = draw_data->zbuf_adr + ((LONG)xs * draw_data->zbuf_bpp);
      DDbug(printf("[MAGGIE3D] => dbuf=0x%X  zbuf=0x%X\n", dest, zbuf);)
      // Start drawing
      maggie->destination = (APTR) dest;
      maggie->zbuffer = (APTR) zbuf;
      maggie->u_start = (LFIXED) (ui * draw_data->scale);
      maggie->v_start = (LFIXED) (vi * draw_data->scale);
      maggie->u_delta = (LFIXED) (du * draw_data->scale);
      maggie->v_delta = (LFIXED) (dv * draw_data->scale);
      maggie->z_start = (LFIXED) (zi * 65536.0);
      maggie->z_delta = (LFIXED) (dz * 65536.0);
      WaitBlit();
      maggie->start_length = (UWORD) dx;
#if _USE_MAGGIE_ == 0
      M3D_EmulateMaggie();
#endif
      DDbug(printf("[MAGGIE3D] => Rendering %d texels\n", (UWORD) dx);)
    }
    // Interpolate next points
    draw_data->crd_xl += draw_data->delta_dxdyl;
    draw_data->crd_zl += draw_data->delta_dzdyl;
    draw_data->crd_ul += draw_data->delta_dudyl;
    draw_data->crd_vl += draw_data->delta_dvdyl;
    draw_data->crd_xr += draw_data->delta_dxdyr;
    draw_data->crd_zr += draw_data->delta_dzdyr;
    draw_data->crd_ur += draw_data->delta_dudyr;
    draw_data->crd_vr += draw_data->delta_dvdyr;
    // Next line address
    draw_data->dest_adr += draw_data->dest_bpr;
    draw_data->zbuf_adr += draw_data->zbuf_bpr;
  }
}

/*****************************************************************************/
/**                     TRIANGLE DRAW                                        */
/*****************************************************************************/

/** Draw a flat textured top triangle */
VOID M3D_DrawFlatTexturedTop(M3D_Context *context, M3D_Triangle *triangle, M3D_DrawData *draw_data)
{
  FLOAT delta_y, clip_y;

  DDbug(printf("[MAGGIE3D] M3D_DrawFlatTexturedTop\n");)
  // Delta height
  delta_y = triangle->v3.y - triangle->v1.y;
  // Left side delta
  draw_data->delta_dxdyl = (triangle->v3.x - triangle->v1.x) / delta_y;
  draw_data->delta_dzdyl = (triangle->v3.z - triangle->v1.z) / delta_y;
  draw_data->delta_dudyl = (triangle->v3.u - triangle->v1.u) / delta_y;
  draw_data->delta_dvdyl = (triangle->v3.v - triangle->v1.v) / delta_y;
  // Right side delta
  draw_data->delta_dxdyr = (triangle->v3.x - triangle->v2.x) / delta_y;
  draw_data->delta_dzdyr = (triangle->v3.z - triangle->v2.z) / delta_y;
  draw_data->delta_dudyr = (triangle->v3.u - triangle->v2.u) / delta_y;
  draw_data->delta_dvdyr = (triangle->v3.v - triangle->v2.v) / delta_y;
  // Start coords & clipping
  if (triangle->v1.y < draw_data->top_clip) {
    DDbug(printf("[MAGGIE3D] - Clipping top vertex\n");)
    clip_y = draw_data->top_clip - triangle->v1.y;
    draw_data->crd_xl = triangle->v1.x + (draw_data->delta_dxdyl * clip_y);
    draw_data->crd_xr = triangle->v2.x + (draw_data->delta_dxdyr * clip_y);
    draw_data->crd_zl = triangle->v1.z + (draw_data->delta_dzdyl * clip_y);
    draw_data->crd_zr = triangle->v2.z + (draw_data->delta_dzdyr * clip_y);
    draw_data->crd_ul = triangle->v1.u + (draw_data->delta_dudyl * clip_y);
    draw_data->crd_ur = triangle->v2.u + (draw_data->delta_dudyr * clip_y);
    draw_data->crd_vl = triangle->v1.v + (draw_data->delta_dvdyl * clip_y);
    draw_data->crd_vr = triangle->v2.v + (draw_data->delta_dvdyr * clip_y);
    delta_y = triangle->v3.y - draw_data->top_clip;
    // Line & zbuf start address
    draw_data->dest_adr = (ULONG)context->drawregion.data + (context->drawregion.bpr * (ULONG)draw_data->top_clip);
    draw_data->zbuf_adr = (ULONG)context->zbuffer.data + (context->zbuffer.bpr * (ULONG)draw_data->top_clip);
  } else {
    draw_data->crd_xl = triangle->v1.x;
    draw_data->crd_xr = triangle->v2.x;
    draw_data->crd_zl = triangle->v1.z;
    draw_data->crd_zr = triangle->v2.z;
    draw_data->crd_ul = triangle->v1.u;
    draw_data->crd_ur = triangle->v2.u;
    draw_data->crd_vl = triangle->v1.v;
    draw_data->crd_vr = triangle->v2.v;
    // Line & zbuf start address
    draw_data->dest_adr = (ULONG)context->drawregion.data + (context->drawregion.bpr * (ULONG)triangle->v1.y);
    draw_data->zbuf_adr = (ULONG)context->zbuffer.data + (context->zbuffer.bpr * (ULONG)triangle->v1.y);
  }
  // Use the v1 light for the flat shading
  draw_data->int_ll = triangle->v1.light;
  // Bottom clipping
  if (triangle->v3.y > draw_data->bottom_clip) {
    DDbug(printf("[MAGGIE3D] - Clipping bottom vertex\n");)
    delta_y -= (triangle->v3.y - draw_data->bottom_clip);
  }
  // Go for mapping
  M3D_FlatTextureMapping((UWORD) delta_y, draw_data);
}

/** Draw a flat textured bottom triangle */
VOID M3D_DrawFlatTexturedBottom(M3D_Context *context, M3D_Triangle *triangle, M3D_DrawData *draw_data)
{
  FLOAT delta_y, clip_y;

  DDbug(printf("[MAGGIE3D] M3D_DrawFlatTexturedBottom\n");)
  // Delta height
  delta_y = triangle->v2.y - triangle->v1.y;
  // Left side delta
  draw_data->delta_dxdyl = (triangle->v2.x - triangle->v1.x) / delta_y;
  draw_data->delta_dzdyl = (triangle->v2.z - triangle->v1.z) / delta_y;
  draw_data->delta_dudyl = (triangle->v2.u - triangle->v1.u) / delta_y;
  draw_data->delta_dvdyl = (triangle->v2.v - triangle->v1.v) / delta_y;
  // Right side delta
  draw_data->delta_dxdyr = (triangle->v3.x - triangle->v1.x) / delta_y;
  draw_data->delta_dzdyr = (triangle->v3.z - triangle->v1.z) / delta_y;
  draw_data->delta_dudyr = (triangle->v3.u - triangle->v1.u) / delta_y;
  draw_data->delta_dvdyr = (triangle->v3.v - triangle->v1.v) / delta_y;
  // Start coords & clipping
  if (triangle->v1.y < draw_data->top_clip) {
    DDbug(printf("[MAGGIE3D] - Clipping top vertex\n");)
    clip_y = draw_data->top_clip - triangle->v1.y;
    draw_data->crd_xl = triangle->v1.x + (draw_data->delta_dxdyl * clip_y);
    draw_data->crd_xr = triangle->v1.x + (draw_data->delta_dxdyr * clip_y);
    draw_data->crd_zl = triangle->v1.z + (draw_data->delta_dzdyl * clip_y);
    draw_data->crd_zr = triangle->v1.z + (draw_data->delta_dzdyr * clip_y);
    draw_data->crd_ul = triangle->v1.u + (draw_data->delta_dudyl * clip_y);
    draw_data->crd_ur = triangle->v1.u + (draw_data->delta_dudyr * clip_y);
    draw_data->crd_vl = triangle->v1.v + (draw_data->delta_dvdyl * clip_y);
    draw_data->crd_vr = triangle->v1.v + (draw_data->delta_dvdyr * clip_y);
    delta_y = triangle->v3.y - draw_data->top_clip;
    // Line & zbuf start address
    draw_data->dest_adr = (ULONG)context->drawregion.data + (context->drawregion.bpr * (ULONG)draw_data->top_clip);
    draw_data->zbuf_adr = (ULONG)context->zbuffer.data + (context->zbuffer.bpr * (ULONG)draw_data->top_clip);
  } else {
    draw_data->crd_xl = triangle->v1.x;
    draw_data->crd_xr = triangle->v1.x;
    draw_data->crd_zl = triangle->v1.z;
    draw_data->crd_zr = triangle->v1.z;
    draw_data->crd_ul = triangle->v1.u;
    draw_data->crd_ur = triangle->v1.u;
    draw_data->crd_vl = triangle->v1.v;
    draw_data->crd_vr = triangle->v1.v;
    // Line & zbuf start address
    draw_data->dest_adr = (ULONG)context->drawregion.data + (context->drawregion.bpr * (ULONG)triangle->v1.y);
    draw_data->zbuf_adr = (ULONG)context->zbuffer.data + (context->zbuffer.bpr * (ULONG)triangle->v1.y);
  }
  // Use the v1 light for the flat shading
  draw_data->int_ll = triangle->v1.light;
  // Bottom clipping
  if (triangle->v3.y > draw_data->bottom_clip) {
    DDbug(printf("[MAGGIE3D] - Clipping bottom vertex\n");)
    delta_y -= (triangle->v3.y - draw_data->bottom_clip);
  }
  // Go for mapping
  M3D_FlatTextureMapping((UWORD) delta_y, draw_data);
}

/** Draw a flat textured generic triangle */
VOID M3D_DrawFlatTexturedGeneric(M3D_Context *context, M3D_Triangle *triangle, M3D_DrawData *draw_data)
{
  FLOAT delta_y1, delta_y2, delta_y3;
  FLOAT dxdy1, dxdy2;
  FLOAT clip_y1, clip_y2;
  BOOL left_long;

  DDbug(printf("[MAGGIE3D] M3D_DrawFlatTexturedGeneric\n");)
  // Delta height
  delta_y1 = triangle->v2.y - triangle->v1.y;
  delta_y2 = triangle->v3.y - triangle->v1.y;
  // Delta X1
  dxdy1 = (triangle->v2.x - triangle->v1.x) / delta_y1;
  // Delta X2
  dxdy2 = (triangle->v3.x - triangle->v1.x) / delta_y2;
  // Calculate sides delta
  if (dxdy1 > dxdy2) {
    // Left side delta
    draw_data->delta_dxdyl = dxdy2;
    draw_data->delta_dzdyl = (triangle->v3.z - triangle->v1.z) / delta_y2;
    // Right side delta
    draw_data->delta_dxdyr = dxdy1;
    draw_data->delta_dzdyr = (triangle->v2.z - triangle->v1.z) / delta_y1;
    // Left side texture delta
    draw_data->delta_dudyl = (triangle->v3.u - triangle->v1.u) / delta_y2;
    draw_data->delta_dvdyl = (triangle->v3.v - triangle->v1.v) / delta_y2;
    // Right side texture delta
    draw_data->delta_dudyr = (triangle->v2.u - triangle->v1.u) / delta_y1;
    draw_data->delta_dvdyr = (triangle->v2.v - triangle->v1.v) / delta_y1;
    // Slope, left long
    left_long = TRUE;
  } else {
    // Left side delta
    draw_data->delta_dxdyl = dxdy1;
    draw_data->delta_dzdyl = (triangle->v2.z - triangle->v1.z) / delta_y1;
    // Right side delta
    draw_data->delta_dxdyr = dxdy2;
    draw_data->delta_dzdyr = (triangle->v3.z - triangle->v1.z) / delta_y2;
    // Left side texture delta
    draw_data->delta_dudyl = (triangle->v2.u - triangle->v1.u) / delta_y1;
    draw_data->delta_dvdyl = (triangle->v2.v - triangle->v1.v) / delta_y1;
    // Right side texture delta
    draw_data->delta_dudyr = (triangle->v3.u - triangle->v1.u) / delta_y2;
    draw_data->delta_dvdyr = (triangle->v3.v - triangle->v1.v) / delta_y2;
    // Slope, right long
    left_long = FALSE;
  }
  // y2 top clipping, we only have to draw the triangle lower part
  if (triangle->v2.y < draw_data->top_clip) {
    DDbug(printf("[MAGGIE3D] - Clipping top vertex 2, draw only lower triangle\n");)
    clip_y1 = draw_data->top_clip - triangle->v1.y;
    clip_y2 = draw_data->top_clip - triangle->v2.y;
    delta_y3 = triangle->v3.y - triangle->v2.y;
    if (left_long) {
      draw_data->delta_dxdyr = (triangle->v3.x - triangle->v2.x) / delta_y3;
      draw_data->delta_dzdyr = (triangle->v3.z - triangle->v2.z) / delta_y3;
      draw_data->delta_dudyr = (triangle->v3.u - triangle->v2.u) / delta_y3;
      draw_data->delta_dvdyr = (triangle->v3.v - triangle->v2.v) / delta_y3;
      draw_data->crd_xl = triangle->v1.x + (draw_data->delta_dxdyl * clip_y1);
      draw_data->crd_xr = triangle->v2.x + (draw_data->delta_dxdyr * clip_y2);
      draw_data->crd_zl = triangle->v1.z + (draw_data->delta_dzdyl * clip_y1);
      draw_data->crd_zr = triangle->v2.z + (draw_data->delta_dzdyr * clip_y2);
      draw_data->crd_ul = triangle->v1.u + (draw_data->delta_dudyl * clip_y1);
      draw_data->crd_ur = triangle->v2.u + (draw_data->delta_dudyr * clip_y2);
      draw_data->crd_vl = triangle->v1.v + (draw_data->delta_dvdyl * clip_y1);
      draw_data->crd_vr = triangle->v2.v + (draw_data->delta_dvdyr * clip_y2);
    } else {
      draw_data->delta_dxdyl = (triangle->v3.x - triangle->v2.x) / delta_y3;
      draw_data->delta_dzdyl = (triangle->v3.z - triangle->v2.z) / delta_y3;
      draw_data->delta_dudyl = (triangle->v3.u - triangle->v2.u) / delta_y3;
      draw_data->delta_dvdyl = (triangle->v3.v - triangle->v2.v) / delta_y3;
      draw_data->crd_xl = triangle->v2.x + (draw_data->delta_dxdyl * clip_y2);
      draw_data->crd_xr = triangle->v1.x + (draw_data->delta_dxdyr * clip_y1);
      draw_data->crd_zl = triangle->v2.z + (draw_data->delta_dzdyl * clip_y2);
      draw_data->crd_zr = triangle->v1.z + (draw_data->delta_dzdyr * clip_y1);
      draw_data->crd_ul = triangle->v2.u + (draw_data->delta_dudyl * clip_y2);
      draw_data->crd_ur = triangle->v1.u + (draw_data->delta_dudyr * clip_y1);
      draw_data->crd_vl = triangle->v2.v + (draw_data->delta_dvdyl * clip_y2);
      draw_data->crd_vr = triangle->v1.v + (draw_data->delta_dvdyr * clip_y1);
    }
    // Line & zbuf start address
    draw_data->dest_adr = (ULONG)context->drawregion.data + (context->drawregion.bpr * (ULONG)draw_data->top_clip);
    draw_data->zbuf_adr = (ULONG)context->zbuffer.data + (context->zbuffer.bpr * (ULONG)draw_data->top_clip);
    // Use the v1 light for the flat shading
    draw_data->int_ll = triangle->v1.light;
    delta_y3 = triangle->v3.y - draw_data->top_clip;
    // Bottom clipping
    if (triangle->v3.y > draw_data->bottom_clip) {
      DDbug(printf("[MAGGIE3D] - Clipping bottom vertex 3\n");)
      delta_y3 -= (triangle->v3.y - draw_data->bottom_clip);
    }
    // Go for mapping
    M3D_FlatTextureMapping((UWORD) delta_y3, draw_data);
  } else {
    // y1 top clipping
    if (triangle->v1.y < draw_data->top_clip) {
      DDbug(printf("[MAGGIE3D] - Clipping top vertex 1\n");)
      clip_y1 = draw_data->top_clip - triangle->v1.y;
      draw_data->crd_xl = triangle->v1.x + (draw_data->delta_dxdyl * clip_y1);
      draw_data->crd_xr = triangle->v1.x + (draw_data->delta_dxdyr * clip_y1);
      draw_data->crd_zl = triangle->v1.z + (draw_data->delta_dzdyl * clip_y1);
      draw_data->crd_zr = triangle->v1.z + (draw_data->delta_dzdyr * clip_y1);
      draw_data->crd_ul = triangle->v1.u + (draw_data->delta_dudyl * clip_y1);
      draw_data->crd_ur = triangle->v1.u + (draw_data->delta_dudyr * clip_y1);
      draw_data->crd_vl = triangle->v1.v + (draw_data->delta_dvdyl * clip_y1);
      draw_data->crd_vr = triangle->v1.v + (draw_data->delta_dvdyr * clip_y1);
      delta_y1 = triangle->v2.y - draw_data->top_clip;
      // Line & zbuf start address
      draw_data->dest_adr = (ULONG)context->drawregion.data + (context->drawregion.bpr * (ULONG)draw_data->top_clip);
      draw_data->zbuf_adr = (ULONG)context->zbuffer.data + (context->zbuffer.bpr * (ULONG)draw_data->top_clip);
    } else {
      draw_data->crd_xl = triangle->v1.x;
      draw_data->crd_xr = triangle->v1.x;
      draw_data->crd_zl = triangle->v1.z;
      draw_data->crd_zr = triangle->v1.z;
      draw_data->crd_ul = triangle->v1.u;
      draw_data->crd_ur = triangle->v1.u;
      draw_data->crd_vl = triangle->v1.v;
      draw_data->crd_vr = triangle->v1.v;
      // Line & zbuf start address
      draw_data->dest_adr = (ULONG)context->drawregion.data + (context->drawregion.bpr * (ULONG)triangle->v1.y);
      draw_data->zbuf_adr = (ULONG)context->zbuffer.data + (context->zbuffer.bpr * (ULONG)triangle->v1.y);
    }
    // Use the v1 light for the flat shading
    draw_data->int_ll = triangle->v1.light;
    // y2 bottom clipping, we only have to draw the triangle upper part
    if (triangle->v2.y > draw_data->bottom_clip) {
      DDbug(printf("[MAGGIE3D] - Clipping bottom vertex 2, draw only upper triangle\n");)
      delta_y1 -= (triangle->v2.y - draw_data->bottom_clip);
      // Go for mapping
      M3D_FlatTextureMapping((UWORD) delta_y1, draw_data);
    } else {
      // Go for mapping
      M3D_FlatTextureMapping((UWORD) delta_y1, draw_data);
      delta_y3 = triangle->v3.y - triangle->v2.y;
      // Calcul new deltas
      if (left_long) {
        draw_data->delta_dxdyr = (triangle->v3.x - triangle->v2.x) / delta_y3;
        draw_data->delta_dzdyr = (triangle->v3.z - triangle->v2.z) / delta_y3;
        draw_data->delta_dudyr = (triangle->v3.u - triangle->v2.u) / delta_y3;
        draw_data->delta_dvdyr = (triangle->v3.v - triangle->v2.v) / delta_y3;
      } else {
        draw_data->delta_dxdyl = (triangle->v3.x - triangle->v2.x) / delta_y3;
        draw_data->delta_dzdyl = (triangle->v3.z - triangle->v2.z) / delta_y3;
        draw_data->delta_dudyl = (triangle->v3.u - triangle->v2.u) / delta_y3;
        draw_data->delta_dvdyl = (triangle->v3.v - triangle->v2.v) / delta_y3;
      }
      // Line & zbuf start address
      draw_data->dest_adr = (ULONG)context->drawregion.data + (context->drawregion.bpr * (ULONG)triangle->v2.y);
      draw_data->zbuf_adr = (ULONG)context->zbuffer.data + (context->zbuffer.bpr * (ULONG)triangle->v2.y);
      // Bottom clipping
      if (triangle->v3.y > draw_data->bottom_clip) {
        DDbug(printf("[MAGGIE3D] - Clipping bottom vertex 3\n");)
        delta_y3 -= (triangle->v3.y - draw_data->bottom_clip);
      }
      // Go for mapping
      M3D_FlatTextureMapping((UWORD) delta_y3, draw_data);
    }
  }
}

/*****************************************************************************/
/**                       QUAD DRAW                                          */
/*****************************************************************************/

/** Draw a flat textured flat top quad */
VOID M3D_DrawQuadFlatTexturedTop(M3D_Context *context, M3D_Quad *quad, M3D_DrawData *draw_data)
{
  FLOAT dyl, dyr, clip_y;

  DDbug(printf("[MAGGIE3D] M3D_DrawQuadFlatTexturedTop\n");)
  dyl = quad->v4.y - quad->v1.y;
  draw_data->delta_dxdyl = (quad->v4.x - quad->v1.x) / dyl;
  draw_data->delta_dzdyl = (quad->v4.z - quad->v1.z) / dyl;
  draw_data->delta_dudyl = (quad->v4.u - quad->v1.u) / dyl;
  draw_data->delta_dvdyl = (quad->v4.v - quad->v1.v) / dyl;
  dyr = quad->v3.y - quad->v2.y;
  draw_data->delta_dxdyr = (quad->v3.x - quad->v2.x) / dyr;
  draw_data->delta_dzdyr = (quad->v3.z - quad->v2.z) / dyr;
  draw_data->delta_dudyr = (quad->v3.u - quad->v2.u) / dyr;
  draw_data->delta_dvdyr = (quad->v3.v - quad->v2.v) / dyr;
  if (quad->v1.y < draw_data->top_clip) {
    DDbug(printf("[MAGGIE3D] - Clipping top vertices V1 & V2\n");)
    clip_y = draw_data->top_clip - quad->v1.y;
    draw_data->crd_xl = quad->v1.x + (draw_data->delta_dxdyl * clip_y);
    draw_data->crd_xr = quad->v2.x + (draw_data->delta_dxdyr * clip_y);
    draw_data->crd_zl = quad->v1.z + (draw_data->delta_dzdyl * clip_y);
    draw_data->crd_zr = quad->v2.z + (draw_data->delta_dzdyr * clip_y);
    draw_data->crd_ul = quad->v1.u + (draw_data->delta_dudyl * clip_y);
    draw_data->crd_ur = quad->v2.u + (draw_data->delta_dudyr * clip_y);
    draw_data->crd_vl = quad->v1.v + (draw_data->delta_dvdyl * clip_y);
    draw_data->crd_vr = quad->v2.v + (draw_data->delta_dvdyr * clip_y);
    dyl = quad->v4.y - draw_data->top_clip;
    dyr = quad->v3.y - draw_data->top_clip;
    // Line & zbuf start address
    draw_data->dest_adr = (ULONG)context->drawregion.data + (context->drawregion.bpr * (ULONG)draw_data->top_clip);
    draw_data->zbuf_adr = (ULONG)context->zbuffer.data + (context->zbuffer.bpr * (ULONG)draw_data->top_clip);
  } else {
    draw_data->crd_xl = quad->v1.x;
    draw_data->crd_xr = quad->v2.x;
    draw_data->crd_zl = quad->v1.z;
    draw_data->crd_zr = quad->v2.z;
    draw_data->crd_ul = quad->v1.u;
    draw_data->crd_ur = quad->v2.u;
    draw_data->crd_vl = quad->v1.v;
    draw_data->crd_vr = quad->v2.v;
    // Line & zbuf start address
    draw_data->dest_adr = (ULONG)context->drawregion.data + (context->drawregion.bpr * (ULONG)quad->v1.y);
    draw_data->zbuf_adr = (ULONG)context->zbuffer.data + (context->zbuffer.bpr * (ULONG)quad->v1.y);
  }
  // Use the v1 light for the flat shading
  draw_data->int_ll = quad->v1.light;
  if (quad->v3.y < quad->v4.y) {
    // Something to draw on upper side ?
    if (quad->v3.y >= draw_data->top_clip) {
      // Bottom clipping
      if (quad->v3.y > draw_data->bottom_clip) {
        DDbug(printf("[MAGGIE3D] - Clipping bottom vertex V3\n");)
        dyr -= (quad->v3.y - draw_data->bottom_clip);
        // Upper side
        M3D_FlatTextureMapping((UWORD) dyr, draw_data);
        return;       // No more to draw
      }
      // Upper side
      M3D_FlatTextureMapping((UWORD) dyr, draw_data);
      // Lower side
      dyr = quad->v4.y - quad->v3.y;
      draw_data->delta_dxdyr = (quad->v4.x - quad->v3.x) / dyr;
      draw_data->delta_dzdyr = (quad->v4.z - quad->v3.z) / dyr;
      draw_data->delta_dudyr = (quad->v4.u - quad->v3.u) / dyr;
      draw_data->delta_dvdyr = (quad->v4.v - quad->v3.v) / dyr;
    } else {
      // Lower side
      dyr = quad->v4.y - quad->v3.y;
      draw_data->delta_dxdyr = (quad->v4.x - quad->v3.x) / dyr;
      draw_data->delta_dzdyr = (quad->v4.z - quad->v3.z) / dyr;
      draw_data->delta_dudyr = (quad->v4.u - quad->v3.u) / dyr;
      draw_data->delta_dvdyr = (quad->v4.v - quad->v3.v) / dyr;
      DDbug(printf("[MAGGIE3D] - Clipping top vertex V3\n");)
      clip_y = draw_data->top_clip - quad->v3.y;
      draw_data->crd_xr = quad->v3.x + (draw_data->delta_dxdyr * clip_y);
      draw_data->crd_zr = quad->v3.z + (draw_data->delta_dzdyr * clip_y);
      draw_data->crd_ur = quad->v3.u + (draw_data->delta_dudyr * clip_y);
      draw_data->crd_vr = quad->v3.v + (draw_data->delta_dvdyr * clip_y);
      dyr = quad->v4.y - draw_data->top_clip;
    }
    if (quad->v4.y > draw_data->bottom_clip) {
      DDbug(printf("[MAGGIE3D] - Clipping bottom vertex V4\n");)
      dyr -= (quad->v4.y - draw_data->bottom_clip);
    }
    M3D_FlatTextureMapping((UWORD) dyr, draw_data);
  } else {
    // Something to draw on upper side ?
    if (quad->v4.y >= draw_data->top_clip) {
      // Bottom clipping
      if (quad->v4.y > draw_data->bottom_clip) {
        DDbug(printf("[MAGGIE3D] - Clipping bottom vertex V4\n");)
        dyl -= (quad->v4.y - draw_data->bottom_clip);
        // Upper side
        M3D_FlatTextureMapping((UWORD) dyl, draw_data);
        return;       // No more to draw
      }
      // Upper side
      M3D_FlatTextureMapping((UWORD) dyl, draw_data);
      // Lower side
      dyl = quad->v3.y - quad->v4.y;
      draw_data->delta_dxdyl = (quad->v3.x - quad->v4.x) / dyl;
      draw_data->delta_dzdyl = (quad->v3.z - quad->v4.z) / dyl;
      draw_data->delta_dudyl = (quad->v3.u - quad->v4.u) / dyl;
      draw_data->delta_dvdyl = (quad->v3.v - quad->v4.v) / dyl;
    } else {
      // Lower side
      dyl = quad->v3.y - quad->v4.y;
      draw_data->delta_dxdyl = (quad->v3.x - quad->v4.x) / dyl;
      draw_data->delta_dzdyl = (quad->v3.z - quad->v4.z) / dyl;
      draw_data->delta_dudyl = (quad->v3.u - quad->v4.u) / dyl;
      draw_data->delta_dvdyl = (quad->v3.v - quad->v4.v) / dyl;
      DDbug(printf("[MAGGIE3D] - Clipping top vertex V4\n");)
      clip_y = draw_data->top_clip - quad->v4.y;
      draw_data->crd_xl = quad->v4.x + (draw_data->delta_dxdyl * clip_y);
      draw_data->crd_zl = quad->v4.z + (draw_data->delta_dzdyl * clip_y);
      draw_data->crd_ul = quad->v4.u + (draw_data->delta_dudyl * clip_y);
      draw_data->crd_vl = quad->v4.v + (draw_data->delta_dvdyl * clip_y);
      dyl = quad->v3.y - draw_data->top_clip;
    }
    if (quad->v3.y > draw_data->bottom_clip) {
      DDbug(printf("[MAGGIE3D] - Clipping bottom vertex V3\n");)
      dyl -= (quad->v3.y - draw_data->bottom_clip);
    }
    M3D_FlatTextureMapping((UWORD) dyl, draw_data);
  }
}

/** Draw a flat shaded flat bottom quad */
VOID M3D_DrawQuadFlatTexturedBottom(M3D_Context *context, M3D_Quad *quad, M3D_DrawData *draw_data)
{
  FLOAT dyl, dyr, clip_y;

  DDbug(printf("[MAGGIE3D] M3D_DrawQuadFlatTexturedBottom\n");)
  dyl = quad->v4.y - quad->v1.y;
  draw_data->delta_dxdyl = (quad->v4.x - quad->v1.x) / dyl;
  draw_data->delta_dzdyl = (quad->v4.z - quad->v1.z) / dyl;
  draw_data->delta_dudyl = (quad->v4.u - quad->v1.u) / dyl;
  draw_data->delta_dvdyl = (quad->v4.v - quad->v1.v) / dyl;
  dyr = quad->v2.y - quad->v1.y;
  draw_data->delta_dxdyr = (quad->v2.x - quad->v1.x) / dyr;
  draw_data->delta_dzdyr = (quad->v2.z - quad->v1.z) / dyr;
  draw_data->delta_dudyr = (quad->v2.u - quad->v1.u) / dyr;
  draw_data->delta_dvdyr = (quad->v2.v - quad->v1.v) / dyr;
  if (quad->v1.y < draw_data->top_clip) {
    DDbug(printf("[MAGGIE3D] - Clipping top vertex V1\n");)
    clip_y = draw_data->top_clip - quad->v1.y;
    draw_data->crd_xl = quad->v1.x + (draw_data->delta_dxdyl * clip_y);
    draw_data->crd_xr = quad->v1.x + (draw_data->delta_dxdyr * clip_y);
    draw_data->crd_zl = quad->v1.z + (draw_data->delta_dzdyl * clip_y);
    draw_data->crd_zr = quad->v1.z + (draw_data->delta_dzdyr * clip_y);
    draw_data->crd_ul = quad->v1.u + (draw_data->delta_dudyl * clip_y);
    draw_data->crd_ur = quad->v1.u + (draw_data->delta_dudyr * clip_y);
    draw_data->crd_vl = quad->v1.v + (draw_data->delta_dvdyl * clip_y);
    draw_data->crd_vr = quad->v1.v + (draw_data->delta_dvdyr * clip_y);
    dyl = quad->v4.y - draw_data->top_clip;
    dyr = quad->v2.y - draw_data->top_clip;
    // Line & zbuf start address
    draw_data->dest_adr = (ULONG)context->drawregion.data + (context->drawregion.bpr * (ULONG)draw_data->top_clip);
    draw_data->zbuf_adr = (ULONG)context->zbuffer.data + (context->zbuffer.bpr * (ULONG)draw_data->top_clip);
  } else {
    draw_data->crd_xl = quad->v1.x;
    draw_data->crd_xr = quad->v1.x;
    draw_data->crd_zl = quad->v1.z;
    draw_data->crd_zr = quad->v1.z;
    draw_data->crd_ul = quad->v1.u;
    draw_data->crd_ur = quad->v1.u;
    draw_data->crd_vl = quad->v1.v;
    draw_data->crd_vr = quad->v1.v;
    // Line & zbuf start address
    draw_data->dest_adr = (ULONG)context->drawregion.data + (context->drawregion.bpr * (ULONG)quad->v1.y);
    draw_data->zbuf_adr = (ULONG)context->zbuffer.data + (context->zbuffer.bpr * (ULONG)quad->v1.y);
  }
  // Use the v1 light for the flat shading
  draw_data->int_ll = quad->v1.light;
  if (quad->v2.y < quad->v4.y) {
    // Something to draw on upper side ?
    if (quad->v2.y >= draw_data->top_clip) {
      // Bottom clipping
      if (quad->v2.y > draw_data->bottom_clip) {
        DDbug(printf("[MAGGIE3D] - Clipping bottom vertex V2\n");)
        dyr -= (quad->v2.y - draw_data->bottom_clip);
        // Upper side
        M3D_FlatTextureMapping((UWORD) dyr, draw_data);
        return;       // No more to draw
      }
      // Upper side
      M3D_FlatTextureMapping((UWORD) dyr, draw_data);
      // Lower side
      dyr = quad->v3.y - quad->v2.y;
      draw_data->delta_dxdyr = (quad->v3.x - quad->v2.x) / dyr;
      draw_data->delta_dzdyr = (quad->v3.z - quad->v2.z) / dyr;
      draw_data->delta_dudyr = (quad->v3.u - quad->v2.u) / dyr;
      draw_data->delta_dvdyr = (quad->v3.v - quad->v2.v) / dyr;
    } else {
      // Lower side
      dyr = quad->v3.y - quad->v2.y;
      draw_data->delta_dxdyr = (quad->v3.x - quad->v2.x) / dyr;
      draw_data->delta_dzdyr = (quad->v3.z - quad->v2.z) / dyr;
      draw_data->delta_dudyr = (quad->v3.u - quad->v2.u) / dyr;
      draw_data->delta_dvdyr = (quad->v3.v - quad->v2.v) / dyr;
      DDbug(printf("[MAGGIE3D] - Clipping top vertex V2\n");)
      clip_y = draw_data->top_clip - quad->v2.y;
      draw_data->crd_xr = quad->v2.x + (draw_data->delta_dxdyr * clip_y);
      draw_data->crd_zr = quad->v2.z + (draw_data->delta_dzdyr * clip_y);
      draw_data->crd_ur = quad->v2.u + (draw_data->delta_dudyr * clip_y);
      draw_data->crd_vr = quad->v2.v + (draw_data->delta_dvdyr * clip_y);
      dyr = quad->v3.y - draw_data->top_clip;
    }
    if (quad->v4.y > draw_data->bottom_clip) {
      DDbug(printf("[MAGGIE3D] - Clipping bottom vertex V4\n");)
      dyr -= (quad->v4.y - draw_data->bottom_clip);
    }
    M3D_FlatTextureMapping((UWORD) dyr, draw_data);
  } else {
    // Something to draw on upper side ?
    if (quad->v4.y >= draw_data->top_clip) {
      // Bottom clipping
      if (quad->v4.y > draw_data->bottom_clip) {
        DDbug(printf("[MAGGIE3D] - Clipping bottom vertex V4\n");)
        dyl -= (quad->v4.y - draw_data->bottom_clip);
        // Upper side
        M3D_FlatTextureMapping((UWORD) dyl, draw_data);
        return;       // No more to draw
      }
      // Upper side
      M3D_FlatTextureMapping((UWORD) dyl, draw_data);
      // Lower side
      dyl = quad->v3.y - quad->v4.y;
      draw_data->delta_dxdyl = (quad->v3.x - quad->v4.x) / dyl;
      draw_data->delta_dzdyl = (quad->v3.z - quad->v4.z) / dyl;
      draw_data->delta_dudyl = (quad->v3.u - quad->v4.u) / dyl;
      draw_data->delta_dvdyl = (quad->v3.v - quad->v4.v) / dyl;
    } else {
      // Lower side
      dyl = quad->v3.y - quad->v4.y;
      draw_data->delta_dxdyl = (quad->v3.x - quad->v4.x) / dyl;
      draw_data->delta_dzdyl = (quad->v3.z - quad->v4.z) / dyl;
      draw_data->delta_dudyl = (quad->v3.u - quad->v4.u) / dyl;
      draw_data->delta_dvdyl = (quad->v3.v - quad->v4.v) / dyl;
      DDbug(printf("[MAGGIE3D] - Clipping top vertex V4\n");)
      clip_y = draw_data->top_clip - quad->v4.y;
      draw_data->crd_xl = quad->v4.x + (draw_data->delta_dxdyl * clip_y);
      draw_data->crd_zl = quad->v4.z + (draw_data->delta_dzdyl * clip_y);
      draw_data->crd_ul = quad->v4.u + (draw_data->delta_dudyl * clip_y);
      draw_data->crd_vl = quad->v4.v + (draw_data->delta_dvdyl * clip_y);
      dyl = quad->v3.y - draw_data->top_clip;
    }
    if (quad->v3.y > draw_data->bottom_clip) {
      DDbug(printf("[MAGGIE3D] - Clipping bottom vertex V3\n");)
      dyl -= (quad->v3.y - draw_data->bottom_clip);
    }
    M3D_FlatTextureMapping((UWORD) dyl, draw_data);
  }
}

/** Draw a flat textured flat both quad */
VOID M3D_DrawQuadFlatTexturedBoth(M3D_Context *context, M3D_Quad *quad, M3D_DrawData *draw_data)
{
  FLOAT delta_y, clip_y;

  DDbug(printf("[MAGGIE3D] M3D_DrawQuadFlatTexturedBoth\n");)
  delta_y = quad->v4.y - quad->v1.y;
  draw_data->delta_dxdyl = (quad->v4.x - quad->v1.x) / delta_y;
  draw_data->delta_dzdyl = (quad->v4.z - quad->v1.z) / delta_y;
  draw_data->delta_dudyl = (quad->v4.u - quad->v1.u) / delta_y;
  draw_data->delta_dvdyl = (quad->v4.v - quad->v1.v) / delta_y;
  draw_data->delta_dxdyr = (quad->v3.x - quad->v2.x) / delta_y;
  draw_data->delta_dzdyr = (quad->v3.z - quad->v2.z) / delta_y;
  draw_data->delta_dudyr = (quad->v3.u - quad->v2.u) / delta_y;
  draw_data->delta_dvdyr = (quad->v3.v - quad->v2.v) / delta_y;
  if (quad->v1.y < draw_data->top_clip) {
    DDbug(printf("[MAGGIE3D] - Clipping top vertices V1 & V2\n");)
    clip_y = draw_data->top_clip - quad->v1.y;
    draw_data->crd_xl = quad->v1.x + (draw_data->delta_dxdyl * clip_y);
    draw_data->crd_xr = quad->v2.x + (draw_data->delta_dxdyr * clip_y);
    draw_data->crd_zl = quad->v1.z + (draw_data->delta_dzdyl * clip_y);
    draw_data->crd_zr = quad->v2.z + (draw_data->delta_dzdyr * clip_y);
    draw_data->crd_ul = quad->v1.u + (draw_data->delta_dudyl * clip_y);
    draw_data->crd_ur = quad->v2.u + (draw_data->delta_dudyr * clip_y);
    draw_data->crd_vl = quad->v1.v + (draw_data->delta_dvdyl * clip_y);
    draw_data->crd_vr = quad->v2.v + (draw_data->delta_dvdyr * clip_y);
    delta_y = quad->v4.y - draw_data->top_clip;
    // Line & zbuf start address
    draw_data->dest_adr = (ULONG)context->drawregion.data + (context->drawregion.bpr * (ULONG)draw_data->top_clip);
    draw_data->zbuf_adr = (ULONG)context->zbuffer.data + (context->zbuffer.bpr * (ULONG)draw_data->top_clip);
  } else {
    draw_data->crd_xl = quad->v1.x;
    draw_data->crd_xr = quad->v2.x;
    draw_data->crd_zl = quad->v1.z;
    draw_data->crd_zr = quad->v2.z;
    draw_data->crd_ul = quad->v1.u;
    draw_data->crd_ur = quad->v2.u;
    draw_data->crd_vl = quad->v1.v;
    draw_data->crd_vr = quad->v2.v;
    // Line & zbuf start address
    draw_data->dest_adr = (ULONG)context->drawregion.data + (context->drawregion.bpr * (ULONG)quad->v1.y);
    draw_data->zbuf_adr = (ULONG)context->zbuffer.data + (context->zbuffer.bpr * (ULONG)quad->v1.y);
  }
  // Use the v1 light for the flat shading
  draw_data->int_ll = quad->v1.light;
  // Bottom clipping
  if (quad->v4.y > draw_data->bottom_clip) {
    DDbug(printf("[MAGGIE3D] - Clipping bottom vertices V3 & V4\n");)
    delta_y -= (quad->v4.y - draw_data->bottom_clip);
  }
  // Full side
  M3D_FlatTextureMapping((UWORD) delta_y, draw_data);
}

/** Draw a flat textured generic quad */
VOID M3D_DrawQuadFlatTexturedGeneric(M3D_Context *context, M3D_Quad *quad, M3D_DrawData *draw_data)
{
  FLOAT dyl, dyr, clip_y;

  DDbug(printf("[MAGGIE3D] M3D_DrawQuadFlatTexturedGeneric\n");)
  dyl = quad->v4.y - quad->v1.y;
  draw_data->delta_dxdyl = (quad->v4.x - quad->v1.x) / dyl;
  draw_data->delta_dzdyl = (quad->v4.z - quad->v1.z) / dyl;
  draw_data->delta_dudyl = (quad->v4.u - quad->v1.u) / dyl;
  draw_data->delta_dvdyl = (quad->v4.v - quad->v1.v) / dyl;
  dyr = quad->v2.y - quad->v1.y;
  draw_data->delta_dxdyr = (quad->v2.x - quad->v1.x) / dyr;
  draw_data->delta_dzdyr = (quad->v2.z - quad->v1.z) / dyr;
  draw_data->delta_dudyr = (quad->v2.u - quad->v1.u) / dyr;
  draw_data->delta_dvdyr = (quad->v2.v - quad->v1.v) / dyr;
  if (quad->v1.y < draw_data->top_clip) {
    DDbug(printf("[MAGGIE3D] - Clipping top vertices V1 & V2\n");)
    clip_y = draw_data->top_clip - quad->v1.y;
    draw_data->crd_xl = quad->v1.x + (draw_data->delta_dxdyl * clip_y);
    draw_data->crd_xr = quad->v1.x + (draw_data->delta_dxdyr * clip_y);
    draw_data->crd_zl = quad->v1.z + (draw_data->delta_dzdyl * clip_y);
    draw_data->crd_zr = quad->v1.z + (draw_data->delta_dzdyr * clip_y);
    draw_data->crd_ul = quad->v1.u + (draw_data->delta_dudyl * clip_y);
    draw_data->crd_ur = quad->v1.u + (draw_data->delta_dudyr * clip_y);
    draw_data->crd_vl = quad->v1.v + (draw_data->delta_dvdyl * clip_y);
    draw_data->crd_vr = quad->v1.v + (draw_data->delta_dvdyr * clip_y);
    dyl = quad->v4.y - draw_data->top_clip;
    dyr = quad->v2.y - draw_data->top_clip;
    // Line & zbuf start address
    draw_data->dest_adr = (ULONG)context->drawregion.data + (context->drawregion.bpr * (ULONG)draw_data->top_clip);
    draw_data->zbuf_adr = (ULONG)context->zbuffer.data + (context->zbuffer.bpr * (ULONG)draw_data->top_clip);
  } else {
    draw_data->crd_xl = quad->v1.x;
    draw_data->crd_xr = quad->v1.x;
    draw_data->crd_zl = quad->v1.z;
    draw_data->crd_zr = quad->v1.z;
    draw_data->crd_ul = quad->v1.u;
    draw_data->crd_ur = quad->v1.u;
    draw_data->crd_vl = quad->v1.v;
    draw_data->crd_vr = quad->v1.v;
    // Line & zbuf start address
    draw_data->dest_adr = (ULONG)context->drawregion.data + (context->drawregion.bpr * (ULONG)quad->v1.y);
    draw_data->zbuf_adr = (ULONG)context->zbuffer.data + (context->zbuffer.bpr * (ULONG)quad->v1.y);
  }
  // Use the v1 light for the flat shading
  draw_data->int_ll = quad->v1.light;
  if (quad->v2.y < quad->v4.y) {
    // Something to draw on upper side ?
    if (quad->v2.y >= draw_data->top_clip) {
      // Bottom clipping
      if (quad->v2.y > draw_data->bottom_clip) {
        DDbug(printf("[MAGGIE3D] - Clipping bottom vertex V2\n");)
        dyr -= (quad->v2.y - draw_data->bottom_clip);
        // Upper side
        M3D_FlatTextureMapping((UWORD) dyr, draw_data);
        return;       // No more to draw
      }
      // Upper side
      M3D_FlatTextureMapping((UWORD) dyr, draw_data);
      // Mid left
      dyl = quad->v4.y - quad->v2.y;
      // Mid right
      dyr = quad->v3.y - quad->v2.y;
      draw_data->delta_dxdyr = (quad->v3.x - quad->v2.x) / dyr;
      draw_data->delta_dzdyr = (quad->v3.z - quad->v2.z) / dyr;
      draw_data->delta_dudyr = (quad->v3.u - quad->v2.u) / dyr;
      draw_data->delta_dvdyr = (quad->v3.v - quad->v2.v) / dyr;
    } else {
      // No upper side, start with mid side
      dyr = quad->v3.y - quad->v2.y;
      draw_data->delta_dxdyr = (quad->v3.x - quad->v2.x) / dyr;
      draw_data->delta_dzdyr = (quad->v3.z - quad->v2.z) / dyr;
      draw_data->delta_dudyr = (quad->v3.u - quad->v2.u) / dyr;
      draw_data->delta_dvdyr = (quad->v3.v - quad->v2.v) / dyr;
      DDbug(printf("[MAGGIE3D] - Clipping top vertex V2\n");)
      clip_y = draw_data->top_clip - quad->v2.y;
      draw_data->crd_xr = quad->v2.x + (draw_data->delta_dxdyr * clip_y);
      draw_data->crd_zr = quad->v2.z + (draw_data->delta_dzdyr * clip_y);
      draw_data->crd_ur = quad->v2.u + (draw_data->delta_dudyr * clip_y);
      draw_data->crd_vr = quad->v2.v + (draw_data->delta_dvdyr * clip_y);
      dyr = quad->v3.y - draw_data->top_clip;
      dyl = quad->v4.y - draw_data->top_clip;
    }
    if (quad->v4.y < quad->v3.y) {
      // Something to draw on mid side
      if (quad->v4.y >= draw_data->top_clip) {
        // Bottom clipping
        if (quad->v4.y > draw_data->bottom_clip) {
          DDbug(printf("[MAGGIE3D] - Clipping bottom vertex V4\n");)
          dyl -= (quad->v4.y - draw_data->bottom_clip);
          // Mid side
          M3D_FlatTextureMapping((UWORD) dyl, draw_data);
          return;       // No more to draw
        }
        // Mid side
        M3D_FlatTextureMapping((UWORD) dyl, draw_data);
        // Lower side
        dyl = quad->v3.y - quad->v4.y;
        draw_data->delta_dxdyl = (quad->v3.x - quad->v4.x) / dyl;
        draw_data->delta_dzdyl = (quad->v3.z - quad->v4.z) / dyl;
        draw_data->delta_dudyl = (quad->v3.u - quad->v4.u) / dyl;
        draw_data->delta_dvdyl = (quad->v3.v - quad->v4.v) / dyl;
      } else {
        // Lower side
        dyl = quad->v3.y - quad->v4.y;
        draw_data->delta_dxdyl = (quad->v3.x - quad->v4.x) / dyl;
        draw_data->delta_dzdyl = (quad->v3.z - quad->v4.z) / dyl;
        draw_data->delta_dudyl = (quad->v3.u - quad->v4.u) / dyl;
        draw_data->delta_dvdyl = (quad->v3.v - quad->v4.v) / dyl;
        DDbug(printf("[MAGGIE3D] - Clipping top vertex V4\n");)
        clip_y = draw_data->top_clip - quad->v4.y;
        draw_data->crd_xl = quad->v4.x + (draw_data->delta_dxdyl * clip_y);
        draw_data->crd_zl = quad->v4.z + (draw_data->delta_dzdyl * clip_y);
        draw_data->crd_ul = quad->v4.u + (draw_data->delta_dudyl * clip_y);
        draw_data->crd_vl = quad->v4.v + (draw_data->delta_dvdyl * clip_y);
        dyl = quad->v3.y - draw_data->top_clip;
      }
      if (quad->v3.y > draw_data->bottom_clip) {
        DDbug(printf("[MAGGIE3D] - Clipping bottom vertex V3\n");)
        dyl -= (quad->v3.y - draw_data->bottom_clip);
      }
      M3D_FlatTextureMapping((UWORD) dyl, draw_data);
    } else {
      // Something to draw on mid side
      if (quad->v3.y >= draw_data->top_clip) {
        // Bottom clipping
        if (quad->v3.y > draw_data->bottom_clip) {
          DDbug(printf("[MAGGIE3D] - Clipping bottom vertex V3\n");)
          dyr -= (quad->v3.y - draw_data->bottom_clip);
          // Mid side
          M3D_FlatTextureMapping((UWORD) dyr, draw_data);
          return;       // No more to draw
        }
        // Mid side
        M3D_FlatTextureMapping((UWORD) dyr, draw_data);
        // Lower side
        dyr = quad->v4.y - quad->v3.y;
        draw_data->delta_dxdyr = (quad->v4.x - quad->v3.x) / dyr;
        draw_data->delta_dzdyr = (quad->v4.z - quad->v3.z) / dyr;
        draw_data->delta_dudyr = (quad->v4.u - quad->v3.u) / dyr;
        draw_data->delta_dvdyr = (quad->v4.v - quad->v3.v) / dyr;
      } else {
        // Lower side
        dyr = quad->v4.y - quad->v3.y;
        draw_data->delta_dxdyr = (quad->v4.x - quad->v3.x) / dyr;
        draw_data->delta_dzdyr = (quad->v4.z - quad->v3.z) / dyr;
        draw_data->delta_dudyr = (quad->v4.u - quad->v3.u) / dyr;
        draw_data->delta_dvdyr = (quad->v4.v - quad->v3.v) / dyr;
        DDbug(printf("[MAGGIE3D] - Clipping top vertex V3\n");)
        clip_y = draw_data->top_clip - quad->v3.y;
        draw_data->crd_xr = quad->v3.x + (draw_data->delta_dxdyr * clip_y);
        draw_data->crd_zr = quad->v3.z + (draw_data->delta_dzdyr * clip_y);
        draw_data->crd_ur = quad->v3.u + (draw_data->delta_dudyr * clip_y);
        draw_data->crd_vr = quad->v3.v + (draw_data->delta_dvdyr * clip_y);
        dyr = quad->v4.y - draw_data->top_clip;
      }
      if (quad->v4.y > draw_data->bottom_clip) {
        DDbug(printf("[MAGGIE3D] - Clipping bottom vertex V4\n");)
        dyr -= (quad->v4.y - draw_data->bottom_clip);
      }
      M3D_FlatTextureMapping((UWORD) dyr, draw_data);
    }
  } else if (quad->v2.y == quad->v4.y) {
    // Something to draw on upper side ?
    if (quad->v2.y >= draw_data->top_clip) {
      // Bottom clipping
      if (quad->v2.y > draw_data->bottom_clip) {
        DDbug(printf("[MAGGIE3D] - Clipping bottom vertex V2\n");)
        dyr -= (quad->v2.y - draw_data->bottom_clip);
        // Upper side
        M3D_FlatTextureMapping((UWORD) dyr, draw_data);
        return;       // No more to draw
      }
      // Upper side
      M3D_FlatTextureMapping((UWORD) dyr, draw_data);
      dyl = quad->v3.y - quad->v4.y;
      draw_data->delta_dxdyl = (quad->v3.x - quad->v4.x) / dyl;
      draw_data->delta_dzdyl = (quad->v3.z - quad->v4.z) / dyl;
      draw_data->delta_dudyl = (quad->v3.u - quad->v4.u) / dyl;
      draw_data->delta_dvdyl = (quad->v3.v - quad->v4.v) / dyl;
      dyr = quad->v3.y - quad->v2.y;
      draw_data->delta_dxdyr = (quad->v3.x - quad->v2.x) / dyr;
      draw_data->delta_dzdyr = (quad->v3.z - quad->v2.z) / dyr;
      draw_data->delta_dudyr = (quad->v3.u - quad->v2.u) / dyr;
      draw_data->delta_dvdyr = (quad->v3.v - quad->v2.v) / dyr;
    } else {
      dyl = quad->v3.y - quad->v4.y;
      draw_data->delta_dxdyl = (quad->v3.x - quad->v4.x) / dyl;
      draw_data->delta_dzdyl = (quad->v3.z - quad->v4.z) / dyl;
      draw_data->delta_dudyl = (quad->v3.u - quad->v4.u) / dyl;
      draw_data->delta_dvdyl = (quad->v3.v - quad->v4.v) / dyl;
      dyr = quad->v3.y - quad->v2.y;
      draw_data->delta_dxdyr = (quad->v3.x - quad->v2.x) / dyr;
      draw_data->delta_dzdyr = (quad->v3.z - quad->v2.z) / dyr;
      draw_data->delta_dudyr = (quad->v3.u - quad->v2.u) / dyr;
      draw_data->delta_dvdyr = (quad->v3.v - quad->v2.v) / dyr;
      DDbug(printf("[MAGGIE3D] - Clipping top vertex V2 & V4\n");)
      clip_y = draw_data->top_clip - quad->v2.y;
      draw_data->crd_xl = quad->v4.x + (draw_data->delta_dxdyl * clip_y);
      draw_data->crd_zl = quad->v4.z + (draw_data->delta_dzdyl * clip_y);
      draw_data->crd_xr = quad->v2.x + (draw_data->delta_dxdyr * clip_y);
      draw_data->crd_zr = quad->v2.z + (draw_data->delta_dzdyr * clip_y);
      draw_data->crd_ul = quad->v4.u + (draw_data->delta_dudyl * clip_y);
      draw_data->crd_ur = quad->v2.u + (draw_data->delta_dudyr * clip_y);
      draw_data->crd_vl = quad->v4.v + (draw_data->delta_dvdyl * clip_y);
      draw_data->crd_vr = quad->v2.v + (draw_data->delta_dvdyr * clip_y);
      dyr = quad->v3.y - draw_data->top_clip;
    }
    // Flat bottom, no more to draw
    if (quad->v3.y == quad->v2.y) {
      return;
    }
    // Lower side
    if (quad->v3.y > draw_data->bottom_clip) {
      DDbug(printf("[MAGGIE3D] - Clipping bottom vertex V3\n");)
      dyr -= (quad->v3.y - draw_data->bottom_clip);
    }
    M3D_FlatTextureMapping((UWORD) dyr, draw_data);
  } else {
    // Something to draw on upper side ?
    if (quad->v4.y >= draw_data->top_clip) {
      // Bottom clipping
      if (quad->v4.y > draw_data->bottom_clip) {
        DDbug(printf("[MAGGIE3D] - Clipping bottom vertex V4\n");)
        dyl -= (quad->v4.y - draw_data->bottom_clip);
        // Upper side
        M3D_FlatTextureMapping((UWORD) dyl, draw_data);
        return;       // No more to draw
      }
      // Upper side
      M3D_FlatTextureMapping((UWORD) dyl, draw_data);
      // Mid right
      dyr = quad->v2.y - quad->v4.y;
      // Mid left
      dyl = quad->v3.y - quad->v4.y;
      draw_data->delta_dxdyl = (quad->v3.x - quad->v4.x) / dyl;
      draw_data->delta_dzdyl = (quad->v3.z - quad->v4.z) / dyl;
      draw_data->delta_dudyl = (quad->v3.u - quad->v4.u) / dyl;
      draw_data->delta_dvdyl = (quad->v3.v - quad->v4.v) / dyl;
    } else {
      // No upper side, start with mid side
      dyl = quad->v3.y - quad->v4.y;
      draw_data->delta_dxdyl = (quad->v3.x - quad->v4.x) / dyl;
      draw_data->delta_dzdyl = (quad->v3.z - quad->v4.z) / dyl;
      draw_data->delta_dudyl = (quad->v3.u - quad->v4.u) / dyl;
      draw_data->delta_dvdyl = (quad->v3.v - quad->v4.v) / dyl;
      DDbug(printf("[MAGGIE3D] - Clipping top vertex V4\n");)
      clip_y = draw_data->top_clip - quad->v4.y;
      draw_data->crd_xl = quad->v4.x + (draw_data->delta_dxdyl * clip_y);
      draw_data->crd_zl = quad->v4.z + (draw_data->delta_dzdyl * clip_y);
      draw_data->crd_ul = quad->v4.u + (draw_data->delta_dudyl * clip_y);
      draw_data->crd_vl = quad->v4.v + (draw_data->delta_dvdyl * clip_y);
      dyr = quad->v2.y - draw_data->top_clip;
      dyl = quad->v3.y - draw_data->top_clip;
    }
    if (quad->v2.y < quad->v3.y) {
      // Something to draw on mid side
      if (quad->v2.y >= draw_data->top_clip) {
        // Bottom clipping
        if (quad->v2.y > draw_data->bottom_clip) {
          DDbug(printf("[MAGGIE3D] - Clipping bottom vertex V2\n");)
          dyr -= (quad->v2.y - draw_data->bottom_clip);
          // Mid side
          M3D_FlatTextureMapping((UWORD) dyr, draw_data);
          return;       // No more to draw
        }
        // Mid side
        M3D_FlatTextureMapping((UWORD) dyr, draw_data);
        // Lower side
        dyr = quad->v3.y - quad->v2.y;
        draw_data->delta_dxdyr = (quad->v3.x - quad->v2.x) / dyr;
        draw_data->delta_dzdyr = (quad->v3.z - quad->v2.z) / dyr;
        draw_data->delta_dudyr = (quad->v3.u - quad->v2.u) / dyr;
        draw_data->delta_dvdyr = (quad->v3.v - quad->v2.v) / dyr;
      } else {
        // Lower side
        dyr = quad->v3.y - quad->v2.y;
        draw_data->delta_dxdyr = (quad->v3.x - quad->v2.x) / dyr;
        draw_data->delta_dzdyr = (quad->v3.z - quad->v2.z) / dyr;
        draw_data->delta_dudyr = (quad->v3.u - quad->v2.u) / dyr;
        draw_data->delta_dvdyr = (quad->v3.v - quad->v2.v) / dyr;
        DDbug(printf("[MAGGIE3D] - Clipping top vertex V2\n");)
        clip_y = draw_data->top_clip - quad->v2.y;
        draw_data->crd_xr = quad->v2.x + (draw_data->delta_dxdyr * clip_y);
        draw_data->crd_zr = quad->v2.z + (draw_data->delta_dzdyr * clip_y);
        draw_data->crd_ur = quad->v2.u + (draw_data->delta_dudyr * clip_y);
        draw_data->crd_vr = quad->v2.v + (draw_data->delta_dvdyr * clip_y);
        dyr = quad->v3.y - draw_data->top_clip;
      }
      if (quad->v3.y > draw_data->bottom_clip) {
        DDbug(printf("[MAGGIE3D] - Clipping bottom vertex V3\n");)
        dyr -= (quad->v3.y - draw_data->bottom_clip);
      }
      M3D_FlatTextureMapping((UWORD) dyr, draw_data);
    } else {
      // Something to draw on mid side
      if (quad->v3.y >= draw_data->top_clip) {
        // Bottom clipping
        if (quad->v3.y > draw_data->bottom_clip) {
          DDbug(printf("[MAGGIE3D] - Clipping bottom vertex V3\n");)
          dyl -= (quad->v3.y - draw_data->bottom_clip);
          // Mid side
          M3D_FlatTextureMapping((UWORD) dyl, draw_data);
          return;       // No more to draw
        }
        // Mid side
        M3D_FlatTextureMapping((UWORD) dyl, draw_data);
        // Lower side
        dyl = quad->v2.y - quad->v3.y;
        draw_data->delta_dxdyl = (quad->v2.x - quad->v3.x) / dyl;
        draw_data->delta_dzdyl = (quad->v2.z - quad->v3.z) / dyl;
        draw_data->delta_dudyl = (quad->v2.u - quad->v3.u) / dyl;
        draw_data->delta_dvdyl = (quad->v2.v - quad->v3.v) / dyl;
      } else {
        // Lower side
        dyl = quad->v2.y - quad->v3.y;
        draw_data->delta_dxdyl = (quad->v2.x - quad->v3.x) / dyl;
        draw_data->delta_dzdyl = (quad->v2.z - quad->v3.z) / dyl;
        draw_data->delta_dudyl = (quad->v2.u - quad->v3.u) / dyl;
        draw_data->delta_dvdyl = (quad->v2.v - quad->v3.v) / dyl;
        DDbug(printf("[MAGGIE3D] - Clipping top vertex V3\n");)
        clip_y = draw_data->top_clip - quad->v3.y;
        draw_data->crd_xl = quad->v3.x + (draw_data->delta_dxdyl * clip_y);
        draw_data->crd_zl = quad->v3.z + (draw_data->delta_dzdyl * clip_y);
        draw_data->crd_ul = quad->v3.u + (draw_data->delta_dudyl * clip_y);
        draw_data->crd_vl = quad->v3.v + (draw_data->delta_dvdyl * clip_y);
        dyl = quad->v2.y - draw_data->top_clip;
      }
      if (quad->v2.y > draw_data->bottom_clip) {
        DDbug(printf("[MAGGIE3D] - Clipping bottom vertex V2\n");)
        dyl -= (quad->v2.y - draw_data->bottom_clip);
      }
      M3D_FlatTextureMapping((UWORD) dyl, draw_data);
    }
  }
}
