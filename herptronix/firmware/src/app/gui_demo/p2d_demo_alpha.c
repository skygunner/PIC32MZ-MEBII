/**
 * @file p2d_demo_alpha.c
 * @brief 2D primitives demo, with alpha blending (experimental)
 * @author Duboisset Philippe <duboisset.philippe@gmail.com>
 * @version N/A -- draft / experimental
 * @date (yyyy-mm-dd) 2014-03-08
 * Licence: MIT
 */

#include "gui_demo.h"
#include "resources.h"
#include "delay.h"

#ifdef INCLUDE_GUI_DEMO


/**
 * Local variable
 */
#define TIME_TEST 4000
static volatile uint8_t done;
static coord_t _x0, _y0, _x1, _y1;

/**
 * Local functions
 */
static void TestAlphaFillrect(void) ;
static void TestAlphaFillPoly(void);
static void TestAlphaFont(void);
static void TestAlphaSprite(void);
static void TestAlphaRect(void);
static void TestAlphaCircle(void);
static void TestAlphaLine(void);
static void TestAlphaPixel(void);
static void StartTest(const void *title);
static void EndTest(const void *str);
static void WdtCallback(void);
static void TestSetClip(void);


 /**
 * @function P2D_DemoAlpha
 * @brief 2D primitives demo with alpha enabled
 * @param signal_t sig: unused
 * @return none
 */
void P2D_DemoAlpha(signal_t sig) {

  /*test area definition*/
  _x0 = 5;
  _y0 = 20;
  _x1 = P2D_GetLcdWidth() - _x0;
  _y1 = P2D_GetLcdHeight() - _x0;

  TestAlphaFillPoly();
  TestAlphaFont();
  TestAlphaSprite();
  TestAlphaFillrect();
  TestAlphaRect();
  TestAlphaCircle();
  TestAlphaLine();
  TestAlphaPixel();

  /*return to demo menu*/
  GUI_SetUserTask(Gui_Demo);
}


/**
 * @function TestAlphaFillPoly
 * @brief alpha filled polygon + double buffering (sprited background) demo
 * @param none
 * @return none
 */
static void TestAlphaFillPoly(void) {

  char str[50];
  surfaceId_t surface;
  rect_st dim, dst, src;
  uint16_t frame = 0, angle = 0;
  lut8bpp_st lut;
  rect_st d;
  length_t wSprite;

  #define NB_POINT  7
  point_st ar[NB_POINT] = { {100,100}, {150,150}, {120,140}, {150,200}, {50 ,200}, {80 ,140}, {50, 150} };
  point_st arTmp[NB_POINT];
  point_st ref;

  /*surface creation*/
  dim.w = 160;
  dim.h = 120;
  surface = P2D_SurfaceCreate(&dim);

  /*source rect, for buffer flip; here, the whole buffer is selected*/
  src.x = 0;
  src.y = 0;
  src.w = dim.w;
  src.h = dim.h;

  /*destination rect, for buffer flip; here, the whole selection is put at screen center*/
  dst.x = (P2D_GetLcdWidth() - dim.w) / 2;
  dst.y = (P2D_GetLcdHeight() - dim.h) / 2;
  dst.w = dim.w;
  dst.h = dim.h;

  /*find the polygon center, and move it to the center of the buffer*/
  P2D_FindPolyCenter(ar, NB_POINT, &ref);
  P2D_P_Move(ar, NB_POINT, -ref.x, -ref.y);  /*now centered to x=y=0*/
  ref.x = dim.w / 2;
  ref.y = dim.h / 2;
  P2D_P_Move(ar, NB_POINT, ref.x, ref.y);   /*now centered to the surface' center*/

  /*sprite init*/
  P2D_InitLut8BPP(&lut, spriteLutFile00, LUT_E_COPY);
  P2D_SpriteSetLut8BPP(&lut);
  wSprite = P2D_SpriteGetWidth(spriteFile00);

  sprintf(str, "Filled polygon (16BPP buffer %dx%d)", dim.w, dim.h);
  StartTest(str);

  /*main loop*/
  while(done == 0) {

    /* copy the polygon points list in another points list
     * this step is mandatory because of the successive P2D_P_Rotate() function calls;
     * this function usually introduces rounding errors, and will distort the polygon
     */
    P2D_P_Copy(ar, arTmp, NB_POINT);
    P2D_P_Rotate(arTmp, NB_POINT, &ref, angle);

    /*put next operation in the internal surface*/
    P2D_SetDest(surface);

    /*clear all, redraw dimension rect*/
    P2D_SetColor(0);
    P2D_Clear();

    /*fill the background with sprite*/
    d.y = 0;
    for(d.x = 0; d.x < dim.w; d.x += wSprite) {
      P2D_Sprite(NULL, &d, spriteFile00);
    }

    /*draw the polygon in internal surface*/
    P2D_SetColor(COLOR_RED);
    P2D_SetAlpha(128);
    P2D_FillPoly(arTmp, NB_POINT);
    P2D_SetAlpha(255);

    /*put the internal surface to screen*/
    P2D_SetDest(SURFACE_LCD);
    P2D_CopySurface(surface, &src, &dst);

    angle += 5;
    frame++;
  }

  P2D_SurfaceDelete(surface);

  sprintf(str, "%d fps", frame * 1000 / TIME_TEST);
  EndTest(str);
}


