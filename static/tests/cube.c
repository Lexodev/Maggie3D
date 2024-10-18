/**
 * Magie3D static library
 *
 * Cube test
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.2 June 2024
 */

#include <exec/exec.h>
#include <intuition/intuitionbase.h>
#include <cybergraphx/cybergraphics.h>

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/cybergraphics.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "/src/Maggie3D.h"

#define INTUITIONVERSION      39L
#define CYBERGFXVERSION       41L

#define SCREEN_WANTED_WIDTH   640L
#define SCREEN_WANTED_HEIGHT  480L
#define SCREEN_WANTED_DEPTH   32L

#define WINDOW_WIDTH          SCREEN_WANTED_WIDTH
#define WINDOW_HEIGHT         SCREEN_WANTED_HEIGHT

#define SIGMASK(w) (1L<<((w)->UserPort->mp_SigBit))
#define GETIMSG(w) ((struct IntuiMessage *)GetMsg((w)->UserPort))

#define ESCAPE_KEY            69
#define UP_KEY                76
#define DOWN_KEY              77
#define LEFT_KEY              79
#define RIGHT_KEY             78
#define SPACE_KEY             64
#define T_KEY                 20
#define I_KEY                 23
#define O_KEY                 24
#define D_KEY                 34
#define F_KEY                 35
#define G_KEY                 36
#define B_KEY                 53

#define CUBE_POINTS           8
#define CUBE_FACES            6

struct cube_point {
  FLOAT x, y, z;
};

struct cube_face {
  ULONG p1, p2, p3, p4, color, texture;
  ULONG u1, v1, u2, v2, u3, v3, u4, v4;
};

struct cube_object {
  WORD anglex,angley,anglez;
  FLOAT posx, posy, posz;
  struct cube_point points[CUBE_POINTS];
  struct cube_face faces[CUBE_FACES];
};

// Demo Data
struct cube_object Cube = {
  0,0,0,
  0.0,0.0,50.0,
  {
    { -10.0,10.0,-10.0 },
    { 10.0,10.0,-10.0 },
    { 10.0,-10.0,-10.0 },
    { -10.0,-10.0,-10.0 },
    { -10.0,10.0,10.0 },
    { 10.0,10.0,10.0 },
    { 10.0,-10.0,10.0 },
    { -10.0,-10.0,10.0 }
  },
  {
    { 0,1,2,3, 0xffffff, 0, 0,0,255,0,255,255,0,255 },
    { 1,5,6,2, 0xff0000, 1, 0,0,255,0,255,255,0,255 },
    { 5,4,7,6, 0x00ff00, 0, 0,0,255,0,255,255,0,255 },
    { 4,0,3,7, 0x0000ff, 1, 0,0,255,0,255,255,0,255 },
    { 4,5,1,0, 0xff00ff, 2, 0,0,127,0,127,127,0,127 },
    { 3,2,6,7, 0xffff00, 3, 0,0,127,0,127,127,0,127 }
  }
};
struct cube_point transf[8];

// Engine
#define RAD(x)                ((x)*PI/180.0)
FLOAT Sinus[360];
FLOAT Cosinus[360];

// Libs
struct IntuitionBase *IntuitionBase = NULL;
struct Library *CyberGfxBase = NULL;

// Screen
struct Screen *CyberScreen = NULL;
struct Window *CyberWindow = NULL;
struct IntuiMessage *IntMsg = NULL;

// Maggie
M3D_Context *mycontext = NULL;
M3D_Texture *mytexture1 = NULL;
M3D_Texture *mytexture2 = NULL;
M3D_Texture *mytexture3 = NULL;
M3D_Texture *mytexture4 = NULL;
UWORD filtering = M3D_NEAREST;
LONG error;

/** Open libraries */
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

/** Close libraries */
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

