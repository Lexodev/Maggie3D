/**
 * Magie3D shared library
 *
 * Simple test
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.0 April 2024
 */

#include <exec/exec.h>
#include <intuition/intuitionbase.h>
#include <cybergraphx/cybergraphics.h>

#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/cybergraphics.h>

#include <pragmas/Maggie3D_pragmas.h>
#include <clib/Maggie3D_protos.h>
#include <Maggie3D/Maggie3D.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define INTUITIONVERSION      39L
#define CYBERGFXVERSION       41L

#define SCREEN_WANTED_WIDTH   640L
#define SCREEN_WANTED_HEIGHT  480L
#define SCREEN_WANTED_DEPTH   16L

#define WINDOW_WIDTH          SCREEN_WANTED_WIDTH
#define WINDOW_HEIGHT         SCREEN_WANTED_HEIGHT

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
#define Z_KEY                 17
#define T_KEY                 20
#define D_KEY                 34
#define F_KEY                 35
#define G_KEY                 36
#define B_KEY                 53

/** @var Intuition library */
struct IntuitionBase *IntuitionBase = NULL;

/** @var CybergraphX library */
struct Library *CyberGfxBase = NULL;

/** @var Maggie3D library */
struct Library *Maggie3DBase = NULL;

/** Maggie3D context */
M3D_Context *mycontext = NULL;
/** Maggie texture data */
M3D_Texture *mytexture1 = NULL, *mytexture2 = NULL;

// Left Top 1
M3D_Triangle LT1_triangle = {
  10.0, 10.0, 10.0, 0.0, 0.0, 0.0, 1.0,
  266.0, 10.0, 10.0, 0.0, 255.0, 0.0, 0.75,
  10.0, 266.0, 10.0, 0.0, 0.0, 255.0, 0.5,
  NULL,
  0xffffff
};

// Right bottom 1
M3D_Triangle RB1_triangle = {
  270.0, 10.0, 10.0, 0.0, 255.0, 0.0, 0.3,
  14.0, 266.0, 10.0, 0.0, 0.0, 255.0, 1.0,
  270.0, 266.0, 10.0, 0.0, 255.0, 255.0, 0.8,
  NULL,
  0xff0000
};

// Left top 2
M3D_Triangle LT2_triangle = {
  300.0, 10.0, 10.0, 0.0, 0.0, 0.0, 0.60,
  556.0, 10.0, 10.0, 0.0, 255.0, 0.0, 0.80,
  300.0, 266.0, 10.0, 0.0, 0.0, 255.0, 1.0,
  NULL,
  0x0000ff
};

// Right bottom 2
M3D_Triangle RB2_triangle = {
  560.0, 10.0, 10.0, 0.0, 255.0, 0.0, 0.33,
  304.0, 266.0, 10.0, 0.0, 0.0, 255.0, 0.66,
  560.0, 266.0, 10.0, 0.0, 255.0, 255.0, 1.0,
  NULL,
  0x00ff00
};

// List of triangles
M3D_Triangle *Triangles[] = {&LT1_triangle, &RB1_triangle, &LT2_triangle, &RB2_triangle};

