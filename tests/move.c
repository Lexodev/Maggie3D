/**
 * Magie3D library
 *
 * Move test
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.0 November 2023
 */

#include <exec/exec.h>
#include <intuition/intuitionbase.h>
#include <cybergraphx/cybergraphics.h>

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/cybergraphics.h>

#include <stdlib.h>
#include <stdio.h>

#include "/src/Maggie3D.h"

#define INTUITIONVERSION      39L
#define CYBERGFXVERSION       41L

#define SCREEN_WANTED_WIDTH   640L
#define SCREEN_WANTED_HEIGHT  480L
#define SCREEN_WANTED_DEPTH   16L

#define WINDOW_WIDTH          640L
#define WINDOW_HEIGHT         480L
#define WINDOW_BPP            2L

#define SIGMASK(w) (1L<<((w)->UserPort->mp_SigBit))
#define GETIMSG(w) ((struct IntuiMessage *)GetMsg((w)->UserPort))

#define ESCAPE_KEY            69
#define F1_KEY                80
#define F2_KEY                81
#define F3_KEY                82
#define F4_KEY                83
#define F5_KEY                84
#define F6_KEY                85
#define F7_KEY                86
#define F8_KEY                87
#define F9_KEY                88
#define F10_KEY               89
#define UP_KEY                76
#define DOWN_KEY              77
#define LEFT_KEY              79
#define RIGHT_KEY             78
#define SPACE_KEY             64

/** @var Intuition library */
struct IntuitionBase *IntuitionBase = NULL;

/** @var CybergraphX library */
struct Library *CyberGfxBase = NULL;

/** Maggie3D context */
M3D_Context *mycontext = NULL;
/** Maggie texture data */
M3D_Texture *mytexture1 = NULL, *mytexture2 = NULL;
/** Maggie active triangle */
M3D_Triangle *mytriangle = NULL;

// Flat top
M3D_Triangle FT_triangle = {
  200.0, 140.0, 110.0, 0.0, 250.0, 250.0, 1.0,
  560.0, 10.0, 50.0, 0.0, 250.0, 0.0, 1.0,
  110.0, 10.0, 10.0, 0.0, 0.0, 0.0, 1.0,
  NULL,
  0xff0000
};

// Flat bottom
M3D_Triangle FB_triangle = {
  250.0, 250.0, 110.0, 0.0, 250.0, 250.0, 1.0,
  560.0, 410.0, 50.0, 0.0, 250.0, 0.0, 1.0,
  110.0, 410.0, 10.0, 0.0, 0.0, 0.0, 1.0,
  NULL,
  0x00ff00
};

// Generic 1, long left
M3D_Triangle G1_triangle = {
  350.0, 40.0, 110.0, 0.0, 250.0, 250.0, 1.0,
  560.0, 210.0, 50.0, 0.0, 250.0, 0.0, 1.0,
  210.0, 310.0, 10.0, 0.0, 0.0, 0.0, 1.0,
  NULL,
  0x0000ff
};

// Generic 2, long right
M3D_Triangle G2_triangle = {
  150.0, 40.0, 110.0, 0.0, 250.0, 250.0, 1.0,
  460.0, 310.0, 50.0, 0.0, 250.0, 0.0, 1.0,
  60.0, 210.0, 10.0, 0.0, 0.0, 0.0, 1.0,
  NULL,
  0xff00ff
};

/**
 * Open libraries
 *
 * @return Operation success
 */
BOOL OpenAllLibs(void)
{
  printf("* Open librairies *\n");
  if ((IntuitionBase = (struct IntuitionBase *) OpenLibrary("intuition.library", INTUITIONVERSION)) == NULL) {
    printf("ERROR: can't open intuition.library V%d\n", INTUITIONVERSION);
    return FALSE;
  }
  if ((CyberGfxBase = OpenLibrary(CYBERGFXNAME, CYBERGFXVERSION)) == NULL) {
    printf("ERROR: can't open cybergraphx.library V%d\n", CYBERGFXVERSION);
    return FALSE;
  }
  return TRUE;
}

/**
 * Close libraries
 */
