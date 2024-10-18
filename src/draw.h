/**
 * maggie.h
 *
 * Maggie3D shared library
 * Drawing functions
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.6 June 2024 (updated: 24/06/2024)
 */

#ifndef _DRAW_H_
#define _DRAW_H_

#include "Maggie3D.h"

#define M3D_MAGGIEBASE      0xdff250

/** 8:8 fixed floating point */
typedef signed short SFIXED;
typedef unsigned short UFIXED;

/** 16:16 fixed floating point */
typedef signed long LFIXED;

// Maggie registers
typedef struct
{
  APTR texture;         /* $DFF250 32bit texture source */
  APTR destination;     /* $DFF254 32bit Destination Screen Addr */
  APTR zbuffer;         /* $DFF258 32bit ZBuffer Addr */
  UWORD unused1;        /* $DFF25C */
  UWORD start_length;   /* $DFF25E 16bit LEN and START */
  UWORD tex_size;       /* $DFF260 16bit MIP texture size (9=512/8=256/7=128/6=64) */
  UWORD mode;           /* $DFF262 16bit MODE (Bit0=Bilinear) (Bit1=Zbuffer) (Bit2=16bit output) (Bit3=inhibit Zbuffer) */
  UWORD unused2;        /* $DFF264 */ 
  UWORD modulo;         /* $DFF266 16bit Destination Step */
  ULONG unused3;        /* $DFF268 */ 
  ULONG unused4;        /* $DFF26C */
  LFIXED u_start;       /* $DFF270 32bit U (16:16 fixed) */
  LFIXED v_start;       /* $DFF274 32bit V (16:16 fixed) */
  LFIXED u_delta;       /* $DFF278 32bit dU (16:16 fixed) */
  LFIXED v_delta;       /* $DFF27C 32bit dV (16:16 fixed) */
  UFIXED light_start;   /* $DFF280 16bit Light Ll (8:8 fixed) */
  SFIXED light_delta;   /* $DFF282 16bit Light dLl (8:8 fixed) */
  ULONG color;          /* $DFF284 32bit Light color (ARGB) */
  LFIXED z_start;       /* $DFF288 32bit Z (16:16 fixed) */
  LFIXED z_delta;       /* $DFF28C 32bit Delta (16:16 fixed) */
} M3D_MaggieRegs;

// Texture mapping data
typedef struct {
  // Clipping
  FLOAT left_clip, top_clip, right_clip, bottom_clip;
  // Delta X, Z, U, V & L for the left side
  FLOAT delta_dxdyl, delta_dzdyl, delta_dudyl, delta_dvdyl, delta_dldyl;
  // Delta X, Z, U, V & L for the right side
  FLOAT delta_dxdyr, delta_dzdyr, delta_dudyr, delta_dvdyr, delta_dldyr;
  // Coordinates X & Z for left & right side
  FLOAT crd_xl, crd_xr, crd_zl, crd_zr;
  // Coordinates U & V for left and right side
  FLOAT crd_ul, crd_ur, crd_vl, crd_vr, scale;
  // Ligh intensity L for left and right side
  FLOAT int_ll, int_lr;
  // Line start adr
  ULONG dest_adr, dest_bpr, dest_bpp;
  // Zbuf start adr
  ULONG zbuf_adr, zbuf_bpr, zbuf_bpp;
} M3D_DrawData;

// Triangle type
#define TRI_REJECTED          0
#define TRI_FLATTOP           1
#define TRI_FLATBOTTOM        2
#define TRI_GENERIC           3

// Quad type
#define QUAD_GENERIC          0
#define QUAD_FLATTOP          1
#define QUAD_FLATBOTTOM       2
#define QUAD_FLATBOTH         3
#define QUAD_REJECTED         4

#if _USE_MAGGIE_ == 0
VOID M3D_EmulateMaggie(VOID);
#endif

#if _USE_FASTASM_ == 1
extern VOID __asm M3D_FastEmulateMaggie(
  register __a6 APTR maggie
);
#endif

/**
 * Drawing functions
 */

// Flat shaded triangle
VOID M3D_DrawFlatShadedTop(M3D_Context *, M3D_Triangle *, M3D_DrawData *);
VOID M3D_DrawFlatShadedBottom(M3D_Context *, M3D_Triangle *, M3D_DrawData *);
VOID M3D_DrawFlatShadedGeneric(M3D_Context *, M3D_Triangle *, M3D_DrawData *);

