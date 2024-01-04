/**
 * convert.c
 *
 * Maggie3D library
 * Convert RAW texture to DXT1
 *
 * @author Morten Brenna
 * @version 1.0 November 2023
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

typedef struct {
  UWORD col0;
  UWORD col1;
  ULONG pixels;
} DXTBlock;

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

static int MOB_SqrInt(int a)
{
  return a * a;
}

static float MOB_SqrFloat(float a)
{
  return (float)(a * a);
}

static float MOB_QuantizeBlock3(DXTBlock *block, UBYTE *src, int width, int pixelSize, int rVec, int gVec, int bVec, int rOrigo, int gOrigo, int bOrigo, float ooLenSq)
{
  float error = 0.0f;

  int rCols[3];
  int gCols[3];
  int bCols[3];
  int i,j;
  int r,g,b;
  int pixVal;
  float dotProd;
  
  rCols[0] = ((block->col0 >> 8) & 0xf8);
  gCols[0] = ((block->col0 >> 3) & 0xfc);
  bCols[0] = ((block->col0 << 3) & 0xf8);
  rCols[1] = ((block->col1 >> 8) & 0xf8);
  gCols[1] = ((block->col1 >> 3) & 0xfc);
  bCols[1] = ((block->col1 << 3) & 0xf8);
  rCols[2] = (rCols[0] + rCols[1]) / 2;
  gCols[2] = (gCols[0] + gCols[1]) / 2;
  bCols[2] = (bCols[0] + bCols[1]) / 2;

  block->pixels = 0;

  for (i = 0; i < 4; ++i) {
    for (j = 0; j < 4; ++j) {
      r = src[(i * width + j) * pixelSize + 0];
      g = src[(i * width + j) * pixelSize + 1];
      b = src[(i * width + j) * pixelSize + 2];

      dotProd = ((r - rOrigo) * rVec + (g - gOrigo) * gVec + (b - bOrigo) * bVec) * ooLenSq;
      pixVal = 0;

      if (dotProd < (1.0f / 3.0f)) {
        pixVal = 1;
      } else if (dotProd < (2.0f / 3.0f)) {
        pixVal = 2;
      }
      error += MOB_SqrInt(r - rCols[pixVal]) + MOB_SqrInt(g - gCols[pixVal]) + MOB_SqrInt(b - bCols[pixVal]);
      block->pixels |= pixVal << (((i) * 4 + j) * 2);
    }
  }
  return error;
}

static float MOB_QuantizeBlock4(DXTBlock *block, UBYTE *src, int width, int pixelSize, int rVec, int gVec, int bVec, int rOrigo, int gOrigo, int bOrigo, float ooLenSq)
{
  float error = 0.0f;

  float rCols[4];
  float gCols[4];
  float bCols[4];
  int i,j;
  int r,g,b;
  int pixVal;
  float dotProd;
  
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
      r = src[(i * width + j) * pixelSize + 0];
      g = src[(i * width + j) * pixelSize + 1];
      b = src[(i * width + j) * pixelSize + 2];

      dotProd = ((r - rOrigo) * rVec + (g - gOrigo) * gVec + (b - bOrigo) * bVec) * ooLenSq;
      pixVal = 0;

      if (dotProd < (3.0f / 16.0f)) {
        pixVal = 1;
      } else if (dotProd < (1.0f / 2.0f)) {
        pixVal = 3;
      } else if (dotProd < (13.0f / 16.0f)) {
        pixVal = 2;
      }
      error += MOB_SqrFloat((float)(r - rCols[pixVal])) + MOB_SqrFloat((float)(g - gCols[pixVal])) + MOB_SqrFloat((float)(b - bCols[pixVal]));
      block->pixels |= pixVal << (((i) * 4 + j) * 2);
    }
  }
  return error;
}

void MOB_CompressRGB(UBYTE *dst, UBYTE *src, int width, int height, int pixelSize, int quality)
{
  DXTBlock *block = (DXTBlock *)dst;
  int bMin,gMin,rMin,bMax,gMax,rMax;
  int rVec,gVec,bVec;
  int x,y,i,j,blk4;
  float lenSq;
  
  for (y = 0; y < height; y += 4) {
    for (x = 0; x < width; x += 4) {
      bMin = 255;
      gMin = 255;
      rMin = 255;
      bMax = 0;
      gMax = 0;
      rMax = 0;
      for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
          int r = src[((y + i) * width + x + j) * pixelSize + 0];
          int g = src[((y + i) * width + x + j) * pixelSize + 1];
          int b = src[((y + i) * width + x + j) * pixelSize + 2];
          rMin = MOB_MinVal(rMin, r);
          gMin = MOB_MinVal(gMin, g);
          bMin = MOB_MinVal(bMin, b);
          rMax = MOB_MaxVal(rMax, r);
          gMax = MOB_MaxVal(gMax, g);
          bMax = MOB_MaxVal(bMax, b);
        }
      }

      rVec = rMax - rMin;
      gVec = gMax - gMin;
      bVec = bMax - bMin;
      lenSq = bVec * bVec + gVec * gVec + rVec * rVec;

      block->col0 = MOB_RGBTo16Bit((rMax << 16) | (gMax << 8) | bMax);
      block->col1 = MOB_RGBTo16Bit((rMin << 16) | (gMin << 8) | bMin);
      block->pixels = 0;
      blk4 = 1;
      if (lenSq > 0.0f) {
        float lowestError;
        float ooLenSq = 1.0f / lenSq;

        lowestError = MOB_QuantizeBlock4(block, &src[(y * width + x) * pixelSize], width, pixelSize, rVec, gVec, bVec, rMin, gMin, bMin, ooLenSq);
        if (quality) {
          float error;
          DXTBlock testBlk;
          testBlk.col0 = MOB_RGBTo16Bit((rMax << 16) | (gMax << 8) | bMax);
          testBlk.col1 = MOB_RGBTo16Bit((rMin << 16) | (gMin << 8) | bMin);
          testBlk.pixels = 0;
          error = MOB_QuantizeBlock3(&testBlk, &src[(y * width + x) * pixelSize], width, pixelSize, rVec, gVec, bVec, rMin, gMin, bMin, ooLenSq);
          if (lowestError > error) {
            lowestError = error;
            *block = testBlk;
            blk4 = 0;
          }

          testBlk.col0 = MOB_RGBTo16Bit((rMax << 16) | (gMax << 8) | bMin);
          testBlk.col1 = MOB_RGBTo16Bit((rMin << 16) | (gMin << 8) | bMax);
          error = MOB_QuantizeBlock4(&testBlk, &src[(y * width + x) * pixelSize], width, pixelSize, rVec, gVec, -bVec, rMin, gMin, bMax, ooLenSq);
          if (lowestError > error) {
            lowestError = error;
            *block = testBlk;
            blk4 = 1;
          }
          error = MOB_QuantizeBlock3(&testBlk, &src[(y * width + x) * pixelSize], width, pixelSize, rVec, gVec, -bVec, rMin, gMin, bMax, ooLenSq);
          if (lowestError > error) {
            lowestError = error;
            *block = testBlk;
            blk4 = 0;
          }

          testBlk.col0 = MOB_RGBTo16Bit((rMax << 16) | (gMin << 8) | bMax);
          testBlk.col1 = MOB_RGBTo16Bit((rMin << 16) | (gMax << 8) | bMin);
          error = MOB_QuantizeBlock4(&testBlk, &src[(y * width + x) * pixelSize], width, pixelSize, rVec, -gVec, bVec, rMin, gMax, bMin, ooLenSq);
          if (lowestError > error) {
            lowestError = error;
            *block = testBlk;
            blk4 = 1;
          }
          error = MOB_QuantizeBlock3(&testBlk, &src[(y * width + x) * pixelSize], width, pixelSize, rVec, -gVec, bVec, rMin, gMax, bMin, ooLenSq);
          if (lowestError > error) {
            lowestError = error;
            *block = testBlk;
            blk4 = 0;
          }

          testBlk.col0 = MOB_RGBTo16Bit((rMax << 16) | (gMin << 8) | bMin);
          testBlk.col1 = MOB_RGBTo16Bit((rMin << 16) | (gMax << 8) | bMax);
          error = MOB_QuantizeBlock4(&testBlk, &src[(y * width + x) * pixelSize], width, pixelSize, rVec, -gVec, -bVec, rMin, gMax, bMax, ooLenSq);
          if (lowestError > error) {
            lowestError = error;
            *block = testBlk;
            blk4 = 1;
          }
          error = MOB_QuantizeBlock3(&testBlk, &src[(y * width + x) * pixelSize], width, pixelSize, rVec, -gVec, -bVec, rMin, gMax, bMax, ooLenSq);
          if (lowestError > error) {
            lowestError = error;
            *block = testBlk;
            blk4 = 0;
          }
        }
        if (blk4) {
          if (block->col0 < block->col1) {
            UWORD tmp = block->col0;
            block->col0 = block->col1;
            block->col1 = tmp;
            block->pixels ^= 0x55555555;
          }
        } else {
          if (block->col0 > block->col1) {
            UWORD tmp = block->col0;
            block->col0 = block->col1;
            block->col1 = tmp;
            block->pixels = ((~(block->pixels >> 1)) & 0x55555555) ^ block->pixels;
          }
        }
      }
      block->pixels = MOB_BSwap32(block->pixels);
      block->col0 = MOB_BSwap16(block->col0);
      block->col1 = MOB_BSwap16(block->col1);
      block++;
    }
  }
}

/** Convert RAW texture data to DXT1 */
LONG M3D_ConvertToDXT1(M3D_Texture *texture, APTR data, ULONG data_size, UWORD pixformat)
{
  UBYTE *dst;
  
  Dbug(printf("[DEBUG] Convert texture data to DXT1 format\n");)
  dst = (UBYTE *)texture->data + MOB_GetTextureMipMapOffset(texture->mipsize, texture->mipsize);
  MOB_CompressRGB(dst, data, texture->width, texture->height, 3, TRUE);
  return M3D_SUCCESS;
}
