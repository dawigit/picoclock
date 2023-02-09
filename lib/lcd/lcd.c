#include "lcd.h"

static const uint8_t SPEED = 2;
static const uint8_t PIXEL_SIZE = 2;
static uint16_t angle;

uint16_t* img=NULL;
uint8_t slice_num;
bool lcd_alpha=false;
bool test_init = true;
uint16_t test_color;
int16_t test_f=0;
int16_t test_fr=0;
int16_t test_ps=0;

int16_t test_ax;
int16_t test_ay;
int16_t test_bx;
int16_t test_by;

int16_t test_x0;
int16_t test_y0;
int16_t test_x1;
int16_t test_y1;
int16_t test_x2;
int16_t test_y2;

int16_t test_txa;
int16_t test_tya;
int16_t test_txb;
int16_t test_tyb;
int16_t test_cx;
int16_t test_cy;
int16_t test_x;
int16_t test_y;

//uint8_t* b0=NULL; //db
//uint8_t* b1=NULL; //db

uint16_t pbuf[64*64*2];
char cbuf[32];

float tcos[DEGS] = {0};
float tsin[DEGS] = {0};

void lcd_init(){
  lcd_module_init();
  printf("module init ok\n");
  lcd_reset();
  sleep_ms(100);
  lcd_setatt(HORIZONTAL);
  lcd_init_reg();
}

void lcd_gpio_init(){
  // GPIO Config
  gpio_init(LCD_RST_PIN);
  gpio_init(LCD_DC_PIN);
  gpio_init(LCD_CS_PIN);
  gpio_init(LCD_BL_PIN);
  gpio_set_dir(LCD_RST_PIN, 1);
  gpio_set_dir(LCD_DC_PIN,  1);
  gpio_set_dir(LCD_CS_PIN,  1);
  gpio_set_dir(LCD_BL_PIN,  1);

  gpio_put(LCD_CS_PIN, 1);
  gpio_put(LCD_DC_PIN, 0);
  gpio_put(LCD_BL_PIN, 1);
}

