/**
 * Magie3D library
 *
 * Benchmark test
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.0 November 2023
 */

#include <exec/exec.h>
#include <exec/io.h>
#include <devices/timer.h>
#include <intuition/intuitionbase.h>
#include <cybergraphx/cybergraphics.h>

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/cybergraphics.h>

#include <clib/alib_protos.h>
#include <clib/timer_protos.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "/src/Maggie3D.h"

#define INTUITIONVERSION      39L
#define CYBERGFXVERSION       41L

/** Benchmark data */
#define BENCH_LOOP            20
#define BENCH_TRI             100

typedef struct {
  STRPTR title;                 // Benchmark title
  ULONG scrw, scrh, scrd;       // Screen width, height and depth
  BOOL bilinear, zbuffer, gouraud;  // Maggie states
  LONG min, max, avg;           // Execution time in microseconds
} Benchmark;

typedef struct {
  LONG x1, y1, x2, y2, x3, y3;
} Vertex;

Vertex vertices[BENCH_TRI];

M3D_Triangle triangles[BENCH_TRI];

Benchmark mybenchs[] = {
  { "Bench 1", 320, 240, 16, FALSE, FALSE, FALSE, 0, 0, 0 },
  { "Bench 2", 320, 240, 16, TRUE, TRUE, TRUE, 0, 0, 0 },
  { "Bench 3", 640, 480, 16, FALSE, FALSE, FALSE, 0, 0, 0 },
  { "Bench 4", 640, 480, 16, TRUE, TRUE, TRUE, 0, 0, 0 },
  { NULL, 0, 0, 0, FALSE, FALSE, FALSE, 0, 0, 0 }
};

/** Librairies */
struct IntuitionBase *IntuitionBase = NULL;
struct Library *CyberGfxBase = NULL;
struct Library * TimerBase = NULL;

/** Timer */
struct timeval time_old, time_current;
struct MsgPort *timer_port = NULL;
struct timerequest *io_request = NULL;
ULONG seconds, micro_seconds;

/** Bench screen & window */
struct Screen *myscreen = NULL;
struct Window *mywindow = NULL;

/** Maggie3D data */
M3D_Context *mycontext = NULL;
M3D_Texture *mytexture = NULL;

