#ifndef __GCLCD_H
#define __GCLCD_H


#include <stdlib.h>
#include "stdio.h"
#include "hardware/adc.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include <math.h>

#define SPI_PORT spi1
#define I2C_PORT i2c1
/**
 * GPIOI config
 **/

#define LCD_DC_PIN 8
#define LCD_CS_PIN 9
#define LCD_CLK_PIN 10
#define LCD_MOSI_PIN 11
#define LCD_RST_PIN 12
#define LCD_BL_PIN 25

#define DEV_SDA_PIN     (6)
#define DEV_SCL_PIN     (7)

#define BAT_ADC_PIN     (29)
#define BAR_CHANNEL     (3)

#define LCD_W 240
#define LCD_H 240
#define LCD_W2 120
#define LCD_H2 120
#define LCD_SZ LCD_W*LCD_H*2

#define HORIZONTAL 0
#define VERTICAL   1

#define WHITE          0xFFFF
#define BLACK          0x0000
#define BLUE           0x001F
#define BRED           0XF81F
#define GRED           0XFFE0
#define ORANGE         0XFE20
#define GBLUE          0X07FF
#define RED            0xF800
#define MAGENTA        0xF81F
#define GREEN          0x07E0
#define CYAN           0x7FFF
#define YELLOW         0xFFE0
#define BROWN          0XBC40
#define BRRED          0XFC07
#define GRAY           0X8430
#define LGRAY          0X8551
#define NBLACK         0x0821
#define NWHITE         0xFFFE

#define IMAGE_BACKGROUND    WHITE
#define FONT_FOREGROUND     BLACK
#define FONT_BACKGROUND     WHITE

#define PI 3.14159265
#define PIdi (3.14159265/180)

#define DEGS 1024
#define QDEG DEGS/4
#define mdeg (DEGS/60)
#define sdeg (DEGS/60)
#define hdeg (DEGS/12)

#define to_rad(angleInDegrees) ((angleInDegrees) * M_PI / 180.0f)
#define to_deg(angleInRadians) ((angleInRadians) * 180.0f / M_PI)


typedef struct _tFont
{
  const uint8_t *data;
  uint16_t w;
  uint16_t h;
} sFONT;

#define BX 120
#define BY 120

typedef struct Bez2_t {
  uint16_t frame;
  uint16_t frames;
  uint16_t color;
  uint16_t ps;

  int16_t x0;
  int16_t y0;
  int16_t x1;
  int16_t y1;

  int16_t x2;
  int16_t y2;
  int16_t x3;
  int16_t y3;

  int16_t ax;
  int16_t ay;
  int16_t bx;
  int16_t by;


  int16_t x;
  int16_t y;
  bool init;
} Bez2_t;

typedef struct Vec2{
  int16_t x;
  int16_t y;
} Vec2;

void lcd_init();

void lcd_gpio_init();
void lcd_module_init();
void lcd_reset();
void lcd_setatt(uint8_t scandir);
void lcd_set_brightness(uint8_t value);
void lcd_cmd(uint8_t reg);
void lcd_data(uint8_t data);
void lcd_datan(uint8_t* data, uint32_t size);
void lcd_init_reg();
void lcd_setwin(uint8_t xs, uint8_t ys, uint8_t xe, uint8_t ye);
void lcd_clr(uint16_t color);
void lcd_display(uint8_t*image);
void lcd_displaypart(uint8_t xs, uint8_t ys, uint8_t xe, uint8_t ye, uint8_t* image);
void lcd_pixel(uint16_t X, uint16_t Y, uint16_t color);
void lcd_setimg(uint16_t* image);
void lcd_copyalpha(uint16_t* dst, uint16_t* src, uint8_t xs, uint8_t ys, uint16_t alpha);
void lcd_blit(uint8_t x, uint8_t y, uint8_t xs, uint8_t ys, uint16_t alpha, const uint8_t* src);
void lcd_line(uint8_t xs, uint8_t ys, uint8_t xe, uint8_t ye, uint16_t color, uint8_t ps);
void lcd_aline(uint8_t xs, uint8_t ys, uint8_t xe, uint8_t ye, uint16_t color, uint8_t ps);
void lcd_char(uint8_t x, uint8_t y, uint8_t c, sFONT* font, uint16_t cf, uint16_t cb, bool cn);
void lcd_str(uint8_t x, uint8_t y, char* data, sFONT* font, uint16_t cf, uint16_t cb);
void lcd_strc(uint8_t x, uint8_t y, char* data, sFONT* font, uint16_t cf, uint16_t cb);
void lcd_string(uint8_t x, uint8_t y, char* data ,sFONT* font, bool cn, uint16_t cf, uint16_t cb);
void lcd_number(uint8_t x, uint8_t y, uint32_t n ,sFONT* font, uint16_t cf, uint16_t cb);
void lcd_float(uint8_t x, uint8_t y, float f ,sFONT* font, uint16_t cf, uint16_t cb);
void lcd_floats(uint8_t x, uint8_t y, float f ,sFONT* font, uint16_t cf, uint16_t cb, bool columns);
void lcd_floatshort(uint8_t x, uint8_t y, float f ,sFONT* font, uint16_t cf, uint16_t cb);
void lcd_sleepon();
void lcd_sleepoff();

