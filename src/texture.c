/**
 * texture.c
 *
 * Maggie3D library
 * Texture management functions
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.0 November 2023
 */

#include <stdio.h>
#include <string.h>

#include <dos/dos.h>

#include <proto/dos.h>

#include "debug.h"
#include "memory.h"
#include "texture.h"
#include "Maggie3D.h"

/** Check if texture size is valid */
BOOL M3D_CheckTextureSize(ULONG width, ULONG height)
{
  if (width != height) {
    return FALSE;
  }
  if (width != 64 && width != 128 && width != 256 && width != 512) {
    return FALSE;
  }
  return TRUE;
}

/** Check for a DDS file */
BOOL M3D_CheckDDSFile(STRPTR file_name)
{
  BPTR file_handle;
  LONG dds_tag, bytes_read;
  
  file_handle = Open(file_name, MODE_OLDFILE);
  if (file_handle != 0) {
    // Check if it's a DDS file
    bytes_read = Read(file_handle, &dds_tag, sizeof(dds_tag));
    if (bytes_read != sizeof(dds_tag)) {
      Close(file_handle);
      return FALSE;
    }
    Close(file_handle);
    if (dds_tag == TEX_DDSTAG) {
      return TRUE;
    }
  }
  return FALSE;
}

/** Load a DDS texture */
M3D_TextureFile *M3D_LoadDDSTexture(LONG *error, STRPTR file_name)
{
  BPTR file_handle;
  M3D_DDSHeader header;
  M3D_TextureFile *texfile;
  LONG file_size, bytes_read;

  Dbug(printf("[DEBUG] M3D_LoadDDSTexture %s\n", file_name);)
  file_handle = Open(file_name, MODE_OLDFILE);
  if (file_handle != 0) {
    texfile = (M3D_TextureFile *) M3D_AllocMem(sizeof(M3D_TextureFile));
    if (texfile == NULL) {
      Close(file_handle);
      *error = M3D_NOMEMORY;
      return NULL;
    }
    // Get the file size
    bytes_read = Seek(file_handle, 0, OFFSET_END);
    file_size = Seek(file_handle, 0, OFFSET_BEGINNING);
    // Read the header
    bytes_read = Read(file_handle, &header, sizeof(header));
    if (bytes_read != sizeof(header)) {
      M3D_FreeMem(texfile);
      Close(file_handle);
      *error = M3D_FILEREAD;
      return NULL;
    }
    texfile->width = M3D_LONGTOBE(header.width);
    texfile->height = M3D_LONGTOBE(header.height);
    texfile->depth = 24;
    texfile->data_size = file_size - sizeof(header);
    texfile->pixformat = M3D_PIXFMT_DXT1;
    Dbug(printf("[DEBUG] DDS header => width=%d, height=%d, depth=%d, data_size=%d\n", texfile->width, texfile->height, texfile->depth, texfile->data_size);)
    // Read the body
    if ((texfile->data = M3D_AllocMem(texfile->data_size)) == NULL) {
      M3D_FreeMem(texfile);
      Close(file_handle);
      *error = M3D_NOMEMORY;
      return NULL;
    }
    bytes_read = Read(file_handle, texfile->data, texfile->data_size);
    if (bytes_read != texfile->data_size) {
      M3D_FreeMem(texfile->data);
      M3D_FreeMem(texfile);
      Close(file_handle);
      *error = M3D_FILEREAD;
      return NULL;
    }
    Close(file_handle);
    *error = M3D_SUCCESS;
    return texfile;
  }
  *error = M3D_FILEREAD;
  return NULL;
}

/** Get the mipmap size value depending on texture size */
UWORD M3D_GetTextureMipmapsize(UWORD size)
{
  switch (size) {
    case 64:
      return M3D_TEX64;
    case 128:
      return M3D_TEX128;
    case 256:
      return M3D_TEX256;
    case 512:
      return M3D_TEX512;
  }
  return 0;
}

/** Get the texure data size */
ULONG M3D_GetTextureDataSize(UWORD mipmap)
{
  switch (mipmap) {
    case 9 : return (512*512 + 256*256 + 128*128 + 64*64) / 2;
    case 8 : return (256*256 + 128*128 + 64*64) / 2;
    case 7 : return (128*128 + 64*64) / 2;
    case 6 : return (64*64) / 2;
  }
  return 0;
}

