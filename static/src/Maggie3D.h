/**
 * Maggie3D.h
 *
 * Maggie3D static library
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.6 June 2024 (updated: 24/06/2024)
 */

#ifndef _MAGGIE3D_H_
#define _MAGGIE3D_H_

#include <exec/types.h>
#include <graphics/gfx.h>
#include <utility/tagitem.h>

// Error codes
#define M3D_SUCCESS               0             // No error
#define M3D_NOMAGGIE              -1            // Maggie not available
#define M3D_NOMEMORY              -2            // No more memory
#define M3D_NOCONTEXT             -3            // Context not available
#define M3D_NOBITMAP              -4            // Bitmap not available
#define M3D_NOZBUFFER             -5            // Z buffer not available
#define M3D_FILEREAD              -6            // Read file error
#define M3D_TEXTYPE               -7            // Bad texture type
#define M3D_TEXSIZE               -8            // Bad texture size
#define M3D_BITMAPTYPE            -9            // Bad bitmap type (not CGX)
#define M3D_NOTRIANGLE            -10           // Triangle is degenerated and not drawable
#define M3D_NOTEXTURE             -11           // No texture data
#define M3D_NOFILTER              -12           // Unsupported filtering
#define M3D_TEXLIMIT              -13           // Limit of textures number reached
#define M3D_NOPALETTE             -14           // No texture palette
#define M3D_TEXRESIZE             -15           // Texture resize error
#define M3D_NOQUAD                -16           // Quad is degenerated and not drawable
#define M3D_UNKNOW                -42           // Unknown error

// Maggie mode
#define M3D_M_BILINEAR            (1 << 0)      // Activate bilinear filtering
#define M3D_M_ZBUFFER             (1 << 1)      // Activate Z buffering
#define M3D_M_16BITS              (1 << 2)      // Output 16bits
#define M3D_M_INHIBZBUF           (1 << 3)      // Inhib Z buffer writing
#define M3D_M_24BITS              (1 << 4)      // Output 24bits

// Context states
#define M3D_TEXMAPPING            (1 << 0)      // Texture mapping
#define M3D_FILTERING             (1 << 1)      // Texture filtering
#define M3D_GOURAUD               (1 << 2)      // Gouraud shading
#define M3D_ZBUFFER               (1 << 3)      // Z buffer
#define M3D_ZBUFFERUPDATE         (1 << 4)      // Z buffer update
#define M3D_BLENDING              (1 << 5)      // Color blending
#define M3D_TEXCRDNORM            (1 << 6)      // Texture coordinates normalized
#define M3D_FAST                  (1 << 7)      // Modify triangle data
#define M3D_PERSPECTIVE           (1 << 8)      // Perspective correction

#define M3D_DISABLE               0             // Disable the state
#define M3D_ENABLE                1             // Enable the state

// Maggie texture size
#define M3D_TEX64                 6             // Texture 64x64
#define M3D_TEX128                7             // Texture 128x128
#define M3D_TEX256                8             // Texture 256x256
#define M3D_TEX512                9             // Texture 512x512

// Maggie texture filtering
#define M3D_NEAREST               0             // Nearest no filter
#define M3D_LINEAR                1             // Bilinear filtering

// Texture pixel format
#define M3D_PIXFMT_UNKNOWN        0
#define M3D_PIXFMT_CLUT           1
#define M3D_PIXFMT_RGB15          2
#define M3D_PIXFMT_RGB16          3
#define M3D_PIXFMT_RGB24          4
#define M3D_PIXFMT_ARGB32         5
#define M3D_PIXFMT_DXT1           6

#define M3D_MAX_TEXTURE           256           // Maximum number of textures supported

