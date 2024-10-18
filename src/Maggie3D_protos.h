/**
 * maggie3d_protos.h
 *
 * Maggie3D shared library
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.5 June 2024 (updated: 01/06/2024)
 */

#ifndef _MAGGIE3D_PROTOS_H_
#define _MAGGIE3D_PROTOS_H_

#include <graphics/gfx.h>
#include <Maggie3D/Maggie3D.h>

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