/** Open libraries */
BOOL OpenLibraries(VOID)
{
  printf("- Open librairies\n");
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
VOID CloseLibraries(void)
{
  printf("- Close librairies\n");
  if (CyberGfxBase != NULL) {
    CloseLibrary(CyberGfxBase);
  }
  if (IntuitionBase != NULL) {
    CloseLibrary(&IntuitionBase->LibNode);
  }
}

/** Init timer */
BOOL InitTimer(VOID)
{
  printf("- Initialize timer\n");
  timer_port = CreatePort(NULL, 0L);
  if (timer_port != NULL ) {
    io_request = (struct timerequest *) CreateExtIO(timer_port, sizeof(struct timerequest));
    if (io_request != NULL ) {
      if (!OpenDevice(TIMERNAME, UNIT_MICROHZ, (struct IORequest *) io_request, 0L)) {
        TimerBase = (struct Library *) io_request->tr_node.io_Device;
        return TRUE;
      }
    }
  }
  printf("ERROR: can't create Timer port or request\n");
  return FALSE;
}

/** Release timer */
VOID ReleaseTimer(VOID)
{
  printf("- Release timer\n");
  if (io_request != NULL) {
    CloseDevice((struct IORequest *) io_request);
    DeleteExtIO((struct IORequest *) io_request);
  }
  if (timer_port != NULL) {
    DeletePort(timer_port);
  }
}

/** Init vertices */
VOID InitVertices(VOID)
{
  UWORD idx;
  
  printf("- Generate vertices\n");
  srand(0x68C0DE);
  for (idx = 0;idx < BENCH_TRI;idx++) {
    vertices[idx].x1 = rand();
    vertices[idx].y1 = rand();
    vertices[idx].x2 = rand();
    vertices[idx].y2 = rand();
    vertices[idx].x3 = rand();
    vertices[idx].y3 = rand();
    printf(" => add vertice %d : %d,%d -> %d,%d -> %d,%d\n", idx,
      vertices[idx].x1,vertices[idx].y1,
      vertices[idx].x2,vertices[idx].y2,
      vertices[idx].x3,vertices[idx].y3
    );
  }
}

/** Initialize benchmark */
BOOL InitBenchmark(VOID)
{
  if (!OpenLibraries()) {
    return FALSE;
  }
  if (!InitTimer()) {
    return FALSE;
  }
  InitVertices();
  return TRUE;
}

/** Restore benchmark */
VOID RestoreBenchmark(VOID)
{
  ReleaseTimer();
  CloseLibraries();
}

/******************************************************************************/
/**         Bench functions                                                   */
/******************************************************************************/

/** Open screen and window */
BOOL OpenScreenWindow(ULONG scrw, ULONG scrh, ULONG scrd)
{
  ULONG display_id;

  printf("- Open the screen %dx%dx%d\n", scrw, scrh, scrd);
  // Find best screen mode
  display_id = BestCModeIDTags(
    CYBRBIDTG_NominalWidth, scrw,
    CYBRBIDTG_NominalHeight, scrh,
    CYBRBIDTG_Depth, scrd,
    TAG_DONE
  );
  if (display_id != INVALID_ID) {
    // Open the screen
    myscreen = OpenScreenTags(
      NULL,
      SA_Title, NULL,
      SA_ShowTitle, FALSE,
      SA_DisplayID, display_id,
      SA_Type, CUSTOMSCREEN,
      TAG_DONE
    );
    if (myscreen != NULL) {
      // Open the window
      mywindow = OpenWindowTags(
        NULL,
        WA_Title, NULL,
        WA_Flags, WFLG_ACTIVATE|WFLG_BACKDROP|WFLG_BORDERLESS|WFLG_RMBTRAP,
        WA_IDCMP, IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS,
        WA_Left, 0,
        WA_Top, 0,
        WA_Width, scrw,
        WA_Height, scrh,
        WA_CustomScreen, myscreen,
        TAG_DONE
      );
      if (mywindow != NULL) {
        return TRUE;
      }
    }
  }
  printf("ERROR: can't open screen (%dx%dx%d) or window\n", scrw, scrh, scrd);
  return FALSE;
}

/** Close screen and window */
VOID CloseScreenWindow(VOID)
{
  printf("- Close the screen\n");
  if (mywindow != NULL) {
    CloseWindow(mywindow);
    mywindow = NULL;
  }
  if (myscreen != NULL) {
    CloseScreen(myscreen);
    myscreen = NULL;
  }
}

/** Load the texture */
BOOL InitTexture(VOID)
{
  LONG error;

  printf("- Load texture\n");
  mytexture = M3D_AllocTextureFile(mycontext, &error, "texture.bmp");
  if (mytexture != NULL) {
    return TRUE;
  }
  printf("ERROR: can't allocate texture\n");
  return FALSE;
}

/** Release texture */
VOID ReleaseTexture(VOID)
{
  printf("- Release texture\n");
  if (mytexture != NULL) {
    M3D_FreeTexture(mycontext, mytexture);
    mytexture = NULL;
  }
}

/** Init triangles */
BOOL InitTriangles(ULONG scrw, ULONG scrh)
{
  UWORD idx;
  
  printf("- Generate triangles\n");
  for (idx = 0;idx < BENCH_TRI;idx++) {
    triangles[idx].v1.x = (FLOAT)(vertices[idx].x1 % scrw);
    triangles[idx].v1.y = (FLOAT)(vertices[idx].y1 % scrh);
    triangles[idx].v1.z = 50.0;
    triangles[idx].v1.u = 0.0F;
    triangles[idx].v1.v = 0.0F;
    triangles[idx].v1.light = 1.0F;
    triangles[idx].v2.x = (FLOAT)(vertices[idx].x2 % scrw);
    triangles[idx].v2.y = (FLOAT)(vertices[idx].y2 % scrh);
    triangles[idx].v2.z = 50.0;
    triangles[idx].v2.u = 0.0F;
    triangles[idx].v2.v = 255.0F;
    triangles[idx].v2.light = 0.8F;
    triangles[idx].v3.x = (FLOAT)(vertices[idx].x3 % scrw);
    triangles[idx].v3.y = (FLOAT)(vertices[idx].y3 % scrh);
    triangles[idx].v3.z = 50.0;
    triangles[idx].v3.u = 255.0F;
    triangles[idx].v3.v = 128.0F;
    triangles[idx].v3.light = 0.5F;
    triangles[idx].texture = mytexture;
    triangles[idx].color = 0xFFFFFFFF;
  }
  return TRUE;
}

/** Clear the screen */
ULONG ClearScreen(struct BitMap *bitmap, ULONG scrw, ULONG scrh, ULONG scrd)
{
  APTR handle = NULL;
  ULONG address = 0;

  if (GetCyberMapAttr(bitmap, CYBRMATTR_ISCYBERGFX)) {
    handle = LockBitMapTags(bitmap, LBMI_BASEADDRESS, &address, TAG_DONE);
  }
  if (handle != NULL) {
    // Clear here
    UnLockBitMap(handle);
  }
  return address;
}

/** Prepare bench test */
BOOL PrepareBench(UWORD bench_idx)
{
  LONG error;

  if (!OpenScreenWindow(mybenchs[bench_idx].scrw, mybenchs[bench_idx].scrh, mybenchs[bench_idx].scrd)) {
    return FALSE;
  }
  printf("- Create context\n");
  if ((mycontext = M3D_CreateContext(&error, myscreen->RastPort.BitMap)) == NULL) {
    return FALSE;
  }
  if (!InitTexture()) {
    return FALSE;
  }
  if (!InitTriangles(mybenchs[bench_idx].scrw, mybenchs[bench_idx].scrh)) {
    return FALSE;
  }
  printf("- Set render states\n");
  if (mybenchs[bench_idx].bilinear) {
    M3D_SetState(mycontext, M3D_BILINEAR, M3D_ENABLE);
  } else {
    M3D_SetState(mycontext, M3D_BILINEAR, M3D_DISABLE);
  }
  if (mybenchs[bench_idx].zbuffer) {
    if (M3D_AllocZBuffer(mycontext) != M3D_SUCCESS) {
      return FALSE;
    }
    M3D_SetState(mycontext, M3D_ZBUFFER, M3D_ENABLE);
  } else {
    M3D_SetState(mycontext, M3D_ZBUFFER, M3D_DISABLE);
  }
  if (mybenchs[bench_idx].gouraud) {
    M3D_SetState(mycontext, M3D_GOURAUD, M3D_ENABLE);
  } else {
    M3D_SetState(mycontext, M3D_GOURAUD, M3D_DISABLE);
  }
  if (mybenchs[bench_idx].scrd == 16) {
    M3D_SetState(mycontext, M3D_16BITS, M3D_ENABLE);
  } else {
    M3D_SetState(mycontext, M3D_16BITS, M3D_DISABLE);
  }
  M3D_SetState(mycontext, M3D_TEXMAPPING, M3D_ENABLE);
  M3D_SetState(mycontext, M3D_FAST, M3D_ENABLE);
  mybenchs[bench_idx].min = 0xffffffff;
  mybenchs[bench_idx].max = 0;
  mybenchs[bench_idx].avg = 0;
  return TRUE;
}

/** Restore bench test */
VOID RestoreBench(VOID)
{
  ReleaseTexture();
  M3D_FreeZBuffer(mycontext);
  M3D_DestroyContext(mycontext);
  mycontext = NULL;
  CloseScreenWindow();
}

/** Dsiplay bench results */
VOID DisplayResults(VOID)
{
  UWORD bench_idx = 0;
  printf("\n========================================\n");
  printf("=      Maggie3D benchmark results      =\n");
  printf("========================================\n");
  printf("= Drawing %d triangles %d times\n", BENCH_TRI, BENCH_LOOP);
  printf("========================================\n");
  while (mybenchs[bench_idx].title != NULL) {
    printf("* %s, screen of %dx%dx%d\n", mybenchs[bench_idx].title, mybenchs[bench_idx].scrw, mybenchs[bench_idx].scrh, mybenchs[bench_idx].scrd);
    printf("  Bilinear is %s\n", (mybenchs[bench_idx].bilinear ? "ON" : "OFF"));
    printf("  Z buffer is %s\n", (mybenchs[bench_idx].zbuffer ? "ON" : "OFF"));
    printf("  Gouraud is %s\n", (mybenchs[bench_idx].gouraud ? "ON" : "OFF"));
    printf("  minimum %d �s, maximum %d �s, average %d �s\n", mybenchs[bench_idx].min, mybenchs[bench_idx].max, mybenchs[bench_idx].avg); 
    printf("========================================\n");
    bench_idx++;
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
  UWORD bench_idx = 0, repeat;
  struct timeval start_time, end_time;

  printf("*******************************\n");
  printf("**    Maggie 3D benchmark    **\n");
  printf("**     V1.0 January 2024     **\n");
  printf("**  Fabrice 'Lexo' Labrador  **\n");
  printf("*******************************\n");
  if (InitBenchmark()) {
    while (mybenchs[bench_idx].title != NULL) {
      printf("\n* Running %s\n", mybenchs[bench_idx].title);
      if (PrepareBench(bench_idx)) {
        printf("- Run benchmark\n");
        repeat = BENCH_LOOP;
        while (repeat--) {
          if (M3D_LockHardware(mycontext) == M3D_SUCCESS) {
            GetSysTime(&start_time);
            if (mybenchs[bench_idx].zbuffer) {
              M3D_ClearZBuffer(mycontext);
            }
            M3D_DrawTriangleArray(mycontext, triangles, BENCH_TRI);
            GetSysTime(&end_time);
            M3D_UnlockHardware(mycontext);
            SubTime(&end_time, &start_time);
            if (mybenchs[bench_idx].min > end_time.tv_micro) {
              mybenchs[bench_idx].min = end_time.tv_micro;
            }
            if (mybenchs[bench_idx].max < end_time.tv_micro) {
              mybenchs[bench_idx].max = end_time.tv_micro;
            }
            mybenchs[bench_idx].avg += end_time.tv_micro;
          }
        }
        mybenchs[bench_idx].avg /= BENCH_LOOP;
      }
      RestoreBench();
      bench_idx++;
    }
    DisplayResults();
  }
  RestoreBenchmark();
  printf("** Program terminated **\n");
  return 0;
}
