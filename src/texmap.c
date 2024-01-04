/**
 * texmap.c
 *
 * Maggie3D library
 * Texture mapping functions
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.0 November 2023
 */

#include <stdio.h>
#include <string.h>

#include <proto/graphics.h>

#include "debug.h"
#include "draw.h"

/** Maggie registers */
extern M3D_MaggieRegs *maggie;

#if _ACTIVATE_DEBUG_ == 1
extern BOOL draw_debug;
#endif

/*****************************************************************************/
/**                  GOURAUD MAPPING                                         */
/*****************************************************************************/

/** Map a gouraud shaded & textured triangle with Maggie */
VOID M3D_GouraudTextureMapping(UWORD nblines, M3D_DrawData *draw_data)
{
  FLOAT xs, xe, dx;
  FLOAT du, dv, dz, dl;
  FLOAT ui, vi, zi, li;
  ULONG dest, zbuf;
  LONG round;

  DDbug(printf("[DEBUG] - Go gouraud mapping for %d lines\n", nblines);)
  while (nblines--) {
    DDbug(printf("[DEBUG] Render line %d\n", nblines);)
    // Calcul edge coords
    round = (LONG) draw_data->crd_xl; xs = (FLOAT) round;
    round = (LONG) draw_data->crd_xr; xe = (FLOAT) round;
    DDbug(printf("[DEBUG] => xs=%f  xe=%f\n", xs, xe);)
    // Draw if line is not outside of clipping region
    if (xs < draw_data->right_clip && xe >= draw_data->left_clip) {
      // Calcul interpolations
      du = draw_data->crd_ur - draw_data->crd_ul;
      dv = draw_data->crd_vr - draw_data->crd_vl;
      dz = draw_data->crd_zr - draw_data->crd_zl;
      dl = draw_data->int_lr - draw_data->int_ll;
      dx = xe - xs + 1.0;
      // DX could be 0 in some situations
      if (dx > 0.0) {
        du /= dx;
        dv /= dx;
        dz /= dx;
        dl /= dx;
      }
      // Calcul texture coords
      ui = draw_data->crd_ul;
      vi = draw_data->crd_vl;
      // Calcul Z value
      zi = draw_data->crd_zl;
      // Calcul light intensity
      li = draw_data->int_ll;
      // Horizontal clipping
      if (xs < draw_data->left_clip) {
        DDbug(printf("[DEBUG] => left clipping\n");)
        dx = draw_data->left_clip - xs;
        ui += dx * du;
        vi += dx * dv;
        zi += dx * dz;
        li += dx * dl;
        xs = draw_data->left_clip;
        dx = xe - xs + 1.0;
      }
      if (xe >= draw_data->right_clip) {
        DDbug(printf("[DEBUG] => right clipping\n");)
        dx = draw_data->right_clip - xs + 1.0;
      }
      DDbug(printf("[DEBUG] => xs=%f  ui=%f  vi=%f  zi=%f  li=%f\n", xs, ui, vi, zi, li);)
      // Draw the line
      DDbug(printf("[DEBUG] => dx=%f  du=%f  dv=%f  dz=%f  dl=%f\n", dx, du, dv, dz, dl);)
      // Destination address
      dest = draw_data->dest_adr + ((LONG)xs * draw_data->dest_bpp);
      // Z buffer address
      zbuf = draw_data->zbuf_adr + ((LONG)xs * draw_data->zbuf_bpp);
      DDbug(printf("[DEBUG] => dbuf=0x%X  zbuf=0x%X\n", dest, zbuf);)
      // Start drawing
      maggie->destination = (APTR) dest;
      maggie->zbuffer = (APTR) zbuf;
      maggie->u_start = (LFIXED) (ui * draw_data->scale);
      maggie->v_start = (LFIXED) (vi * draw_data->scale);
      maggie->u_delta = (LFIXED) (du * draw_data->scale);
      maggie->v_delta = (LFIXED) (dv * draw_data->scale);
      maggie->light_start = (UFIXED) (li * 65535.0);
      maggie->light_delta = (SFIXED) (dl * 32768.0);
      maggie->z_start = (LFIXED) (zi * 65536.0);
      maggie->z_delta = (LFIXED) (dz * 65536.0);
      WaitBlit();
      maggie->start_length = (UWORD) dx;
#if _USE_MAGGIE_ == 0
      M3D_EmulateMaggie();
#endif
      DDbug(printf("[DEBUG] => Rendering %d texels\n", (UWORD) dx);)
    }
    // Interpolate next left side points
    draw_data->crd_xl += draw_data->delta_dxdyl;
    draw_data->crd_zl += draw_data->delta_dzdyl;
    draw_data->crd_ul += draw_data->delta_dudyl;
    draw_data->crd_vl += draw_data->delta_dvdyl;
    draw_data->int_ll += draw_data->delta_dldyl;
    // Interpolate next right side points
    draw_data->crd_xr += draw_data->delta_dxdyr;
    draw_data->crd_zr += draw_data->delta_dzdyr;
    draw_data->crd_ur += draw_data->delta_dudyr;
    draw_data->crd_vr += draw_data->delta_dvdyr;
    draw_data->int_lr += draw_data->delta_dldyr;
    // Next line address
    draw_data->dest_adr += draw_data->dest_bpr;
    draw_data->zbuf_adr += draw_data->zbuf_bpr;
  }
}

