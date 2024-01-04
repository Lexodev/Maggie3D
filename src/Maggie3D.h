/**
 * Maggie3D.h
 *
 * Maggie3D library
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.0 November 2023
 */

#ifndef _MAGGIE3D_H_
#define _MAGGIE3D_H_

#include <exec/types.h>
#include <graphics/gfx.h>

// Error codes
#define M3D_SUCCESS               0             // No error
#define M3D_NOMAGGIE              -1            // Maggie not available
#define M3D_NOMEMORY              -2            // No more memory
#define M3D_NOCONTEXT             -3            // Context not available
#define M3D_NOBITMAP              -4            // Bitmap not abailable
#define M3D_NOZBUFFER             -5            // Z buffer not available
#define M3D_FILEREAD              -6            // Read file error
#define M3D_TEXTYPE               -7            // Bad texture type
#define M3D_TEXSIZE               -8            // Bad texture size
#define M3D_BITMAPTYPE            -9            // Bad bitmap type (not CGX)
#define M3D_NOTRIANGLE            -10           // Triangle is degenerated and not drawable
#define M3D_NOTEXTURE             -11           // No texture data

// Maggie mode
#define M3D_BILINEAR              (1 << 0)
#define M3D_ZBUFFER               (1 << 1)
#define M3D_16BITS                (1 << 2)
#define M3D_INHIBZBUF             (1 << 3)
#define M3D_24BITS                (1 << 4)
#define M3D_TEXMAPPING            (1 << 8)
#define M3D_GOURAUD               (1 << 9)
#define M3D_TEXNORMCRD            (1 << 10)
#define M3D_FAST                  (1 << 15)

#define M3D_DISABLE               0
#define M3D_ENABLE                1

// Maggie texture size
#define M3D_TEX64                 6
#define M3D_TEX128                7
#define M3D_TEX256                8
#define M3D_TEX512                9

// Texture pixel format
#define M3D_PIXFMT_UNKNOWN        0
#define M3D_PIXFMT_CLUT           1
#define M3D_PIXFMT_RGB15          2
#define M3D_PIXFMT_RGB16          3
#define M3D_PIXFMT_RGB24          4
#define M3D_PIXFMT_ARGB32         5
#define M3D_PIXFMT_DXT1           6

// Maggie3D vertex
typedef struct {
  FLOAT x, y, z;
  FLOAT w;
  FLOAT u, v;
  FLOAT light;
} M3D_Vertex;

// Maggie3D texture
typedef struct {
  APTR data;
  ULONG width, height;
  UWORD mipsize;
} M3D_Texture;

// Maggie3D triangle
typedef struct {
  M3D_Vertex v1, v2, v3;
  M3D_Texture *texture;
  ULONG color;
} M3D_Triangle;

// Maggie3D clipping scissor
typedef struct {
  ULONG left, top, width, height;
} M3D_Scissor;

// Maggie3D Z buffer
typedef struct {
  APTR data;
  ULONG width, height;
  ULONG bpr, bpp;
} M3D_ZBuffer;

// Maggie3D bitmap
typedef struct {
  struct BitMap *bitmap;
  APTR data;
  ULONG width, height;
  ULONG depth, bpr, bpp;
} M3D_Bitmap;

// Maggie3D context
typedef struct {
  WORDBITS states;
  M3D_Bitmap drawregion;
  M3D_Scissor clipping;
  M3D_ZBuffer zbuffer;
  ULONG *flat_shading;
  BOOL maggie_available;
} M3D_Context;

/** Check if Maggie is present */
BOOL M3D_CheckMaggie(VOID);

/** Create a new context */
M3D_Context *M3D_CreateContext(LONG *, struct BitMap *);

/** Destroy context and free all resources */
VOID M3D_DestroyContext(M3D_Context *);

/** Set the drawing region */
LONG M3D_SetDrawRegion(M3D_Context *, struct BitMap *, M3D_Scissor *);

/** Set the rendering state */
LONG M3D_SetState(M3D_Context *, UWORD, BOOL);

/** Lock the hardware before drawing */
LONG M3D_LockHardware(M3D_Context *);

/** Unlock the hardware */
VOID M3D_UnlockHardware(M3D_Context *);

/** Allocate the Z buffer */
LONG M3D_AllocZBuffer(M3D_Context *);

/** Free the Z buffer resources */
VOID M3D_FreeZBuffer(M3D_Context *);

/** Clear the Z buffer */
LONG M3D_ClearZBuffer(M3D_Context *);

/** Allocate a texture */
M3D_Texture *M3D_AllocTexture(M3D_Context *, LONG *, APTR, UWORD, ULONG, ULONG, ULONG *);

/** Allocate a texture from a file */
M3D_Texture *M3D_AllocTextureFile(M3D_Context *, LONG *, STRPTR);

/** Release a texture */
VOID M3D_FreeTexture(M3D_Context *, M3D_Texture *);

/** Draw a single triangle */
LONG M3D_DrawTriangle(M3D_Context *, M3D_Triangle *);

/** Draw an array of triangles */
LONG M3D_DrawTriangleArray(M3D_Context *, M3D_Triangle *, ULONG);

/** Draw a list of triangles */
LONG M3D_DrawTriangleList(M3D_Context *, M3D_Triangle **, ULONG);

/** Draw a texture for test purpose */
LONG M3D_DrawTest(M3D_Context *, M3D_Texture *);

#endif
