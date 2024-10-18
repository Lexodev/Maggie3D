/**
 * Magie3D static library
 *
 * Simple quad test
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.6 June 2024
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

/** Maggie3D context */
M3D_Context *mycontext = NULL;
/** Texture data */
M3D_Texture *mytexture = NULL;
/** Active quad */
M3D_Quad *myquad = NULL;
/** Scissor */
M3D_Scissor myscissor = { 20, 20, 600, 440 };

// Debug
extern BOOL draw_debug;

// Quad generic losange
M3D_Quad GN_quad1 = {
  350.0, 50.0, 100.0, 0.0, 0.0, 0.0, 1.0,
  600.0, 150.0, 100.0, 0.0, 1.0, 0.0, 0.6,
  330.0, 280.0, 100.0, 0.0, 1.0, 1.0, 0.5,
  100.0, 150.0, 100.0, 0.0, 0.0, 1.0, 0.8,
  NULL,
  0xffff00
};

// Quad generic V2 < V4 et V4 < V3
M3D_Quad GN_quad2 = {
  350.0, 50.0, 110.0, 0.0, 0.0, 0.0, 1.0,
  500.0, 150.0, 50.0, 0.0, 1.0, 0.0, 0.6,
  330.0, 380.0, 10.0, 0.0, 1.0, 1.0, 0.5,
  160.0, 250.0, 10.0, 0.0, 0.0, 1.0, 0.8,
  NULL,
  0x00ffff
};

// Quad generic V2 < V4 et V4 > V3
M3D_Quad GN_quad3 = {
  350.0, 50.0, 110.0, 0.0, 0.0, 0.0, 1.0,
  500.0, 150.0, 50.0, 0.0, 1.0, 0.0, 0.6,
  330.0, 280.0, 10.0, 0.0, 1.0, 1.0, 0.5,
  170.0, 380.0, 10.0, 0.0, 0.0, 1.0, 0.8,
  NULL,
  0xff00ff
};

// Quad generic V2 > V4 et V3 > V2
M3D_Quad GN_quad4 = {
  350.0, 50.0, 110.0, 0.0, 0.0, 0.0, 1.0,
  500.0, 250.0, 50.0, 0.0, 1.0, 0.0, 0.6,
  330.0, 380.0, 10.0, 0.0, 1.0, 1.0, 0.5,
  160.0, 150.0, 10.0, 0.0, 0.0, 1.0, 0.8,
  NULL,
  0x0000ff
};

// Quad generic V2 > V4 et V3 < V2
M3D_Quad GN_quad5 = {
  350.0, 50.0, 110.0, 0.0, 0.0, 0.0, 1.0,
  500.0, 380.0, 50.0, 0.0, 1.0, 0.0, 0.6,
  220.0, 250.0, 10.0, 0.0, 1.0, 1.0, 0.5,
  160.0, 150.0, 10.0, 0.0, 0.0, 1.0, 0.8,
  NULL,
  0xffffff
};

// Flat top
M3D_Quad FT_quad1 = {
  200.0, 180.0, 100.0, 0.0, 0.0, 1.0, 0.6,
  110.0, 70.0, 100.0, 0.0, 0.0, 0.0, 0.5,
  560.0, 70.0, 100.0, 0.0, 1.0, 0.0, 0.2,
  440.0, 260.0, 100.0, 0.0, 1.0, 1.0, 1.0,
  NULL,
  0xff0000
};

// Flat top
M3D_Quad FT_quad2 = {
  200.0, 260.0, 100.0, 0.0, 0.0, 1.0, 1.0,
  110.0, 70.0, 100.0, 0.0, 0.0, 0.0, 0.5,
  560.0, 70.0, 100.0, 0.0, 1.0, 0.0, 0.2,
  440.0, 180.0, 100.0, 0.0, 1.0, 1.0, 0.6,
  NULL,
  0xff00ff
};

// Flat bottom
M3D_Quad FB_quad1 = {
  250.0, 50.0, 110.0, 0.0, 0.0, 0.0, 1.0,
  460.0, 210.0, 50.0, 0.0, 1.0, 0.0, 0.8,
  380.0, 330.0, 10.0, 0.0, 1.0, 1.0, 0.6,
  140.0, 330.0, 210.0, 0.0, 0.0, 1.0, 0.6,
  NULL,
  0x00ff00
};

// Flat bottom
M3D_Quad FB_quad2 = {
  250.0, 50.0, 110.0, 0.0, 0.0, 0.0, 1.0,
  460.0, 330.0, 50.0, 0.0, 1.0, 0.0, 0.6,
  380.0, 330.0, 10.0, 0.0, 1.0, 1.0, 0.6,
  140.0, 210.0, 210.0, 0.0, 0.0, 1.0, 0.8,
  NULL,
  0xffff00
};