void lcd_pixel_raw(uint16_t x, uint16_t y, uint16_t c); // no bswap color
void lcd_pixel_raw_save(uint16_t x, uint16_t y, uint16_t c);
void lcd_pixel_rawps(uint16_t x, uint16_t y, uint16_t c, uint16_t ps); //no swap with pointstrength
void lcd_circle(uint16_t X_Center, uint16_t Y_Center, uint16_t Radius, uint16_t Color, uint16_t ps, bool fill);
void lcd_frame(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color, uint8_t ps);
void lcd_xline(uint8_t x, uint8_t y, uint8_t l, uint16_t color, uint8_t ps);
void lcd_yline(uint8_t x, uint8_t y, uint8_t l, uint16_t color, uint8_t ps);
void lcd_xlineq(uint16_t x, uint16_t y, uint16_t l, uint16_t c);
uint16_t lcd_colrgb(uint8_t r, uint8_t g, uint8_t b);
void lcd_bez2curve(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t fr, uint16_t color, uint16_t ps);
void lcd_bez2curvet(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t fr, uint16_t color, uint16_t ps);
void lcd_bez2curvet16(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t fr, uint16_t color, uint16_t ps);
Bez2_t* lcd_bez2test(Bez2_t* bez, int16_t x0, int16_t y0, int16_t x1, int16_t y1,
  int16_t x2, int16_t y2, int16_t x3, int16_t y3, int16_t fr, uint16_t color, uint16_t ps);
void lcd_bez2l(Bez2_t* b);
void lcd_bez2p(Bez2_t* b,uint16_t color, int16_t ps);
void lcd_bez2circ(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, int16_t fr, uint16_t color, uint16_t ps);
Bez2_t* lcd_bez2initfull(Bez2_t* bez,
    int16_t x0, int16_t y0, int16_t x1, int16_t y1,
    int16_t x2, int16_t y2, int16_t x3, int16_t y3,
    int16_t fr, uint16_t color, uint16_t ps);

void lcd_bez3curve(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, int16_t f, int16_t fr, uint16_t color, uint16_t ps);
void lcd_bez3curvel(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, int16_t f, int16_t fr, uint16_t color, uint16_t ps);

void lcd_bez3circle(int16_t x, int16_t y, int16_t r, int16_t f, int16_t fr, uint16_t color, int16_t ps, int16_t xo, int16_t yo);
void lcd_bez3circ(int16_t x, int16_t y, int16_t r,uint16_t color, int16_t ps, int16_t xo, int16_t yo);

void lcd_bez3curver(int16_t* rx, int16_t* ry, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, int16_t f, int16_t fr);
void lcd_bez2curver(int16_t* rx, int16_t* ry, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t fr);
void lcd_alpha_on();
void lcd_alpha_off();
void lcd_alpha_line(uint8_t xs, uint8_t ys, uint8_t xe, uint8_t ye, uint16_t color, int16_t ps);
void lcd_apixel_raw(uint16_t X, uint16_t Y, uint16_t color);
uint16_t lcd_darker(uint16_t c);
void lcd_make_cosin();
void lcd_line_deg(Vec2 vs, int16_t deg, int16_t l, uint16_t color, int16_t ps);
void lcd_alpha_line_deg(Vec2 vs, int16_t deg, int16_t l, uint16_t color, int16_t ps);
void lcd_alpha_line2(Vec2 v0, Vec2 v1, Vec2 v2, Vec2 v3, uint16_t color);
Vec2* lcd_linev2list(Vec2 ve, int16_t* rsret);
void lcd_linev2(Vec2 vs, Vec2 ve, uint16_t color, int16_t ps);

Vec2 gvdl(int16_t deg, int16_t l);
void gxyld(int16_t* x, int16_t* y, uint16_t l, uint16_t deg);
Vec2 gvdl(int16_t deg, int16_t l);
int16_t gdeg(int16_t d);
void fxyd(float* x, float* y, int16_t deg);

Vec2 vadd(Vec2 a, Vec2 b);
Vec2 vsub(Vec2 a, Vec2 b);
Vec2 vval(Vec2 a);
Vec2 vrot(Vec2 v, int16_t deg);
void vrotv(Vec2* v, int16_t deg);
Vec2 vset(int16_t x, int16_t y);
void lcd_roto(const uint8_t* src, int16_t w, int16_t h);
void lcd_rotoa();
void lcd_blit_deg(Vec2 vs, Vec2 ve, Vec2 vts, int16_t deg, const uint8_t* src, uint16_t alpha,bool centric);
int16_t chkdeg(int16_t d);
void lcd_dither(uint16_t x, uint16_t y, uint16_t sz);
void lcd_magnify(uint8_t sx, uint8_t sy, uint8_t sz, uint8_t mx, uint8_t my, uint8_t mf);
//void lcd_blit_deg2(Vec2 vs, Vec2 ve, Vec2 vts, Vec2 pos, int16_t deg, const uint8_t* src, uint16_t alpha);
// origin , uv-size, texture-size
void lcd_blit_deg2(Vec2 vo, Vec2 vuv, Vec2 vs, int16_t deg, const uint8_t* src, uint16_t alpha, bool centric);
Vec2* lcd_linev2list2(Vec2 vs, Vec2 ve, int16_t* rsret);
extern uint8_t slice_num;
#endif //__GC9A01_H
