#ifndef __GCDRAW_H
#define __GCDRAW_H
#include "lcd/lcd.h"

typedef enum {
  PS_NORMAL,
  PS_ALPHA,
  PS_TEXTURE,
  PS_BENDER
} PSTYLE; //Clock Pointer Style


typedef struct {
  Vec2 vpos;
  char* text;
  uint16_t color;
} DOText;

typedef struct {
  Vec2 vpos;
  Vec2 vsize;
  uint16_t alpha;
  const uint8_t* data;
} DOImage;


typedef struct {
  Vec2 vpos;
  Vec2 vs;
  Vec2 vts;
  uint16_t alpha;
  uint16_t color;
  const uint8_t* data;
} DOTexture;

DOImage* DOImage_newv2(  Vec2 vpos,  Vec2 vs,  uint16_t alpha,  const uint8_t* data);
DOImage* DOImage_new(int16_t x, int16_t y, int16_t x1, int16_t y1,  uint16_t alpha,  const uint8_t* data);
void draw_pointer_mode(Vec2 vs, Vec2 vts, int16_t tu, uint16_t color, const uint8_t* sr, uint16_t alpha, PSTYLE cps);
void draw_dotex(DOTexture* doi);
void draw_doimage(DOImage* doi);

#endif