// Flat shaded quad
VOID M3D_DrawQuadFlatShadedTop(M3D_Context *, M3D_Quad *, M3D_DrawData *);
VOID M3D_DrawQuadFlatShadedBottom(M3D_Context *, M3D_Quad *, M3D_DrawData *);
VOID M3D_DrawQuadFlatShadedBoth(M3D_Context *, M3D_Quad *, M3D_DrawData *);
VOID M3D_DrawQuadFlatShadedGeneric(M3D_Context *, M3D_Quad *, M3D_DrawData *);

// Gouraud shaded triangle
VOID M3D_DrawGouraudShadedTop(M3D_Context *, M3D_Triangle *, M3D_DrawData *);
VOID M3D_DrawGouraudShadedBottom(M3D_Context *, M3D_Triangle *, M3D_DrawData *);
VOID M3D_DrawGouraudShadedGeneric(M3D_Context *, M3D_Triangle *, M3D_DrawData *);

// Gouraud shaded quad
VOID M3D_DrawQuadGouraudShadedTop(M3D_Context *, M3D_Quad *, M3D_DrawData *);
VOID M3D_DrawQuadGouraudShadedBottom(M3D_Context *, M3D_Quad *, M3D_DrawData *);
VOID M3D_DrawQuadGouraudShadedBoth(M3D_Context *, M3D_Quad *, M3D_DrawData *);
VOID M3D_DrawQuadGouraudShadedGeneric(M3D_Context *, M3D_Quad *, M3D_DrawData *);

// Flat textured triangle
VOID M3D_DrawFlatTexturedTop(M3D_Context *, M3D_Triangle *, M3D_DrawData *);
VOID M3D_DrawFlatTexturedBottom(M3D_Context *, M3D_Triangle *, M3D_DrawData *);
VOID M3D_DrawFlatTexturedGeneric(M3D_Context *, M3D_Triangle *, M3D_DrawData *);

// Flat textured quad
VOID M3D_DrawQuadFlatTexturedTop(M3D_Context *, M3D_Quad *, M3D_DrawData *);
VOID M3D_DrawQuadFlatTexturedBottom(M3D_Context *, M3D_Quad *, M3D_DrawData *);
VOID M3D_DrawQuadFlatTexturedBoth(M3D_Context *, M3D_Quad *, M3D_DrawData *);
VOID M3D_DrawQuadFlatTexturedGeneric(M3D_Context *, M3D_Quad *, M3D_DrawData *);

// Gouraud textured triangle
VOID M3D_DrawGouraudTexturedTop(M3D_Context *, M3D_Triangle *, M3D_DrawData *);
VOID M3D_DrawGouraudTexturedBottom(M3D_Context *, M3D_Triangle *, M3D_DrawData *);
VOID M3D_DrawGouraudTexturedGeneric(M3D_Context *, M3D_Triangle *, M3D_DrawData *);

// Gouraud textured quad
VOID M3D_DrawQuadGouraudTexturedTop(M3D_Context *, M3D_Quad *, M3D_DrawData *);
VOID M3D_DrawQuadGouraudTexturedBottom(M3D_Context *, M3D_Quad *, M3D_DrawData *);
VOID M3D_DrawQuadGouraudTexturedBoth(M3D_Context *, M3D_Quad *, M3D_DrawData *);
VOID M3D_DrawQuadGouraudTexturedGeneric(M3D_Context *, M3D_Quad *, M3D_DrawData *);

/** External function for rounding coordinates */
extern VOID __asm M3D_FastRoundTriangle(
  register __a0 M3D_Triangle *triangle
);

/** External function for rounding coordinates */
extern VOID __asm M3D_FastRoundQuad(
  register __a0 M3D_Quad *quad
);

/** External function for draw region clear */
extern VOID __asm M3D_FastClearRegion16(
  register __a0 APTR region,
  register __d0 ULONG width,
  register __d1 ULONG height,
  register __d2 ULONG bpr,
  register __d3 ULONG color
);

extern VOID __asm M3D_FastClearRegion24(
  register __a0 APTR region,
  register __d0 ULONG width,
  register __d1 ULONG height,
  register __d2 ULONG bpr,
  register __d3 ULONG color
);

extern VOID __asm M3D_FastClearRegion32(
  register __a0 APTR region,
  register __d0 ULONG width,
  register __d1 ULONG height,
  register __d2 ULONG bpr,
  register __d3 ULONG color
);

#if _ACTIVATE_DEBUG_ == 1
VOID M3D_DumpTriangle(M3D_Triangle *);
VOID M3D_DumpQuad(M3D_Quad *);
VOID M3D_DumpSprite(M3D_Sprite *);
VOID M3D_DumpDrawData(M3D_DrawData *);
#endif

#define RAD(x)       ((x)*PI/180.0)

#endif
