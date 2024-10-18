/**
 * Magie3D shared library
 *
 * Small test
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.2 June 2024
 */

// Compile with : SC LINK CPU=68040 MATH=68882 maggie3d_test.c

#include <exec/exec.h>
#include <intuition/intuitionbase.h>
#include <cybergraphx/cybergraphics.h>

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/cybergraphics.h>

#include <stdlib.h>
#include <stdio.h>

#include <pragmas/Maggie3D_pragmas.h>
#include <clib/Maggie3D_protos.h>
#include <Maggie3D/Maggie3D.h>

#define INTUITIONVERSION      39L
#define CYBERGFXVERSION       41L

#define SCREEN_WANTED_WIDTH   640L
#define SCREEN_WANTED_HEIGHT  480L
#define SCREEN_WANTED_DEPTH   24L

#define WINDOW_WIDTH          SCREEN_WANTED_WIDTH
#define WINDOW_HEIGHT         SCREEN_WANTED_HEIGHT

#define SIGMASK(w) (1L<<((w)->UserPort->mp_SigBit))
#define GETIMSG(w) ((struct IntuiMessage *)GetMsg((w)->UserPort))

#define ESCAPE_KEY            69

/** @var Intuition library */
struct IntuitionBase *IntuitionBase = NULL;

/** @var CybergraphX library */
struct Library *CyberGfxBase = NULL;

/** @var Maggie3D library */
struct Library *Maggie3DBase = NULL;

/** Maggie3D context */
M3D_Context *mycontext = NULL;
/** Maggie3D texture data */
M3D_Texture *mytexture_dds = NULL, *mytexture_bmp = NULL, *mytexture_spr = NULL;
/** Maggie3D scissor */
M3D_Scissor myscissor = { 100, 50, 440, 380 };

// Triangle
M3D_Triangle Triangle = {
  252.902466, 147.134598, 40.721924, 0.0, 0.0, 0.0, 1.0,
  390.396179, 140.904434, 39.326794, 0.0, 255.0, 0.0, 0.75,
  380.396179, 305.095581, 39.326794, 0.0, 255.0, 255.0, 0.5,
  NULL,
  0xffffff
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

// List of triangles
M3D_Triangle *LS_triangles[] = {&FT_triangle, &FB_triangle, &G1_triangle, &G2_triangle};

// Quad
M3D_Quad Quad = {
  250.0, 40.0, 42.0, 0.0, 0.0, 0.0, 1.0,
  370.0, 310.0, 42.0, 0.0, 250.0, 0.0, 0.5,
  260.0, 410.0, 42.0, 0.0, 250.0, 250.0, 0.5,
  130.0, 130.0, 42.0, 0.0, 0.0, 250.0, 0.8,
  NULL,
  0xffffff
};

// Sprite
M3D_Sprite Sprite = {
  0, 0, 176, 254,           // Left, top, width, height
  1.0, 1.2, 40.0,           // X zoom, Y zoom, angle
  FALSE, FALSE,             // X flip, Y flip
  NULL,                     // Texture
  1.0,                      // Light intensity
  0xffffff                  // Color
};

struct TagItem TexTags[] = {
  M3D_TT_FILENAME, (ULONG) "warrior.bmp",
  M3D_TT_AUTORESIZE, TRUE,
  M3D_TT_TRANSPARENCY, TRUE,
  M3D_TT_TRSCOLOR, 0xff00ff
};

/**
 * Open libraries
 */
BOOL OpenAllLibs(VOID)
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
VOID CloseAllLibs(VOID)
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

/**
 * Main function
 */
int main(int argc, char **argv)
{
  struct Screen *CyberScreen = NULL;
  struct Window *CyberWindow = NULL;
  struct IntuiMessage *IntMsg = NULL;
  ULONG DisplayID;
  LONG error;
  BOOL Done;

  printf("********************************\n");
  printf("**    Maggie 3D small demo    **\n");
  printf("**    Quit with escape key    **\n");
  printf("********************************\n");
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

          /**********************************************/
          /**     Start of Maggie3D stuff               */
          /**********************************************/
          if (M3D_CheckMaggie()) {
            printf("! Maggie support detected !\n");
          }
          printf("- Create Maggie context\n");
          mycontext = M3D_CreateContext(&error, CyberScreen->RastPort.BitMap);
          if (mycontext != NULL) {
            printf("- Allocate Z buffer\n");
            if (M3D_AllocZBuffer(mycontext) == M3D_SUCCESS) {
              printf("- Activate Z buffer\n");
              M3D_SetState(mycontext, M3D_ZBUFFER, M3D_ENABLE);
            }
            printf("- Activate color blending\n");
            M3D_SetState(mycontext, M3D_BLENDING, M3D_ENABLE);
            printf("- Allocate textures\n");
            mytexture_dds = M3D_AllocTextureFile(mycontext, &error, "texture.dds");
            mytexture_bmp = M3D_AllocTextureFile(mycontext, &error, "texture.bmp");
            mytexture_spr = M3D_AllocTextureTagList(mycontext, &error, TexTags);
            if (mytexture_dds != NULL && mytexture_bmp != NULL && mytexture_spr != NULL) {
              printf("- Textures loaded & allocated\n");
              Triangle.texture = mytexture_dds;
              Quad.texture = mytexture_bmp;
              FT_triangle.texture = mytexture_dds;
              FB_triangle.texture = mytexture_bmp;
              G1_triangle.texture = mytexture_dds;
              G2_triangle.texture = mytexture_bmp;
              Sprite.texture = mytexture_spr;
              M3D_SetDrawRegion(mycontext, CyberScreen->RastPort.BitMap, &myscissor);
              if (M3D_LockHardware(mycontext) == M3D_SUCCESS) {
                M3D_ClearZBuffer(mycontext);
                M3D_ClearDrawRegion(mycontext, 0xfcfc60);
                printf("- Disable texture mapping\n");
                M3D_SetState(mycontext, M3D_TEXMAPPING, M3D_DISABLE);
                printf("- Draw triangle\n");
                M3D_DrawTriangle(mycontext, &Triangle);
                printf("- Activate texture mapping\n");
                M3D_SetState(mycontext, M3D_TEXMAPPING, M3D_ENABLE);
                printf("- Draw triangles list\n");
                M3D_DrawTriangleList(mycontext, LS_triangles, 4);
                printf("- Draw a quad\n");
                M3D_DrawQuad(mycontext, &Quad);
                printf("- Draw a sprite\n");
                M3D_DrawSprite(mycontext, &Sprite, 180, 80);
                M3D_UnlockHardware(mycontext);
              }
              M3D_FreeTexture(mycontext, mytexture_spr);
              M3D_FreeTexture(mycontext, mytexture_dds);
              M3D_FreeTexture(mycontext, mytexture_bmp);
            } else {
              printf("- Failed to load or allocate texture with error %d\n", error);
            }
            M3D_FreeZBuffer(mycontext);
            M3D_DestroyContext(mycontext);
          } else {
            printf("- Failed to create context with error %d\n", error);
          }
          /**********************************************/
          /**     End of Maggie3D stuff                 */
          /**********************************************/

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
