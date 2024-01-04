/**
 * Magie3D library
 *
 * Cube test
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
#include <math.h>

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
#define UP_KEY                76
#define DOWN_KEY              77
#define LEFT_KEY              79
#define RIGHT_KEY             78
#define SPACE_KEY             64
#define D_KEY                 34

#define CUBE_POINTS           8
#define CUBE_FACES            6

struct cube_point {
  FLOAT x, y, z;
};

struct cube_face {
  ULONG p1, p2, p3, p4, color;
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
    { 0,1,2,3, 0xffffff, 0,0,255,0,255,255,0,255 },
    { 1,5,6,2, 0xff0000, 0,0,255,0,255,255,0,255 },
    { 5,4,7,6, 0x00ff00, 0,0,255,0,255,255,0,255 },
    { 4,0,3,7, 0x0000ff, 0,0,255,0,255,255,0,255 },
    { 4,5,1,0, 0xff00ff, 0,0,255,0,255,255,0,255 },
    { 3,2,6,7, 0xffff00, 0,0,255,0,255,255,0,255 }
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
M3D_Texture *mytexture = NULL;
LONG error;

// Debug
extern BOOL draw_debug;

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
  DisplayID = BestCModeIDTags(
    CYBRBIDTG_NominalWidth, SCREEN_WANTED_WIDTH,
    CYBRBIDTG_NominalHeight, SCREEN_WANTED_HEIGHT,
    CYBRBIDTG_Depth, SCREEN_WANTED_DEPTH,
    TAG_DONE
  );
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
  mycontext = M3D_CreateContext(&error, CyberScreen->RastPort.BitMap);
  if (mycontext == NULL) {
    printf("Create context error !\n");
    return FALSE;
  }
  printf("- Maggie context created\n");
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
  return TRUE;
}

BOOL InitCube(VOID)
{
  return TRUE;
}

BOOL InitTexture(VOID)
{
//  mytexture = M3D_AllocTextureFile(mycontext, &error, "texture.dds");
  mytexture = M3D_AllocTextureFile(mycontext, &error, "texture.bmp");
  if (mytexture == NULL) {
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
  printf("* Free texture *\n");
  M3D_FreeTexture(mycontext, mytexture);
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
      Cube.posx = 0;
      Cube.posy = 0;
      Cube.posz = 50;
      break;
    case UP_KEY:
      Cube.anglex += 1;
      if (Cube.anglex >= 360) Cube.anglex -= 360;
      break;
    case DOWN_KEY:
      Cube.anglex -= 1;
      if (Cube.anglex < 0) Cube.anglex += 360;
      break;
    case LEFT_KEY:
      Cube.angley += 1;
      if (Cube.angley >= 360) Cube.angley -= 360;
      break;
    case RIGHT_KEY:
      Cube.angley -= 1;
      if (Cube.angley < 0) Cube.angley += 360;
      break;
    case D_KEY:
      //draw_debug = TRUE;
      break;
  }
}

/** Draw the cube */
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
  mytriangle.texture = mytexture;
  for (i = 0;i < CUBE_FACES;i++) {
    mytriangle.color = 0xffffff; //Cube.faces[i].color;
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
    mytriangle.v2.light = 1.0;
    mytriangle.v3.x = transf[Cube.faces[i].p3].x + (SCREEN_WANTED_WIDTH/2);
    mytriangle.v3.y = transf[Cube.faces[i].p3].y + (SCREEN_WANTED_HEIGHT/2);
    mytriangle.v3.z = transf[Cube.faces[i].p3].z;
    mytriangle.v3.u = Cube.faces[i].u3;
    mytriangle.v3.v = Cube.faces[i].v3;
    mytriangle.v3.light = 1.0;
    if (((mytriangle.v1.y-mytriangle.v2.y)*(mytriangle.v2.x-mytriangle.v3.x)) < ((mytriangle.v2.y-mytriangle.v3.y)*(mytriangle.v1.x-mytriangle.v2.x))) {
      M3D_DrawTriangle(mycontext, &mytriangle);
      mytriangle.v1.x = transf[Cube.faces[i].p1].x + (SCREEN_WANTED_WIDTH/2);
      mytriangle.v1.y = transf[Cube.faces[i].p1].y + (SCREEN_WANTED_HEIGHT/2);
      mytriangle.v1.z = transf[Cube.faces[i].p1].z;
      mytriangle.v1.u = Cube.faces[i].u1;
      mytriangle.v1.v = Cube.faces[i].v1;
      mytriangle.v2.x = transf[Cube.faces[i].p4].x + (SCREEN_WANTED_WIDTH/2);
      mytriangle.v2.y = transf[Cube.faces[i].p4].y + (SCREEN_WANTED_HEIGHT/2);
      mytriangle.v2.z = transf[Cube.faces[i].p4].z;
      mytriangle.v2.u = Cube.faces[i].u4;
      mytriangle.v2.v = Cube.faces[i].v4;
      mytriangle.v3.x = transf[Cube.faces[i].p3].x + (SCREEN_WANTED_WIDTH/2);
      mytriangle.v3.y = transf[Cube.faces[i].p3].y + (SCREEN_WANTED_HEIGHT/2);
      mytriangle.v3.z = transf[Cube.faces[i].p3].z;
      mytriangle.v3.u = Cube.faces[i].u3;
      mytriangle.v3.v = Cube.faces[i].v3;
      M3D_DrawTriangle(mycontext, &mytriangle);
    }
  }
}

VOID _Render(VOID)
{
  ClearScreen(CyberScreen->RastPort.BitMap);
  if (M3D_LockHardware(mycontext) == M3D_SUCCESS) {
    M3D_SetDrawRegion(mycontext, CyberScreen->RastPort.BitMap, NULL);
    M3D_ClearZBuffer(mycontext);
    DrawCube();
    M3D_UnlockHardware(mycontext);
  } else {
    printf("Hardware lock failed !\n");
  }
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
        draw_debug = FALSE;
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
