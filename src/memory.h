/**
 * memory.h
 *
 * Maggie3D library
 * Memory management functions
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.0 November 2023
 */

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <exec/exec.h>

// Little endian to Big endian conversion
#define M3D_WORDTOBE(value)  ((value & 0xff00) >> 8) | ((value & 0xff) << 8)
#define M3D_LONGTOBE(value)  ((value & 0xff) << 24) | ((value & 0xff00) << 8) | ((value & 0xff000000) >> 24) | ((value & 0xff0000) >> 8)

typedef struct _memory_node {
  /** Base address of bloc, before alignment */
  APTR base_address;
  /** Address of memory bloc */
  APTR memory_bloc;
  /** Size of memory bloc */
  ULONG bloc_size;
  /** Previous memory node */
  struct _memory_node * previous;
  /** Next memory node */
  struct _memory_node * next;
} M3D_MemoryNode;

/** Memory manager */
typedef struct {
  /** Head of memory list */
  M3D_MemoryNode * head;
  /** Tail of memory list */
  M3D_MemoryNode * tail;
} M3D_MemoryManager;

/** Allocate public memory */
APTR M3D_AllocMem(ULONG);

/** Allocate align public memory */
APTR M3D_AllocAlignMem(ULONG, ULONG);

/** Free any kind of memory */
VOID M3D_FreeMem(APTR);

/** Release all memory blocs */
VOID M3D_ReleaseMem(VOID);

#endif
