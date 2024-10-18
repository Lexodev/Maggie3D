/**
 * texture.h
 *
 * Maggie3D shared library
 * Texture management functions
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.5 June 2024 (updated: 01/06/2024)
 */

#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <exec/types.h>
#include "Maggie3D.h"

/** DDS texture */
#define TEX_DDSTAG            0x44445320
#define TEX_DATAALIGN         8

/** DDS file header */
typedef struct {
  ULONG tag;
  ULONG size, flags, height, width;
  ULONG dummy, depth, mipmap;
  ULONG reserved1[11];
  ULONG pixsize, pixflags, pixfourcc;
  ULONG pixrgb, pixrmask, pixgmask, pixbmask, pixamask;
  ULONG caps, caps2, caps3, caps4;
  ULONG reserved2;
} M3D_DDSHeader;

/** BMP constants */
#define TEX_BMPTAG            0x424d
#define TEX_BMPHSIZE          14
#define TEX_BMPWIN3           40L
#define TEX_BMPWIN4           108L
#define TEX_BMPWIN5           124L

/** BPM picture header */
typedef struct {
  LONG size, width, height;
  WORD planes, depth;
  LONG compression, imgsize;
  LONG hresol, vresol, colors, important;
} M3D_BMPHeader;

/** Texture file */
typedef struct {
  ULONG width, height, depth, data_size;
  UWORD pixformat;
  APTR *data;
  ULONG palette[256];
} M3D_TextureFile;

BOOL M3D_CheckTextureSize(ULONG, ULONG);
BOOL M3D_CheckBMPFile(STRPTR);
BOOL M3D_CheckDDSFile(STRPTR);
M3D_TextureFile *M3D_LoadDDSTexture(LONG *, STRPTR);
M3D_TextureFile *M3D_LoadBMPTexture(LONG *, STRPTR);
LONG M3D_ConvertToDXT1(M3D_Texture *, APTR, ULONG);
LONG M3D_ConvertFromDXT1(M3D_Texture *, APTR);

#endif
