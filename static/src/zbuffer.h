/**
 * zbuffer.h
 *
 * Maggie3D static library
 * Z buffer management functions
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.5 May 2024 (updated: 27/05/2024)
 */

#ifndef _ZBUFFER_H_
#define _ZBUFFER_H_

#define ZBUF_BPP                    2
#define ZBUF_ALIGN                  8
#define ZBUF_CLEARVAL               0xffff

/** External function for Z buffer clear */
extern BOOL __asm M3D_FastClearZBuffer(
  register __a0 ULONG source,
  register __d0 UWORD lines,
  register __d1 UWORD bytes
);

#endif