/** Draw a gouraud shaded & textured top triangle */
VOID M3D_DrawGouraudTexturedTop(M3D_Context *context, M3D_Triangle *triangle, M3D_DrawData *draw_data)
{
  FLOAT delta_y, clip_y;

  DDbug(printf("[DEBUG] M3D_DrawGouraudTexturedTop\n");)
  // Delta height
  delta_y = triangle->v3.y - triangle->v1.y;
  // Left side delta
  draw_data->delta_dxdyl = (triangle->v3.x - triangle->v1.x) / delta_y;
  draw_data->delta_dzdyl = (triangle->v3.z - triangle->v1.z) / delta_y;
  draw_data->delta_dudyl = (triangle->v3.u - triangle->v1.u) / delta_y;
  draw_data->delta_dvdyl = (triangle->v3.v - triangle->v1.v) / delta_y;
  draw_data->delta_dldyl = (triangle->v3.light - triangle->v1.light) / delta_y;
  // Right side delta
  draw_data->delta_dxdyr = (triangle->v3.x - triangle->v2.x) / delta_y;
  draw_data->delta_dzdyr = (triangle->v3.z - triangle->v2.z) / delta_y;
  draw_data->delta_dudyr = (triangle->v3.u - triangle->v2.u) / delta_y;
  draw_data->delta_dvdyr = (triangle->v3.v - triangle->v2.v) / delta_y;
  draw_data->delta_dldyr = (triangle->v3.light - triangle->v2.light) / delta_y;
  // Start coords & clipping
  if (triangle->v1.y < draw_data->top_clip) {
    DDbug(printf("[DEBUG] - Clipping top vertex\n");)
    clip_y = draw_data->top_clip - triangle->v1.y;
    draw_data->crd_xl = triangle->v1.x + (draw_data->delta_dxdyl * clip_y);
    draw_data->crd_xr = triangle->v2.x + (draw_data->delta_dxdyr * clip_y);
    draw_data->crd_zl = triangle->v1.z + (draw_data->delta_dzdyl * clip_y);
    draw_data->crd_zr = triangle->v2.z + (draw_data->delta_dzdyr * clip_y);
    draw_data->crd_ul = triangle->v1.u + (draw_data->delta_dudyl * clip_y);
    draw_data->crd_ur = triangle->v2.u + (draw_data->delta_dudyr * clip_y);
    draw_data->crd_vl = triangle->v1.v + (draw_data->delta_dvdyl * clip_y);
    draw_data->crd_vr = triangle->v2.v + (draw_data->delta_dvdyr * clip_y);
    draw_data->int_ll = triangle->v1.light + (draw_data->delta_dldyl * clip_y);
    draw_data->int_lr = triangle->v2.light + (draw_data->delta_dldyr * clip_y);
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
    draw_data->int_ll = triangle->v1.light;
    draw_data->int_lr = triangle->v2.light;
    // Line & zbuf start address
    draw_data->dest_adr = (ULONG)context->drawregion.data + (context->drawregion.bpr * (ULONG)triangle->v1.y);
    draw_data->zbuf_adr = (ULONG)context->zbuffer.data + (context->zbuffer.bpr * (ULONG)triangle->v1.y);
  }
  DDbug(M3D_DumpDrawData(draw_data);)
  // Bottom clipping
  if (triangle->v3.y > draw_data->bottom_clip) {
    DDbug(printf("[DEBUG] - Clipping bottom vertex\n");)
    delta_y -= (triangle->v3.y - draw_data->bottom_clip);
  }
  // Go for mapping
  M3D_GouraudTextureMapping((UWORD) delta_y, draw_data);
}