/**
 * @function TestAlphaFont
 * @brief 4BPP font test + alpha
 * @param none
 * @return none
 */
static void TestAlphaFont(void) {
  rect_st rec;
  uint32_t cnt = 0;
  char str[50];
  const char *testStr = "herptronix";

  StartTest("Random clipped 4BPP (AA/alpha/transparent) text");

  P2D_SetFont(FontFreeSerif_4bpp_n_16);
  P2D_SetDisplayMode(DISPLAY_TRANSPARENT);

  while(done == 0) {

    rec.x = P2D_Rand(P2D_GetLcdWidth());
    rec.y = P2D_Rand(P2D_GetLcdHeight());

    P2D_SetColors(P2D_Rand(0xFFFF), 0);
    P2D_SetAlpha(P2D_Rand(255));
    P2D_PutText(rec.x, rec.y, testStr);
    cnt++;
  }

  P2D_SetDisplayMode(DISPLAY_SOLID);
  P2D_SetAlpha(255);

  cnt *= gstrlen(testStr);
  sprintf(str, "%d glyphs/s", cnt * 1000 / TIME_TEST);
  EndTest(str);
}


/**
 * @function TestAlphaSprite
 * @brief 8BPP sprite demo + alpha
 * @param none
 * @return none
 */
static void TestAlphaSprite(void) {

  lut8bpp_st lut;
  uint32_t cnt = 0;
  char str[50];
  rect_st dst, src;

  src.x = 0;
  src.w = P2D_SpriteGetWidth(spriteFile00);
  src.h = src.w;

  P2D_InitLut8BPP(&lut, spriteLutFile00, LUT_E_COPY);
  P2D_SpriteSetLut8BPP(&lut);

  sprintf(str, "Clipped 8BPP/alpha sprite %dx%dpx", src.w, src.h);
  StartTest(str);

  while(done == 0) {

    dst.x = P2D_Rand(P2D_GetLcdWidth());
    dst.y = P2D_Rand(P2D_GetLcdHeight());
    src.y = (coord_t) P2D_Rand(43) * src.h;

    P2D_SetAlpha(P2D_Rand(255));
    P2D_Sprite(&src, &dst, spriteFile00);
    cnt++;
  }

  P2D_SetAlpha(255);

  sprintf(str, "%d sprite/s", 1000 * cnt / TIME_TEST);
  EndTest(str);
}


/**
 * @function TestAlphaFillrect
 * @brief alpha filled rect demo
 * @param none
 * @return none
 */
static void TestAlphaFillrect(void) {

  rect_st rec;
  uint32_t cnt = 0;
  char str[50];

  StartTest("Random clipped alpha Fillrect");

  while(done == 0) {
    rec.x = P2D_Rand(P2D_GetLcdWidth());
    rec.y = P2D_Rand(P2D_GetLcdHeight());
    rec.w = P2D_Rand(P2D_GetLcdWidth() / 2);
    rec.h = P2D_Rand(P2D_GetLcdHeight() / 2);
    P2D_SetColor(P2D_Rand(0xFFFF));
    P2D_SetAlpha(P2D_Rand(255));

    P2D_FillRect(&rec);

    P2D_ClipFit(&rec);
    cnt += P2D_GetPixelCnt(&rec);
  }

  P2D_SetAlpha(255);

  sprintf(str, "%.3fMpx/s", (double)cnt / TIME_TEST / 1000);
  EndTest(str);
}


/**
 * @function TestAlphaRect
 * @brief rect demo + alpha
 * @param none
 * @return none
 */
static void TestAlphaRect(void) {

  rect_st rec;
  uint32_t cnt = 0;
  char str[50];

  StartTest("Random clipped alpha rect");

  while(done == 0) {

    rec.x = P2D_Rand(P2D_GetLcdWidth());
    rec.y = P2D_Rand(P2D_GetLcdHeight());
    rec.w = P2D_Rand(P2D_GetLcdWidth() / 2);
    rec.h = P2D_Rand(P2D_GetLcdHeight() / 2);

    P2D_SetColor(P2D_Rand(0xFFFF));
    P2D_SetAlpha(P2D_Rand(255));
    P2D_Rect(&rec);
    cnt++;
  }
  
  P2D_SetAlpha(255);

  sprintf(str, "%d rect/s", 1000 * cnt / TIME_TEST);
  EndTest(str);
}


/**
 * @function TestAlphaCircle
 * @brief circle demo + alpha
 * @param none
 * @return none
 */