VOID CloseAllLibs(void)
{
  printf("* Close librairies *\n");
  if (CyberGfxBase != NULL) {
    CloseLibrary(CyberGfxBase);
  }
  if (IntuitionBase != NULL) {
    CloseLibrary(&IntuitionBase->LibNode);
  }
}

VOID GetPixelFormat(ULONG format)
{
  printf(" Pixel format is ");
  switch (format) {
    case PIXFMT_LUT8:
      printf("LUT8\n");
      break;
    case PIXFMT_RGB15:
      printf("RGB15\n");
      break;
    case PIXFMT_BGR15:
      printf("BGR15\n");
      break;
    case PIXFMT_RGB15PC:
      printf("RGB15PC\n");
      break;
    case PIXFMT_BGR15PC:
      printf("BGR15PC\n");
      break;
    case PIXFMT_RGB16:
      printf("RGB16\n");
      break;
    case PIXFMT_BGR16:
      printf("BGR16\n");
      break;
    case PIXFMT_RGB16PC:
      printf("RGB16PC\n");
      break;
    case PIXFMT_BGR16PC:
      printf("BGR16PC\n");
      break;
    case PIXFMT_RGB24:
      printf("RGB24\n");
      break;
    case PIXFMT_BGR24:
      printf("BGR24\n");
      break;
    case PIXFMT_ARGB32:
      printf("ARGB32\n");
      break;
    case PIXFMT_BGRA32:
      printf("BGRA32\n");
      break;
   case PIXFMT_RGBA32:
      printf("RGBA32\n");
      break;
    default:
      printf("Unknown");
  }
}

ULONG GetBitmapAddress(struct BitMap *bitmap)
{
  APTR Handle = NULL;
  ULONG BaseAddress = 0, BytesPerRow = 0;

  if (GetCyberMapAttr(bitmap, CYBRMATTR_ISCYBERGFX)) {
    Handle = LockBitMapTags(
      bitmap,
      LBMI_BASEADDRESS, &BaseAddress,
      LBMI_BYTESPERROW, &BytesPerRow,
      TAG_DONE
    );
  }
  if (Handle != NULL) {
    UnLockBitMap(Handle);
  }
  return BaseAddress;
}

VOID DumpBitmap(struct BitMap *bitmap)
{
  UWORD planes;

  printf("Dumping BitMap at 0x%X\n", bitmap);
  printf(" BytesPerRow           %d\n", bitmap->BytesPerRow);
  printf(" Rows                  %d\n", bitmap->Rows);
  printf(" Depth                 %d\n", bitmap->Depth);
  for (planes = 0;planes < 8;planes++) {
    printf(" Plane %d               0x%X\n", planes, bitmap->Planes[planes]);
  }
  if (GetCyberMapAttr(bitmap, CYBRMATTR_ISCYBERGFX)) {
    printf(" This is a Cybergfx BitMap\n");
    printf(" Width                 %d\n", GetCyberMapAttr(bitmap, CYBRMATTR_WIDTH));
    printf(" Height                %d\n", GetCyberMapAttr(bitmap, CYBRMATTR_HEIGHT));
    printf(" Depth                 %d\n", GetCyberMapAttr(bitmap, CYBRMATTR_DEPTH));
    printf(" Bytes per row         %d\n", GetCyberMapAttr(bitmap, CYBRMATTR_XMOD));
    printf(" Bytes per pixel       %d\n", GetCyberMapAttr(bitmap, CYBRMATTR_BPPIX));
    GetPixelFormat(GetCyberMapAttr(bitmap, CYBRMATTR_PIXFMT));
    GetBitmapAddress(bitmap);
  }
}

VOID ClearScreen(struct BitMap *bitmap)
{
  ULONG *buffer, size;
  
  buffer = (ULONG *) GetBitmapAddress(bitmap);
  if (buffer != 0) {
    size = SCREEN_WANTED_WIDTH * SCREEN_WANTED_HEIGHT / 2;
    while (size--) {
      *buffer++ = 0x88888888;
    }
  }
}