/** Draw a gouraud shaded & textured bottom triangle */
VOID M3D_DrawGouraudTexturedBottom(M3D_Context *context, M3D_Triangle *triangle, M3D_DrawData *draw_data)
{
  FLOAT delta_y, clip_y;

  DDbug(printf("[DEBUG] M3D_DrawGouraudTexturedBottom\n");)
  // Delta height
  delta_y = triangle->v2.y - triangle->v1.y;
  // Left side delta
  draw_data->delta_dxdyl = (triangle->v2.x - triangle->v1.x) / delta_y;
  draw_data->delta_dzdyl = (triangle->v2.z - triangle->v1.z) / delta_y;
  draw_data->delta_dudyl = (triangle->v2.u - triangle->v1.u) / delta_y;
  draw_data->delta_dvdyl = (triangle->v2.v - triangle->v1.v) / delta_y;
  draw_data->delta_dldyl = (triangle->v2.light - triangle->v1.light) / delta_y;
  // Right side delta
  draw_data->delta_dxdyr = (triangle->v3.x - triangle->v1.x) / delta_y;
  draw_data->delta_dzdyr = (triangle->v3.z - triangle->v1.z) / delta_y;
  draw_data->delta_dudyr = (triangle->v3.u - triangle->v1.u) / delta_y;
  draw_data->delta_dvdyr = (triangle->v3.v - triangle->v1.v) / delta_y;
  draw_data->delta_dldyr = (triangle->v3.light - triangle->v1.light) / delta_y;
  // Start coords & clipping
  if (triangle->v1.y < draw_data->top_clip) {
    DDbug(printf("[DEBUG] - Clipping top vertex\n");)
    clip_y = draw_data->top_clip - triangle->v1.y;
    draw_data->crd_xl = triangle->v1.x + (draw_data->delta_dxdyl * clip_y);
    draw_data->crd_xr = triangle->v1.x + (draw_data->delta_dxdyr * clip_y);
    draw_data->crd_zl = triangle->v1.z + (draw_data->delta_dzdyl * clip_y);
    draw_data->crd_zr = triangle->v1.z + (draw_data->delta_dzdyr * clip_y);
    draw_data->crd_ul = triangle->v1.u + (draw_data->delta_dudyl * clip_y);
    draw_data->crd_ur = triangle->v1.u + (draw_data->delta_dudyr * clip_y);
    draw_data->crd_vl = triangle->v1.v + (draw_data->delta_dvdyl * clip_y);
    draw_data->crd_vr = triangle->v1.v + (draw_data->delta_dvdyr * clip_y);
    draw_data->int_ll = triangle->v1.light + (draw_data->delta_dldyl * clip_y);
    draw_data->int_lr = triangle->v1.light + (draw_data->delta_dldyr * clip_y);
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
    draw_data->int_ll = triangle->v1.light;
    draw_data->int_lr = triangle->v1.light;
    // Line & zbuf start address
    draw_data->dest_adr = (ULONG)context->drawregion.data + (context->drawregion.bpr * (ULONG)triangle->v1.y);
    draw_data->zbuf_adr = (ULONG)context->zbuffer.data + (context->zbuffer.bpr * (ULONG)triangle->v1.y);
  }
  DDbug(M3D_DumpDrawData(draw_data);)
  // Bottom clipping
  if (triangle->v3.y > draw_data->bottom_clip) {
    DDbug(printf("[DEBUG] - Clipping bottom vertex\n");)
    delta_y -= (triangle->v3.y - draw_data->bottom_clip);
  }
  // Go for mapping
  M3D_GouraudTextureMapping((UWORD) delta_y, draw_data);
}

