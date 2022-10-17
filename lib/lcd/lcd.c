#include "lcd.h"

uint16_t* img=NULL;
uint8_t slice_num;
//uint8_t* b0=NULL; //db
//uint8_t* b1=NULL; //db
uint16_t pbuf[64*64*2];
char cbuf[32];
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
  MemoryAccessReg = (scandir == HORIZONTAL)?0xc8:0x68;
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
    img[py*LCD_W+px]=color;
    if(ps>1){
      if(ps>=2){
        img[(1+py)*LCD_W+px]=color;
        img[(py)*LCD_W+px+1]=color;
        img[(1+py)*LCD_W+px+1]=color;
      }
      if(ps>=3){
        img[(2+py)*LCD_W+px]=color;
        img[(py)*LCD_W+px+2]=color;
        img[(2+py)*LCD_W+px+2]=color;
      }
      if(ps>=4){
        img[(3+py)*LCD_W+px]=color;
        img[(py)*LCD_W+px+3]=color;
        img[(3+py)*LCD_W+px+3]=color;
      }
      if(ps>=5){
        img[(4+py)*LCD_W+px]=color;
        img[(py)*LCD_W+px+4]=color;
        img[(4+py)*LCD_W+px+4]=color;
      }
    }

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
  cf = __builtin_bswap16(cf);
  cb = __builtin_bswap16(cb);
  lcd_string(x,y,data,font,false,cf,cb);
}

void lcd_strc(uint8_t x, uint8_t y, char* data, sFONT* font, uint16_t cf, uint16_t cb){
  cf = __builtin_bswap16(cf);
  cb = __builtin_bswap16(cb);
  lcd_string(x,y,data,font,true,cf,cb);
}

void lcd_number(uint8_t x, uint8_t y, uint32_t n ,sFONT* font, uint16_t cf, uint16_t cb){
  sprintf(cbuf,"%d\0",n);
  lcd_str(x,y,&cbuf[0],font,cf,cb);
}

void lcd_float(uint8_t x, uint8_t y, float f ,sFONT* font, uint16_t cf, uint16_t cb){
  sprintf(cbuf,"%0.2f\0",f);
  lcd_str(x,y,&cbuf[0],font,cf,cb);
}

void lcd_sleepon(){  lcd_cmd(0x10);}
void lcd_sleepoff(){  lcd_cmd(0x11);}


inline void lcd_pixel_raw(uint16_t x, uint16_t y, uint16_t c){
  img[LCD_W*y+x] = c;
}

inline void lcd_xlineq(uint16_t x, uint16_t y, uint16_t l, uint16_t c){
  uint16_t i=0;
  while(i<l){ img[LCD_W*y+x+i] = c;++i; }
}

inline void lcd_pixel_rawps(uint16_t x, uint16_t y, uint16_t c, uint16_t ps){
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

void lcd_rect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color, uint8_t ps){
    color=__builtin_bswap16(color);
    lcd_xline(x0,y0,x1-x0,color,ps);
    lcd_xline(x0,y1-ps,x1-x0,color,ps);
    lcd_yline(x0,y0,y1-y0,color,ps);
    lcd_yline(x1-ps,y0,y1-y0,color,ps);
}