UWORD filtering = M3D_NEAREST;

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
  if ((Maggie3DBase = OpenLibrary(MAGGIE3DNAME, MAGGIE3DVERSION)) == NULL) {
    printf("ERROR: can't open Maggie3D.library V%d\n", MAGGIE3DVERSION);
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
  if (Maggie3DBase != NULL) {
    CloseLibrary(Maggie3DBase);
  }
  if (CyberGfxBase != NULL) {
    CloseLibrary(CyberGfxBase);
  }
  if (IntuitionBase != NULL) {
    CloseLibrary(&IntuitionBase->LibNode);
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

BOOL InitDemo(struct BitMap *bitmap)
{
  LONG error;

  if (M3D_CheckMaggie()) {
    printf("* Maggie support detected !\n");
  }
  printf("- Create Maggie context\n");
  mycontext = M3D_CreateContext(&error, bitmap);
  if (mycontext != NULL) {
    printf("- Allocate Z buffer\n");
    if (M3D_AllocZBuffer(mycontext) == M3D_SUCCESS) {
      printf("- Enable Z buffer\n");
      M3D_SetState(mycontext, M3D_ZBUFFER, M3D_ENABLE);
    }
    mytexture1 = M3D_AllocTextureFile(mycontext, &error, "texture.dds");
    if (mytexture1 == NULL) {
      return FALSE;
    }
    printf("- DDS texture loaded & allocated\n");
    LT1_triangle.texture = mytexture1;
    RB2_triangle.texture = mytexture1;
    mytexture2 = M3D_AllocTextureFile(mycontext, &error, "texture.bmp");
    if (mytexture2 == NULL) {
      return FALSE;
    }
    printf("- BMP texture loaded & allocated\n");
    LT2_triangle.texture = mytexture2;
    RB1_triangle.texture = mytexture2;
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
  M3D_SetDrawRegion(mycontext, bitmap, NULL);
  if (M3D_LockHardware(mycontext) == M3D_SUCCESS) {
    M3D_ClearDrawRegion(mycontext, 0xff80ff);
    M3D_ClearZBuffer(mycontext);
    M3D_DrawTriangleList(mycontext, Triangles, 4);
    M3D_UnlockHardware(mycontext);
  } else {
    printf("Hardware lock failed !\n");
  }
}

VOID CheckKey(UBYTE key)
{
  switch(key) {
    case B_KEY:
      if (M3D_GetState(mycontext, M3D_BLENDING) == M3D_DISABLE) {
        M3D_SetState(mycontext, M3D_BLENDING, M3D_ENABLE);
      } else {
        M3D_SetState(mycontext, M3D_BLENDING, M3D_DISABLE);
      }
      break;
    case T_KEY:
      if (M3D_GetState(mycontext, M3D_TEXMAPPING) == M3D_DISABLE) {
        M3D_SetState(mycontext, M3D_TEXMAPPING, M3D_ENABLE);
      } else {
        M3D_SetState(mycontext, M3D_TEXMAPPING, M3D_DISABLE);
      }
      break;
    case F_KEY:
      if (filtering == M3D_LINEAR) {
        filtering = M3D_NEAREST;
      } else {
        filtering = M3D_LINEAR;
      }
      M3D_SetFilter(mycontext, mytexture1, filtering);
      M3D_SetFilter(mycontext, mytexture2, filtering);
      break;
    case G_KEY:
      if (M3D_GetState(mycontext, M3D_GOURAUD) == M3D_DISABLE) {
        M3D_SetState(mycontext, M3D_GOURAUD, M3D_ENABLE);
      } else {
        M3D_SetState(mycontext, M3D_GOURAUD, M3D_DISABLE);
      }
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
  STRPTR text = "T - Texmap on/off  G - Gourad on/off  B - Blending on/off  F - Filtering on/off  ESC - Quit";

  printf("**************************\n");
  printf("**    Maggie 3D test    **\n");
  printf("** Quit with escape key **\n");
  printf("**************************\n");
  if (OpenAllLibs()) {
    
    printf("Search best mode for a screen of %dx%dx%d\n", SCREEN_WANTED_WIDTH, SCREEN_WANTED_HEIGHT, SCREEN_WANTED_DEPTH);
    
    // Find best screen mode
    DisplayID = M3D_BestModeID(SCREEN_WANTED_WIDTH, SCREEN_WANTED_HEIGHT, SCREEN_WANTED_DEPTH);
    if (DisplayID != INVALID_ID) {
      printf("Found a suitable screenmode with ID 0x%X\n", DisplayID);
      if (IsCyberModeID(DisplayID)) {
        printf("This is a CyberGfx mode ID\n");
        printf(" Width is %d\n", GetCyberIDAttr(CYBRIDATTR_WIDTH, DisplayID));
        printf(" Height is %d\n", GetCyberIDAttr(CYBRIDATTR_HEIGHT, DisplayID));
        printf(" Depth is %d\n", GetCyberIDAttr(CYBRIDATTR_DEPTH, DisplayID));
        printf(" Bytes per pixel is %d\n", GetCyberIDAttr(CYBRIDATTR_BPPIX, DisplayID));
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
          
          if (InitDemo(CyberScreen->RastPort.BitMap)) {
            AnimateDemo(CyberScreen->RastPort.BitMap);
            Move(&(CyberScreen->RastPort), 2, 300);
            Text(&(CyberScreen->RastPort), text, strlen(text));
            
            // Main loop
            printf("* Enter the main loop *\n");
            Done = FALSE;
            while (!Done) {
              M3D_SetState(NULL, 0xFFFF, M3D_DISABLE);       // Available only on debug
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
              Move(&(CyberScreen->RastPort), 2, 300);
              Text(&(CyberScreen->RastPort), text, strlen(text));
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