/** Convert CLUT to RGB24 format */
VOID M3D_ConvertCLUTToRGB24(UBYTE *source, UBYTE *dest, ULONG width, ULONG height, ULONG *palette)
{
  UBYTE pixel;
  ULONG size, color;
  
  Dbug(printf("[DEBUG] Convert CLUT to RGB24 format\n");)
  size = width * height;
  while (size--) {
    pixel = *source++;
    color = palette[pixel];
    dest[0] = (UBYTE)((color & 0xff0000) >> 16);
    dest[1] = (UBYTE)((color & 0xff00) >> 8);
    dest[2] = (UBYTE)(color & 0xff);
    dest += 3;
  }
}

/** Convert RGB16 to RGB24 format */
VOID M3D_ConvertRBG16ToRGB24(UWORD *source, UBYTE *dest, ULONG width, ULONG height)
{
  UWORD pixel;
  ULONG size;
  
  Dbug(printf("[DEBUG] Convert RGB16 to RGB24 format\n");)
  size = width * height;
  while (size--) {
    pixel = *source++;
    dest[0] = (UBYTE)((pixel & 0xf800) >> 8);
    dest[1] = (UBYTE)((pixel & 0x7e0) >> 3);
    dest[2] = (UBYTE)((pixel & 0x1f) << 3);
    dest += 3;
  }
}

/** Copy RGB24 to RGB24 format */
VOID M3D_CopyRBG24ToRGB24(UBYTE *source, UBYTE *dest, ULONG width, ULONG height)
{
  ULONG size;
  
  Dbug(printf("[DEBUG] Copy RGB24 to RGB24\n");)
  size = width * height;
  while (size--) {
    dest[0] = source[0];
    dest[1] = source[1];
    dest[2] = source[2];
    source += 3;
    dest += 3;
  }
}

/** Convert ARGB32 to RGB24 format */
VOID M3D_ConvertARBG32ToRGB24(ULONG *source, UBYTE *dest, ULONG width, ULONG height)
{
  ULONG pixel, size;
  
  Dbug(printf("[DEBUG] Convert ARGB32 to RGB24 format\n");)
  size = width * height;
  while (size--) {
    pixel = *source++;
    dest[0] = (UBYTE)((pixel & 0xff0000) >> 16);
    dest[1] = (UBYTE)((pixel & 0xff00) >> 8);
    dest[2] = (UBYTE)(pixel & 0xff);
    dest += 3;
  }
}