/** Draw a gouraud shaded & textured generic triangle */
VOID M3D_DrawGouraudTexturedGeneric(M3D_Context *context, M3D_Triangle *triangle, M3D_DrawData *draw_data)
{
  FLOAT delta_y1, delta_y2, delta_y3;
  FLOAT dxdy1, dxdy2;
  FLOAT clip_y1, clip_y2;
  BOOL left_long;

  DDbug(printf("[DEBUG] M3D_DrawGouraudTexturedGeneric\n");)
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
    // Left side light delta
    draw_data->delta_dldyl = (triangle->v3.light - triangle->v1.light) / delta_y2;
    // Right side light delta
    draw_data->delta_dldyr = (triangle->v2.light - triangle->v1.light) / delta_y1;
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
    // Left side light delta
    draw_data->delta_dldyl = (triangle->v2.light - triangle->v1.light) / delta_y2;
    // Right side light delta
    draw_data->delta_dldyr = (triangle->v3.light - triangle->v1.light) / delta_y1;
    // Slope, right long
    left_long = FALSE;
  }
  // y2 top clipping, we only have to draw the triangle lower part
  if (triangle->v2.y < draw_data->top_clip) {
    DDbug(printf("[DEBUG] - Clipping top vertex 2, draw only lower triangle\n");)
    clip_y1 = draw_data->top_clip - triangle->v1.y;
    clip_y2 = draw_data->top_clip - triangle->v2.y;
    delta_y3 = triangle->v3.y - triangle->v2.y;
    if (left_long) {
      draw_data->delta_dxdyr = (triangle->v3.x - triangle->v2.x) / delta_y3;
      draw_data->delta_dzdyr = (triangle->v3.z - triangle->v2.z) / delta_y3;
      draw_data->delta_dudyr = (triangle->v3.u - triangle->v2.u) / delta_y3;
      draw_data->delta_dvdyr = (triangle->v3.v - triangle->v2.v) / delta_y3;
      draw_data->delta_dldyr = (triangle->v3.light - triangle->v2.light) / delta_y3;
      draw_data->crd_xl = triangle->v1.x + (draw_data->delta_dxdyl * clip_y1);
      draw_data->crd_xr = triangle->v2.x + (draw_data->delta_dxdyr * clip_y2);
      draw_data->crd_zl = triangle->v1.z + (draw_data->delta_dzdyl * clip_y1);
      draw_data->crd_zr = triangle->v2.z + (draw_data->delta_dzdyr * clip_y2);
      draw_data->crd_ul = triangle->v1.u + (draw_data->delta_dudyl * clip_y1);
      draw_data->crd_ur = triangle->v2.u + (draw_data->delta_dudyr * clip_y2);
      draw_data->crd_vl = triangle->v1.v + (draw_data->delta_dvdyl * clip_y1);
      draw_data->crd_vr = triangle->v2.v + (draw_data->delta_dvdyr * clip_y2);
      draw_data->int_ll = triangle->v1.light + (draw_data->delta_dldyl * clip_y1);
      draw_data->int_lr = triangle->v2.light + (draw_data->delta_dldyr * clip_y2);
    } else {
      draw_data->delta_dxdyl = (triangle->v3.x - triangle->v2.x) / delta_y3;
      draw_data->delta_dzdyl = (triangle->v3.z - triangle->v2.z) / delta_y3;
      draw_data->delta_dudyl = (triangle->v3.u - triangle->v2.u) / delta_y3;
      draw_data->delta_dvdyl = (triangle->v3.v - triangle->v2.v) / delta_y3;
      draw_data->delta_dldyl = (triangle->v3.light - triangle->v2.light) / delta_y3;
      draw_data->crd_xl = triangle->v2.x + (draw_data->delta_dxdyl * clip_y2);
      draw_data->crd_xr = triangle->v1.x + (draw_data->delta_dxdyr * clip_y1);
      draw_data->crd_zl = triangle->v2.z + (draw_data->delta_dzdyl * clip_y2);
      draw_data->crd_zr = triangle->v1.z + (draw_data->delta_dzdyr * clip_y1);
      draw_data->crd_ul = triangle->v2.u + (draw_data->delta_dudyl * clip_y2);
      draw_data->crd_ur = triangle->v1.u + (draw_data->delta_dudyr * clip_y1);
      draw_data->crd_vl = triangle->v2.v + (draw_data->delta_dvdyl * clip_y2);
      draw_data->crd_vr = triangle->v1.v + (draw_data->delta_dvdyr * clip_y1);
      draw_data->int_ll = triangle->v2.light + (draw_data->delta_dldyl * clip_y2);
      draw_data->int_lr = triangle->v1.light + (draw_data->delta_dldyr * clip_y1);
    }
    // Line & zbuf start address
    draw_data->dest_adr = (ULONG)context->drawregion.data + (context->drawregion.bpr * (ULONG)draw_data->top_clip);
    draw_data->zbuf_adr = (ULONG)context->zbuffer.data + (context->zbuffer.bpr * (ULONG)draw_data->top_clip);
    DDbug(M3D_DumpDrawData(draw_data);)
    delta_y3 = triangle->v3.y - draw_data->top_clip;
    // Bottom clipping
    if (triangle->v3.y > draw_data->bottom_clip) {
      DDbug(printf("[DEBUG] - Clipping bottom vertex 3\n");)
      delta_y3 -= (triangle->v3.y - draw_data->bottom_clip);
    }
    // Go for mapping
    M3D_GouraudTextureMapping((UWORD) delta_y3, draw_data);
  } else {
    // y1 top clipping
    if (triangle->v1.y < draw_data->top_clip) {
      DDbug(printf("[DEBUG] - Clipping top vertex 1\n");)
      clip_y1 = draw_data->top_clip - triangle->v1.y;
      draw_data->crd_xl = triangle->v1.x + (draw_data->delta_dxdyl * clip_y1);
      draw_data->crd_xr = triangle->v1.x + (draw_data->delta_dxdyr * clip_y1);
      draw_data->crd_zl = triangle->v1.z + (draw_data->delta_dzdyl * clip_y1);
      draw_data->crd_zr = triangle->v1.z + (draw_data->delta_dzdyr * clip_y1);
      draw_data->crd_ul = triangle->v1.u + (draw_data->delta_dudyl * clip_y1);
      draw_data->crd_ur = triangle->v1.u + (draw_data->delta_dudyr * clip_y1);
      draw_data->crd_vl = triangle->v1.v + (draw_data->delta_dvdyl * clip_y1);
      draw_data->crd_vr = triangle->v1.v + (draw_data->delta_dvdyr * clip_y1);
      draw_data->int_ll = triangle->v1.light + (draw_data->delta_dldyl * clip_y1);
      draw_data->int_lr = triangle->v1.light + (draw_data->delta_dldyr * clip_y1);
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
      draw_data->int_ll = triangle->v1.light;
      draw_data->int_lr = triangle->v1.light;
      // Line & zbuf start address
      draw_data->dest_adr = (ULONG)context->drawregion.data + (context->drawregion.bpr * (ULONG)triangle->v1.y);
      draw_data->zbuf_adr = (ULONG)context->zbuffer.data + (context->zbuffer.bpr * (ULONG)triangle->v1.y);
    }
    DDbug(M3D_DumpDrawData(draw_data);)
    // y2 bottom clipping, we only have to draw the triangle upper part
    if (triangle->v2.y > draw_data->bottom_clip) {
      DDbug(printf("[DEBUG] - Clipping bottom vertex 2, draw only upper triangle\n");)
      delta_y1 -= (triangle->v2.y - draw_data->bottom_clip);
      // Go for mapping
      M3D_GouraudTextureMapping((UWORD) delta_y1, draw_data);
    } else {
      // Go for mapping
      M3D_GouraudTextureMapping((UWORD) delta_y1, draw_data);
      delta_y3 = triangle->v3.y - triangle->v2.y;
      // Calcul new deltas
      if (left_long) {
        draw_data->delta_dxdyr = (triangle->v3.x - triangle->v2.x) / delta_y3;
        draw_data->delta_dzdyr = (triangle->v3.z - triangle->v2.z) / delta_y3;
        draw_data->delta_dudyr = (triangle->v3.u - triangle->v2.u) / delta_y3;
        draw_data->delta_dvdyr = (triangle->v3.v - triangle->v2.v) / delta_y3;
        draw_data->delta_dldyr = (triangle->v3.light - triangle->v2.light) / delta_y3;
      } else {
        draw_data->delta_dxdyl = (triangle->v3.x - triangle->v2.x) / delta_y3;
        draw_data->delta_dzdyl = (triangle->v3.z - triangle->v2.z) / delta_y3;
        draw_data->delta_dudyl = (triangle->v3.u - triangle->v2.u) / delta_y3;
        draw_data->delta_dvdyl = (triangle->v3.v - triangle->v2.v) / delta_y3;
        draw_data->delta_dldyl = (triangle->v3.light - triangle->v2.light) / delta_y3;
      }
      // Line & zbuf start address
      draw_data->dest_adr = (ULONG)context->drawregion.data + (context->drawregion.bpr * (ULONG)triangle->v2.y);
      draw_data->zbuf_adr = (ULONG)context->zbuffer.data + (context->zbuffer.bpr * (ULONG)triangle->v2.y);
      DDbug(M3D_DumpDrawData(draw_data);)
      // Bottom clipping
      if (triangle->v3.y > draw_data->bottom_clip) {
        DDbug(printf("[DEBUG] - Clipping bottom vertex 3\n");)
        delta_y3 -= (triangle->v3.y - draw_data->bottom_clip);
      }
      // Go for mapping
      M3D_GouraudTextureMapping((UWORD) delta_y3, draw_data);
    }
  }
}