void lcd_module_init(){
  lcd_gpio_init();
  // ADC
  adc_init();
  adc_gpio_init(BAT_ADC_PIN);
  adc_select_input(BAR_CHANNEL);
  // PWM Config
  gpio_set_function(LCD_BL_PIN, GPIO_FUNC_PWM);
  slice_num = pwm_gpio_to_slice_num(LCD_BL_PIN);
  pwm_set_wrap(slice_num, 100);
  pwm_set_chan_level(slice_num, PWM_CHAN_B, 0);
  pwm_set_clkdiv(slice_num, 50);
  pwm_set_enabled(slice_num, true);
  // SPI Config
  spi_init(SPI_PORT, 40000 * 1000);
  gpio_set_function(LCD_CLK_PIN, GPIO_FUNC_SPI);
  gpio_set_function(LCD_MOSI_PIN, GPIO_FUNC_SPI);
  // I2C Config
  i2c_init(I2C_PORT, 400 * 1000);
  gpio_set_function(DEV_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(DEV_SCL_PIN, GPIO_FUNC_I2C);
  gpio_pull_up(DEV_SDA_PIN);
  gpio_pull_up(DEV_SCL_PIN);
  printf("DEV_Module_Init OK \r\n");
}


void lcd_setatt(uint8_t scandir){

  uint8_t MemoryAccessReg=0x08;
  switch(scandir){
    case 0: MemoryAccessReg=0x08;break;
    case 1: MemoryAccessReg=0x68;break;
    case 2: MemoryAccessReg=0xc8;break;
    case 3: MemoryAccessReg=0xa8;break;
  }
  lcd_cmd(0x36); //MX, MY, RGB mode
  lcd_data(MemoryAccessReg);	//0x08 set RGB
}

void lcd_set_brightness(uint8_t value){
  if(value>100 || value < 0){value=100;}
  pwm_set_chan_level(slice_num, PWM_CHAN_B, value);
}

void lcd_reset(){
    gpio_put(LCD_RST_PIN, 1);
    sleep_ms(100);
    gpio_put(LCD_RST_PIN, 0);
    sleep_ms(100);
    gpio_put(LCD_RST_PIN, 1);
	  gpio_put(LCD_CS_PIN, 0);
    sleep_ms(100);
}

void lcd_cmd(uint8_t reg){
    gpio_put(LCD_DC_PIN, 0);
    spi_write_blocking(SPI_PORT, &reg, 1);
}

void lcd_data(uint8_t data){
    gpio_put(LCD_DC_PIN, 1);
    spi_write_blocking(SPI_PORT, &data, 1);
}

void lcd_datan(uint8_t* data, uint32_t size){
    gpio_put(LCD_DC_PIN, 1);
    spi_write_blocking(SPI_PORT, &data[0], size);
}


void lcd_init_reg(){
  lcd_cmd(0xEF);
	lcd_cmd(0xEB);
	lcd_data(0x14);

  lcd_cmd(0xFE);
	lcd_cmd(0xEF);

	lcd_cmd(0xEB);
	lcd_data(0x14);

	lcd_cmd(0x84);
	lcd_data(0x40);

	lcd_cmd(0x85);
	lcd_data(0xFF);

	lcd_cmd(0x86);
	lcd_data(0xFF);

	lcd_cmd(0x87);
	lcd_data(0xFF);

	lcd_cmd(0x88);
	lcd_data(0x0A);

	lcd_cmd(0x89);
	lcd_data(0x21);

	lcd_cmd(0x8A);
	lcd_data(0x00);

	lcd_cmd(0x8B);
	lcd_data(0x80);

	lcd_cmd(0x8C);
	lcd_data(0x01);

	lcd_cmd(0x8D);
	lcd_data(0x01);

	lcd_cmd(0x8E);
	lcd_data(0xFF);

	lcd_cmd(0x8F);
	lcd_data(0xFF);


	lcd_cmd(0xB6);
	lcd_data(0x00);
	lcd_data(0x20);

	lcd_cmd(0x36);
	lcd_data(0x08);//bgr

	lcd_cmd(0x3A);
	lcd_data(0x55);//16bits/pixel


	lcd_cmd(0x90);
	lcd_data(0x08);
	lcd_data(0x08);
	lcd_data(0x08);
	lcd_data(0x08);

	lcd_cmd(0xBD);
	lcd_data(0x06);

	lcd_cmd(0xBC);
	lcd_data(0x00);

	lcd_cmd(0xFF);
	lcd_data(0x60);
	lcd_data(0x01);
	lcd_data(0x04);

	lcd_cmd(0xC3);
	lcd_data(0x13);
	lcd_cmd(0xC4);
	lcd_data(0x13);

	lcd_cmd(0xC9);
	lcd_data(0x22);

	lcd_cmd(0xBE);
	lcd_data(0x11);

	lcd_cmd(0xE1);
	lcd_data(0x10);
	lcd_data(0x0E);

	lcd_cmd(0xDF);
	lcd_data(0x21);
	lcd_data(0x0c);
	lcd_data(0x02);

	lcd_cmd(0xF0);
	lcd_data(0x45);
	lcd_data(0x09);
	lcd_data(0x08);
	lcd_data(0x08);
	lcd_data(0x26);
 	lcd_data(0x2A);

 	lcd_cmd(0xF1);
 	lcd_data(0x43);
 	lcd_data(0x70);
 	lcd_data(0x72);
 	lcd_data(0x36);
 	lcd_data(0x37);
 	lcd_data(0x6F);


 	lcd_cmd(0xF2);
 	lcd_data(0x45);
 	lcd_data(0x09);
 	lcd_data(0x08);
 	lcd_data(0x08);
 	lcd_data(0x26);
 	lcd_data(0x2A);

 	lcd_cmd(0xF3);
 	lcd_data(0x43);
 	lcd_data(0x70);
 	lcd_data(0x72);
 	lcd_data(0x36);
 	lcd_data(0x37);
 	lcd_data(0x6F);

	lcd_cmd(0xED);
	lcd_data(0x1B);
	lcd_data(0x0B);

	lcd_cmd(0xAE);
	lcd_data(0x77);

	lcd_cmd(0xCD);
	lcd_data(0x63);


	lcd_cmd(0x70);
	lcd_data(0x07);
	lcd_data(0x07);
	lcd_data(0x04);
	lcd_data(0x0E);
	lcd_data(0x0F);
	lcd_data(0x09);
	lcd_data(0x07);
	lcd_data(0x08);
	lcd_data(0x03);

	lcd_cmd(0xE8);
	lcd_data(0x34);

	lcd_cmd(0x62);
	lcd_data(0x18);
	lcd_data(0x0D);
	lcd_data(0x71);
	lcd_data(0xED);
	lcd_data(0x70);
	lcd_data(0x70);
	lcd_data(0x18);
	lcd_data(0x0F);
	lcd_data(0x71);
	lcd_data(0xEF);
	lcd_data(0x70);
	lcd_data(0x70);

	lcd_cmd(0x63);
	lcd_data(0x18);
	lcd_data(0x11);
	lcd_data(0x71);
	lcd_data(0xF1);
	lcd_data(0x70);
	lcd_data(0x70);
	lcd_data(0x18);
	lcd_data(0x13);
	lcd_data(0x71);
	lcd_data(0xF3);
	lcd_data(0x70);
	lcd_data(0x70);

	lcd_cmd(0x64);
	lcd_data(0x28);
	lcd_data(0x29);
	lcd_data(0xF1);
	lcd_data(0x01);
	lcd_data(0xF1);
	lcd_data(0x00);
	lcd_data(0x07);

	lcd_cmd(0x66);
	lcd_data(0x3C);
	lcd_data(0x00);
	lcd_data(0xCD);
	lcd_data(0x67);
	lcd_data(0x45);
	lcd_data(0x45);
	lcd_data(0x10);
	lcd_data(0x00);
	lcd_data(0x00);
	lcd_data(0x00);

	lcd_cmd(0x67);
	lcd_data(0x00);
	lcd_data(0x3C);
	lcd_data(0x00);
	lcd_data(0x00);
	lcd_data(0x00);
	lcd_data(0x01);
	lcd_data(0x54);
	lcd_data(0x10);
	lcd_data(0x32);
	lcd_data(0x98);

	lcd_cmd(0x74);
	lcd_data(0x10);
	lcd_data(0x85);
	lcd_data(0x80);
	lcd_data(0x00);
	lcd_data(0x00);
	lcd_data(0x4E);
	lcd_data(0x00);

  lcd_cmd(0x98);
	lcd_data(0x3e);
	lcd_data(0x07);

	lcd_cmd(0x35);
	lcd_cmd(0x21);

	lcd_cmd(0x11);
	sleep_ms(120);
	lcd_cmd(0x29);
	sleep_ms(20);
}



void lcd_setwin(uint8_t xs, uint8_t ys, uint8_t xe, uint8_t ye){
  //set the X coordinates
    lcd_cmd(0x2A);
    lcd_data(0x00);
    lcd_data(xs);
	  lcd_data(0x00);
    lcd_data(xe);

  //set the Y coordinates
    lcd_cmd(0x2B);
    lcd_data(0x00);
	  lcd_data(ys);
	  lcd_data(0x00);
    lcd_data(ye);

    lcd_cmd(0X2C);
}

void lcd_clr(uint16_t color){
    int j;
    uint16_t* p=(uint16_t*)img;
    __builtin_bswap16(color);
    for (j=0;j<LCD_W*LCD_H;j++){ p[j]=color; }
    lcd_setwin(0, 0, LCD_W, LCD_H);
    gpio_put(LCD_DC_PIN, 1);
    for(j = 0; j < LCD_H-1; j++){
        spi_write_blocking(SPI_PORT, (uint8_t *)&p[j*LCD_W], LCD_W*2);
    }
}

void lcd_display(uint8_t* image){
    lcd_setwin(0, 0, LCD_W-1, LCD_H-1);
    gpio_put(LCD_DC_PIN, 1);
    spi_write_blocking(SPI_PORT,image,LCD_SZ);
}

void lcd_displaypart(uint8_t xs, uint8_t ys, uint8_t xe, uint8_t ye, uint8_t* image){
    lcd_setwin(xs, ys, xe , ye);
    gpio_put(LCD_DC_PIN, 1);
    spi_write_blocking(SPI_PORT,image,(xe-xs)*(ye-ys)*2);
}

void lcd_pixel(uint16_t X, uint16_t Y, uint16_t color){
    __builtin_bswap16(color);
    lcd_setwin(X,Y,X,Y);
    lcd_datan((uint8_t*)&color,2);
}

void lcd_setimg(uint16_t* image){
  img = image;
}

void lcd_blit(uint8_t x, uint8_t y, uint8_t xs, uint8_t ys, uint16_t alpha, const uint8_t* src){
  //__builtin_bswap16(alpha);
  uint16_t* s = (uint16_t*)src;
  uint32_t o = y*LCD_W+x;
  uint32_t i=0;
  for(uint16_t iy=0;iy<ys;iy++){
    for(uint16_t ix=0;ix<xs;ix++){
      if(s[i]!=alpha){ img[(o+ix)]=s[i]; }
      i++;
    }
    o+=LCD_W;
  }
}

void lcd_copyalpha(uint16_t* dst, uint16_t* src, uint8_t xs, uint8_t ys, uint16_t alpha){
  uint32_t o=0,i=0;
  //__builtin_bswap16(alpha);
  for(uint16_t iy=0;iy<ys;iy++){
    for(uint16_t ix=0;ix<xs;ix++){
      if(src[i]!=alpha){dst[(o+ix)]=src[i];}
      ++i;
    }
    o+=LCD_W;
  }
}

void lcd_line(uint8_t xs, uint8_t ys, uint8_t xe, uint8_t ye, uint16_t color, uint8_t ps){
  //printf("L: %d %d %d %d [%04x] %d\n",xs,ys,xe,ye,color,ps);
  uint8_t t;
  int16_t px=xs,py=ys;
  int dx = (int)xe - (int)xs >= 0 ? xe - xs : xs - xe;
  int dy = (int)ye - (int)ys <= 0 ? ye - ys : ys - ye;
  int XAddway = xs < xe ? 1 : -1;
  int YAddway = ys < ye ? 1 : -1;
  int esp = dx + dy;
  color = __builtin_bswap16(color);
  while(true){
    //img[py*LCD_W+px]=color;
    lcd_pixel_rawps(px,py,color,ps);
    if(2*esp >= dy) {
      if (px == xe){break;}
      esp += dy;
      px += XAddway;
    }
    if(2*esp <= dx) {
      if (py == ye){break;}
      esp += dx;
      py += YAddway;
    }
  }
}

void lcd_char(uint8_t x, uint8_t y, uint8_t c, sFONT* font, uint16_t cf, uint16_t cb, bool cn){
  uint16_t fw = font->w;
	uint16_t fh = font->h;
	uint32_t size = fw*fh;
  if(!cn){c-=' ';}
  uint32_t offset = c * fh * (fw / 8 + ((fw%8)?1:0));
	const unsigned char *ptr = &font->data[offset];
  uint32_t i,j,yo;
	uint8_t cd;
	j=0;
	cd=0;
	for(yo=0;yo<fh;yo++){
		cd = *ptr;
		for(i=0;i<fw;i++){
			if(cd & 0x80){	pbuf[j] = cf;
			}else{					pbuf[j] = cb; }
			j++;
			cd<<=1;
			if(i % 8 == 7){ ++ptr; cd = *ptr;}
		}
		++ptr;
	}
  lcd_blit(x,y,font->w,font->h,cb,(const uint8_t*)pbuf);
}
void lcd_string(uint8_t x, uint8_t y, char* data, sFONT* font,bool cn, uint16_t cf, uint16_t cb){
  uint8_t c,px=x,py=y;
  cf = __builtin_bswap16(cf);
  cb = __builtin_bswap16(cb);

  while(*data){
    c=*data;
    //printf("lcd_string: '%c'\n",c);
    lcd_char(px,py,c,font,cf,cb,cn);
    if(cn){      py+=font->h;
    }else{      px+=font->w;    }
    ++data;
  }
}

void lcd_str(uint8_t x, uint8_t y, char* data, sFONT* font, uint16_t cf, uint16_t cb){
  //cf = __builtin_bswap16(cf);
  //cb = __builtin_bswap16(cb);
  lcd_string(x,y,data,font,false,cf,cb);
}

void lcd_strc(uint8_t x, uint8_t y, char* data, sFONT* font, uint16_t cf, uint16_t cb){
  //cf = __builtin_bswap16(cf);
  //cb = __builtin_bswap16(cb);
  lcd_string(x,y,data,font,true,cf,cb);
}

void lcd_number(uint8_t x, uint8_t y, uint32_t n ,sFONT* font, uint16_t cf, uint16_t cb){
  sprintf(cbuf,"%d\0",n);
  lcd_str(x,y,&cbuf[0],font,cf,cb);
}

void lcd_float(uint8_t x, uint8_t y, float f ,sFONT* font, uint16_t cf, uint16_t cb){
  lcd_floats(x,y,f,font,cf,cb,false);
}
void lcd_floatshort(uint8_t x, uint8_t y, float f ,sFONT* font, uint16_t cf, uint16_t cb){
  lcd_floats(x,y,f,font,cf,cb,true);
}

void lcd_floats(uint8_t x, uint8_t y, float f ,sFONT* font, uint16_t cf, uint16_t cb, bool column){
  if(column){ // 4 columns@all
    sprintf(cbuf,"%+4.2f\0",f);
  }else{ // 8 columns@all
    sprintf(cbuf,"%+8.2f\0",f);
  }
  lcd_str(x,y,&cbuf[0],font,cf,cb);
}

void lcd_sleepon(){  lcd_cmd(0x10);}
void lcd_sleepoff(){  lcd_cmd(0x11);}

inline uint16_t lcd_darker(uint16_t c){
    c=__builtin_bswap16(c);
    c&=0b1111011111011110;
    c>>=1;
    return __builtin_bswap16(c);
}
inline uint16_t lcd_lighter(uint16_t c){
  uint16_t cr=(c&0xf800)|0x1800;
  uint16_t cg=(c&0x07e0)|0x0120;
  uint16_t cb=(c&0x001f)|0x0003;
  return (((cr<<2)&0xf800)|0x1800 | ((cg<<2)&0x07e0)|0x0120 | ((cb<<2)&0x001f)|0x0003);
}

inline void lcd_apixel_raw(uint16_t x, uint16_t y, uint16_t c){
  uint16_t p=__builtin_bswap16(img[LCD_W*y+x]);
  if(c==BLACK){     c = lcd_darker(p);}
  else if(p==WHITE){ c = c^p;}
  else{             c = (p|c);}
  img[LCD_W*y+x] = __builtin_bswap16(c);
}

inline void lcd_pixel_raw(uint16_t x, uint16_t y, uint16_t c){
  img[LCD_W*y+x] = c;
}

inline void lcd_pixel_raw_save(uint16_t x, uint16_t y, uint16_t c){
  if(x<240&&y<240){
    img[LCD_W*y+x] = c;
  }
}


inline void lcd_xlineq(uint16_t x, uint16_t y, uint16_t l, uint16_t c){
  uint16_t i=0;
  while(i<l){ img[LCD_W*y+x+i] = c;++i; }
}

inline void lcd_pixel_rawps(uint16_t x, uint16_t y, uint16_t c, uint16_t ps){
  if(x>=LCD_W){x=LCD_W-1;}
  if(y>=LCD_H){y=LCD_H-1;}
  uint16_t i=0;
  x-=(ps>>1);
  y-=(ps>>1);
  while(i<ps){    lcd_xlineq(x,y+i,ps,c);++i;  }

}

void lcd_circle(uint16_t X_Center, uint16_t Y_Center, uint16_t Radius, uint16_t Color, uint16_t ps, bool fill)
{
    int16_t XCurrent, YCurrent;
    XCurrent = 0;
    YCurrent = Radius;
    Color = __builtin_bswap16(Color);
    int16_t Line_width = ps;
    int16_t Esp = 3 - (Radius << 1 );
    int16_t sCountY;
    if (fill == true) {
        while (XCurrent <= YCurrent ) { //Realistic circles
            for (sCountY = XCurrent; sCountY <= YCurrent; sCountY ++ ) {
                lcd_pixel_raw(X_Center + XCurrent, Y_Center + sCountY, Color);//1
                lcd_pixel_raw(X_Center - XCurrent, Y_Center + sCountY, Color);//2
                lcd_pixel_raw(X_Center - sCountY, Y_Center + XCurrent, Color);//3
                lcd_pixel_raw(X_Center - sCountY, Y_Center - XCurrent, Color);//4
                lcd_pixel_raw(X_Center - XCurrent, Y_Center - sCountY, Color);//5
                lcd_pixel_raw(X_Center + XCurrent, Y_Center - sCountY, Color);//6
                lcd_pixel_raw(X_Center + sCountY, Y_Center - XCurrent, Color);//7
                lcd_pixel_raw(X_Center + sCountY, Y_Center + XCurrent, Color);
            }
            if (Esp < 0 ){                Esp += 4 * XCurrent + 6;
            }else {
                Esp += 10 + 4 * (XCurrent - YCurrent );
                YCurrent --;
            }
            XCurrent ++;
        }
    } else { //Draw a hollow circle
        while (XCurrent <= YCurrent ) {
            lcd_pixel_rawps(X_Center + XCurrent, Y_Center + YCurrent, Color, Line_width);//1
            lcd_pixel_rawps(X_Center - XCurrent, Y_Center + YCurrent, Color, Line_width);//2
            lcd_pixel_rawps(X_Center - YCurrent, Y_Center + XCurrent, Color, Line_width);//3
            lcd_pixel_rawps(X_Center - YCurrent, Y_Center - XCurrent, Color, Line_width);//4
            lcd_pixel_rawps(X_Center - XCurrent, Y_Center - YCurrent, Color, Line_width);//5
            lcd_pixel_rawps(X_Center + XCurrent, Y_Center - YCurrent, Color, Line_width);//6
            lcd_pixel_rawps(X_Center + YCurrent, Y_Center - XCurrent, Color, Line_width);//7
            lcd_pixel_rawps(X_Center + YCurrent, Y_Center + XCurrent, Color, Line_width);//0

            if (Esp < 0 ){ Esp += 4 * XCurrent + 6;
            }else {
                Esp += 10 + 4 * (XCurrent - YCurrent );
                YCurrent --;
            }
            XCurrent ++;
        }
    }
}

inline void lcd_xline(uint8_t x, uint8_t y, uint8_t l, uint16_t color, uint8_t ps){
  while(l--){
    uint8_t tps=ps;
    while(tps--){      img[LCD_W*(y+tps)+x+l]=color;    }
  }
}

inline void lcd_yline(uint8_t x, uint8_t y, uint8_t l, uint16_t color, uint8_t ps){
  while(l--){
    uint8_t tps=ps;
    while(tps--){      img[LCD_W*(y+l)+x+tps]=color;    }
  }
}

void lcd_frame(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color, uint8_t ps){
    color=__builtin_bswap16(color);
    lcd_xline(x0,y0,x1-x0,color,ps);
    lcd_xline(x0,y1-ps,x1-x0,color,ps);
    lcd_yline(x0,y0,y1-y0,color,ps);
    lcd_yline(x1-ps,y0,y1-y0,color,ps);
}

uint16_t lcd_colrgb(uint8_t r, uint8_t g, uint8_t b){
  return (uint16_t)((r>>3)<<11)+((g>>2)<<5)+(b>>3);
}



// used for testing, now playground
// based on the Wikipedia animations


Bez2_t bez = {
  0,64,
  10,10,90,30,
  90,30,30,90,
  0,0,0,0,0,0
};

void bez2init(Bez2_t* b){
  b->ax = b->x2-b->x0;
  b->ay = b->y2-b->y0;
  b->bx = b->x3-b->x1;
  b->by = b->y3-b->y1;
}

Bez2_t* lcd_bez2initfull(Bez2_t* bez,
    int16_t x0, int16_t y0, int16_t x1, int16_t y1,
    int16_t x2, int16_t y2, int16_t x3, int16_t y3,
    int16_t fr, uint16_t color, uint16_t ps)
{
    if(!bez){
      printf("bez2test init\n");
      printf("bez: %08x\n",bez);
      bez=malloc(sizeof(Bez2_t));
      if(!bez){printf("kaa bezi ned, heast!\n");return NULL;}
      printf("bez: %08x\n",bez);
    }

    color=__builtin_bswap16(color);
    bez->frame = 0;
    bez->frames = fr;
    bez->color = color;
    bez->ps = ps;
    bez->x0 = x0;
    bez->y0 = y0;
    bez->x1 = x1;
    bez->y1 = y1;
    bez->x2 = x2;
    bez->y2 = y2;
    bez->x3 = x3;
    bez->y3 = y3;
    bez->ax = bez->x1-bez->x0;
    bez->ay = bez->y1-bez->y0;
    bez->bx = bez->x3-bez->x2;
    bez->by = bez->y3-bez->y2;
    bez->init = false;
    return bez;
}

Bez2_t* lcd_bez2test(Bez2_t* bez,
    int16_t x0, int16_t y0, int16_t x1, int16_t y1,
    int16_t x2, int16_t y2, int16_t x3, int16_t y3,
    int16_t fr, uint16_t color, uint16_t ps)
{
  if(!bez){
    printf("bez2test init\n");
    printf("bez: %08x\n",bez);
    bez=malloc(sizeof(Bez2_t));
    if(!bez){return NULL;}
    printf("bez: %08x\n",bez);
    bez->init = true;
  }
  color=__builtin_bswap16(color);
  bez->color = color;
  if(bez->init){
    bez->frame = 0;
    bez->frames = fr;
    bez->ps = ps;
    bez->x0 = x0;
    bez->y0 = y0;
    bez->x1 = x1;
    bez->y1 = y1;
    bez->x2 = x2;
    bez->y2 = y2;
    bez->x3 = x3;
    bez->y3 = y3;
    bez->ax = bez->x1-bez->x0;
    bez->ay = bez->y1-bez->y0;
    bez->bx = bez->x3-bez->x2;
    bez->by = bez->y3-bez->y2;
    bez->init = false;
  }

  lcd_line(BX+bez->x0,BY+bez->y0,BX+bez->x1,BY+bez->y1,__builtin_bswap16(YELLOW),1);
  lcd_line(BX+bez->x2,BY+bez->y2,BX+bez->x3,BY+bez->y3,__builtin_bswap16(GREEN),1);
  int16_t txa = ((int32_t)(bez->x0+((bez->ax*bez->frame)/bez->frames)));
  int16_t tya = ((int32_t)(bez->y0+((bez->ay*bez->frame)/bez->frames)));
  int16_t txb = ((int32_t)(bez->x2+((bez->bx*bez->frame)/bez->frames)));
  int16_t tyb = ((int32_t)(bez->y2+((bez->by*bez->frame)/bez->frames)));
  lcd_pixel_rawps((uint16_t)BX+txa,(uint16_t)BY+tya,__builtin_bswap16(BLUE),bez->ps);
  lcd_pixel_rawps((uint16_t)BX+txb,(uint16_t)BY+tyb,__builtin_bswap16(CYAN),bez->ps);
  lcd_line(BX+txa,BY+tya,BX+txb,BY+tyb,RED,1);
  int16_t cx = txb - txa;
  int16_t cy = tyb - tya;
  bez->x = ((int32_t)(txa+((cx*bez->frame)/bez->frames)));
  bez->y = ((int32_t)(tya+((cy*bez->frame)/bez->frames)));
  lcd_pixel_rawps((uint16_t)BX+bez->x,(uint16_t)BY+bez->y,bez->color,bez->ps+1);
  bez->frame++;
  if(bez->frame==bez->frames){bez->frame=0;bez->init=true;}
  return bez;
}


void lcd_bez2(Bez2_t* b){
  int16_t t;
  lcd_line(BX+b->x0,BY+b->y0,BX+b->x2,BY+b->y2,YELLOW,1);
  lcd_line(BX+b->x1,BY+b->y1,BX+b->x3,BY+b->y3,GREEN,1);
  int16_t txa = ((int32_t)(b->x0+((b->ax*b->frame)/b->frames)));
  int16_t tya = ((int32_t)(b->y0+((b->ay*b->frame)/b->frames)));
  int16_t txb = ((int32_t)(b->x2+((b->bx*b->frame)/b->frames)));
  int16_t tyb = ((int32_t)(b->y2+((b->by*b->frame)/b->frames)));
  lcd_pixel_rawps((uint16_t)BX+txa,(uint16_t)BY+tya,__builtin_bswap16(BLUE),3);
  lcd_pixel_rawps((uint16_t)BX+txb,(uint16_t)BY+tyb,__builtin_bswap16(CYAN),3);
  lcd_line(BX+txa,BY+tya,BX+txb,BY+tyb,RED,1);
  int16_t cx = txb - txa;
  int16_t cy = tyb - tya;
  b->x = ((int32_t)(txa+((cx*b->frame)/b->frames)));
  b->y = ((int32_t)(tya+((cy*b->frame)/b->frames)));
  lcd_pixel_rawps((uint16_t)BX+b->x,(uint16_t)BY+b->y,__builtin_bswap16(BRED),3);
}

void lcd_bez2l(Bez2_t* b){
  int16_t tf = 0;
  while(tf<b->frames+1){
    int16_t t;
    int16_t txa = ((int32_t)(b->x0+((b->ax*tf)/b->frames)));
    int16_t tya = ((int32_t)(b->y0+((b->ay*tf)/b->frames)));
    int16_t txb = ((int32_t)(b->x2+((b->bx*tf)/b->frames)));
    int16_t tyb = ((int32_t)(b->y2+((b->by*tf)/b->frames)));
    int16_t cx = txb - txa;
    int16_t cy = tyb - tya;
    int16_t x = ((int32_t)(txa+((cx*tf)/b->frames)));
    int16_t y = ((int32_t)(tya+((cy*tf)/b->frames)));
    lcd_pixel_rawps((uint16_t)BX+x,(uint16_t)BY+y,b->color,b->ps+1);

    tf++;
  }
}

void lcd_bez2p(Bez2_t* b,uint16_t color, int16_t ps){
  lcd_pixel_rawps((uint16_t)BX+b->x,(uint16_t)BY+b->y,__builtin_bswap16(color),ps);
}

void lcd_bez2curvet(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t fr, uint16_t color, uint16_t ps){
  lcd_bez2curve(x0,y0,x2,y2,x1,y1,fr,color,ps);
}

void lcd_bez2curve(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t fr, uint16_t color, uint16_t ps){
  color = __builtin_bswap16(color);
  int16_t f=0;
  int16_t ax = x2-x0;
  int16_t ay = y2-y0;
  int16_t bx = x2-x1;
  int16_t by = y2-y1;
  while(f<fr){
    int16_t txa = ((int32_t)(x0+((ax*f)/fr)));
    int16_t tya = ((int32_t)(y0+((ay*f)/fr)));
    int16_t txb = ((int32_t)(x1+((bx*f)/fr)));
    int16_t tyb = ((int32_t)(y1+((by*f)/fr)));
    int16_t cx = txb - txa;
    int16_t cy = tyb - tya;
    int16_t x = ((int32_t)(txa+((cx*f)/fr)));
    int16_t y = ((int32_t)(tya+((cy*f)/fr)));
    lcd_pixel_rawps((uint16_t)BX+x,(uint16_t)BY+y,color,ps);
    ++f;
  }
}

// for testing / better visualization
void lcd_bez3curve(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, int16_t f, int16_t fr, uint16_t color, uint16_t ps){
  color = __builtin_bswap16(color);
  int16_t ax = x1-x0;
  int16_t ay = y1-y0;
  int16_t bx = x2-x1;
  int16_t by = y2-y1;
  int16_t cx = x3-x2;
  int16_t cy = y3-y2;
  lcd_line(BX+x0,BY+y0,BX+x1,BY+y1,RED,1);
  lcd_line(BX+x1,BY+y1,BX+x2,BY+y2,RED,1);
  lcd_line(BX+x2,BY+y2,BX+x3,BY+y3,RED,1);

  int16_t tx0 = (int16_t)(x0+ax*f/fr);
  int16_t ty0 = (int16_t)(y0+ay*f/fr);
  int16_t tx1 = (int16_t)(x1+bx*f/fr);
  int16_t ty1 = (int16_t)(y1+by*f/fr);
  int16_t tx2 = (int16_t)(x2+cx*f/fr);
  int16_t ty2 = (int16_t)(y2+cy*f/fr);

  lcd_pixel_rawps(BX+tx0,BY+ty0,CYAN,3);
  lcd_pixel_rawps(BX+tx1,BY+ty1,CYAN,3);
  lcd_pixel_rawps(BX+tx2,BY+ty2,CYAN,3);

  lcd_line(BX+tx0,BY+ty0,BX+tx1,BY+ty1,BLUE,1);
  lcd_line(BX+tx1,BY+ty1,BX+tx2,BY+ty2,BLUE,1);

  // got 3 points
  int16_t cx0 = tx1 - tx0;
  int16_t cy0 = ty1 - ty0;
  int16_t cx1 = tx2 - tx1;
  int16_t cy1 = ty2 - ty1;

  int16_t tcx0 = ((int32_t)(tx0+((cx0*f)/fr)));
  int16_t tcy0 = ((int32_t)(ty0+((cy0*f)/fr)));
  int16_t tcx1 = ((int32_t)(tx1+((cx1*f)/fr)));
  int16_t tcy1 = ((int32_t)(ty1+((cy1*f)/fr)));

  lcd_line(BX+tcx0,BY+tcy0, BX+tcx1,BY+tcy1, GREEN,1);
  lcd_pixel_rawps((uint16_t)BX+tcx0,(uint16_t)BY+tcy0,color,ps);
  lcd_pixel_rawps((uint16_t)BX+tcx1,(uint16_t)BY+tcy1,color,ps);

  int16_t dx0 = tcx1-tcx0;
  int16_t dy0 = tcy1-tcy0;

  int16_t dtx = ((int32_t)(tcx0+((dx0*f)/fr)));
  int16_t dty = ((int32_t)(tcy0+((dy0*f)/fr)));

  lcd_pixel_rawps((uint16_t)BX+dtx,(uint16_t)BY+dty,color,ps);
}

void lcd_bez3curver(int16_t* rx, int16_t* ry, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, int16_t f, int16_t fr){
  int16_t ax = x1-x0;
  int16_t ay = y1-y0;
  int16_t bx = x2-x1;
  int16_t by = y2-y1;
  int16_t cx = x3-x2;
  int16_t cy = y3-y2;
  int16_t tx0 = (int16_t)(x0+ax*f/fr);
  int16_t ty0 = (int16_t)(y0+ay*f/fr);
  int16_t tx1 = (int16_t)(x1+bx*f/fr);
  int16_t ty1 = (int16_t)(y1+by*f/fr);
  int16_t tx2 = (int16_t)(x2+cx*f/fr);
  int16_t ty2 = (int16_t)(y2+cy*f/fr);
  // got 3 points
  int16_t cx0 = tx1 - tx0;
  int16_t cy0 = ty1 - ty0;
  int16_t cx1 = tx2 - tx1;
  int16_t cy1 = ty2 - ty1;

  int16_t tcx0 = ((int32_t)(tx0+((cx0*f)/fr)));
  int16_t tcy0 = ((int32_t)(ty0+((cy0*f)/fr)));
  int16_t tcx1 = ((int32_t)(tx1+((cx1*f)/fr)));
  int16_t tcy1 = ((int32_t)(ty1+((cy1*f)/fr)));

  int16_t dx0 = tcx1-tcx0;
  int16_t dy0 = tcy1-tcy0;

  *rx = ((int32_t)(tcx0+((dx0*f)/fr)));
  *ry = ((int32_t)(tcy0+((dy0*f)/fr)));

  //lcd_pixel_rawps((uint16_t)BX+dtx,(uint16_t)BY+dty,color,ps);

}

void lcd_bez3curvel(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, int16_t f, int16_t fr, uint16_t color, uint16_t ps){
  color = __builtin_bswap16(color);
  int16_t ax = x1-x0;
  int16_t ay = y1-y0;
  int16_t bx = x2-x1;
  int16_t by = y2-y1;
  int16_t cx = x3-x2;
  int16_t cy = y3-y2;
  while(f<=fr){
  int16_t tx0 = (int16_t)(x0+ax*f/fr);
  int16_t ty0 = (int16_t)(y0+ay*f/fr);
  int16_t tx1 = (int16_t)(x1+bx*f/fr);
  int16_t ty1 = (int16_t)(y1+by*f/fr);
  int16_t tx2 = (int16_t)(x2+cx*f/fr);
  int16_t ty2 = (int16_t)(y2+cy*f/fr);
  // got 3 points
  int16_t cx0 = tx1 - tx0;
  int16_t cy0 = ty1 - ty0;
  int16_t cx1 = tx2 - tx1;
  int16_t cy1 = ty2 - ty1;

  int16_t tcx0 = ((int32_t)(tx0+((cx0*f)/fr)));
  int16_t tcy0 = ((int32_t)(ty0+((cy0*f)/fr)));
  int16_t tcx1 = ((int32_t)(tx1+((cx1*f)/fr)));
  int16_t tcy1 = ((int32_t)(ty1+((cy1*f)/fr)));

  int16_t dx0 = tcx1-tcx0;
  int16_t dy0 = tcy1-tcy0;

  int16_t dtx = ((int32_t)(tcx0+((dx0*f)/fr)));
  int16_t dty = ((int32_t)(tcy0+((dy0*f)/fr)));

  lcd_pixel_rawps((uint16_t)BX+dtx,(uint16_t)BY+dty,color,ps);
  ++f;
  }
}
void lcd_bez3circ(int16_t x, int16_t y, int16_t r,uint16_t color, int16_t ps, int16_t xo, int16_t yo){
  lcd_bez3circle(x-120,y-120,r,0,100,color,ps,xo,yo);
}
void lcd_bez3circle(int16_t x, int16_t y, int16_t r, int16_t f, int16_t fr, uint16_t color, int16_t ps, int16_t xo, int16_t yo){
  lcd_bez3curvel(x-r+(xo>>2),y,  x-r-(xo>>1),y+(r+(r>>1)),  x+r+(xo>>2),y+(r+(r>>1)), x+r+(xo>>1),y ,f,fr,color,ps);
  lcd_bez3curvel(x-r+(xo>>2),y,  x-r-(xo>>1),y-(r+(r>>1)),  x+r+(xo>>2),y-(r+(r>>1)), x+r+(xo>>1),y ,f,fr,color,ps);
}

void lcd_bez2curver(int16_t* rx, int16_t* ry, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t fr){
  int16_t f=0;
  int16_t ax = x2-x0;
  int16_t ay = y2-y0;
  int16_t bx = x2-x1;
  int16_t by = y2-y1;
  int16_t txa = ((int32_t)(x0+((ax*f)/fr)));
  int16_t tya = ((int32_t)(y0+((ay*f)/fr)));
  int16_t txb = ((int32_t)(x1+((bx*f)/fr)));
  int16_t tyb = ((int32_t)(y1+((by*f)/fr)));
  int16_t cx = txb - txa;
  int16_t cy = tyb - tya;
  *rx = ((int32_t)(txa+((cx*f)/fr)));
  *ry = ((int32_t)(tya+((cy*f)/fr)));
}

//from hagl [https://github.com/tuupola/hagl]
void lcd_roto(const uint8_t* src, int16_t w, int16_t h)
{
    float s, c, z;
    fxyd(&c, &s, angle);
    z = 0.5f + s;
    uint16_t* ps=(uint16_t*)src;
    uint16_t col;
    for (uint16_t x = 0; x < LCD_W-1; x = x + PIXEL_SIZE) {
        for (uint16_t y = 0; y < LCD_H-1; y = y + PIXEL_SIZE) {
            /* Get a rotated pixel from the head image. */
            int16_t u = (int16_t)(( x * c - y * s) * z) % w;
            int16_t v = (int16_t)(( x * s + y * c) * z) % h;
            u = abs(u);
            if (v < 0) {                v += h;            }
            uint16_t *color = (uint16_t*) (src + w * sizeof(uint16_t) * v + sizeof(uint16_t) * u);
            col=*color;
            if (1 == PIXEL_SIZE) {
              lcd_pixel_raw(x, y,    col);
            }else{
              lcd_pixel_raw(x, y,    col);
              lcd_pixel_raw(x+1, y,  col);
              lcd_pixel_raw(x+1, y+1,col);
              lcd_pixel_raw(x, y+1,  col);
            }
        }
    }
}


void lcd_rotoa()
{
    angle+=3;
    if(angle>=DEGS){angle-=DEGS;}
}
//from hagl [https://github.com/tuupola/hagl]

void lcd_alpha_on(){lcd_alpha=true;}
void lcd_alpha_off(){lcd_alpha=false;}


void lcd_alpha_line_deg(Vec2 vs, int16_t deg, int16_t l, uint16_t color, int16_t ps){
  Vec2 ve = gvdl(deg,(int16_t)l);
  Vec2* pv=NULL;
  int16_t rs=0;
  pv = lcd_linev2list(ve,&rs);
  //printf("pv = %08x %d\n",pv,rs);
  Vec2 l1 = vsub(pv[1],pv[0]);

  for(int16_t i=0;i<rs;i++){    lcd_apixel_raw(vs.x+pv[i].x,vs.y+pv[i].y,color);  }
  for(int16_t j=1;j<(ps>>1);j++){
    for(int16_t i=0;i<rs-j;i++){
      if(l1.x!=0){
        lcd_apixel_raw(vs.x+pv[i].x,vs.y+pv[i].y+l1.x*j,color);
        lcd_apixel_raw(vs.x+pv[i].x,vs.y+pv[i].y-l1.x*j,color);
      }
      if(l1.y!=0){
        lcd_apixel_raw(vs.x+pv[i].x+l1.y*j,vs.y+pv[i].y,color);
        lcd_apixel_raw(vs.x+pv[i].x-l1.y*j,vs.y+pv[i].y,color);
      }
    }
  }
  free(pv);
  return;
}


void lcd_line_deg(Vec2 vs, int16_t deg, int16_t l, uint16_t color, int16_t ps){
  int16_t px=vs.x;
  int16_t py=vs.y;
  Vec2 ve = gvdl(deg,(int16_t)l);
  ve = vadd(ve,vs);
  int16_t dx = (int16_t)ve.x - (int16_t)vs.x >= 0 ? ve.x - vs.x : vs.x - ve.x;
  int16_t dy = (int16_t)ve.y - (int16_t)vs.y <= 0 ? ve.y - vs.y : vs.y - ve.y;
  int16_t XAddway = vs.x < ve.x ? 1 : -1;
  int16_t YAddway = vs.y < ve.y ? 1 : -1;
  int16_t esp = dx + dy;
  color=__builtin_bswap16(color);
  while(true){
    lcd_pixel_rawps(px,py,color,ps);
    if(2*esp >= dy) {
      if (px == ve.x){break;}
      esp += dy;      px += XAddway;
    }
    if(2*esp <= dx) {
      if (py == ve.y){break;}
      esp += dx;      py += YAddway;
    }
  }
}

void lcd_linev2(Vec2 vs, Vec2 ve, uint16_t color, int16_t ps){
  int16_t px=vs.x;
  int16_t py=vs.y;
  int16_t dx = ve.x - vs.x >= 0 ? ve.x - vs.x : vs.x - ve.x;
  int16_t dy = ve.y - vs.y <= 0 ? ve.y - vs.y : vs.y - ve.y;
  int16_t XAddway = vs.x < ve.x ? 1 : -1;
  int16_t YAddway = vs.y < ve.y ? 1 : -1;
  int16_t esp = dx + dy;
  color=__builtin_bswap16(color);
  while(true){
    lcd_pixel_rawps(px,py,color,ps);
    if(2*esp >= dy) {
      if (px == ve.x){break;}
      esp += dy;      px += XAddway;
    }
    if(2*esp <= dx) {
      if (py == ve.y){break;}
      esp += dx;      py += YAddway;
    }
  }
}


void lcd_make_cosin(){
    float d=0;
    uint16_t i;
    while(i<DEGS){
      tsin[i] = sinf( to_rad(-d-90) );
      tcos[i] = sinf( to_rad(-d-180) );
      d+=(360.0f/DEGS);
      ++i;
    }
}
//float gcosin(uint16_t d){
//  return cosin[d];
//}

inline void gxyld(int16_t* x, int16_t* y, uint16_t l, uint16_t deg){
  *x = (int16_t)(tcos[deg]*l);
  *y = (int16_t)(tsin[deg]*l);
}

inline void fxyd(float* x, float* y, int16_t deg){
  *x = tcos[deg];
  *y = tsin[deg];
}

inline Vec2 gvdl(int16_t deg, int16_t l){
  Vec2 v;
  if(deg>=DEGS){deg-=DEGS;}
  if(deg<0){deg+=DEGS;}
  v.x = (int16_t)(tcos[deg]*l);
  v.y = (int16_t)(tsin[deg]*l);
  //printf("gvdl %d %d %d %d\n",v.x,v.y,deg,l);
  return v;
}

inline int16_t gdeg(int16_t d){
  if(d<0) return d+DEGS;
  if(d>=DEGS)return d-DEGS;
}

inline Vec2 vadd(Vec2 a, Vec2 b){
  a.x+=b.x;
  a.y+=b.y;
  return a;
}
inline Vec2 vsub(Vec2 a, Vec2 b){
  a.x-=b.x;
  a.y-=b.y;
  return a;
}

inline Vec2 vval(Vec2 a){
  if(a.x<0)a.x*=-1;
  if(a.y<0)a.y*=-1;
  return a;
}

inline void vprint(Vec2 v){
  printf("%d %d\n",v.x,v.y);
}

inline Vec2 vrot(Vec2 v, int16_t deg){
  uint16_t d=(uint16_t)deg;
  Vec2 vr = { (int16_t)(v.x * tcos[d] - v.y * tsin[d]), (int16_t)(v.x * tsin[d] + v.y * tcos[d]) };
  return vr;
}

inline void vrotv(Vec2* v, int16_t deg){
  uint16_t d=(uint16_t)deg;
  int16_t x = (int16_t)(v->x * tcos[d] - v->y * tsin[d]);
  int16_t y = (int16_t)(v->x * tsin[d] + v->y * tcos[d]);
  v->x=x;
  v->y=y;
}


inline Vec2 vset(int16_t x,int16_t y){
  Vec2 r ={x,y};
  return r;
}


void lcd_alpha_line(uint8_t xs, uint8_t ys, uint8_t xe, uint8_t ye, uint16_t color, int16_t ps){
  //printf("L: %d %d %d %d [%04x] %d\n",xs,ys,xe,ye,color,ps);
  uint8_t t;
  int16_t px=xs,py=ys;
  int dx = (int)xe - (int)xs >= 0 ? xe - xs : xs - xe;
  int dy = (int)ye - (int)ys <= 0 ? ye - ys : ys - ye;
  int XAddway = xs < xe ? 1 : -1;
  int YAddway = ys < ye ? 1 : -1;
  int esp = dx + dy;
  //color = __builtin_bswap16(color);
  while(true){
    //img[py*LCD_W+px]=color;
    lcd_apixel_raw(px,py,color);
    if(2*esp >= dy) {
      if (px == xe){break;}
      esp += dy;
      px += XAddway;
    }
    if(2*esp <= dx) {
      if (py == ye){break;}
      esp += dx;
      py += YAddway;
    }
  }
}


Vec2* lcd_linev2list2(Vec2 vs, Vec2 ve, int16_t* rsret){
  Vec2* pv;
  int16_t rs=0;
  int16_t px=vs.x;
  int16_t py=vs.y;
  int16_t XAddway = vs.x < ve.x ? 1 : -1;
  int16_t YAddway = vs.y < ve.y ? 1 : -1;
  int16_t dx = (ve.x - vs.x >= 0 )? ve.x - vs.x : vs.x - ve.x;
  int16_t dy = (ve.y - vs.y <= 0 )? ve.y - vs.y : vs.y - ve.y;
  int32_t esp = dx + dy;
  int16_t ax = dx>=0?dx:-dx;
  int16_t ay = dy>=0?dy:-dy;
  if(ax>ay){ rs = ax+1; }
  else{      rs = ay+1; }
  pv = (Vec2*)malloc(sizeof(Vec2) * rs);
  //printf("v2l: dx%d dy%d ax%d ay%d : rs %d [%d %d]\n",dx,dy,ax,ay,rs,ve.x,ve.y);
  uint16_t i=0;
  uint16_t scape=0;
  while(true){
    pv[i].x=px;
    pv[i].y=py;
    //printf("lv2l: %d %d\n",px,py);
    i++;
    if(2*esp >= dy) {
      if (px==ve.x){break;}
      esp += dy;      px += XAddway;
    }
    if(2*esp <= dx) {
      if (py==ve.y){break;}
      esp += dx;      py += YAddway;
    }
  }
  if(i<rs){
    pv[i].x=px;
    pv[i].y=py;
  }
  //printf("v2l: { %d %d }\n",pv[118].x,pv[118].y);

  *rsret = rs;
  return pv;
}



Vec2* lcd_linev2list(Vec2 ve, int16_t* rsret){
  Vec2* pv;
  int16_t rs=0;
  int16_t px=0;
  int16_t py=0;
  int16_t XAddway = ve.x > 0 ? 1 : -1;
  int16_t YAddway = ve.y > 0 ? 1 : -1;
  int16_t dx = (ve.x < 0) ? -ve.x:ve.x;
  int16_t dy = (ve.y > 0) ? -ve.y:ve.y;
  int16_t esp = dx + dy;
  int16_t ax = dx>=0?dx:-dx;
  int16_t ay = dy>=0?dy:-dy;
  if(ax>ay){ rs = ax+1; }
  else{      rs = ay+1; }
  pv = (Vec2*)malloc(sizeof(Vec2) * rs);
  //printf("v2l: dx%d dy%d ax%d ay%d : rs %d [%d %d]\n",dx,dy,ax,ay,rs,ve.x,ve.y);
  uint16_t i=0;
  uint16_t scape=0;
  while(true){
    pv[i].x=px;
    pv[i].y=py;
    //printf("lv2l: %d %d\n",px,py);
    i++;
    if(2*esp >= dy) {
      if (px==ve.x){break;}
      esp += dy;      px += XAddway;
    }
    if(2*esp <= dx) {
      if (py==ve.y){break;}
      esp += dx;      py += YAddway;
    }
  }
  if(i<rs){
    pv[i].x=px;
    pv[i].y=py;
  }

  *rsret = rs;
  return pv;
}



void lcd_blit_deg(Vec2 vs, Vec2 ve, Vec2 vts, int16_t deg, const uint8_t* src, uint16_t alpha,bool centric){
  int16_t lenx=0;
  int16_t leny=0;
  float cos=tcos[deg];
  float sin=tsin[deg];


  //ve = vrot(ve,deg);

  Vec2 vx = gvdl(deg,ve.x);
  Vec2* pvx = lcd_linev2list(vx,&lenx);
  Vec2 vy = gvdl(deg+QDEG,ve.y);
  Vec2* pvy = lcd_linev2list(vy,&leny);
  if(lenx>ve.x)lenx=ve.x;
  if(leny>ve.y)leny=ve.y;

  //if(!centric){
  //  printf("pvx = %08x %d (%d %d)\n",pvx,lenx,vx.x,vx.y);
  //  printf("pvy = %08x %d (%d %d)\n",pvy,leny,vy.x,vy.y);
  //  for(int i=0;i<leny;++i)printf("pvxy[%d]= %d %d , %d %d\n",i,pvx[i].x,pvx[i].y,pvy[i].x,pvy[i].y);
  //  printf("\n");
  //}
  int16_t px,py;
  uint16_t* ps=(uint16_t*)src;

  float flx = (float)vts.x/lenx;
  float fly = (float)vts.y/leny;
  float flx1 = (float)lenx/vts.x;
  float fly1 = (float)leny/vts.y;
  if(flx<0)flx=-flx;
  if(fly<0)fly=-fly;
  if(flx1<0)flx1=-flx1;
  if(fly1<0)fly1=-fly1;

  bool vexsm = (ve.x<lenx);
  bool veysm = (ve.y<leny);
  int16_t nx = vexsm?ve.x:lenx;
  int16_t ny = veysm?ve.y:leny;
  //if(!centric){
  //  printf("deg: %d:: %d %d - %f %f %f %f %d %d %d %d\n",deg,nx,ny,flx,fly,flx1,fly1, ve.x,ve.y, lenx,leny);
  //}

  int16_t utab[256];
  int16_t vtab[256];

  if(ve.x>=lenx&&lenx!=1){
    for(uint16_t i=0;i<lenx;i++){      utab[i]=(int16_t)i*flx;    }
  }else if(lenx==1){
    for(uint16_t i=0;i<ve.x;i++){      utab[i]=i;    }
  }else{
    for(uint16_t i=0;i<ve.x;i++){      utab[i]=(int16_t)i*flx1;    }
  }

  if(ve.y>=leny&&leny!=1){
    for(uint16_t i=0;i<leny;i++){      vtab[i]=(int16_t)i*fly;}
  }else if(leny==1){
      for(uint16_t i=0;i<ve.y;i++){       vtab[i]=i;}
  }else{
    for(uint16_t i=0;i<ve.y;i++){      vtab[i]= (int16_t)i*fly1;}
  }
  int16_t sx,sy;
  int16_t hax=ve.x>>1;
  int16_t hay=ve.y>>1;
  if(centric){
    sx = (int16_t)(hax * cos - hay * sin);
    sy = (int16_t)(hay * sin + hax * cos);
    sx=vs.x-sx+hax;
    sy=vs.y-sy+hay;
  }else{
    sx = (int16_t)(hay * cos - hay * sin);
    sy = (int16_t)(hay * sin + hay * cos);
    sx=vs.x-sx;
    sy=vs.y-sy;
  }


  //int16_t sx = -ve.y/2;
  //int16_t sy = -ve.x/2;
  //printf("sxy: %d %d\n",sx,sy);
  //if(nx==1){nx=ve.x;}
  //else if(ny==1){ny=ve.y;}
  bool nyone=(ny==1);
  if(nyone){ny=ve.y;}
  bool nxone=(nx==1);
  if(nxone){nx=ve.x;}
  //printf("nx: %d %d %d %d\n",nxone,nyone,nx,ny);
  if(nyone){
    for(int16_t y=0;y<ny;y++){
      px = sx+pvy[0].x;
      py = sy+pvy[0].y;
      for(int16_t x=0;x<nx;x++){
        int16_t ax=pvx[x].x+px;
        int16_t ay=pvx[x].y+py;
        uint16_t c = ps[vtab[y]*vts.x + utab[x]];
        if(c!=alpha){
          lcd_pixel_raw(ax ,  ay   ,c);        lcd_pixel_raw(ax+1 ,ay   ,c);        lcd_pixel_raw(ax+1 ,ay+1 ,c);        lcd_pixel_raw(ax ,  ay+1 ,c);
        }
      }
    }
  }else if(nxone){
    for(int16_t y=0;y<ny;y++){
      px = sx+pvy[y].x;
      py = sy+pvy[y].y;
      for(int16_t x=0;x<nx;x++){
        int16_t ax=pvx[0].x+px;
        int16_t ay=pvx[0].y+py;
        uint16_t c = ps[vtab[y]*vts.x + utab[x]];
        if(c!=alpha){
          lcd_pixel_raw(ax ,  ay   ,c);       lcd_pixel_raw(ax+1 ,ay   ,c);        lcd_pixel_raw(ax+1 ,ay+1 ,c);        lcd_pixel_raw(ax ,  ay+1 ,c);
//          lcd_pixel_raw(px+ax ,  py+ay   ,c);        lcd_pixel_raw(px+ax+1 ,py+ay   ,c);        lcd_pixel_raw(px+ax+1 ,py+ay+1 ,c);        lcd_pixel_raw(px+ax ,  py+ay+1 ,c);
        }
      }
    }
  }else{
    for(int16_t y=0;y<ny;y++){
      px = sx+pvy[y].x;
      py = sy+pvy[y].y;
      for(int16_t x=0;x<nx;x++){
        int16_t ax=pvx[x].x+px;
        int16_t ay=pvx[x].y+py;
        uint16_t c = ps[vtab[y]*vts.x + utab[x]];
        if(c!=alpha){
          lcd_pixel_raw(ax ,  ay   ,c);        lcd_pixel_raw(ax+1 ,ay   ,c);        lcd_pixel_raw(ax+1 ,ay+1 ,c);        lcd_pixel_raw(ax ,  ay+1 ,c);
//          lcd_pixel_raw(px+ax ,  py+ay   ,c);        lcd_pixel_raw(px+ax+1 ,py+ay   ,c);        lcd_pixel_raw(px+ax+1 ,py+ay+1 ,c);        lcd_pixel_raw(px+ax ,  py+ay+1 ,c);
        }
      }
    }
  }
  free(pvx);
  free(pvy);
}


inline int16_t chkdeg(int16_t d){
  if(d<0)     {d+=DEGS;}
  if(d>=DEGS)  {d-=DEGS;}
  return d;
}

void lcd_dither(uint16_t sx, uint16_t sy, uint16_t sz){
  uint32_t o=(sy*sz)+(sx<<1);
  uint32_t i=0;
  uint16_t c0, c1;
  uint32_t yo=0;
  uint16_t dc=0;
  for(uint16_t y=0;y<sz;y++){
    for(uint16_t x=0;x<sz;x++){
      c0 = img[yo+x];
      c1 = img[yo+x+1];
      if(c0&&c1){continue;}
      if( c0==0 && c1>0 ){        img[yo+x]=lcd_darker(c1);x++;dc++;      } //l
      if( c1==0 && c0>0 ){        img[yo+x+1]=lcd_darker(c0);x++;dc++;      } //r
    }
    yo+=LCD_W;
  }
  printf("dc=%d\n",dc);
}


void lcd_magnify(uint8_t sx, uint8_t sy, uint8_t sz, uint8_t mx, uint8_t my, uint8_t mf){
  uint32_t yos = sy*LCD_W;
  //uint32_t yod = my*LCD_W;
  uint16_t c;
  for(uint8_t y=0;y<sz;y++){
    for(uint8_t x=0;x<sz;x++){
      c = img[yos+sx+x];
      lcd_pixel_rawps(mx+x*mf,my+y*mf,c,mf);
    }
    yos+=LCD_W;
//    yod+=(LCD_W<<mf);
  }
}


void lcd_blit_deg2(Vec2 vo, Vec2 vuv, Vec2 vs, int16_t deg, const uint8_t* src, uint16_t alpha, bool centric){
//  printf("lcd_blit_deg2\n");
  uint16_t pslx[256];
  uint16_t psly[256];
  Vec2 vrs,vre,vcs,vce;
  if(centric){
    vrs.x = -(vuv.x/2);
    vrs.y = 0;
    vre.x = vuv.x/2;
    vre.y = 0;
    vcs.x = 0;
    vcs.y = -(vuv.y/2);
    vce.x = 0;
    vce.y = vuv.y/2;
  }else{
    vrs.x=0;      vrs.y=0;
    vre.x=vuv.x;  vre.y=0;
    vcs.x=0;      vcs.y=-(vs.y/2);
    vce.x=0;      vce.y=vuv.y-(vs.y/2);
  }
  uint16_t* ps = (uint16_t*)src;
  vrotv(&vrs,deg);
  vrotv(&vre,deg);
  int16_t l0,l1;
  uint16_t i=0;
  Vec2* pvx = lcd_linev2list2(vrs,vre,&l0);
  vrotv(&vcs,deg);
  vrotv(&vce,deg);
  Vec2* pvy = lcd_linev2list2(vcs,vce,&l1);
  l0--;
  l1--;
  Vec2 p;
  bool l0one = (l0==1);
  bool l1one = (l1==1);
  if(l0one){l0=l1;}
  if(l1one){l1=l0;}
  int16_t ll = l0-1;
  float difx = (float)vs.x/(float)(l0);
  float dx = 0.0f;
  i=0;
  while(i < l0){
    pslx[i]=(uint16_t)(dx);
    dx+=difx;
    ++i;
  }
  float dify = (float)vs.y/(float)(l1);
  float dy = 0.0f;
  ll = l1-1;
  i=0;
  while(i < l1){
    psly[i]=(uint16_t)(dy);
    dy +=dify;
    ++i;
  }
  uint16_t c = 0;
  if(l0one){
    for(uint16_t y=0;y<l1;y++){
      for(uint16_t x=0;x<l0;x++){
        c = ps[psly[y]*vs.x + pslx[x]];
        if(c!=alpha){
          p.x = pvx[0].x+pvy[y].x+vo.x;
          p.y = pvx[0].y+pvy[y].y+vo.y;
          lcd_pixel_raw(p.x ,  p.y   ,c);
        }
      }
    }
  }else if(l1one){
    //printf("l1:\n");
    for(uint16_t y=0;y<l1;y++){
      p.y=pvy[0].y+y;
      for(uint16_t x=0;x<l0;x++){
        c = ps[psly[y]*vs.x + pslx[x]];
        if(c!=alpha){
          p.x = pvx[x].x+pvy[0].x+vo.x;
          p.y = pvx[x].y+pvy[0].y+vo.y;
          lcd_pixel_raw(p.x ,p.y   ,c);
        }
      }
    }
  }else{
    //printf("lX:\n");
    for(uint16_t y=0;y<l1;y++){
      p.y = pvy[y].y+vo.y;
      p.x = pvy[y].x+vo.x;
      for(uint16_t x=0;x<l0;x++){
        c = ps[psly[y]*vs.x + pslx[x]];
        if(c!=alpha){
          lcd_pixel_raw(p.x+pvx[x].x , p.y+pvx[x].y   ,c);
          if(x>0&&x<l0-1 && y>0&&y<l1-1){
            lcd_pixel_raw(p.x+pvx[x].x+1 , p.y+pvx[x].y   ,c);
          }
        }
      }
    }
  }
  free(pvx);
  free(pvy);
}
