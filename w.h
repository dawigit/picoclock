#ifndef W_H
#define W_H

#include "stdlib.h"
#include "pico/binary_info.h"
#include "lcd.h"


#define W_BOX_CHILDREN 4

typedef enum {
  wt_none = 0,
  wt_root = 1,
  wt_box,
  wt_text,
  wt_textr,
  wt_button,
  wt_image,
  wt_imager,
  wt_imagef,
  wt_blinker_circle,
  wt_blinker_rect,
  wt_spinner,
} W_t;

typedef enum {
  st_none = 0,
  st_value_u8,
  st_value_u16,
  st_value_u32,
  st_value_i8,
  st_value_i16,
  st_value_i32,
  st_value_float,
  st_value_char,
  st_box,
  st_hbox,
  st_vbox,
  st_text_ghost,
  st_text_cn,
  st_text_cn_ghost,
  st_imager,
  st_spinner_char_v,
  st_spinner_char_h,
  st_blink_once,
} W_st;

typedef enum {
  ws_none = 0,
  ws_shown = 1,
  ws_hidden = 2,
} W_status;

typedef struct {
  void* p;          // parent
  int16_t x;        // x pos
  int16_t y;        // y pos
  int16_t w;        // width
  int16_t h;        // height
  W_t t;            // type
  W_st st;          // sub-type
  W_status ws;      // widget status
  void* d;          // data
  uint32_t lt;      // last touch
} W;

typedef struct {
  uint16_t nc;
  uint16_t size;
  W** ch;
} W_box;


typedef struct {
  char* text;
  font_t** fonts;
  uint16_t cf;
  uint16_t cb;
  uint16_t cfr;
  uint8_t fs;
  int8_t marge;
  int8_t pad;
  uint8_t fid;
  uint8_t o;
} W_text;

typedef struct {
  char* (*get_text)();
  font_t** fonts;
  uint16_t cf;
  uint16_t cb;
  uint16_t cfr;
  uint8_t fs;
  int8_t marge;
  int8_t pad;
  uint8_t fid;
  uint8_t o;
} W_textr;

typedef struct {
  int16_t ps;
  uint16_t ceven;
  uint16_t codd;
  uint8_t count;
  uint8_t max_count;
  bool bmode;
} W_blinker;

typedef struct {
  uint8_t (*image_function)();
  uint16_t* image_data;
  uint8_t* index;
  uint8_t max_index;
} W_imagef;

#define SPT_INT8 1
#define SPT_INT16 2
#define SPT_CHAR 3

typedef struct {
  uint8_t pos;
  uint8_t nc;
  void** e;
  char* (*get_text)(uint8_t i);
  font_t** fonts;
  uint16_t cf;
  uint16_t cb;
  uint16_t cfr;
  int16_t fpos;
  int16_t velocity;
  bool moved;
  bool fixed;
  uint8_t fid;
} W_spinner;


typedef struct {
  uint16_t frame;
  uint16_t frames;

  int16_t x0; //start
  int16_t y0;

  int16_t x1; //way point
  int16_t y1;

  int16_t x2; //end
  int16_t y2;

  int16_t ax;
  int16_t ay;
  int16_t bx;
  int16_t by;

  int16_t* x;
  int16_t* y;
  bool done;
} WBez2_t;

typedef struct {
  W* w;
  WBez2_t* b;
  int16_t xo;
  int16_t yo;
} WMove_t;

typedef struct {
  W** wl;
  uint16_t index;
  uint16_t nc;
  int16_t deg;
} W_circmenu;


void init_root();
bool wadd(W* p, W* ch);

int16_t wfind(W* p, W* ch);
W* wfindxy(W* w, int16_t x, int16_t y);
void wdraw(W* w);
bool resize(W* box, uint16_t new_size);
void wset_st(W* w, W_st st);

//W* wnew();
W* wadd_box(W* p, int16_t x, int16_t y, int16_t w, int16_t h);
W* wadd_image(W* p, uint16_t* data, int16_t x, int16_t y, int16_t w, int16_t h);
W* wadd_imager(W* p, uint16_t* (*get_image)(), int16_t x, int16_t y, int16_t w, int16_t h);
W* wadd_imagef(W* p, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t* data, uint8_t (*image_function)(), uint8_t* index, uint8_t max_index);
W* wadd_text(W* p, int16_t x, int16_t y, int16_t w, int16_t h,
  char* text, font_t** fonts, uint16_t cf, uint16_t cb, uint16_t cfr,uint8_t fs, uint8_t fid, uint8_t o);

W* wadd_textr(W* p, int16_t x, int16_t y, int16_t w, int16_t h,
  char* (*get_value)(), font_t** fonts, uint16_t cf, uint16_t cb, uint16_t cfr,uint8_t fs, uint8_t fid, uint8_t o);

W* wadd_none(W* p, void (*do_func)());
W* wadd_blinker(W* p, int16_t x, int16_t y, int16_t w, int16_t h, int16_t ps,
  uint16_t ceven, uint16_t codd, uint8_t max_count, bool mode_rect);
void whide(W* w);
void wshow(W* w);

void whidem(W** w, uint16_t n);
void wshowm(W** w, uint16_t n);


W* wadd_spinner(W*p, int16_t x, int16_t y, int16_t w, int16_t h, W_st st,
 uint8_t pos, uint8_t nc, void** e, char* (*get_text)(uint8_t i), font_t** fonts,
 uint8_t fid, uint16_t cf, uint16_t cb, uint16_t cfr);

void wspinner_draw(W* w);
void wspinner_draw_h(W* w);
void wspinner_set(W* w, uint8_t i); //set pos, then set fpos by pos
void wspinner_set_h(W* w, uint8_t i);
void wspinner_set_max(W* w, uint8_t i);
void wspinner_setfp(W* w);  // set pos by fpos
void wspinner_setfp_h(W* w);
void wspinner_adjust(W_spinner* wsp);
void wspinner_adjust_h(W_spinner* wsp);

WMove_t* wmove_make(WBez2_t* b, W* w);

bool wbez2_next(WBez2_t* b);
WBez2_t* wbez2_make(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t fr);
void wbez2_del(WBez2_t* b);
void wbez2_reset(WBez2_t* b);
void wbez2_loop(WBez2_t* b);
void wbez2_movem(WBez2_t* b, W* w);
bool wbez2_move (WBez2_t* b, W* w);
bool wbez2_mover(WBez2_t* b, W* w);

#endif
