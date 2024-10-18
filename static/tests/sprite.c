/**
 * Magie3D static library
 *
 * Simple sprite test
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
#define F1_KEY                80
#define F2_KEY                81
#define F3_KEY                82
#define F4_KEY                83
#define F5_KEY                84
#define F6_KEY                85
#define UP_KEY                76
#define DOWN_KEY              77
#define LEFT_KEY              79
#define RIGHT_KEY             78
#define SPACE_KEY             64
#define D_KEY                 34
#define F_KEY                 35
#define H_KEY                 37
#define V_KEY                 52

/** @var Intuition library */
struct IntuitionBase *IntuitionBase = NULL;

/** @var CybergraphX library */
struct Library *CyberGfxBase = NULL;

/** Maggie3D context */
M3D_Context *mycontext = NULL;
/** Maggie texture data */
M3D_Texture *mytexture = NULL;

// Sprite
M3D_Sprite Sprite = {
  0, 0, 176, 254,           // Left, top, width, height
  1.0, 1.0, 0.0,            // X zoom, Y zoom, angle
  FALSE, FALSE,             // X flip, Y flip
  NULL,                     // Texture
  1.0,                      // Light intensity
  0xffffff                  // Color
};

struct TagItem Texture[] = {
  M3D_TT_FILENAME, (ULONG) "warrior.bmp",
  M3D_TT_AUTORESIZE, TRUE,
  M3D_TT_TRANSPARENCY, TRUE,
  M3D_TT_TRSCOLOR, 0xff00ff
};

LONG spr_posx = 232, spr_posy = 112;

// Debug
extern BOOL draw_debug;

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
    mytexture = M3D_AllocTextureTagList(mycontext, &error, Texture);
    if (mytexture == NULL) {
      return FALSE;
    }
    printf("- Texture loaded & allocated\n");
    Sprite.texture = mytexture;
    return TRUE;
  }
  return FALSE;
}

VOID RestoreDemo(VOID)
{
  printf("- Release texture\n");
  M3D_FreeTexture(mycontext, mytexture);
  printf("- Release context\n");
  M3D_DestroyContext(mycontext);
}

VOID AnimateDemo(struct BitMap *bitmap)
{
  M3D_SetDrawRegion(mycontext, bitmap, NULL);
  if (M3D_LockHardware(mycontext) == M3D_SUCCESS) {
    M3D_ClearDrawRegion(mycontext, 0xff80ff);
    M3D_DrawSprite(mycontext, &Sprite, spr_posx, spr_posy);
    M3D_UnlockHardware(mycontext);
  } else {
    printf("Hardware lock failed !\n");
  }
}

VOID CheckKey(UBYTE key)
{
  draw_debug = FALSE;
  switch(key) {
    case D_KEY:
      draw_debug = TRUE;
      break;
    case SPACE_KEY:
      spr_posx = 232;
      spr_posy = 112;
      Sprite.x_zoom = 1.0;
      Sprite.y_zoom = 1.0;
      Sprite.angle = 0.0;
      Sprite.x_flip = FALSE;
      Sprite.y_flip = FALSE;
      break;
    case F1_KEY:
      Sprite.x_zoom = 0.5;
      Sprite.y_zoom = 0.5;
      break;
    case F2_KEY:
      Sprite.x_zoom = 1.0;
      Sprite.y_zoom = 1.0;
      break;
    case F3_KEY:
      Sprite.x_zoom = 1.5;
      Sprite.y_zoom = 1.5;
      break;
    case F4_KEY:
      Sprite.x_zoom = 2.0;
      Sprite.y_zoom = 2.0;
      break;
    case F5_KEY:
      Sprite.angle -= 1.0;
      break;
    case F6_KEY:
      Sprite.angle += 1.0;
      break;
    case F_KEY:
      if (M3D_GetState(mycontext, M3D_FILTERING) == M3D_DISABLE) {
        M3D_SetState(mycontext, M3D_FILTERING, M3D_ENABLE);
      } else {
        M3D_SetState(mycontext, M3D_FILTERING, M3D_DISABLE);
      }
      break;
    case H_KEY:
      if (Sprite.x_flip) {
        Sprite.x_flip = FALSE;
      } else {
        Sprite.x_flip = TRUE;
      }
      break;
    case V_KEY:
      if (Sprite.y_flip) {
        Sprite.y_flip = FALSE;
      } else {
        Sprite.y_flip = TRUE;
      }
      break;
    case UP_KEY:
      spr_posy -= 2;
      break;
    case DOWN_KEY:
      spr_posy += 2;
      break;
    case LEFT_KEY:
      spr_posx -= 2;
      break;
    case RIGHT_KEY:
      spr_posx += 2;
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

  printf("*********************************\n");
  printf("**    Maggie 3D sprite test    **\n");
  printf("**    Quit with escape key     **\n");
  printf("*********************************\n");
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
