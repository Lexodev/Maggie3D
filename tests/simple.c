/**
 * Magie3D library
 *
 * Simple test
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

/** @var Intuition library */
struct IntuitionBase *IntuitionBase = NULL;

/** @var CybergraphX library */
struct Library *CyberGfxBase = NULL;

/** Maggie3D context */
M3D_Context *mycontext = NULL;
/** Maggie texture data */
M3D_Texture *mytexture = NULL;

// Debug
extern BOOL draw_debug;

// Triangle
M3D_Triangle Triangle = {
  200.0, 50.0, 110.0, 0.0, 1.0, 1.0, 1.0,
  600.0, 150.0, 50.0, 0.0, 1.0, 0.0, 1.0,
  400.0, 250.0, 10.0, 0.0, 0.0, 0.0, 1.0,
  NULL,
  0xffff00
};

// Flat top
M3D_Triangle FT_triangle = {
  200.0, 140.0, 110.0, 0.0, 250.0, 250.0, 1.0,
  560.0, 10.0, 50.0, 0.0, 250.0, 0.0, 0.2,
  110.0, 10.0, 10.0, 0.0, 0.0, 0.0, 0.5,
  NULL,
  0xff0000
};

// Flat bottom
M3D_Triangle FB_triangle = {
  250.0, 250.0, 110.0, 0.0, 250.0, 250.0, 1.0,
  560.0, 410.0, 50.0, 0.0, 250.0, 0.0, 0.6,
  110.0, 410.0, 10.0, 0.0, 0.0, 0.0, 0.5,
  NULL,
  0x00ff00
};

// Generic 1, long left
M3D_Triangle G1_triangle = {
  350.0, 40.0, 110.0, 0.0, 250.0, 250.0, 0.4,
  560.0, 210.0, 50.0, 0.0, 250.0, 0.0, 0.8,
  210.0, 310.0, 10.0, 0.0, 0.0, 0.0, 1.0,
  NULL,
  0x0000ff
};

// Generic 2, long right
M3D_Triangle G2_triangle = {
  150.0, 40.0, 110.0, 0.0, 250.0, 250.0, 1.0,
  460.0, 210.0, 50.0, 0.0, 250.0, 0.0, 0.05,
  60.0, 310.0, 10.0, 0.0, 0.0, 0.0, 0.25,
  NULL,
  0xff00ff
};

// Top left
M3D_Triangle TL_triangle = {
  100.0, 100.0, 50.0, 0.0, 0.0, 0.0, 1.0,
  356.0, 100.0, 50.0, 0.0, 255.0, 0.0, 0.5,
  100.0, 356.0, 50.0, 0.0, 0.0, 255.0, 0.8,
  NULL,
  0xffffff
};

// Bottom right
M3D_Triangle BR_triangle = {
  356.0, 100.0, 50.0, 0.0, 255.0, 0.0, 0.5,
  356.0, 356.0, 50.0, 0.0, 255.0, 255.0, 0.65,
  100.0, 356.0, 50.0, 0.0, 0.0, 255.0, 0.8,
  NULL,
  0xffffff
};

// Array of triangles
M3D_Triangle AR_triangles[] = {
  {
    200.0, 140.0, 110.0, 0.0, 250.0, 250.0, 1.0,
    560.0, 10.0, 50.0, 0.0, 250.0, 0.0, 0.3,
    110.0, 10.0, 10.0, 0.0, 0.0, 0.0, 0.6,
    NULL,
    0xff0000
  },
  {
    250.0, 250.0, 110.0, 0.0, 250.0, 250.0, 0.8,
    560.0, 410.0, 50.0, 0.0, 250.0, 0.0, 0.5,
    110.0, 410.0, 10.0, 0.0, 0.0, 0.0, 1.0,
    NULL,
    0x00ff00
  },
  {
    350.0, 40.0, 110.0, 0.0, 250.0, 250.0, 0.66,
    560.0, 210.0, 50.0, 0.0, 250.0, 0.0, 0.33,
    210.0, 310.0, 10.0, 0.0, 0.0, 0.0, 1.0,
    NULL,
    0x0000ff
  },
  {
    150.0, 40.0, 110.0, 0.0, 250.0, 250.0, 0.53,
    460.0, 310.0, 50.0, 0.0, 250.0, 0.0, 1.0,
    60.0, 210.0, 10.0, 0.0, 0.0, 0.0, 0.81,
    NULL,
    0xff00ff
  }
};