BOOL InitDemo(struct BitMap *bitmap)
{
  LONG error;

  if (M3D_CheckMaggie()) {
    printf("* Maggie support detected !\n");
  }
  mycontext = M3D_CreateContext(&error, bitmap);
  if (mycontext != NULL) {
    printf("- Maggie context created\n");
    M3D_SetState(mycontext, M3D_ZBUFFER, M3D_ENABLE);
    if (M3D_AllocZBuffer(mycontext) == M3D_SUCCESS) {
      printf("- Z buffer available\n");
    }
    M3D_SetState(mycontext, M3D_16BITS, M3D_ENABLE);
    printf("- Activate 16bits output\n");
    printf("- Enable gouraud shading\n");
    M3D_SetState(mycontext, M3D_GOURAUD, M3D_ENABLE);
    printf("- Enable texture mapping\n");
    M3D_SetState(mycontext, M3D_TEXMAPPING, M3D_ENABLE);
    mytexture1 = M3D_AllocTextureFile(mycontext, &error, "texture.dds");
    if (mytexture1 == NULL) {
      return FALSE;
    }
    printf("- DDS texture loaded & allocated\n");
    FT_triangle.texture = mytexture1;
    G1_triangle.texture = mytexture1;
    mytexture2 = M3D_AllocTextureFile(mycontext, &error, "texture.bmp");
    if (mytexture2 == NULL) {
      return FALSE;
    }
    printf("- BMP texture loaded & allocated\n");
    FB_triangle.texture = mytexture2;
    G2_triangle.texture = mytexture2;
    mytriangle = &FT_triangle;
    return TRUE;
  }
  return FALSE;
}

VOID RestoreDemo(VOID)
{
  printf("- Release textures\n");
  M3D_FreeTexture(mycontext, mytexture1);
  M3D_FreeTexture(mycontext, mytexture2);
  printf("- Release Z buffer\n");
  M3D_FreeZBuffer(mycontext);
  printf("- Release context\n");
  M3D_DestroyContext(mycontext);
}

VOID AnimateDemo(struct BitMap *bitmap)
{
  ClearScreen(bitmap);
  if (M3D_LockHardware(mycontext) == M3D_SUCCESS) {
    M3D_SetDrawRegion(mycontext, bitmap, NULL);
    M3D_ClearZBuffer(mycontext);
    M3D_DrawTriangle(mycontext, mytriangle);
    M3D_UnlockHardware(mycontext);
  } else {
    printf("Hardware lock failed !\n");
  }
}

VOID UpdateTriangle(FLOAT dx, FLOAT dy)
{
  mytriangle->v1.x += dx;
  mytriangle->v1.y += dy;
  mytriangle->v2.x += dx;
  mytriangle->v2.y += dy;
  mytriangle->v3.x += dx;
  mytriangle->v3.y += dy;
}

VOID CheckKey(UBYTE key)
{
  switch(key) {
    case F1_KEY:
      mytriangle = &FT_triangle;
      break;
    case F2_KEY:
      mytriangle = &FB_triangle;
      break;
    case F3_KEY:
      mytriangle = &G1_triangle;
      break;
    case F4_KEY:
      mytriangle = &G2_triangle;
      break;
    case UP_KEY:
      UpdateTriangle((FLOAT)0.0, (FLOAT)-10.0);
      break;
    case DOWN_KEY:
      UpdateTriangle((FLOAT)0.0, (FLOAT)10.0);
      break;
    case LEFT_KEY:
      UpdateTriangle((FLOAT)-10.0, (FLOAT)0.0);
      break;
    case RIGHT_KEY:
      UpdateTriangle((FLOAT)10.0, (FLOAT)0.0);
      break;
  }
}

/**
 * Main function
 *
 * @param argc Number of command line arguments
 * @param argv Array of arguments
 *
 * @return Error code
 */
