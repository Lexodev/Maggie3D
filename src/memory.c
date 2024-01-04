/**
 * memory.c
 *
 * Maggie3D library
 * Memory management functions
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.0 November 2023
 */

#include <proto/exec.h>

#include <stdio.h>

#include "debug.h"
#include "memory.h"
#include "Maggie3D.h"

M3D_MemoryManager M3D_Memory = { NULL, NULL };

/**
 * Add a memory entry to the memory list
 *
 * @param address Bloc address
 * @param size    Bloc size
 *
 * @return Operation success
 */
BOOL M3D_AddMemoryEntry(APTR base, APTR address, ULONG size)
{
  M3D_MemoryNode * new_node;

  new_node = (M3D_MemoryNode *) AllocMem(sizeof(M3D_MemoryNode), MEMF_PUBLIC);
  if (new_node) {
    new_node->base_address = base;
    new_node->memory_bloc = address;
    new_node->bloc_size = size;
    if (M3D_Memory.head == NULL) {
      new_node->previous = NULL;
      new_node->next = NULL;
      M3D_Memory.head = new_node;
      M3D_Memory.tail = new_node;
    } else {
      new_node->previous = M3D_Memory.tail;
      new_node->next = NULL;
      M3D_Memory.tail->next = new_node;
      M3D_Memory.tail = new_node;
    }
    return TRUE;
  }
  return FALSE;
}

/**
 * Check if alignment is on a power 2 boundary
 *
 * @param align Alignment in bytes
 *
 * @return Valid alignment
 */
ULONG M3D_CheckAlignment(ULONG align)
{
  ULONG limit;
  
  // Align max on 64k boundary
  limit = 65536;
  while (limit) {
    if (align >= limit) {
      return limit;
    }
    limit >>= 1;
  }
  return 0;
}

/**
 * Allocate a memory bloc and register it to the memory manager
 *
 * @param size       Bloc size
 * @param attributes Bloc attributes
 * @param align      Bloc alignment, must be a multiple of 2
 *
 * @return Memory bloc address or NULL on error
 */
APTR M3D_AllocMemoryBloc(ULONG size, ULONG attributes, ULONG align)
{
  APTR base, memory;

  // Make sure that align is a multiple of 2
  align = M3D_CheckAlignment(align);
  // Reserve enough space for alignment
  size += align;
  // Allocate the required bloc
  base = AllocMem(size, attributes);
  if (base == NULL) {
    return NULL;
  }
  // Align the bloc if necessary
  if (align) {
    memory = (APTR)(((ULONG)base + align) & (-align));
  } else {
    memory = base;
  }
  if (!M3D_AddMemoryEntry(base, memory, size)) {
    FreeMem(base, size);
    return NULL;
  }
  Dbug(printf("[DEBUG] Memory allocation 0x%X (0x%X) of %d bytes (align %d)\n", base, memory, size, align);)
  return memory;
}

/**
 * Allocate generic memory bloc
 *
 * @param size Bloc size
 *
 * @return Memory bloc address or NULL on error
 */
APTR M3D_AllocMem(ULONG size)
{
  return M3D_AllocMemoryBloc(size, MEMF_PUBLIC|MEMF_CLEAR, 0);
}

/**
 * Allocate aligned generic memory bloc
 *
 * @param size  Bloc size
 * @param align Bloc alignment, must be a multiple of 2
 *
 * @return Memory bloc address or NULL on error
 */
APTR M3D_AllocAlignMem(ULONG size, ULONG align)
{
  return M3D_AllocMemoryBloc(size, MEMF_PUBLIC|MEMF_CLEAR, align);
}

/**
 * Release a memory bloc and unregister it from the memory manager
 *
 * @param address Bloc address
 */
VOID M3D_FreeMem(APTR address)
{
  M3D_MemoryNode * node;

  if (address == NULL) {
    return;
  }
  node = M3D_Memory.head;
  while (node != NULL && node->memory_bloc != address) {
    node = node->next;
  }
  // Memory bloc found
  if (node != NULL) {
    Dbug(printf("[DEBUG] Memory release 0x%X of %d bytes\n", node->base_address, node->bloc_size);)
    FreeMem(node->base_address, node->bloc_size);
    if (M3D_Memory.head == node) {
      M3D_Memory.head = node->next;
    }
    if (M3D_Memory.tail == node) {
      M3D_Memory.tail = node->previous;
    }
    if (node->previous != NULL) {
      node->previous->next = node->next;
    }
    if (node->next != NULL) {
      node->next->previous = node->previous;
    }
    FreeMem(node, sizeof(M3D_MemoryNode));
  }
}

/**
 * Release all memory bloc and unregister them from the memory manager
 */
VOID M3D_ReleaseMem()
{
  M3D_MemoryNode * node;

  node = M3D_Memory.head;
  while (node != NULL) {
    Dbug(printf("[DEBUG] Releasing memory bloc 0x%X of %d bytes\n", node->base_address, node->bloc_size);)
    FreeMem(node->base_address, node->bloc_size);
    M3D_Memory.head = node->next;
    FreeMem(node, sizeof(M3D_MemoryNode));
    node = M3D_Memory.head;
  }
  M3D_Memory.head = NULL;
  M3D_Memory.tail = NULL;
}