/** Allocate a texture */
M3D_Texture *M3D_AllocTexture(M3D_Context *context, LONG *error, APTR data, UWORD pixformat, ULONG width, ULONG height, ULONG *palette)
{
  M3D_Texture *texture;
  APTR tmp_data;

  if (context == NULL) {
    *error = M3D_NOCONTEXT;
    return NULL;
  }
  if (data != NULL) {
    if (!M3D_CheckTextureSize(width, height)) {
      *error = M3D_TEXSIZE;
      return NULL;
    }
    // Allocate and init texture
    texture = (M3D_Texture *) M3D_AllocMem(sizeof(M3D_Texture));
    if (texture != NULL) {
      texture->width = width;
      texture->height = height;
      texture->mipsize = M3D_GetTextureMipmapsize(width);
      Dbug(printf("[DEBUG] Alloc texture => width=%d, height=%d, mipsize=%d\n", texture->width, texture->height, texture->mipsize);)
      if (pixformat == M3D_PIXFMT_DXT1) {
        Dbug(printf("[DEBUG] Native DXT1 texture, no conversion needed\n");)
        if ((texture->data = M3D_AllocAlignMem(M3D_GetTextureDataSize(texture->mipsize), TEX_DATAALIGN)) == NULL) {
          M3D_FreeMem(texture);
          *error = M3D_NOMEMORY;
          return NULL;
        }
        memcpy(texture->data, data, M3D_GetTextureDataSize(texture->mipsize));
        *error = M3D_SUCCESS;
        return texture;
      } else {
        Dbug(printf("[DEBUG] Not a native DXT1 texture, conversion needed\n");)
        if (pixformat != M3D_PIXFMT_CLUT && pixformat != M3D_PIXFMT_RGB16 && pixformat != M3D_PIXFMT_RGB24 && pixformat != M3D_PIXFMT_ARGB32) {
          M3D_FreeMem(texture);
          *error = M3D_TEXTYPE;
          return NULL;
        }
        // Buffer for source conversion
        if ((tmp_data = M3D_AllocMem(texture->width * texture->height * 3)) == NULL) {
          M3D_FreeMem(texture);
          *error = M3D_NOMEMORY;
          return NULL;
        }
        // Convert source to RGB format
        Dbug(printf("[DEBUG] Convert source data to RGB24 format\n");)
        if (pixformat == M3D_PIXFMT_CLUT) {
          M3D_ConvertCLUTToRGB24((UBYTE *)data, (UBYTE *)tmp_data, texture->width, texture->height, palette);
        } else if (pixformat == M3D_PIXFMT_RGB16) {
          M3D_ConvertRBG16ToRGB24((UWORD *)data, (UBYTE *)tmp_data, texture->width, texture->height);
        } else if (pixformat == M3D_PIXFMT_RGB24) {
          M3D_CopyRBG24ToRGB24((UBYTE *)data, (UBYTE *)tmp_data, texture->width, texture->height);
        } else if (pixformat == M3D_PIXFMT_ARGB32) {
          M3D_ConvertARBG32ToRGB24((ULONG *)data, (UBYTE *)tmp_data, texture->width, texture->height);
        }
#if _USE_MAGGIE_ == 1
        if ((texture->data = M3D_AllocAlignMem(M3D_GetTextureDataSize(texture->mipsize), TEX_DATAALIGN)) == NULL) {
          M3D_FreeMem(tmp_data);
          M3D_FreeMem(texture);
          *error = M3D_NOMEMORY;
          return NULL;
        }
        *error = M3D_ConvertToDXT1(texture, tmp_data, texture->width * texture->height * 3, pixformat);
        M3D_FreeMem(tmp_data);
        if (*error != M3D_SUCCESS) {
          M3D_FreeMem(texture->data);
          M3D_FreeMem(texture);
          return NULL;
        }
#else
        Dbug(printf("[DEBUG] No conversion to DXT1 (Maggie not used)\n");)
        texture->data = tmp_data;
#endif
        return texture;
      }
    }
    *error = M3D_NOMEMORY;
    return NULL;
  }
  *error = M3D_NOTEXTURE;
  return NULL;
}

/** Load & allocate a texture from a file */
M3D_Texture *M3D_AllocTextureFile(M3D_Context *context, LONG *error, STRPTR filename)
{
  M3D_TextureFile *texfile;
  M3D_Texture *texture;

  if (context == NULL) {
    *error = M3D_NOCONTEXT;
    return NULL;
  }
  if (M3D_CheckDDSFile(filename)) {
    if ((texfile = M3D_LoadDDSTexture(error, filename)) == NULL) {
      return NULL;
    }
    Dbug(printf("[DEBUG] DDS texture loaded\n");)
    texture = M3D_AllocTexture(
        context,
        error,
        texfile->data,
        texfile->pixformat,
        texfile->width,
        texfile->height,
        texfile->palette
    );
    M3D_FreeMem(texfile->data);
    M3D_FreeMem(texfile);
    if (texture == NULL) {
      return NULL;
    }
    *error = M3D_SUCCESS;
    return texture;
  } else if (M3D_CheckBMPFile(filename)) {
    if ((texfile = M3D_LoadBMPTexture(error, filename)) == NULL) {
      return NULL;
    }
    Dbug(printf("[DEBUG] BMP texture loaded\n");)
    texture = M3D_AllocTexture(
        context,
        error,
        texfile->data,
        texfile->pixformat,
        texfile->width,
        texfile->height,
        texfile->palette
    );
    M3D_FreeMem(texfile->data);
    M3D_FreeMem(texfile);
    if (texture == NULL) {
      return NULL;
    }
    *error = M3D_SUCCESS;
    return texture;
  } else {
    Dbug(printf("[DEBUG] Texture file format not yet supported !\n");)
  }
  *error = M3D_TEXTYPE;
  return NULL;
}

/** Release a texture */
VOID M3D_FreeTexture(M3D_Context *context, M3D_Texture *texture)
{
  if (context == NULL) {
    return;
  }
  if (texture != NULL) {
    M3D_FreeMem(texture->data);
    M3D_FreeMem(texture);
  }
}