// Flat both
M3D_Quad FF_quad1 = {
  50.0, 40.0, 10.0, 0.0, 0.0, 0.0, 0.4,
  260.0, 40.0, 10.0, 0.0, 1.0, 0.0, 0.8,
  460.0, 210.0, 10.0, 0.0, 1.0, 1.0, 1.0,
  200.0, 210.0, 10.0, 0.0, 0.0, 1.0, 0.7,
  NULL,
  0x0000ff
};

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
    printf("- Enable gouraud shading\n");
    M3D_SetState(mycontext, M3D_GOURAUD, M3D_DISABLE);
    printf("- Enable texture mapping\n");
    M3D_SetState(mycontext, M3D_TEXMAPPING, M3D_DISABLE);
    printf("- Enable color blending\n");
    M3D_SetState(mycontext, M3D_BLENDING, M3D_DISABLE);
    printf("- Enable normalized texture coordinates\n");
    M3D_SetState(mycontext, M3D_TEXCRDNORM, M3D_ENABLE);
    mytexture = M3D_AllocTextureFile(mycontext, &error, "texture.bmp");
    if (mytexture == NULL) {
      return FALSE;
    }
    printf("- BMP texture loaded & allocated\n");
    GN_quad1.texture = mytexture;
    GN_quad2.texture = mytexture;
    GN_quad3.texture = mytexture;
    GN_quad4.texture = mytexture;
    GN_quad5.texture = mytexture;
    FT_quad1.texture = mytexture;
    FT_quad2.texture = mytexture;
    FB_quad1.texture = mytexture;
    FB_quad2.texture = mytexture;
    FF_quad1.texture = mytexture;
    myquad = &GN_quad1;
    return TRUE;
  }
  return FALSE;
}

VOID RestoreDemo(VOID)
{
  printf("- Release textures\n");
  M3D_FreeTexture(mycontext, mytexture);
  printf("- Release Z buffer\n");
  M3D_FreeZBuffer(mycontext);
  printf("- Release context\n");
  M3D_DestroyContext(mycontext);
}

VOID AnimateDemo(struct BitMap *bitmap)
{
  M3D_SetDrawRegion(mycontext, bitmap, &myscissor);
  M3D_ClearZBuffer(mycontext);
  if (M3D_LockHardware(mycontext) == M3D_SUCCESS) {
    M3D_ClearDrawRegion(mycontext, 0xff80ff);
    M3D_DrawQuad(mycontext, myquad);
    M3D_UnlockHardware(mycontext);
  } else {
    printf("Hardware lock failed !\n");
  }
}

VOID UpdateQuad(FLOAT dx, FLOAT dy)
{
  myquad->v1.x += dx;
  myquad->v1.y += dy;
  myquad->v2.x += dx;
  myquad->v2.y += dy;
  myquad->v3.x += dx;
  myquad->v3.y += dy;
  myquad->v4.x += dx;
  myquad->v4.y += dy;
}

VOID CheckKey(UBYTE key)
{
  switch(key) {
    case F1_KEY:
      myquad = &GN_quad1;
      break;
    case F2_KEY:
      myquad = &GN_quad2;
      break;
    case F3_KEY:
      myquad = &GN_quad3;
      break;
    case F4_KEY:
      myquad = &GN_quad4;
      break;
    case F5_KEY:
      myquad = &GN_quad5;
      break;
    case F6_KEY:
      myquad = &FT_quad1;
      break;
    case F7_KEY:
      myquad = &FT_quad2;
      break;
    case F8_KEY:
      myquad = &FB_quad1;
      break;
    case F9_KEY:
      myquad = &FB_quad2;
      break;
    case F10_KEY:
      myquad = &FF_quad1;
      break;
    case D_KEY:
      M3D_SetState(NULL, 0xFFFF, M3D_ENABLE);       // Available only on debug
      break;
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
    case G_KEY:
      if (M3D_GetState(mycontext, M3D_GOURAUD) == M3D_DISABLE) {
        M3D_SetState(mycontext, M3D_GOURAUD, M3D_ENABLE);
      } else {
        M3D_SetState(mycontext, M3D_GOURAUD, M3D_DISABLE);
      }
      break;
    case F_KEY:
      if (filtering == M3D_NEAREST) {
        filtering = M3D_LINEAR;
      } else {
        filtering = M3D_NEAREST;
      }
      M3D_SetFilter(mycontext, mytexture, filtering);
      break;
    case UP_KEY:
      UpdateQuad((FLOAT)0.0, (FLOAT)-10.0);
      break;
    case DOWN_KEY:
      UpdateQuad((FLOAT)0.0, (FLOAT)10.0);
      break;
    case LEFT_KEY:
      UpdateQuad((FLOAT)-10.0, (FLOAT)0.0);
      break;
    case RIGHT_KEY:
      UpdateQuad((FLOAT)10.0, (FLOAT)0.0);
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
  LONG error;
  BOOL Done;

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