BOOL OpenMyScreen(VOID)
{
  ULONG DisplayID;
  
  // Find best screen mode
  DisplayID = M3D_BestModeID(SCREEN_WANTED_WIDTH, SCREEN_WANTED_HEIGHT, SCREEN_WANTED_DEPTH);
  if (DisplayID == INVALID_ID) {
    printf("No screen available !\n");
    return FALSE;
  }
  printf("Found a suitable screenmode with ID 0x%X\n", DisplayID);
  if (!IsCyberModeID(DisplayID)) {
    printf("This is not a CyberGfx mode !\n");
    return FALSE;
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
  if (CyberScreen == NULL) {
    printf("Open screen error !\n");
    return FALSE;
  }
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
  if (CyberWindow == NULL) {
    printf("Open window error !\n");
  }
  return TRUE;
}

BOOL InitEngine(VOID)
{
  FLOAT angle;
  ULONG i;

  angle = 0.0;
  for (i = 0;i < 360;i++) {
    Sinus[i] = sin(RAD(angle));
    Cosinus[i] = cos(RAD(angle));
    angle += 1.0;
  }
  return TRUE;
}

BOOL InitMaggie(VOID)
{
  if (M3D_CheckMaggie()) {
    printf("* Maggie support detected !\n");
  }
  printf("- Create Maggie context\n");
  mycontext = M3D_CreateContext(&error, CyberScreen->RastPort.BitMap);
  if (mycontext == NULL) {
    printf("Create context error !\n");
    return FALSE;
  }
  printf("- Allocate Z buffer\n");
  if (M3D_AllocZBuffer(mycontext) == M3D_SUCCESS) {
    printf("- Activate Z buffer\n");
    M3D_SetState(mycontext, M3D_ZBUFFER, M3D_ENABLE);
  }
  printf("- Enable texture mapping\n");
  M3D_SetState(mycontext, M3D_TEXMAPPING, M3D_ENABLE);
  printf("- Enable gouraud shading\n");
  M3D_SetState(mycontext, M3D_GOURAUD, M3D_ENABLE);
  printf("- Enable color blending\n");
  M3D_SetState(mycontext, M3D_BLENDING, M3D_ENABLE);
  printf("- Enable fast mode\n");
  M3D_SetState(mycontext, M3D_FAST, M3D_ENABLE);
  return TRUE;
}

BOOL InitCube(VOID)
{
  return TRUE;
}

BOOL InitTexture(VOID)
{
  mytexture1 = M3D_AllocTextureFile(mycontext, &error, "texture.dds");
  if (mytexture1 == NULL) {
    return FALSE;
  }
  mytexture2 = M3D_AllocTextureFile(mycontext, &error, "texture.bmp");
  if (mytexture2 == NULL) {
    return FALSE;
  }
  mytexture3 = M3D_AllocTextureFile(mycontext, &error, "vamptex.dds");
  if (mytexture3 == NULL) {
    return FALSE;
  }
  mytexture4 = M3D_AllocTextureFile(mycontext, &error, "vamptex.bmp");
  if (mytexture4 == NULL) {
    return FALSE;
  }
  return TRUE;
}

BOOL _Init(VOID)
{
  printf("Init demo\n");
  if (!OpenMyScreen()) {
    return FALSE;
  }
  if (!InitEngine()) {
    return FALSE;
  }
  if (!InitMaggie()) {
    return FALSE;
  }
  if (!InitCube()) {
    return FALSE;
  }
  if (!InitTexture()) {
    return FALSE;
  }
  return TRUE;
}

VOID _Restore(VOID)
{
  printf("* Free textures *\n");
  M3D_FreeTexture(mycontext, mytexture1);
  M3D_FreeTexture(mycontext, mytexture2);
  M3D_FreeTexture(mycontext, mytexture3);
  M3D_FreeTexture(mycontext, mytexture4);
  printf("* Free ZBuffer *\n");
  M3D_FreeZBuffer(mycontext);
  printf("* Destroy context *\n");
  M3D_DestroyContext(mycontext);
  printf("* Close the window *\n");
  if (CyberWindow != NULL) {
    CloseWindow(CyberWindow);
  }
  printf("* Close the screen *\n");
  if (CyberScreen != NULL) {
    CloseScreen(CyberScreen);
  }
}

VOID _Update(UBYTE key)
{
  switch(key) {
    case SPACE_KEY:
      Cube.anglex = 0.0;
      Cube.angley = 0.0;
      Cube.anglez = 0.0;
      Cube.posx = 0.0;
      Cube.posy = 0.0;
      Cube.posz = 50.0;
      break;
    case UP_KEY:
      Cube.anglex += 4;
      if (Cube.anglex >= 360) Cube.anglex -= 360;
      break;
    case DOWN_KEY:
      Cube.anglex -= 4;
      if (Cube.anglex < 0) Cube.anglex += 360;
      break;
    case LEFT_KEY:
      Cube.angley += 4;
      if (Cube.angley >= 360) Cube.angley -= 360;
      break;
    case RIGHT_KEY:
      Cube.angley -= 4;
      if (Cube.angley < 0) Cube.angley += 360;
      break;
    case I_KEY:
      Cube.posz -= 2;
      if (Cube.posz <= 20.0) Cube.posz = 20.0;
      break;
    case O_KEY:
      Cube.posz += 2;
      if (Cube.posz >= 100.0) Cube.posz = 100.0;
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
      M3D_SetFilter(mycontext, mytexture1, filtering);
      M3D_SetFilter(mycontext, mytexture2, filtering);
      break;
  }
}

/** Draw the cube */
VOID TransformPoints(VOID)
{
  FLOAT x,y,z,xa,ya,za,xb,yb,zb;
  LONG idx;

  for (idx = 0;idx < CUBE_POINTS;idx++) {
    xa = Cube.points[idx].x;
    ya = Cube.points[idx].y;
    za = Cube.points[idx].z;
    // Rotate X
    yb = ya * Cosinus[Cube.anglex] - za * Sinus[Cube.anglex];
    zb = ya * Sinus[Cube.anglex] + za * Cosinus[Cube.anglex];
    // Rotate Y
    xb = xa * Cosinus[Cube.angley] + zb * Sinus[Cube.angley];
    z = -xa * Sinus[Cube.angley] + zb * Cosinus[Cube.angley];
    // Rotate Z
    x = xb * Cosinus[Cube.anglez] - yb * Sinus[Cube.anglez];
    y = xb * Sinus[Cube.anglez] + yb * Cosinus[Cube.anglez];
    // Translate
    x += Cube.posx;
    y += Cube.posy;
    z += Cube.posz;
    // Perspective
    transf[idx].x = (x * 256.0) / z;
    transf[idx].y = (-y * 256.0) / z;
    transf[idx].z = z;
  }
}

VOID DrawCube(VOID)
{
  UWORD i;
  M3D_Triangle mytriangle;

  TransformPoints();
  for (i = 0;i < CUBE_FACES;i++) {
    if (Cube.faces[i].texture == 0) {
      mytriangle.texture = mytexture1;
    } else if (Cube.faces[i].texture == 1) {
      mytriangle.texture = mytexture2;
    } else if (Cube.faces[i].texture == 2) {
      mytriangle.texture = mytexture3;
    } else {
      mytriangle.texture = mytexture4;
    }
    mytriangle.color = Cube.faces[i].color;
    mytriangle.v1.x = transf[Cube.faces[i].p1].x + (SCREEN_WANTED_WIDTH/2);
    mytriangle.v1.y = transf[Cube.faces[i].p1].y + (SCREEN_WANTED_HEIGHT/2);
    mytriangle.v1.z = transf[Cube.faces[i].p1].z;
    mytriangle.v1.u = Cube.faces[i].u1;
    mytriangle.v1.v = Cube.faces[i].v1;
    mytriangle.v1.light = 1.0;
    mytriangle.v2.x = transf[Cube.faces[i].p2].x + (SCREEN_WANTED_WIDTH/2);
    mytriangle.v2.y = transf[Cube.faces[i].p2].y + (SCREEN_WANTED_HEIGHT/2);
    mytriangle.v2.z = transf[Cube.faces[i].p2].z;
    mytriangle.v2.u = Cube.faces[i].u2;
    mytriangle.v2.v = Cube.faces[i].v2;
    mytriangle.v2.light = 0.75;
    mytriangle.v3.x = transf[Cube.faces[i].p3].x + (SCREEN_WANTED_WIDTH/2);
    mytriangle.v3.y = transf[Cube.faces[i].p3].y + (SCREEN_WANTED_HEIGHT/2);
    mytriangle.v3.z = transf[Cube.faces[i].p3].z;
    mytriangle.v3.u = Cube.faces[i].u3;
    mytriangle.v3.v = Cube.faces[i].v3;
    mytriangle.v3.light = 0.50;
    if (((mytriangle.v1.y-mytriangle.v2.y)*(mytriangle.v2.x-mytriangle.v3.x)) < ((mytriangle.v2.y-mytriangle.v3.y)*(mytriangle.v1.x-mytriangle.v2.x))) {
      M3D_DrawTriangle(mycontext, &mytriangle);
      mytriangle.v1.x = transf[Cube.faces[i].p1].x + (SCREEN_WANTED_WIDTH/2);
      mytriangle.v1.y = transf[Cube.faces[i].p1].y + (SCREEN_WANTED_HEIGHT/2);
      mytriangle.v1.z = transf[Cube.faces[i].p1].z;
      mytriangle.v1.u = Cube.faces[i].u1;
      mytriangle.v1.v = Cube.faces[i].v1;
      mytriangle.v1.light = 1.0;
      mytriangle.v2.x = transf[Cube.faces[i].p4].x + (SCREEN_WANTED_WIDTH/2);
      mytriangle.v2.y = transf[Cube.faces[i].p4].y + (SCREEN_WANTED_HEIGHT/2);
      mytriangle.v2.z = transf[Cube.faces[i].p4].z;
      mytriangle.v2.u = Cube.faces[i].u4;
      mytriangle.v2.v = Cube.faces[i].v4;
      mytriangle.v2.light = 0.25;
      mytriangle.v3.x = transf[Cube.faces[i].p3].x + (SCREEN_WANTED_WIDTH/2);
      mytriangle.v3.y = transf[Cube.faces[i].p3].y + (SCREEN_WANTED_HEIGHT/2);
      mytriangle.v3.z = transf[Cube.faces[i].p3].z;
      mytriangle.v3.u = Cube.faces[i].u3;
      mytriangle.v3.v = Cube.faces[i].v3;
      mytriangle.v3.light = 0.50;
      M3D_DrawTriangle(mycontext, &mytriangle);
    }
  }
}

VOID DrawCube2(VOID)
{
  UWORD i;
  M3D_Quad myquad;

  TransformPoints();
  for (i = 0;i < CUBE_FACES;i++) {
    if (Cube.faces[i].texture == 0) {
      myquad.texture = mytexture1;
    } else if (Cube.faces[i].texture == 1) {
      myquad.texture = mytexture2;
    } else if (Cube.faces[i].texture == 2) {
      myquad.texture = mytexture3;
    } else {
      myquad.texture = mytexture4;
    }
    myquad.color = Cube.faces[i].color;
    myquad.v1.x = transf[Cube.faces[i].p1].x + (SCREEN_WANTED_WIDTH/2);
    myquad.v1.y = transf[Cube.faces[i].p1].y + (SCREEN_WANTED_HEIGHT/2);
    myquad.v1.z = transf[Cube.faces[i].p1].z;
    myquad.v1.u = Cube.faces[i].u1;
    myquad.v1.v = Cube.faces[i].v1;
    myquad.v1.light = 1.0;
    myquad.v2.x = transf[Cube.faces[i].p2].x + (SCREEN_WANTED_WIDTH/2);
    myquad.v2.y = transf[Cube.faces[i].p2].y + (SCREEN_WANTED_HEIGHT/2);
    myquad.v2.z = transf[Cube.faces[i].p2].z;
    myquad.v2.u = Cube.faces[i].u2;
    myquad.v2.v = Cube.faces[i].v2;
    myquad.v2.light = 0.75;
    myquad.v3.x = transf[Cube.faces[i].p3].x + (SCREEN_WANTED_WIDTH/2);
    myquad.v3.y = transf[Cube.faces[i].p3].y + (SCREEN_WANTED_HEIGHT/2);
    myquad.v3.z = transf[Cube.faces[i].p3].z;
    myquad.v3.u = Cube.faces[i].u3;
    myquad.v3.v = Cube.faces[i].v3;
    myquad.v3.light = 0.50;
    myquad.v4.x = transf[Cube.faces[i].p4].x + (SCREEN_WANTED_WIDTH/2);
    myquad.v4.y = transf[Cube.faces[i].p4].y + (SCREEN_WANTED_HEIGHT/2);
    myquad.v4.z = transf[Cube.faces[i].p4].z;
    myquad.v4.u = Cube.faces[i].u4;
    myquad.v4.v = Cube.faces[i].v4;
    myquad.v4.light = 0.25;
    if (((myquad.v1.y-myquad.v2.y)*(myquad.v2.x-myquad.v3.x)) < ((myquad.v2.y-myquad.v3.y)*(myquad.v1.x-myquad.v2.x))) {
      M3D_DrawQuad(mycontext, &myquad);
    }
  }
}

VOID _Render(VOID)
{
  STRPTR utext = "Arrow keys to rotate  I - Zoom in  O - Zoom out  Space - Reset cube";
  STRPTR btext = "T - Texmap on/off  G - Gourad on/off  B - Blending on/off  F - Filtering on/off  ESC - Quit";

  M3D_SetDrawRegion(mycontext, CyberScreen->RastPort.BitMap, NULL);
  M3D_ClearZBuffer(mycontext);
  if (M3D_LockHardware(mycontext) == M3D_SUCCESS) {
    M3D_ClearDrawRegion(mycontext, 0xff80ff);
    //DrawCube();
    DrawCube2();
    M3D_UnlockHardware(mycontext);
  } else {
    printf("Hardware lock failed !\n");
  }
  Move(&(CyberScreen->RastPort), 2, 450);
  Text(&(CyberScreen->RastPort), utext, strlen(utext));
  Move(&(CyberScreen->RastPort), 2, 464);
  Text(&(CyberScreen->RastPort), btext, strlen(btext));
}

void main(int argc, char ** argv)
{
  BOOL Done;

  printf("*******************************\n");
  printf("**    Maggie 3D cube test    **\n");
  printf("**   Quit with escape key    **\n");
  printf("*******************************\n");
  if (OpenAllLibs()) {
    // Init the demo data
    if (_Init()) {
      _Render();
      printf("* Enter the main loop *\n");
      Done = FALSE;
      while (!Done) {
        //draw_debug = FALSE;
        (void)Wait(SIGMASK(CyberWindow));
        while (IntMsg = GETIMSG(CyberWindow)) {
          switch (IntMsg->Class) {
            case IDCMP_RAWKEY:
              if (IntMsg->Code < 128) {
                if (IntMsg->Code == ESCAPE_KEY) {
                  printf("Exit loop\n");
                  Done = TRUE;
                } else {
                  // Update the demo data
                  _Update(IntMsg->Code);
                }
              }
              break;
            default:
              printf("Uncatched event %d - %d\n", IntMsg->Class, IntMsg->Code);
              break;
          }
          ReplyMsg(&IntMsg->ExecMessage);
        }
        // Render the demo
        _Render();
      }
      // Restore the demo
      _Restore();
    }
  }
  CloseAllLibs();
  printf("** Program terminated **\n");
}