/*****************************************************************************/
/**                     FLAT MAPPING                                         */
/*****************************************************************************/

/** Map a flat shaded & textured triangle with Maggie */
VOID M3D_FlatTextureMapping(UWORD nblines, M3D_DrawData *draw_data)
{
  FLOAT xs, xe, dx;
  FLOAT du, dv, dz;
  FLOAT ui, vi, zi;
  ULONG dest, zbuf;
  LONG round;

  DDbug(printf("[DEBUG] - Go flat shade mapping for %d lines\n", nblines);)
  // Light will not change for flat shading
  maggie->light_start = (UFIXED) (draw_data->int_ll * 65535.0);
  maggie->light_delta = (SFIXED) 0;
  while (nblines--) {
    DDbug(printf("[DEBUG] Render line %d\n", nblines);)
    // Calcul edge coords
    round = (LONG) draw_data->crd_xl; xs = (FLOAT) round;
    round = (LONG) draw_data->crd_xr; xe = (FLOAT) round;
    DDbug(printf("[DEBUG] => xs=%f  xe=%f\n", xs, xe);)
    // Draw if line is not outside of clipping region
    if (xs < draw_data->right_clip && xe >= draw_data->left_clip) {
      // Calcul interpolations
      du = draw_data->crd_ur - draw_data->crd_ul;
      dv = draw_data->crd_vr - draw_data->crd_vl;
      dz = draw_data->crd_zr - draw_data->crd_zl;
      dx = xe - xs + 1.0;
      // DX could be 0 in some situations
      if (dx > 0.0) {
        du /= dx;
        dv /= dx;
        dz /= dx;
      }
      // Calcul texture coords
      ui = draw_data->crd_ul;
      vi = draw_data->crd_vl;
      // Calcul Z value
      zi = draw_data->crd_zl;
      // Horizontal clipping
      if (xs < draw_data->left_clip) {
        DDbug(printf("[DEBUG] => left clipping\n");)
        dx = draw_data->left_clip - xs;
        ui += dx * du;
        vi += dx * dv;
        zi += dx * dz;
        xs = draw_data->left_clip;
        dx = xe - xs + 1.0;
      }
      if (xe >= draw_data->right_clip) {
        DDbug(printf("[DEBUG] => right clipping\n");)
        dx = draw_data->right_clip - xs + 1.0;
      }
      DDbug(printf("[DEBUG] => xs=%f  ui=%f  vi=%f  zi=%f\n", xs, ui, vi, zi);)
      // Draw the line
      DDbug(printf("[DEBUG] => dx=%f  du=%f  dv=%f  dz=%f\n", dx, du, dv, dz);)
      // Destination address
      dest = draw_data->dest_adr + ((LONG)xs * draw_data->dest_bpp);
      // Z buffer address
      zbuf = draw_data->zbuf_adr + ((LONG)xs * draw_data->zbuf_bpp);
      DDbug(printf("[DEBUG] => dbuf=0x%X  zbuf=0x%X\n", dest, zbuf);)
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
      DDbug(printf("[DEBUG] => Rendering %d texels\n", (UWORD) dx);)
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

/** Draw a flat shaded & textured top triangle */
VOID M3D_DrawFlatTexturedTop(M3D_Context *context, M3D_Triangle *triangle, M3D_DrawData *draw_data)
{
  FLOAT delta_y, clip_y;

  DDbug(printf("[DEBUG] M3D_DrawFlatTexturedTop\n");)
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
    DDbug(printf("[DEBUG] - Clipping top vertex\n");)
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
  DDbug(M3D_DumpDrawData(draw_data);)
  // Bottom clipping
  if (triangle->v3.y > draw_data->bottom_clip) {
    DDbug(printf("[DEBUG] - Clipping bottom vertex\n");)
    delta_y -= (triangle->v3.y - draw_data->bottom_clip);
  }
  // Go for mapping
  M3D_FlatTextureMapping((UWORD) delta_y, draw_data);
}

/** Draw a flat shaded & textured bottom triangle */
VOID M3D_DrawFlatTexturedBottom(M3D_Context *context, M3D_Triangle *triangle, M3D_DrawData *draw_data)
{
  FLOAT delta_y, clip_y;

  DDbug(printf("[DEBUG] M3D_DrawFlatTexturedBottom\n");)
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
    DDbug(printf("[DEBUG] - Clipping top vertex\n");)
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
  DDbug(M3D_DumpDrawData(draw_data);)
  // Bottom clipping
  if (triangle->v3.y > draw_data->bottom_clip) {
    DDbug(printf("[DEBUG] - Clipping bottom vertex\n");)
    delta_y -= (triangle->v3.y - draw_data->bottom_clip);
  }
  // Go for mapping
  M3D_FlatTextureMapping((UWORD) delta_y, draw_data);
}

/** Draw a flat shaded & textured generic triangle */
VOID M3D_DrawFlatTexturedGeneric(M3D_Context *context, M3D_Triangle *triangle, M3D_DrawData *draw_data)
{
  FLOAT delta_y1, delta_y2, delta_y3;
  FLOAT dxdy1, dxdy2;
  FLOAT clip_y1, clip_y2;
  BOOL left_long;

  DDbug(printf("[DEBUG] M3D_DrawFlatTexturedGeneric\n");)
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
    DDbug(printf("[DEBUG] - Clipping top vertex 2, draw only lower triangle\n");)
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
    DDbug(M3D_DumpDrawData(draw_data);)
    delta_y3 = triangle->v3.y - draw_data->top_clip;
    // Bottom clipping
    if (triangle->v3.y > draw_data->bottom_clip) {
      DDbug(printf("[DEBUG] - Clipping bottom vertex 3\n");)
      delta_y3 -= (triangle->v3.y - draw_data->bottom_clip);
    }
    // Go for mapping
    M3D_FlatTextureMapping((UWORD) delta_y3, draw_data);
  } else {
    // y1 top clipping
    if (triangle->v1.y < draw_data->top_clip) {
      DDbug(printf("[DEBUG] - Clipping top vertex 1\n");)
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
    DDbug(M3D_DumpDrawData(draw_data);)
    // y2 bottom clipping, we only have to draw the triangle upper part
    if (triangle->v2.y > draw_data->bottom_clip) {
      DDbug(printf("[DEBUG] - Clipping bottom vertex 2, draw only upper triangle\n");)
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
      DDbug(M3D_DumpDrawData(draw_data);)
      // Bottom clipping
      if (triangle->v3.y > draw_data->bottom_clip) {
        DDbug(printf("[DEBUG] - Clipping bottom vertex 3\n");)
        delta_y3 -= (triangle->v3.y - draw_data->bottom_clip);
      }
      // Go for mapping
      M3D_FlatTextureMapping((UWORD) delta_y3, draw_data);
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
  maggie->texture = triangle->texture->data;
  maggie->tex_size = triangle->texture->mipsize;
  maggie->color = triangle->color;
  // Setup texture scale
  if (context->states & M3D_TEXNORMCRD) {
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