int main(int argc, char **argv)
{
  struct Screen *CyberScreen = NULL;
  struct Window *CyberWindow = NULL;
  struct IntuiMessage *IntMsg = NULL;
  ULONG DisplayID;
  BOOL Done;

  printf("**************************\n");
  printf("**    Maggie 3D test    **\n");
  printf("** Quit with escape key **\n");
  printf("**************************\n");
  if (OpenAllLibs()) {
    
    printf("Search best mode for a screen of %dx%dx%d\n", SCREEN_WANTED_WIDTH, SCREEN_WANTED_HEIGHT, SCREEN_WANTED_DEPTH);
    
    // Find best screen mode
    DisplayID = BestCModeIDTags(
      CYBRBIDTG_NominalWidth, SCREEN_WANTED_WIDTH,
      CYBRBIDTG_NominalHeight, SCREEN_WANTED_HEIGHT,
      CYBRBIDTG_Depth, SCREEN_WANTED_DEPTH,
      TAG_DONE
    );
    if (DisplayID != INVALID_ID) {
      printf("Found a suitable screenmode with ID 0x%X\n", DisplayID);
      if (IsCyberModeID(DisplayID)) {
        printf("This is a CyberGfx mode ID\n");
        printf(" Width is %d\n", GetCyberIDAttr(CYBRIDATTR_WIDTH, DisplayID));
        printf(" Height is %d\n", GetCyberIDAttr(CYBRIDATTR_HEIGHT, DisplayID));
        printf(" Depth is %d\n", GetCyberIDAttr(CYBRIDATTR_DEPTH, DisplayID));
        printf(" Bytes per pixel is %d\n", GetCyberIDAttr(CYBRIDATTR_BPPIX, DisplayID));
        GetPixelFormat(GetCyberIDAttr(CYBRIDATTR_PIXFMT, DisplayID));
      } else {
        printf("This is not a CyberGfx mode !\n");
      }

      // Open the screen
      printf("* Open the screen *\n");
      CyberScreen = OpenScreenTags(
        NULL,
        SA_Title, NULL,
        SA_ShowTitle, FALSE,
        SA_DisplayID, DisplayID,
        SA_Type, CUSTOMSCREEN,
        TAG_DONE
      );
      if (CyberScreen != NULL) {
        DumpBitmap(CyberScreen->RastPort.BitMap);
        
        // Open the window
        printf("* Open the window *\n");
        CyberWindow = OpenWindowTags(
          NULL,
          WA_Title, NULL,
          WA_Flags, WFLG_ACTIVATE|WFLG_BACKDROP|WFLG_BORDERLESS|WFLG_RMBTRAP,
          WA_IDCMP, IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS,
          WA_Left, 0,
          WA_Top, 0,
          WA_Width, WINDOW_WIDTH,
          WA_Height, WINDOW_HEIGHT,
          WA_CustomScreen, CyberScreen,
          TAG_DONE
        );
        if (CyberWindow != NULL) {
          DumpBitmap(CyberWindow->RPort->BitMap);
          
          if (InitDemo(CyberScreen->RastPort.BitMap)) {
            AnimateDemo(CyberScreen->RastPort.BitMap);
            
            // Main loop
            printf("* Enter the main loop *\n");
            Done = FALSE;
            while (!Done) {
              (void)Wait(SIGMASK(CyberWindow));
              while (IntMsg = GETIMSG(CyberWindow)) {
                switch (IntMsg->Class) {
                  case IDCMP_RAWKEY:
                    if (IntMsg->Code < 128) {
                      if (IntMsg->Code == ESCAPE_KEY) {
                        printf("Exit loop\n");
                        Done = TRUE;
                      } else {
                        CheckKey(IntMsg->Code);
                      }
                    }
                    break;
                  default:
                    printf("Uncatched event %d - %d\n", IntMsg->Class, IntMsg->Code);
                    break;
                }
                ReplyMsg (&IntMsg->ExecMessage);
              }
              AnimateDemo(CyberScreen->RastPort.BitMap);
            }

          }
          RestoreDemo();
          printf("* Close the window *\n");
          CloseWindow(CyberWindow);
        } else {
          printf("ERROR: Failed to open the window\n");
        }
        printf("* Close the screen *\n");
        CloseScreen(CyberScreen);
      } else {
        printf("ERROR: Failed to open the screen\n");
      }
    } else {
      printf("ERROR: Failed to find a suitable resolution\n");
    }
  } else {
    printf("ERROR: Failed to open libraries\n");
  }
  CloseAllLibs();
  printf("** Program terminated **\n");
  return 0;
}
