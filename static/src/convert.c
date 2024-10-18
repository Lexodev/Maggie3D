/**
 * convert.c
 *
 * Maggie3D static library
 * Convert RAW texture to DXT1
 *
 * @author Morten Brenna (compression), Fabrice Labrador (decompression)
 * @version 1.5 May 2024 (updated: 27/05/2024)
 */

#include <exec/types.h>

#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "memory.h"
#include "texture.h"
#include "Maggie3D.h"

#define MOB_MinVal(a,b) (((a) < (b)) ? (a) : (b))
#define MOB_MaxVal(a,b) (((a) > (b)) ? (a) : (b))

#define MOB_AlphaThreshold 128

typedef struct {
  UWORD col0;
  UWORD col1;
  ULONG pixels;
} DXTBlock;

/**
 * DXT1 decompression functions
 */

ULONG FLR_Color16To32(ULONG color)
{
  return (((color & 0xf800) << 16) | ((color & 0x07e0) << 13) | ((color & 0x001f) << 11) | 0xff);
}

// c = 2/3 c1 + 1/3 c2
ULONG FLR_TierColor(ULONG color1, ULONG color2)
{
  ULONG r1, r2, b1, b2, g1, g2;
  
  r1 = (color1 >> 24) & 0xff;
  r2 = (color2 >> 24) & 0xff;
  g1 = (color1 >> 16) & 0xff;
  g2 = (color2 >> 16) & 0xff;
  b1 = (color1 >> 8) & 0xff;
  b2 = (color2 >> 8) & 0xff;
  return (((r1+r1+r2)/3) << 24) | (((g1+g1+g2)/3) << 16) | (((b1+b1+b2)/3) << 8) | 0xff;
}

// c = 1/2 c1 + 1/2 c2
ULONG FLR_MidColor(ULONG color1, ULONG color2)
{
  ULONG r1, r2, b1, b2, g1, g2;
  
  r1 = (color1 >> 24) & 0xff;
  r2 = (color2 >> 24) & 0xff;
  g1 = (color1 >> 16) & 0xff;
  g2 = (color2 >> 16) & 0xff;
  b1 = (color1 >> 8) & 0xff;
  b2 = (color2 >> 8) & 0xff;
  return (((r1+r2)/2) << 24) | (((g1+g2)/2) << 16) | (((b1+b2)/2) << 8) | 0xff;
}

VOID FLR_DecodeColors(ULONG *colors, UWORD color0, UWORD color1)
{
  if (color0 > color1) {
    // Opacity
    colors[0] = FLR_Color16To32(color0);
    colors[1] = FLR_Color16To32(color1);
    colors[2] = FLR_TierColor(colors[0], colors[1]);
    colors[3] = FLR_TierColor(colors[1], colors[0]);
  } else {
    // Transparency
    colors[0] = FLR_Color16To32(color0);
    colors[1] = FLR_Color16To32(color1);
    colors[2] = FLR_MidColor(colors[0], colors[1]);
    colors[3] = 0x0;
  }
}

VOID FLR_DecodePixels(ULONG pixels, ULONG *colors, ULONG *dst, ULONG width)
{
  ULONG slide, index;
  WORD i, j;
  
  slide = 0;
  for (j = 0;j < 4;j++) {
    for (i = 0;i < 4;i++) {
      index = (pixels >> slide) & 0x3;
      *(dst + i) = colors[index];
      slide += 2;
    }
    dst += width;
  }
}

VOID FLR_DecompressDXT1(UBYTE *src, UBYTE *dst, ULONG width, ULONG height)
{
  ULONG wblock, hblock, pixels, colors[4];
  UWORD color0, color1;
  
  for (hblock = 0;hblock < height;hblock += 4) {
    for (wblock = 0;wblock < width;wblock += 4) {
      color0 = (src[1] << 8) | src[0];
      src += 2;
      color1 = (src[1] << 8) | src[0];
      src += 2;
      pixels = (src[3] << 24) | (src[2] << 16) | (src[1] << 8) | src[0];
      src += 4;
      FLR_DecodeColors(colors, color0, color1);
      FLR_DecodePixels(pixels, colors, (ULONG *)dst, width);
      dst += 4 * 4;
    }
    dst += width * 4 * 3;
  }
}