// Texture tags
#define M3D_TT_TAGS               (TAG_USER+0x201000)
#define M3D_TT_DATA               (M3D_TT_TAGS+0) // Texture data
#define M3D_TT_FORMAT             (M3D_TT_TAGS+1) // Pixel format
#define M3D_TT_WIDTH              (M3D_TT_TAGS+2) // Texture width
#define M3D_TT_HEIGHT             (M3D_TT_TAGS+3) // Texture height
#define M3D_TT_PALETTE            (M3D_TT_TAGS+4) // Texture palette
#define M3D_TT_TRANSPARENCY       (M3D_TT_TAGS+5) // Texture has transparency
#define M3D_TT_TRSCOLOR           (M3D_TT_TAGS+6) // Texture transparent color
#define M3D_TT_AUTORESIZE         (M3D_TT_TAGS+7) // Texture auto resize
#define M3D_TT_FILENAME           (M3D_TT_TAGS+8) // Texture file name

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
  UWORD mipsize, filtering;
} M3D_Texture;

// Maggie3D triangle
typedef struct {
  M3D_Vertex v1, v2, v3;
  M3D_Texture *texture;
  ULONG color;
} M3D_Triangle;

// Maggie3D quad
typedef struct {
  M3D_Vertex v1, v2, v3, v4;
  M3D_Texture *texture;
  ULONG color;
} M3D_Quad;

// Maggie3D sprite
typedef struct {
  ULONG left, top, width, height;
  FLOAT x_zoom, y_zoom, angle;
  BOOL x_flip, y_flip;
  M3D_Texture *texture;
  FLOAT light;
  ULONG color;
} M3D_Sprite;

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
  WORDBITS states, mode;
  M3D_Bitmap drawregion;
  M3D_Scissor clipping;
  M3D_ZBuffer zbuffer;
  ULONG *flat_shading;
  BOOL maggie_available;
  M3D_Texture *textures[M3D_MAX_TEXTURE];
} M3D_Context;

/************************** Context functions ***********************************/
M3D_Context *M3D_CreateContext(LONG *, struct BitMap *);
VOID M3D_DestroyContext(M3D_Context *);
LONG M3D_SetState(M3D_Context *, UWORD, BOOL);
BOOL M3D_GetState(M3D_Context *, UWORD);

/************************** Hardware/Driver functions ***************************/
BOOL M3D_CheckMaggie(VOID);
LONG M3D_LockHardware(M3D_Context *);
VOID M3D_UnlockHardware(M3D_Context *);
ULONG M3D_BestModeID(ULONG, ULONG, ULONG);

/************************** Texture functions ***********************************/
M3D_Texture *M3D_AllocTexture(M3D_Context *, LONG *, APTR, UWORD, ULONG, ULONG, ULONG *);
M3D_Texture *M3D_AllocTextureFile(M3D_Context *, LONG *, STRPTR);
M3D_Texture *M3D_AllocTextureTagList(M3D_Context *, LONG *, struct TagItem *);
LONG M3D_SetFilter(M3D_Context *, M3D_Texture *, UWORD);
VOID M3D_FreeTexture(M3D_Context *, M3D_Texture *);
VOID M3D_FreeAllTextures(M3D_Context *);

/************************** Drawing functions ***********************************/
LONG M3D_DrawTriangle(M3D_Context *, M3D_Triangle *);
LONG M3D_DrawTriangleArray(M3D_Context *, M3D_Triangle *, ULONG);
LONG M3D_DrawTriangleList(M3D_Context *, M3D_Triangle **, ULONG);
LONG M3D_DrawQuad(M3D_Context *, M3D_Quad *);
LONG M3D_DrawSprite(M3D_Context *, M3D_Sprite *, LONG, LONG);
LONG M3D_ClearDrawRegion(M3D_Context *, ULONG);

/************************** Effect functions ************************************/
LONG M3D_SetDrawRegion(M3D_Context *, struct BitMap *, M3D_Scissor *);
LONG M3D_SetScissor(M3D_Context *, M3D_Scissor *);

/************************** ZBuffer functions ***********************************/
LONG M3D_AllocZBuffer(M3D_Context *);
VOID M3D_FreeZBuffer(M3D_Context *);
LONG M3D_ClearZBuffer(M3D_Context *);

#endif