// Array of triangles
M3D_Triangle *LS_triangles[] = {&FT_triangle, &FB_triangle, &G1_triangle, &G2_triangle};

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
    printf(" Bitmap was locked with FB address 0x%X (BPR %d)\n", BaseAddress, BytesPerRow);
    if ((BaseAddress & 0xffffffe0) == BaseAddress) {
      printf(" Bitmap is 32 bytes aligned\n");
    } else {
      printf(" Bitmap is not 32 bytes aligned\n");
    }
    if (GetCyberMapAttr(bitmap, CYBRMATTR_ISLINEARMEM)) {
      printf(" Memory is linear\n");
    } else {
      printf(" Memory is not linear\n");
    }
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
  LONG error;
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

          if (M3D_CheckMaggie()) {
            printf("* Maggie support detected !\n");
          }
          mycontext = M3D_CreateContext(&error, CyberScreen->RastPort.BitMap);
          if (mycontext != NULL) {
            printf("- Maggie context created\n");
            printf("- Activate fast drawing\n");
            M3D_SetState(mycontext, M3D_FAST, M3D_ENABLE);
            printf("- Activate Z buffer\n");
            M3D_SetState(mycontext, M3D_ZBUFFER, M3D_ENABLE);
            if (M3D_AllocZBuffer(mycontext) == M3D_SUCCESS) {
              printf("- Z buffer available\n");
            }
            printf("- Activate 16bits output\n");
            M3D_SetState(mycontext, M3D_16BITS, M3D_ENABLE);
            printf("- Enable texture mapping\n");
            M3D_SetState(mycontext, M3D_TEXMAPPING, M3D_ENABLE);
            printf("- Enable gouraud shading\n");
            M3D_SetState(mycontext, M3D_GOURAUD, M3D_ENABLE);
            mytexture = M3D_AllocTextureFile(mycontext, &error, "texture.bmp");
            if (mytexture != NULL) {
              printf("- Texture loaded & allocated\n");
              Triangle.texture = mytexture;
              FT_triangle.texture = mytexture;
              FB_triangle.texture = mytexture;
              G1_triangle.texture = mytexture;
              G2_triangle.texture = mytexture;
              TL_triangle.texture = mytexture;
              BR_triangle.texture = mytexture;
              AR_triangles[0].texture = mytexture;
              AR_triangles[1].texture = mytexture;
              AR_triangles[2].texture = mytexture;
              AR_triangles[3].texture = mytexture;
              if (M3D_LockHardware(mycontext) == M3D_SUCCESS) {
                printf("- Hardware is now locked\n");
                M3D_SetDrawRegion(mycontext, CyberScreen->RastPort.BitMap, NULL);
                M3D_ClearZBuffer(mycontext);
                //draw_debug = TRUE;
                //printf("- Test texture result %d\n", M3D_DrawTest(mycontext, mytexture));
                /*printf("- Draw triangle\n");
                M3D_SetState(mycontext, M3D_TEXNORMCRD, M3D_ENABLE);
                M3D_DrawTriangle(mycontext, &Triangle);*/
                /*printf("- Draw flat top\n");
                M3D_DrawTriangle(mycontext, &FT_triangle);
                printf("- Draw flat bottom\n");
                M3D_DrawTriangle(mycontext, &FB_triangle);
                printf("- Draw generic 1\n");
                M3D_DrawTriangle(mycontext, &G1_triangle);
                printf("- Draw generic 2\n");
                M3D_DrawTriangle(mycontext, &G2_triangle);*/
                printf("- Draw triangles array\n");
                M3D_DrawTriangleArray(mycontext, AR_triangles, 4);
                /*printf("- Draw triangles list\n");
                M3D_DrawTriangleList(mycontext, LS_triangles, 4);*/
                /*printf("- Draw combined triangles\n");
                M3D_DrawTriangle(mycontext, &TL_triangle);
                M3D_DrawTriangle(mycontext, &BR_triangle);*/
                M3D_UnlockHardware(mycontext);
              }
              M3D_FreeTexture(mycontext, mytexture);
            } else {
              printf("- Failed to load or allocate texture with error %d\n", error);
            }
            M3D_FreeZBuffer(mycontext);
            M3D_DestroyContext(mycontext);
          } else {
            printf("- Failed to create context with error %d\n", error);
          }

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
                      printf("Key pressed %d\n", IntMsg->Code);
                    }
                  }
                  break;
                default:
                  printf("Uncatched event %d - %d\n", IntMsg->Class, IntMsg->Code);
                  break;
              }
              ReplyMsg (&IntMsg->ExecMessage);
            }
          }

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