/**
 * DXT1 compression functions
 */

ULONG MOB_GetTextureSize(UWORD mipmap)
{
  switch (mipmap) {
    case 9 : return (512*512 + 256*256 + 128*128 + 64*64) / 2;
    case 8 : return (256*256 + 128*128 + 64*64) / 2;
    case 7 : return (128*128 + 64*64) / 2;
    case 6 : return (64*64) / 2;
  }
  return 0;
}

ULONG MOB_GetMipMapSize(UWORD mipmap)
{
  switch (mipmap) {
    case 9 : return 512*512 / 2;
    case 8 : return 256*256 / 2;
    case 7 : return 128*128 / 2;
    case 6 : return 64*64 / 2;
  }
  return 0;
}

ULONG MOB_GetTexturePixelWidth(UWORD mipmap)
{
  switch (mipmap) {
    case 9 : return 512;
    case 8 : return 256;
    case 7 : return 128;
    case 6 : return 64;
  }
  return 0;
}

ULONG MOB_GetTexturePixelHeight(UWORD mipmap)
{
  switch (mipmap) {
    case 9 : return 512;
    case 8 : return 256;
    case 7 : return 128;
    case 6 : return 64;
  }
  return 0;
}

ULONG MOB_GetTextureMipMapOffset(UWORD topLevel, UWORD mipmap)
{
  if (topLevel < mipmap) {
    return 0;
  }
  return MOB_GetTextureSize(topLevel) & ~MOB_GetTextureSize(mipmap);
}

UWORD MOB_RGBTo16Bit(ULONG rgb)
{
  return (UWORD)(((rgb >> 8) & 0xf800) | ((rgb >> 5) & 0x07e0) | ((rgb >> 3) & 0x001f));
}

ULONG MOB_BSwap32(ULONG val)
{
  return (val >> 24) | ((val >> 8) & 0x0000ff00) | ((val << 8) & 0x00ff0000) | (val << 24);
}

UWORD MOB_BSwap16(UWORD val)
{
  return (UWORD)((val >> 8) | (val << 8));
}

LONG MOB_SqrInt(LONG a)
{
  return a * a;
}

FLOAT MOB_SqrFloat(FLOAT a)
{
  return (FLOAT)(a * a);
}

FLOAT MOB_QuantizeBlock3(DXTBlock *block, UBYTE *src, LONG width, LONG rVec, LONG gVec, LONG bVec, LONG rOrigo, LONG gOrigo, LONG bOrigo, FLOAT ooLenSq)
{
  FLOAT error = 0.0f, dotProd;
  LONG rCols[4], gCols[4], bCols[4];
  LONG i, j, r, g, b, a, pixVal;
  
  rCols[0] = ((block->col1 >> 8) & 0xf8);
  gCols[0] = ((block->col1 >> 3) & 0xfc);
  bCols[0] = ((block->col1 << 3) & 0xf8);
  rCols[1] = ((block->col0 >> 8) & 0xf8);
  gCols[1] = ((block->col0 >> 3) & 0xfc);
  bCols[1] = ((block->col0 << 3) & 0xf8);
  rCols[2] = (rCols[0] + rCols[1]) / 2;
  gCols[2] = (gCols[0] + gCols[1]) / 2;
  bCols[2] = (bCols[0] + bCols[1]) / 2;
  block->pixels = 0;
  for (i = 0; i < 4; ++i) {
    for (j = 0; j < 4; ++j) {
      r = src[(i * width + j) * 4 + 0];
      g = src[(i * width + j) * 4 + 1];
      b = src[(i * width + j) * 4 + 2];
      a = src[(i * width + j) * 4 + 3];
      if (a >= 128) {
        dotProd = ((r - rOrigo) * rVec + (g - gOrigo) * gVec + (b - bOrigo) * bVec) * ooLenSq;
        pixVal = 1;
        if (dotProd < (1.0f / 3.0f)) {
          pixVal = 0;
        } else if (dotProd < (2.0f / 3.0f)) {
          pixVal = 2;
        }
        error += MOB_SqrInt(r - rCols[pixVal]) + MOB_SqrInt(g - gCols[pixVal]) + MOB_SqrInt(b - bCols[pixVal]);
      } else {
        pixVal = 3;
      }
      block->pixels |= pixVal << (((i) * 4 + j) * 2);
    }
  }
  return error;
}

