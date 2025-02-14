/**
 * loader.c
 *
 * Maggie3D static library
 * Load a BMP or DDS file
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.5 may 2024 (updated: 27/05/2024)
 */

#include <stdio.h>
#include <string.h>

#include <dos/dos.h>

#include <proto/dos.h>

#include "debug.h"
#include "memory.h"
#include "texture.h"
#include "Maggie3D.h"

/** Load a DDS texture */
M3D_TextureFile *M3D_LoadDDSTexture(LONG *error, STRPTR file_name)
{
  BPTR file_handle;
  M3D_DDSHeader header;
  M3D_TextureFile *texfile;
  LONG file_size, bytes_read;

  Dbug(printf("[MAGGIE3D] M3D_LoadDDSTexture %s \n", file_name);)
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
    Dbug(printf(
        "[MAGGIE3D] DDS header => width=%d, height=%d, depth=%d, data_size=%d \n",
        texfile->width, texfile->height, texfile->depth, texfile->data_size
    );)
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

/** Load a BMP texture */
M3D_TextureFile *M3D_LoadBMPTexture(LONG *error, STRPTR file_name)
{
  BPTR file_handle;
  M3D_BMPHeader dib_header;
  M3D_TextureFile *texfile;
  UBYTE *picture_body, *picture_data, pixel;
  LONG bytes_read, image_offset, line_size, height, width, color;

  Dbug(printf("[MAGGIE3D] M3D_LoadBMPTexture %s \n", file_name);)
  file_handle = Open(file_name, MODE_OLDFILE);
  if (file_handle != 0) {
    bytes_read = Seek(file_handle, 10, OFFSET_BEGINING);
    texfile = (M3D_TextureFile *) M3D_AllocMem(sizeof(M3D_TextureFile));
    if (texfile == NULL) {
      Close(file_handle);
      *error = M3D_NOMEMORY;
      return NULL;
    }
    // Get header size
    bytes_read = Read(file_handle, &image_offset, sizeof(image_offset));
    if (bytes_read != sizeof(image_offset)) {
      M3D_FreeMem(texfile);
      Close(file_handle);
      *error = M3D_FILEREAD;
      return NULL;
    }
    image_offset = M3D_LONGTOBE(image_offset);
    // Get DIB header
    bytes_read = Read(file_handle, &dib_header, sizeof(dib_header));
    if (bytes_read != sizeof(dib_header)) {
      M3D_FreeMem(texfile);
      Close(file_handle);
      *error = M3D_FILEREAD;
      return NULL;
    }
    dib_header.size = M3D_LONGTOBE(dib_header.size);
    Dbug(printf("[MAGGIE3D] BMP image_offset=%d, dib_size=%d \n", image_offset, dib_header.size);)
    // Load only Windows 3 or + format
    if (dib_header.size != TEX_BMPWIN3 && dib_header.size != TEX_BMPWIN4 && dib_header.size != TEX_BMPWIN5) {
      M3D_FreeMem(texfile);
      Close(file_handle);
      *error = M3D_TEXTYPE;
      return NULL;
    }
    texfile->width = M3D_LONGTOBE(dib_header.width);
    texfile->height = M3D_LONGTOBE(dib_header.height);
    texfile->depth = M3D_WORDTOBE(dib_header.depth);
    texfile->data_size = texfile->width * texfile->height * (texfile->depth / 8);
    // Accept only 8/16/24/32 bits
    if (texfile->depth == 8) {
      Dbug(printf("[MAGGIE3D] Load the color map\n");)
      bytes_read = Seek(file_handle, 0, OFFSET_BEGINNING);
      bytes_read = Seek(file_handle, dib_header.size + TEX_BMPHSIZE, OFFSET_BEGINNING);
      bytes_read = Read(file_handle, texfile->palette, (256 * 4));
      if (bytes_read != (256 * 4)) {
        M3D_FreeMem(texfile);
        Close(file_handle);
        *error = M3D_FILEREAD;
        return NULL;
      }
      for (color = 0;color < 256;color++) {
        texfile->palette[color] = M3D_LONGTOBE(texfile->palette[color]);
      }
      texfile->pixformat = M3D_PIXFMT_CLUT;
    } else if (texfile->depth == 16) {
      texfile->pixformat = M3D_PIXFMT_RGB16;
    } else if (texfile->depth == 24) {
      texfile->pixformat = M3D_PIXFMT_RGB24;
    } else if (texfile->depth == 32) {
      texfile->pixformat = M3D_PIXFMT_ARGB32;
    } else {
      M3D_FreeMem(texfile);
      Close(file_handle);
      *error = M3D_TEXTYPE;
      return NULL;
    }
    Dbug(printf(
        "[MAGGIE3D] BMP header => width=%d, height=%d, depth=%d, data_size=%d \n",
        texfile->width, texfile->height, texfile->depth, texfile->data_size
    );)
    // Reset file pointer
    bytes_read = Seek(file_handle, 0, OFFSET_BEGINNING);
    // Go to bitmap data
    bytes_read = Seek(file_handle, image_offset, OFFSET_BEGINNING);
    // Allocate texture buffer and load it
    picture_body = M3D_AllocMem(texfile->data_size);
    if (picture_body == NULL) {
      M3D_FreeMem(texfile);
      Close(file_handle);
      *error = M3D_NOMEMORY;
      return NULL;
    }
    bytes_read = Read(file_handle, picture_body, texfile->data_size);
    if (bytes_read != texfile->data_size) {
      M3D_FreeMem(picture_body);
      M3D_FreeMem(texfile);
      Close(file_handle);
      *error = M3D_FILEREAD;
      return NULL;
    }
    // Now allocate the texture file data
    texfile->data = M3D_AllocMem(texfile->data_size);
    if (texfile->data == NULL) {
      M3D_FreeMem(picture_body);
      M3D_FreeMem(texfile);
      Close(file_handle);
      *error = M3D_NOMEMORY;
      return NULL;  
    }
    Close(file_handle);
    // Mirror picture
    Dbug(printf("[MAGGIE3D] Mirror picture\n");)
    picture_data = (UBYTE *) texfile->data;
    line_size = texfile->width * (texfile->depth / 8);
    picture_body += line_size * (texfile->height - 1);
    height = texfile->height;
    while (height--) {
      width = line_size;
      while (width--) {
        *picture_data++ = *picture_body++;
      }
      picture_body -= (line_size * 2);
    }
    picture_body += line_size;
    M3D_FreeMem(picture_body);
    if (texfile->depth != 8) {
      // Remap to big endian format
      Dbug(printf("[MAGGIE3D] Remap to Big endian format\n");)
      picture_data = (UBYTE *) texfile->data;
      if (texfile->depth == 16) {
        height = texfile->height;
        while (height--) {
          width = texfile->width;
          while (width--) {
            pixel = picture_data[0];
            picture_data[0] = picture_data[1];
            picture_data[1] = pixel;
            picture_data += 2;
          }
        }
      } else if (texfile->depth == 24) {
        height = texfile->height;
        while (height--) {
          width = texfile->width;
          while (width--) {
            pixel = picture_data[0];
            picture_data[0] = picture_data[2];
            picture_data[2] = pixel;
            picture_data += 3;
          }
        }
      } else if (texfile->depth = 32) {
        height = texfile->height;
        while (height--) {
          width = texfile->width;
          while (width--) {
            pixel = picture_data[0];
            picture_data[0] = picture_data[3];
            picture_data[3] = pixel;
            pixel = picture_data[1];
            picture_data[1] = picture_data[2];
            picture_data[2] = pixel;
            picture_data += 4;
          }
        }
      }
    }
    *error = M3D_SUCCESS;
    return texfile;
  }
  *error = M3D_FILEREAD;
  return NULL;
}