static void TestAlphaCircle(void) {
  uint32_t px = 0;
  coord_t x0, y0;
  uint8_t r;
  char str[50];

  StartTest("Random clipped alpha circle");

  while(done == 0) {

    x0 = P2D_Rand(P2D_GetLcdWidth());
    y0 = P2D_Rand(P2D_GetLcdHeight());
    r = (uint8_t) P2D_Rand(50);
    P2D_SetColor(P2D_Rand(0xFFFF));
    P2D_SetAlpha(P2D_Rand(255));
    P2D_Circle(x0, y0, r);

    px++;
  }
  
  P2D_SetAlpha(255);

  sprintf(str, "%d circle/s", px * 1000 / TIME_TEST);
  EndTest(str);
}


/**
 * @function TestAlphaLine
 * @brief line demo
 * @param none
 * @return none
 */
static void TestAlphaLine(void) {

  uint32_t cnt = 0;
  coord_t x0, y0, x1, y1;
  char str[50];

  StartTest("Random clipped alpha line");;

  while(done == 0) {

    x0 = P2D_Rand(P2D_GetLcdWidth());
    y0 = P2D_Rand(P2D_GetLcdHeight());
    x1 = P2D_Rand(P2D_GetLcdWidth());
    y1 = P2D_Rand(P2D_GetLcdHeight());

    P2D_SetColor(P2D_Rand(0xFFFF));
    P2D_SetAlpha(P2D_Rand(255));
    P2D_Line(x0, y0, x1, y1);
    cnt++;
  }
  
  P2D_SetAlpha(255);

  sprintf(str, "%d lines/s", 1000 * cnt / TIME_TEST);
  EndTest(str);
}


/**
 * @function TestAlphaPixel
 * @brief pixel demo
 * @param none
 * @return none
 */
static void TestAlphaPixel(void) {

  uint32_t px = 0;
  coord_t x0, y0;
  char str[50];

  StartTest("Random clipped alpha pixels");

  while(done == 0) {

    x0 = P2D_Rand(P2D_GetLcdWidth());
    y0 = P2D_Rand(P2D_GetLcdHeight());
    
    P2D_SetAlpha(P2D_Rand(255));
    P2D_SetPixel(x0, y0, P2D_Rand(0xFFFF));

    px++;
  }
  
  P2D_SetAlpha(255);

  sprintf(str, "%d pixels/s", px * 1000 / TIME_TEST);
  EndTest(str);
}


#define COL_FRONT 0xFFFF
#define COL_BACK  0x0000

/**
 * @function StartTest
 * @brief set the test clip & callback
 * @param const void *title: test title
 * @return none
 */
static void StartTest(const void *title) {

  /*clear the whole screen*/
  P2D_SetClip(NULL);
  P2D_SetColor(COL_BACK);
  P2D_Clear();

  /*put the title*/
  P2D_SetFont(FontFreeSerif_4bpp_n_16);
  P2D_SetDisplayMode(DISPLAY_SOLID);
  P2D_SetColors(COL_FRONT, COL_BACK);
  P2D_PutText(_x0, _y0 - P2D_GetTextHeight(), title);

  /*set the test clip*/
  TestSetClip();

  P2D_RandInit(0);

  /*start the software watchdog*/
  done = 0;
  TicksSetWatchdog(WdtCallback, TIME_TEST);
}


/**
 * @function StartTest
 * @brief close test and wait few seconds
 * @param const void *str: test result
 * @return none
 */
static void EndTest(const void *str) {

  /*disable the test clip*/
  P2D_SetClip(NULL);

  /*put the result*/
  P2D_SetFont(FontFreeSerif_4bpp_n_16);
  P2D_SetDisplayMode(DISPLAY_SOLID);
  P2D_SetColors(COL_FRONT, COL_BACK);
  P2D_PutText(_x1 - P2D_GetTextWidth(str), _y0 - P2D_GetTextHeight(), str);

  /*wait a bit before next test*/
  DelayMs(2000);
}


/**
 * @function WdtCallback
 * @brief timer callback, for exiting infinite loops of tests
 * @param none
 * @return none
 */
static void WdtCallback(void) {
  done = 1;
}


/**
 * @function TestSetClip
 * @brief configure P2D for tests
 * @param none
 * @return none
 */
static void TestSetClip(void) {

  /*display the test clip*/
  rect_st rec;

  P2D_SetColor(COLOR_GREEN);
  rec.x = rec.y = 0;
  rec.w = 480;
  rec.h = 272;
  P2D_Rect(&rec);

  P2D_SetColor(COL_FRONT);
  P2D_CoordToRect(&rec, _x0, _y0, _x1, _y1);
  P2D_Rect(&rec);

  /*set the clip (increment x, y.. to avoid overlaps on the previous rect*/
  P2D_CoordToRect(&rec, _x0+1, _y0+1, _x1-1, _y1-1);
  P2D_SetClip(&rec);
}

#endif