FLOAT MOB_QuantizeBlock4(DXTBlock *block, UBYTE *src, LONG width, LONG rVec, LONG gVec, LONG bVec, LONG rOrigo, LONG gOrigo, LONG bOrigo, FLOAT ooLenSq)
{
  FLOAT error = 0.0f, dotProd;
  FLOAT rCols[4], gCols[4], bCols[4];
  LONG i, j, r, g, b, pixVal;
  
  rCols[0] = ((block->col0 >> 8) & 0xf8);
  gCols[0] = ((block->col0 >> 3) & 0xfc);
  bCols[0] = ((block->col0 << 3) & 0xf8);
  rCols[1] = ((block->col1 >> 8) & 0xf8);
  gCols[1] = ((block->col1 >> 3) & 0xfc);
  bCols[1] = ((block->col1 << 3) & 0xf8);
  rCols[2] = (rCols[0] * 10.0f + rCols[1] * 6.0f) / 16.0f;
  gCols[2] = (gCols[0] * 10.0f + gCols[1] * 6.0f) / 16.0f;
  bCols[2] = (bCols[0] * 10.0f + bCols[1] * 6.0f) / 16.0f;
  rCols[3] = (rCols[0] * 6.0f + rCols[1] * 10.0f) / 16.0f;
  gCols[3] = (gCols[0] * 6.0f + gCols[1] * 10.0f) / 16.0f;
  bCols[3] = (bCols[0] * 6.0f + bCols[1] * 10.0f) / 16.0f;
  block->pixels = 0;
  for (i = 0; i < 4; ++i) {
    for (j = 0; j < 4; ++j) {
      r = src[(i * width + j) * 4 + 0];
      g = src[(i * width + j) * 4 + 1];
      b = src[(i * width + j) * 4 + 2];
      dotProd = ((r - rOrigo) * rVec + (g - gOrigo) * gVec + (b - bOrigo) * bVec) * ooLenSq;
      pixVal = 0;
      if (dotProd < (3.0f / 16.0f)) {
        pixVal = 1;
      } else if (dotProd < (1.0f / 2.0f)) {
        pixVal = 3;
      } else if (dotProd < (13.0f / 16.0f)) {
        pixVal = 2;
      }
      error += MOB_SqrFloat((FLOAT)(r - rCols[pixVal])) + MOB_SqrFloat((FLOAT)(g - gCols[pixVal])) + MOB_SqrFloat((FLOAT)(b - bCols[pixVal]));
      block->pixels |= pixVal << (((i) * 4 + j) * 2);
    }
  }
  return error;
}

