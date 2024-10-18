/**
 * texture.c
 *
 * Maggie3D shared library
 * Texture management functions
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.6 June 2024 (updated: 24/06/2024)
 */

#include <stdio.h>
#include <string.h>

#include <dos/dos.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>

#include "debug.h"
#include "memory.h"
#include "texture.h"
#include "Maggie3D.h"

/** Check if texture size is valid */
BOOL M3D_CheckTextureSize(ULONG width, ULONG height)
{
  if (width != 64 && width != 128 && width != 256 && width != 512) {
    return FALSE;
  }
  if (height == 0 || height % 4 != 0) {
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

/** Check for a BMP file */
BOOL M3D_CheckBMPFile(STRPTR file_name)
{
  BPTR file_handle;
  WORD bmp_tag;
  LONG bytes_read;
  
  file_handle = Open(file_name, MODE_OLDFILE);
  if (file_handle != 0) {
    bytes_read = Read(file_handle, &bmp_tag, sizeof(bmp_tag));
    if (bytes_read != sizeof(bmp_tag)) {
      Close(file_handle);
      return FALSE;
    }
    Close(file_handle);
    if (bmp_tag == TEX_BMPTAG) {
      return TRUE;
    }
  }
  return FALSE;
}

/** Get the mipmap size value depending on texture size */
UWORD M3D_GetTextureMipmapSize(UWORD size)
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

/** Convert CLUT to RGBA32 format */
VOID M3D_ConvertCLUTToRGBA32(UBYTE *source, UBYTE *dest, ULONG width, ULONG height, ULONG *palette, BOOL transparency, ULONG tcolor)
{
  UBYTE pixel;
  ULONG size, color;
  
  Dbug(kprintf("[MAGGIE3D] Convert CLUT to RGBA32 format %s transparency (0x%lX)\n", (transparency ? "with" : "without"), tcolor);)
  size = width * height;
  while (size--) {
    pixel = *source++;
    color = palette[pixel];
    dest[0] = (UBYTE)((color & 0xff0000) >> 16);
    dest[1] = (UBYTE)((color & 0xff00) >> 8);
    dest[2] = (UBYTE)(color & 0xff);
    if (transparency && pixel == tcolor) {
      dest[3] = 0x00;
    } else {
      dest[3] = 0xff;
    }
    dest += 4;
  }
}

/** Convert RGB16 to RGBA32 format */
VOID M3D_ConvertRBG16ToRGBA32(UWORD *source, UBYTE *dest, ULONG width, ULONG height, BOOL transparency, ULONG tcolor)
{
  UWORD pixel, color;
  ULONG size;
  
  Dbug(kprintf("[MAGGIE3D] Convert RGB16 to RGBA32 format %s transparency (0x%lX)\n", (transparency ? "with" : "without"), tcolor);)
  color = (UWORD) (((tcolor & 0xf80000) >> 8) | ((tcolor & 0xfc00) >> 5) | ((tcolor & 0xf8) >> 3));
  size = width * height;
  while (size--) {
    pixel = *source++;
    dest[0] = (UBYTE)((pixel & 0xf800) >> 8);
    dest[1] = (UBYTE)((pixel & 0x7e0) >> 3);
    dest[2] = (UBYTE)((pixel & 0x1f) << 3);
    if (transparency && pixel == color) {
      dest[3] = 0x00;
    } else {
      dest[3] = 0xff;
    }
    dest += 4;
  }
}

/** Copy RGB24 to RGBA32 format */
VOID M3D_ConvertRBG24ToRGBA32(UBYTE *source, UBYTE *dest, ULONG width, ULONG height, BOOL transparency, ULONG tcolor)
{
  ULONG size, pixel;
  
  Dbug(kprintf("[MAGGIE3D] Convert RGB24 to RGBA32 %s transparency (0x%lX)\n", (transparency ? "with" : "without"), tcolor);)
  size = width * height;
  while (size--) {
    dest[0] = source[0];
    dest[1] = source[1];
    dest[2] = source[2];
    pixel = (source[0] * 65536) + (source[1] * 256) + source[2];
    if (transparency && pixel == tcolor) {
      dest[3] = 0x00;
    } else {
      dest[3] = 0xff;
    }
    source += 3;
    dest += 4;
  }
}

/** Convert ARGB32 to RGBA32 format */
VOID M3D_ConvertARBG32ToRGBA32(ULONG *source, UBYTE *dest, ULONG width, ULONG height, BOOL transparency, ULONG tcolor)
{
  ULONG pixel, size;
  
  Dbug(kprintf("[MAGGIE3D] Convert ARGB32 to RGBA32 format %s transparency (0x%lX)\n", (transparency ? "with" : "without"), tcolor);)
  size = width * height;
  while (size--) {
    pixel = *source++;
    dest[0] = (UBYTE)((pixel & 0xff0000) >> 16);
    dest[1] = (UBYTE)((pixel & 0xff00) >> 8);
    dest[2] = (UBYTE)(pixel & 0xff);
    if (transparency && (pixel & 0xffffff) == tcolor) {
      dest[3] = 0x00;
    } else {
      dest[3] = (UBYTE)((pixel & 0xff000000) >> 24);
    }
    dest += 4;
  }
}

/** Add a texture to the texture list */
BOOL M3D_AddTexture(M3D_Context *context, M3D_Texture *texture)
{
  UWORD i;
  
  for (i=0;i < M3D_MAX_TEXTURE;i++) {
    if (context->textures[i] == NULL) {
      context->textures[i] = texture;
      Dbug(kprintf("[MAGGIE3D] Texture added to the list\n");)
      return TRUE;
    }
  }
  return FALSE;
}

/** Remove a texture from the texture list */
VOID M3D_RemoveTexture(M3D_Context *context, M3D_Texture *texture)
{
  UWORD i;
  
  for (i=0;i < M3D_MAX_TEXTURE;i++) {
    if (context->textures[i] == texture) {
      context->textures[i] = NULL;
      Dbug(kprintf("[MAGGIE3D] Texture removed from the list\n");)
      return;
    }
  }
}

/** Resize a texture to a standard size */
APTR M3D_ResizeTexture(M3D_Texture *texture, APTR data)
{
  ULONG width, height, x, y, *src, *dest;
  APTR new_data = NULL;
  
  Dbug(kprintf("[MAGGIE3D] Resize texture\n");)
  width = texture->width;
  height = texture->height;
  // Find the best new width
  if (width > 256) width = 512;
  else if (width > 128) width = 256;
  else if (width > 64) width = 128;
  else width = 64;
  // Find the best new height
  height += (height % 4);
  // Resize the texture
  if ((new_data = M3D_AllocMem(width * height * 4)) == NULL) {
    return NULL;
  }
  src = (ULONG *) data;
  dest = (ULONG *) new_data;
  for (y = 0; y < texture->height;y++) {
    for (x = 0;x < texture->width;x++) {
      dest[y * width + x] = src[y * texture->width + x];
    }
  }
  texture->width = width;
  texture->height = height;
  texture->mipsize = M3D_GetTextureMipmapSize(width);
  Dbug(kprintf("[MAGGIE3D] Texture resized to %ld x %ld\n", width, height);)
  return new_data;
}

/** Allocate a texture with Tags */
M3D_Texture __asm __saveds *LIBM3D_AllocTextureTagList(
  register __a0 M3D_Context *context,
  register __a1 LONG *error,
  register __a2 struct TagItem *tags
)
{
  M3D_TextureFile *texfile;
  M3D_Texture *texture;
  STRPTR filename;
  APTR data;
  APTR tmp_data;
  UWORD pixformat;
  ULONG width, height, *palette, color;
  BOOL transparency, autoresize;
  
  Dbug(kprintf("[MAGGIE3D] Allocate new texture\n");)
  if (context == NULL) {
    *error = M3D_NOCONTEXT;
    return NULL;
  }
  // Get the tags
  filename = (STRPTR) GetTagData(M3D_TT_FILENAME, NULL, tags);
  data = (APTR) GetTagData(M3D_TT_DATA, NULL, tags);
  pixformat = (UWORD) GetTagData(M3D_TT_FORMAT, M3D_PIXFMT_UNKNOWN, tags);
  width = (ULONG) GetTagData(M3D_TT_WIDTH, 0L, tags);
  height = (ULONG) GetTagData(M3D_TT_HEIGHT, 0L, tags);
  palette = (ULONG *) GetTagData(M3D_TT_PALETTE, NULL, tags);
  transparency = (BOOL) GetTagData(M3D_TT_TRANSPARENCY, FALSE, tags);
  color = (ULONG) GetTagData(M3D_TT_TRSCOLOR, 0L, tags);
  autoresize = (BOOL) GetTagData(M3D_TT_AUTORESIZE, FALSE, tags);
  // Should we load it from file ?
  if (filename != NULL) {
    if (M3D_CheckDDSFile(filename)) {
      if ((texfile = M3D_LoadDDSTexture(error, filename)) == NULL) {
        return NULL;
      }
      Dbug(kprintf("[MAGGIE3D] DDS texture loaded\n");)
    } else if (M3D_CheckBMPFile(filename)) {
      if ((texfile = M3D_LoadBMPTexture(error, filename)) == NULL) {
        return NULL;
      }
      Dbug(kprintf("[MAGGIE3D] BMP texture loaded\n");)
    } else {
      Dbug(kprintf("[MAGGIE3D] Texture file format not yet supported !\n");)
      *error = M3D_TEXTYPE;
      return NULL;
    }
    data = texfile->data;
    pixformat = texfile->pixformat;
    width = texfile->width;
    height = texfile->height;
    palette = texfile->palette;
    M3D_FreeMem(texfile);
  }
  // Check for params validity
  if (data != NULL) {
    // Slot available for texture
    if (!M3D_AddTexture(context, NULL)) {
      *error = M3D_TEXLIMIT;
      return NULL;
    }
    // Invalid size
    if (width == 0 || height == 0 || width > 512) {
      *error = M3D_TEXSIZE;
      return NULL;
    }
    // No resize for DXT1 texture
    if (pixformat == M3D_PIXFMT_DXT1) {
      autoresize = FALSE;
    }
    // Check size if no resize requested
    if (!autoresize && !M3D_CheckTextureSize(width, height)) {
      *error = M3D_TEXSIZE;
      return NULL;
    }
    // Allocate and init texture
    texture = (M3D_Texture *) M3D_AllocMem(sizeof(M3D_Texture));
    if (texture != NULL) {
      texture->width = width;
      texture->height = height;
      texture->mipsize = M3D_GetTextureMipmapSize(width);
      texture->filtering = M3D_NEAREST;
      Dbug(kprintf(
          "[MAGGIE3D] Allocate texture => width=%ld, height=%ld, mipsize=%ld,  filtering=%ld\n",
          texture->width, texture->height, texture->mipsize, texture->filtering
      );)
      if (pixformat == M3D_PIXFMT_DXT1) {
#if _USE_MAGGIE_ == 1
        Dbug(kprintf("[MAGGIE3D] Native DXT1 texture, no conversion needed\n");)
        if ((texture->data = M3D_AllocAlignMem(M3D_GetTextureDataSize(texture->mipsize), TEX_DATAALIGN)) == NULL) {
          M3D_FreeMem(texture);
          *error = M3D_NOMEMORY;
          return NULL;
        }
        CopyMem(data, texture->data, M3D_GetTextureDataSize(texture->mipsize));
        *error = M3D_SUCCESS;
#else
        Dbug(kprintf("[MAGGIE3D] DXT1 texture, decompression to RGBA needed\n");)
        if ((texture->data = M3D_AllocMem(texture->width * texture->height * 4)) == NULL) {
          M3D_FreeMem(texture);
          *error = M3D_NOMEMORY;
          return NULL;
        }
        *error = M3D_ConvertFromDXT1(texture, data);
#endif
        if (!M3D_AddTexture(context, texture)) {
          M3D_FreeMem(texture);
          *error = M3D_TEXLIMIT;
          return NULL;
        }
        return texture;
      } else {
        Dbug(kprintf("[MAGGIE3D] Not a native DXT1 texture\n");)
        if (pixformat != M3D_PIXFMT_CLUT && pixformat != M3D_PIXFMT_RGB16 && pixformat != M3D_PIXFMT_RGB24 && pixformat != M3D_PIXFMT_ARGB32) {
          M3D_FreeMem(texture);
          *error = M3D_TEXTYPE;
          return NULL;
        }
        // Buffer for source conversion in RGBA format
        if ((tmp_data = M3D_AllocMem(texture->width * texture->height * 4)) == NULL) {
          M3D_FreeMem(texture);
          *error = M3D_NOMEMORY;
          return NULL;
        }
        // Convert source to RGBA format
        Dbug(kprintf("[MAGGIE3D] Convert source data to RGBA32 format\n");)
        if (pixformat == M3D_PIXFMT_CLUT) {
          if (palette == NULL) {
            M3D_FreeMem(texture);
            *error = M3D_NOPALETTE;
            return NULL;
          }
          M3D_ConvertCLUTToRGBA32((UBYTE *)data, (UBYTE *)tmp_data, texture->width, texture->height, palette, transparency, color);
        } else if (pixformat == M3D_PIXFMT_RGB16) {
          M3D_ConvertRBG16ToRGBA32((UWORD *)data, (UBYTE *)tmp_data, texture->width, texture->height, transparency, color);
        } else if (pixformat == M3D_PIXFMT_RGB24) {
          M3D_ConvertRBG24ToRGBA32((UBYTE *)data, (UBYTE *)tmp_data, texture->width, texture->height, transparency, color);
        } else if (pixformat == M3D_PIXFMT_ARGB32) {
          M3D_ConvertARBG32ToRGBA32((ULONG *)data, (UBYTE *)tmp_data, texture->width, texture->height, transparency, color);
        }
        // Resize if requested
        if (autoresize) {
          data = M3D_ResizeTexture(texture, tmp_data);
          M3D_FreeMem(tmp_data);
          if (data == NULL) {
            M3D_FreeMem(texture);
            *error = M3D_TEXRESIZE;
            return NULL;
          }
          tmp_data = data;
        }
#if _USE_MAGGIE_ == 1
        if ((texture->data = M3D_AllocAlignMem(M3D_GetTextureDataSize(texture->mipsize), TEX_DATAALIGN)) == NULL) {
          M3D_FreeMem(tmp_data);
          M3D_FreeMem(texture);
          *error = M3D_NOMEMORY;
          return NULL;
        }
        *error = M3D_ConvertToDXT1(texture, tmp_data, texture->width * texture->height * 4);
        M3D_FreeMem(tmp_data);
        if (*error != M3D_SUCCESS) {
          M3D_FreeMem(texture->data);
          M3D_FreeMem(texture);
          return NULL;
        }
#else
        Dbug(kprintf("[MAGGIE3D] No conversion to DXT1 (Maggie not used)\n");)
        texture->data = tmp_data;
#endif
        if (!M3D_AddTexture(context, texture)) {
          M3D_FreeMem(texture);
          *error = M3D_TEXLIMIT;
          return NULL;
        }
        return texture;
      }
    }
    *error = M3D_NOMEMORY;
    return NULL;
  }
  *error = M3D_NOTEXTURE;
  return NULL;
}

/** Allocate a texture */
M3D_Texture __asm __saveds *LIBM3D_AllocTexture(
    register __a0 M3D_Context *context,
    register __a1 LONG *error,
    register __a2 APTR data,
    register __d0 UWORD pixformat,
    register __d1 ULONG width,
    register __d2 ULONG height,
    register __a3 ULONG *palette
)
{
  struct TagItem tags[6];

  tags[0].ti_Tag = M3D_TT_DATA;
  tags[0].ti_Data = (ULONG) data;
  tags[1].ti_Tag = M3D_TT_FORMAT;
  tags[1].ti_Data = (ULONG) pixformat;
  tags[2].ti_Tag = M3D_TT_WIDTH;
  tags[2].ti_Data = (ULONG) width;
  tags[3].ti_Tag = M3D_TT_HEIGHT;
  tags[3].ti_Data = (ULONG) height;
  tags[4].ti_Tag = M3D_TT_PALETTE;
  tags[4].ti_Data = (ULONG) palette;
  tags[5].ti_Tag = TAG_END;
  return LIBM3D_AllocTextureTagList(context, error, tags);
}

/** Load & allocate a texture from a file */
M3D_Texture __asm __saveds *LIBM3D_AllocTextureFile(register __a0 M3D_Context *context, register __a1 LONG *error, register __a2 STRPTR filename)
{
  struct TagItem tags[2];

  tags[0].ti_Tag = M3D_TT_FILENAME;
  tags[0].ti_Data = (ULONG) filename;
  tags[1].ti_Tag = TAG_END;
  return LIBM3D_AllocTextureTagList(context, error, tags);
}

/** Set texture filtering */
LONG __asm __saveds LIBM3D_SetFilter(register __a0 M3D_Context *context, register __a1 M3D_Texture *texture, register __d0 UWORD filtering )
{
  if (context == NULL) {
    return M3D_NOCONTEXT;
  }
  if (texture == NULL) {
    return M3D_NOTEXTURE;
  }
  if (filtering > M3D_LINEAR) {
    return M3D_NOFILTER;
  }
  texture->filtering = filtering;
  return M3D_SUCCESS;
}

/** Release a texture */
VOID __asm __saveds LIBM3D_FreeTexture(register __a0 M3D_Context *context, register __a1 M3D_Texture *texture)
{
  if (context == NULL) {
    return;
  }
  if (texture != NULL) {
    Dbug(kprintf("[MAGGIE3D] Free texture\n");)
    M3D_RemoveTexture(context, texture);
    M3D_FreeMem(texture->data);
    M3D_FreeMem(texture);
  }
}

/** Release all textures */
VOID __asm __saveds LIBM3D_FreeAllTextures(register __a0 M3D_Context *context)
{
  M3D_Texture *texture;
  UWORD i;
  
  Dbug(kprintf("[MAGGIE3D] Free all textures\n");)
  for (i=0;i < M3D_MAX_TEXTURE;i++) {
    texture = context->textures[i];
    if (texture != NULL) {
      LIBM3D_FreeTexture(context, texture);
    }
  }
}
