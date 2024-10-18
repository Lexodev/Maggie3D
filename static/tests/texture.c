/**
 * Magie3D static library
 *
 * Simple texture test
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

#define WINDOW_WIDTH          SCREEN_WANTED_WIDTH
#define WINDOW_HEIGHT         SCREEN_WANTED_HEIGHT

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

// Triangle
M3D_Triangle Triangle1 = {
  10.0, 10.0, 10.0, 0.0, 0.0, 0.0, 1.0,
  350.0, 10.0, 10.0, 0.0, 1.0, 0.0, 1.0,
  10.0, 350.0, 10.0, 0.0, 0.0, 1.0, 1.0,
  NULL,
  0xffffff
};

M3D_Triangle Triangle2 = {
  350.0, 10.0, 10.0, 0.0, 1.0, 0.0, 1.0,
  350.0, 350.0, 10.0, 0.0, 1.0, 1.0, 1.0,
  10.0, 350.0, 10.0, 0.0, 0.0, 1.0, 1.0,
  NULL,
  0xffffff
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

  printf("**********************************\n");
  printf("**    Maggie 3D texture test    **\n");
  printf("**     Quit with escape key     **\n");
  printf("**********************************\n");
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

          if (M3D_CheckMaggie()) {
            printf("* Maggie support detected !\n");
          }
          mycontext = M3D_CreateContext(&error, CyberScreen->RastPort.BitMap);
          if (mycontext != NULL) {
            printf("- Maggie context created\n");
            printf("- Activate fast drawing\n");
            M3D_SetState(mycontext, M3D_FAST, M3D_ENABLE);
            printf("- Allocate Z buffer\n");
            if (M3D_AllocZBuffer(mycontext) == M3D_SUCCESS) {
              printf("- Activate Z buffer\n");
              M3D_SetState(mycontext, M3D_ZBUFFER, M3D_ENABLE);
            }
            printf("- Enable texture mapping\n");
            M3D_SetState(mycontext, M3D_TEXMAPPING, M3D_ENABLE);
            printf("- Enable gouraud shading\n");
            M3D_SetState(mycontext, M3D_GOURAUD, M3D_ENABLE);
            printf("- Enable texture normalized\n");
            M3D_SetState(mycontext, M3D_TEXCRDNORM, M3D_ENABLE);
            mytexture = M3D_AllocTextureFile(mycontext, &error, "texture.bmp");
            if (mytexture != NULL) {
              printf("- Texture loaded & allocated\n");
              Triangle1.texture = mytexture;
              Triangle2.texture = mytexture;
              M3D_SetDrawRegion(mycontext, CyberScreen->RastPort.BitMap, NULL);
              M3D_ClearZBuffer(mycontext);
              if (M3D_LockHardware(mycontext) == M3D_SUCCESS) {
                printf("- Hardware is now locked\n");
                M3D_ClearDrawRegion(mycontext, 0xff80ff);
                printf("- Draw texture\n");
                M3D_DrawTriangle(mycontext, &Triangle1);
                M3D_DrawTriangle(mycontext, &Triangle2);
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