VOID MOB_CompressRGBA(UBYTE *src, UBYTE *dst, LONG width, LONG height)
{
  DXTBlock *block = (DXTBlock *)dst;
  LONG bMin, gMin, rMin, bMax, gMax, rMax, aMin, aMax;
  LONG rVec, gVec, bVec, x, y, i, j;
  BOOL blk4;
  FLOAT lenSq;
  
  for (y = 0; y < height; y += 4) {
    for (x = 0; x < width; x += 4) {
      bMin = gMin = rMin = aMin = 255;
      bMax = gMax = rMax = aMax = 0;
      for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
          LONG r = src[((y + i) * width + x + j) * 4 + 0];
          LONG g = src[((y + i) * width + x + j) * 4 + 1];
          LONG b = src[((y + i) * width + x + j) * 4 + 2];
          LONG a = src[((y + i) * width + x + j) * 4 + 3];
          if (a >= MOB_AlphaThreshold) {
            rMin = MOB_MinVal(rMin, r);
            gMin = MOB_MinVal(gMin, g);
            bMin = MOB_MinVal(bMin, b);
            rMax = MOB_MaxVal(rMax, r);
            gMax = MOB_MaxVal(gMax, g);
            bMax = MOB_MaxVal(bMax, b);
          }
          aMin = MOB_MinVal(aMin, a);
          aMax = MOB_MaxVal(aMax, a);
        }
      }
      rVec = rMax - rMin;
      gVec = gMax - gMin;
      bVec = bMax - bMin;
      lenSq = bVec * bVec + gVec * gVec + rVec * rVec;

      if (aMin < MOB_AlphaThreshold) {
        // Transparency in this block
        if (aMax < MOB_AlphaThreshold) {
          // Only tansparent pixels
          block->col0 = 0x0000;
          block->col1 = 0xffff;
          block->pixels = 0xffffffff;
          lenSq = 0.0;
        } else {
          // At least one pixel not transparent
          block->col0 = MOB_RGBTo16Bit((rMin << 16) | (gMin << 8) | bMin);
          block->col1 = MOB_RGBTo16Bit((rMax << 16) | (gMax << 8) | bMax);
          block->pixels = 0;
          if (lenSq == 0.0) {
            // Tricky case where non transparent pixels have the same color
            lenSq = 3.0;    // Fake value to force a quantize
          }
        }
        blk4 = FALSE;
      } else {
        // No transparency in this block
        block->col0 = MOB_RGBTo16Bit((rMax << 16) | (gMax << 8) | bMax);
        block->col1 = MOB_RGBTo16Bit((rMin << 16) | (gMin << 8) | bMin);
        block->pixels = 0;
        blk4 = TRUE;
      }

      if (lenSq > 0.0f) {
        FLOAT lowestError;
        FLOAT ooLenSq = 1.0f / lenSq;

        if (blk4) {
          lowestError = MOB_QuantizeBlock4(block, &src[(y * width + x) * 4], width, rVec, gVec, bVec, rMin, gMin, bMin, ooLenSq);
        } else {
          lowestError = MOB_QuantizeBlock3(block, &src[(y * width + x) * 4], width, rVec, gVec, bVec, rMin, gMin, bMin, ooLenSq);
        }
        
      }
      block->pixels = MOB_BSwap32(block->pixels);
      block->col0 = MOB_BSwap16(block->col0);
      block->col1 = MOB_BSwap16(block->col1);
      block++;
    }
  }
}

/** Convert RAW RGBA texture to DXT1 */
LONG M3D_ConvertToDXT1(M3D_Texture *texture, APTR data, ULONG data_size)
{
  UBYTE *dst;
  
  Dbug(printf("[MAGGIE3D] Convert texture data to DXT1 format\n");)
  dst = (UBYTE *)texture->data + MOB_GetTextureMipMapOffset(texture->mipsize, texture->mipsize);
  MOB_CompressRGBA(data, dst, texture->width, texture->height);
  return M3D_SUCCESS;
}

/** Convert DXT1 to RAW RGBA texture */
LONG M3D_ConvertFromDXT1(M3D_Texture *texture, APTR data)
{
  UBYTE *dst;
  
  Dbug(printf("[MAGGIE3D] Convert texture data to RGBA format\n");)
  dst = (UBYTE *)texture->data;
  FLR_DecompressDXT1(data, dst, texture->width, texture->height);
  return M3D_SUCCESS;
}
