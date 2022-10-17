//﻿#include "LCD_Test.h"   //Examples
#include "stdio.h"
#include "pico/stdlib.h"
#include "stdlib.h"
#include "pico/time.h"
#include "pico/util/datetime.h"
#include "hardware/adc.h"
#include "hardware/rtc.h"
#include "hardware/gpio.h"
#include <math.h>
#include <float.h>
#include "pico/types.h"
#include "pico/bootrom/sf_table.h"

#include "lcd.h"
#include "QMI8658.h"

#include "lib/Fonts/fonts.h"
#include "img/Font34.h"
#include "img/Font30.h"
#include "img/bega.h"
#include "img/sand.h"
#include "img/irisa190.h"
#include "img/earth190.h"

//#include "img/maple.h"
#include "img/usa32.h"
#include "img/cn32.h"
#include "img/ger32.h"
#include "img/tr32.h"
#include "img/usa16.h"
#include "img/cn16.h"
#include "img/ger16.h"
#include "img/tr16.h"

#define DEFAULT_THEME 1
#define TFONT Font20
#define CNFONT Font30
#define NO_POS_MODE 1

#define mcpy(d,s,sz) for(int i=0;i<sz;i++){d[i]=s[i];}
#define THEMES 4

#define EYE irisa190

datetime_t t = {
  .year  = 2022,
  .month = 10,
  .day   = 17,
  .dotw  = 1, // 0 is Sunday, so 5 is Friday
  .hour  = 14,
  .min   = 45,
  .sec   = 0
};

#define LOOPWAIT 50

#define DRAW_GFX_FIRST false //1 == text floating above clock
#define to_rad(angleInDegrees) ((angleInDegrees) * M_PI / 180.0)
#define to_deg(angleInRadians) ((angleInRadians) * 180.0 / M_PI)
#define HOURGLASSBORDER 200 // minimum rise/fall of acc_x
#define HOURGLASS 600*(100/LOOPWAIT)  // rise/fall of acc_x border till switch (cw/ccw)
#define BUTTONGLASSC 300
#define BUTTONGLASS 1400
#define SCRSAV 60*(100/LOOPWAIT)
#define SCRSAV2 SCRSAV*2
#define BRIGHTD 20
#define SWITCH_THEME_DELAY 10
#define THRS 12
#define THRLY 120

#define POS_CX 120
#define POS_CY 120

#define POS_ACC_X 60
#define POS_ACC_Y 40

#define POS_BAT_X 70
#define POS_BAT_Y 30
#define POS_BAT_YS 10
#define POS_BAT_PS 2


#define POS_DATE_X 46
#define POS_DATE_Y 86

#define POS_DOW_X 20
#define POS_DOW_Y 111
#define POS_CNDOW_X 190
#define POS_CNDOW_Y 72

#define POS_TIME_X 64
#define POS_TIME_Y 136

#define TFW 14

// eye dimensions
#define EYE_SZ 190
#define EYE_R EYE_SZ/2
#define EYE_X 120-EYE_R
#define EYE_Y 120-EYE_R
#define EYE_MAX 50



void sincosf(float,float*,float*);

typedef enum CMode {
  CM_None = 0,
  CM_Config = 1,
  CM_Editpos = 2,
  CM_Changepos = 3,
  CM_Changetheme = 4
} CMode;

typedef struct ColorTheme_t{
  uint16_t alpha;
  uint16_t col_h;
  uint16_t col_m;
  uint16_t col_s;
  uint16_t col_cs;
  uint16_t col_cs5;
  uint16_t col_dotw;
  uint16_t col_date;
  uint16_t col_time;
} ColorTheme_t;

typedef struct PXY_t{
  uint8_t x;
  uint8_t y;
} PXY_t;

typedef struct ThemePos_t{
  PXY_t pos_dow;
  PXY_t pos_day;
  PXY_t pos_month;
  PXY_t pos_year;
  PXY_t pos_h;
  PXY_t pos_m;
  PXY_t pos_s;
} ThemePos_t;

  //ThemePos_t tpos[1] = {POS_DOW_X,POS_DOW_Y,
  //  POS_DATE_X,POS_DATE_Y,
  //  POS_DATE_X+3*TFW,POS_DATE_Y,
  //  POS_DATE_X+5*TFW,POS_DATE_Y,
  //  POS_TIME_X,      POS_TIME_Y,
  //  POS_TIME_X+3*TFW,POS_TIME_Y,
  //  POS_TIME_X+5*TFW,POS_TIME_Y
  //};

  PXY_t tpos[8] =
  { POS_DOW_X,POS_DOW_Y,
    POS_DATE_X,POS_DATE_Y,
    POS_DATE_X+3*TFW,POS_DATE_Y,
    POS_DATE_X+8*TFW,POS_DATE_Y,
    POS_TIME_X,      POS_TIME_Y,
    POS_TIME_X+3*TFW,POS_TIME_Y,
    POS_TIME_X+6*TFW,POS_TIME_Y,
    POS_CX, POS_CY
  };

  uint8_t pos_matrix_x=1; // start in center
  uint8_t pos_matrix_y=1; // start in center

  int8_t pos_matrix_CN[9] =
  {
    1,2,3,
    7,7,0,
    4,5,6
  };


  int8_t pos_matrix_US[9] =
  {
    1,2,3,
    0,7,7,
    4,5,6
  };

  int8_t pos_matrix[9];
  void copy_pos_matrix(uint8_t n){
    if(n){
      for(int8_t i=0;i<9;i++){pos_matrix[i]=pos_matrix_US[i];}
    }else{
      for(int8_t i=0;i<9;i++){pos_matrix[i]=pos_matrix_CN[i];}

    }
  }


#define USA_Old_Glory_Red 0xB0C8 //0xB31942
#define USA_Old_Glory_Blue 0x098C //0x0A3161
//#define USA_Old_Glory_Red  0xC8B0 //0xB31942
//#define USA_Old_Glory_Blue 0x8C09 //0x0A3161

#define CN_Red 0xE8E4 //0xee1c25
#define CN_Gold 0xFFE0 //0xffff00
//#define CN_Red 0xE4E8 //0xee1c25
//#define CN_Gold 0xE0FF //0xffff00

//#define GER_Gold 0x60FE
//#define GER_Red 0x00F8
#define GER_Gold 0xFE60
#define GER_Red 0xF800

//#define TR_Red 0x00F8
#define TR_Red 0xF800

#define THEMES 4
//#define FLAGS_MAX 4

CMode cmode = CM_None;

uint8_t theme_pos = DEFAULT_THEME;
const uint8_t* flags[THEMES] = {cn32,usa32,ger32,tr32};
const uint8_t* stars[THEMES] = {cn16,usa16,ger16,tr16};
uint16_t alpha[] = {BLACK,BLACK};
const char* backgrounds[THEMES] = {earth190,irisa190,bega,sand};
bool bg_dynamic[THEMES] = {true,true,false,false};
uint8_t theme_bg_dynamic_mode = 0;
const uint16_t edit_colors[THEMES] = {ORANGE,YELLOW,ORANGE,ORANGE};
const uint16_t change_colors[THEMES] = {YELLOW,YELLOW,YELLOW,YELLOW};

ColorTheme_t colt1={BLACK,CN_Red,CN_Red,CN_Gold,CN_Red,CN_Gold,WHITE,WHITE,WHITE};
ColorTheme_t colt2={BLACK,USA_Old_Glory_Red,USA_Old_Glory_Blue,WHITE,USA_Old_Glory_Red,WHITE,WHITE,WHITE,WHITE};
ColorTheme_t colt3={BLACK,GER_Red,0x0001,GER_Gold,GER_Red,GER_Gold,WHITE,WHITE,WHITE};
ColorTheme_t colt4={BLACK,WHITE,TR_Red,WHITE,WHITE,TR_Red,WHITE,WHITE,WHITE};

ColorTheme_t* colt[THEMES];

uint16_t dcol = WHITE;
uint16_t editcol = YELLOW;
uint16_t changecol = YELLOW;
uint16_t acol=WHITE;
uint16_t colors[8]  = {WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE};
uint16_t dcolors[8] = {WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE};

uint16_t blinker[2] = {BLUE,RED};

uint8_t ec_x[7] = {};
uint8_t ec_y[7] = {};

typedef enum Dir_t {
  D_NONE = 0,
  D_PLUS = 1,
  D_MINUS = 2
} Dir_t;


Dir_t dir_x;
Dir_t dir_y;
uint8_t no_pos_x=0;
uint8_t no_pos_y=0;

char timebuffer[16] = {0};
char* ptimebuffer=timebuffer;
bool h24=true;

char datetime_buf[256];
char *datetime_str = &datetime_buf[0];
char* dt_date;
char* dt_time;

//ky-040
#define CCLK 16
#define CDT 17
#define CSW 19

//one button /
#define CBUT 22

bool analog_seconds=false;
bool fire=false;
bool ceasefire=false;
bool tcw=false;
bool tccw=false;
bool clk,dt,sw,oclk,odt,osw;
int gc=0;
char gch;
char gbuf[2] = {'c','d'};
uint32_t stime;
uint8_t tseco;
int hourglass_x=HOURGLASS;
int hourglass_y=HOURGLASS;
int hgx=0;
int hgy=0;
int buttonglass=BUTTONGLASS;
int screensaver=SCRSAV;

int flagsdelay = SWITCH_THEME_DELAY;
int blink_counter = 0;
bool bmode = false;

float tsin[360];
float tcos[360];
float tfsin[600];
float tfcos[600];
bool is_sleeping;

uint8_t editpos=0;
bool edittime=false;
bool changetime=false;
char dbuf[8];
float temperature = -99.99f;

float mag[3];
bool draw_gfx_first = DRAW_GFX_FIRST;
bool usb_loading = false;

uint16_t result;
const float conversion_factor = 3.3f / (1 << 12) * 2;

float acc[3], gyro[3];
unsigned int tim_count = 0;

uint16_t cn_chars=0;
char ftst[128*4] = {0};

char* week_usa[7] = {"Sun\0","Mon\0","Tue\0","Wed\0","Thu\0","Fri\0","Sat\0"};
char* week_cn[7] = {"星期日\0","星期一\0","星期二\0","星期三\0","星期四\0","星期五\0","星期六\0"};
char* week_ger[7] = {"Son\0","Mon\0","Die\0","Mit\0","Don\0","Fre\0","Sam\0"};
char* week_tr[7] = {"PAZ\0","PZT\0","SAL\0","CAR\0","PER\0","CUM\0","CMT\0"};
char** week[THEMES] = {week_cn,week_usa,week_ger,week_tr};
 // dummy month0
uint8_t last[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};

char cn_buffer[32] = {0};

uint16_t to_rgb565(uint8_t r,uint8_t g,uint8_t b){
  r>>=3;
  g>>=2;
  b>>=3;
  return ((r<<11)+(g<<5)+b);
}

uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b){
  return ((r<<11)+(g<<5)+b);
}

void to_rgb(uint16_t rgb, uint8_t* r, uint8_t* g, uint8_t* b){
  *r=((rgb>>11)&0x1f)<<3;
  *g=((rgb>>5)&0x3f)<<3;
  *b=(rgb&0x1f)<<3;
}

void set_dcolors(){
  for(int i=0;i<7;i++){
    colors[i]=dcolors[i];
  }
}

void set_colt_colors(){
  dcolors[0]=colt[theme_pos]->col_dotw;
  dcolors[1]=colt[theme_pos]->col_date;
  dcolors[2]=colt[theme_pos]->col_date;
  dcolors[3]=colt[theme_pos]->col_date;
  dcolors[4]=colt[theme_pos]->col_time;
  dcolors[5]=colt[theme_pos]->col_time;
  dcolors[6]=colt[theme_pos]->col_time;
  dcolors[7]=colt[theme_pos]->col_time;
}

uint8_t find_cc(uint8_t a, uint8_t b, uint8_t c){
  uint fo=0;
  for(int i=0; i<cn_chars+1;i++){
    //printf("[%02x%02x%02x] %02x %02x %02x\n",a,b,c,ftst[fo],ftst[fo+1],ftst[fo+2]);
    if( (ftst[fo+0]==a) && (ftst[fo+1]==b) && (ftst[fo+2]==c) ){
      //printf("find_cc: %d %d\n",i,i+228);
      return i;
    }
    fo+=4;
  }
}

void convert_cs(char* source, char* target){
  uint32_t si=0, ti=0;
  while(source[si]){
    //printf("%02x %02x %02x\n",source[si],source[si+1],source[si+2]);
    target[ti]=find_cc(source[si],source[si+1],source[si+2]);
    //printf("%d [%d]\n",target[ti],target[ti]+228);
    si+=3;
    target[ti]+=(256-32);
    ++ti;
    target[ti]+='\n';
  }
  target[ti]=0;
}

void print_font_table(){
  uint8_t fts=0;
  uint8_t n=0;
  uint8_t nbytes=0;
  uint32_t ft[128];
  uint32_t sti=0;
  char ftc[5] = {0};
  uint8_t cbu[5];
  puts("TESTING...");
  printf("symcheck\n");
  char* pc;
  for(int i=0;i<7;i++){
    int c=0;
    pc = week_cn[i];
    while(pc[c]){
      n=pc[c];

      if((0b10000000&n)==0b00000000){nbytes=1;}
      if((0b11100000&n)==0b11000000){nbytes=2;}
      if((0b11110000&n)==0b11100000){nbytes=3;}
      if((0b11111000&n)==0b11110000){nbytes=4;}
      //printf("n=%02x (%02b) [%d]",n,(n&0b10000000),nbytes);
      switch(nbytes){
        case 1: ft[fts]=n;c+=1;break;
        case 2: ft[fts]=(pc[c+1]<<8)+(pc[c+0]);c+=2;break;
        case 3: ft[fts]=(pc[c+0]<<16)+(pc[c+1]<<8) +(pc[c+2]);c+=3;break;
        case 4: ft[fts]=(pc[c+0]<<24)+(pc[c+1]<<16)+(pc[c+2]<<8)+(pc[c+3]);c+=4;
      }
      //printf("ft=%d\n",ft[fts]);
      bool dupe=false;
      for(int j=0;j<fts;j++){
        if(ft[j]==ft[fts]){dupe=true;break;}
      }
      if(!dupe){++fts;}
    }
  }

  uint32_t i,k;
  uint32_t temp;

  n=fts;
  for(i = 0; i<n-1; i++) {
    for(k = 0; k<n-1-i; k++) {
      if(ft[k] > ft[k+1]) {
        temp = ft[k];
        ft[k] = ft[k+1];
        ft[k+1] = temp;
      }
    }
  }
  pc=(char*)&ft[0];
  sti=0;
  for(i=0;i<fts;i++){
    //printf("%02d : %d %02x %02x %02x %02x\n",i,ft[i],pc[0],pc[1],pc[2],pc[3]);
    ftc[0]=pc[2];
    ftc[1]=pc[1];
    ftc[2]=pc[0];
    ftc[3]=pc[3];
    printf("S: %02x %02x %02x %s\n",ftc[0],ftc[1],ftc[2],ftc);
    ftst[sti+0]=ftc[0];
    ftst[sti+1]=ftc[1];
    ftst[sti+2]=ftc[2];
    ftst[sti+3]='\n';
    pc+=4;
    sti+=4;
  }
  ftst[sti]=0;
  printf("CHARLIST:\n%s\n",ftst);
  cn_chars=fts;
}

void gpio_callback(uint gpio, uint32_t events) {
    if(events&GPIO_IRQ_EDGE_RISE){
      if(gpio==CSW){        osw=true;      }
      if(gpio==CCLK){        gch='c';      }
      if(gpio==CDT){        gch='d';      }
      gbuf[0]=gbuf[1];
      gbuf[1]=gch;
    }

    if(events&GPIO_IRQ_EDGE_FALL){
      if(gpio==CSW){        sw=true;      }
      if(gpio==CCLK){        gch='C';      }
      if(gpio==CDT) {        gch='D';      }
      gbuf[0]=gbuf[1];
      gbuf[1]=gch;
    }
    if(events&GPIO_IRQ_LEVEL_LOW && gpio==CBUT){
      buttonglass-=BUTTONGLASSC;
      if(buttonglass<=0){
        fire=true;
        buttonglass=BUTTONGLASS;
      }
    }
    if(gbuf[0]=='C'&&gbuf[1]=='D'){tcw=true;}
    if(gbuf[0]=='D'&&gbuf[1]=='C'){tccw=true;}
    if(sw){sw=false;fire=true;}
    if(osw){osw=false;ceasefire=true;}
}



void draw_gfx(){
  uint8_t x1,y1,xt,yt;
  uint8_t x0=120;
  uint8_t y0=120;

  //lcd_line(POS_BAT_X    ,POS_BAT_Y,   POS_BAT_X+102, POS_BAT_Y, BLUE,1);//top
  //lcd_line(POS_BAT_X    ,POS_BAT_Y+POS_BAT_YS, POS_BAT_X+102, POS_BAT_Y+POS_BAT_YS, BLUE,1);//bottom
  //lcd_line(POS_BAT_X    ,POS_BAT_Y,   POS_BAT_X,     POS_BAT_Y+POS_BAT_YS, BLUE,1);//left
  //lcd_line(POS_BAT_X+102,POS_BAT_Y,   POS_BAT_X+102, POS_BAT_Y+POS_BAT_YS, BLUE,1);//right

  lcd_rect(POS_BAT_X    ,POS_BAT_Y,   POS_BAT_X+100+(POS_BAT_PS<<1), POS_BAT_Y+POS_BAT_YS, BLUE,POS_BAT_PS);
  uint16_t bat = (uint16_t)(((result * conversion_factor)/4.17)*100);
  //printf("bat :  %03d\n",bat);
  if(bat>100){bat=100;}
  lcd_xline(POS_BAT_X+POS_BAT_PS    ,POS_BAT_Y+POS_BAT_PS,   bat-1, WHITE, POS_BAT_YS-(POS_BAT_PS<<1));//top
  if(!usb_loading){
    sprintf(dbuf,"  %02d%%",bat);
  }else{
    sprintf(dbuf,"LOADING",bat);
  }
  lcd_str(94, 12, dbuf, &Font12, WHITE, RED);
  //lcd_str(94, 12, dbuf, &Font34);

  if(t.sec==0||t.sec==30){
    temperature = QMI8658_readTemp();
  }
  //QMI8658_read_mag(mag);
  //printf("mag: %0.2f %0.2f %0.2f\n",mag[0],mag[1],mag[2]);
  //printf("acc_x   = %4.3fmg , acc_y  = %4.3fmg , acc_z  = %4.3fmg\r\n", acc[0], acc[1], acc[2]);
  //printf("gyro_x  = %4.3fdps, gyro_y = %4.3fdps, gyro_z = %4.3fdps\r\n", gyro[0], gyro[1], gyro[2]);
  //printf("tim_count = %d\r\n", tim_count);
  int xi,yi;
  for(uint16_t i=0;i<60;i++){
    xi = (int)(tcos[i*6]*120);
    yi = (int)(tsin[i*6]*120);
    x1 = (uint8_t)x0+xi;
    y1 = (uint8_t)y0+yi;
    if(!(i%5)){
      xi = (int)(tcos[i*6]*110);
      yi = (int)(tsin[i*6]*110);
      lcd_line((uint8_t)x0+xi,(uint8_t)y0+yi, x1, y1, colt[theme_pos]->col_cs5, 1);
    }else{
      xi = (int)(tcos[i*6]*115);
      yi = (int)(tsin[i*6]*115);
      lcd_line((uint8_t)x0+xi,(uint8_t)y0+yi, x1, y1, colt[theme_pos]->col_cs, 1);
    }
  }
  xi = (int)(tcos[t.min*6]*105);
  yi = (int)(tsin[t.min*6]*105);
  x1 = (uint8_t)x0+xi;
  y1 = (uint8_t)y0+yi;
  lcd_line(x0,y0, x1, y1, colt[theme_pos]->col_m, 3);

  int th=(int)t.hour;
  if(th>=12){th-=12;}
  th*=30;
  th+=(int)(t.min>>1);
  xi = (int)(tcos[th]*64);
  yi = (int)(tsin[th]*64);
  x1 = (uint8_t)x0+xi;
  y1 = (uint8_t)y0+yi;
  lcd_line(x0,y0, x1, y1, colt[theme_pos]->col_h, 5);

  if(tseco!=t.sec){
    tseco=t.sec;
    stime = time_us_32();
  }

  uint32_t st = time_us_32();
  st-=stime;
  st=st/100000;
  //printf("%d\n",st);
  if(!analog_seconds){
    // 'jump' seconds
    xi = (int)(tcos[t.sec*6]*114);
    yi = (int)(tsin[t.sec*6]*114);
    x1 = (uint8_t)x0+xi;
    y1 = (uint8_t)y0+yi;
    lcd_line(x0,y0, x1, y1, colt[theme_pos]->col_s, 1);
    lcd_blit((int)(x0-8+tcos[t.sec*6]*102),(int)(y0-8+tsin[t.sec*6]*102),16,16,colt[theme_pos]->alpha,stars[theme_pos]);
  }else{
    // 'analog' seconds
    xi = (int)(tfcos[t.sec*10+st]*114);
    yi = (int)(tfsin[t.sec*10+st]*114);
    x1 = (uint8_t)x0+xi;
    y1 = (uint8_t)y0+yi;
    lcd_line(x0,y0, x1, y1, colt[theme_pos]->col_s, 1);
    lcd_blit((int)(x0-8+tfcos[t.sec*10+st]*102),(int)(y0-8+tfsin[t.sec*10+st]*102),16,16,colt[theme_pos]->alpha,stars[theme_pos]);

  }

  lcd_blit(120-16,120-16,32,32,colt[theme_pos]->alpha, flags[theme_pos]); // center

}


void draw_text(){
  lcd_str(POS_ACC_X, POS_ACC_Y    , "GYR_X = ", &Font12, WHITE,  CYAN);
  lcd_str(POS_ACC_X, POS_ACC_Y+16 , "GYR_Y = ", &Font12, WHITE,  CYAN);
  lcd_str(POS_ACC_X, POS_ACC_Y+32 , "GYR_Z = ", &Font12, WHITE, CYAN);
  lcd_str(POS_ACC_X, POS_ACC_Y+114 ,"ACC_X = ", &Font12, WHITE, CYAN);
  lcd_str(POS_ACC_X, POS_ACC_Y+128, "ACC_Y = ", &Font12, WHITE, CYAN);
  lcd_str(POS_ACC_X, POS_ACC_Y+142, "ACC_Z = ", &Font12, WHITE, CYAN);
  lcd_float(POS_ACC_Y+70, POS_ACC_Y, acc[0], &Font12,  YELLOW,   WHITE);
  lcd_float(POS_ACC_Y+70, POS_ACC_Y+16 , acc[1], &Font12,  YELLOW,   WHITE);
  lcd_float(POS_ACC_Y+70, POS_ACC_Y+32 , acc[2], &Font12,  YELLOW,  WHITE);
  lcd_float(POS_ACC_Y+70, POS_ACC_Y+114 , gyro[0], &Font12,YELLOW, WHITE);
  lcd_float(POS_ACC_Y+70, POS_ACC_Y+128, gyro[1], &Font12, YELLOW, WHITE);
  lcd_float(POS_ACC_Y+70, POS_ACC_Y+142, gyro[2], &Font12, YELLOW, WHITE);
  lcd_str(70, 194, "TEMP = ", &Font12, WHITE, CYAN);
  lcd_float(120, 194, temperature, &Font12,  YELLOW, WHITE);
  lcd_str(50, 208, "BAT(V)=", &Font16, WHITE, ORANGE);
  lcd_float(130, 208, result * conversion_factor, &Font16, ORANGE, WHITE);

  if(!theme_pos){
    convert_cs(week[theme_pos][t.dotw],cn_buffer);
    lcd_strc(POS_CNDOW_X, POS_CNDOW_Y, cn_buffer, &CNFONT, colors[0], BLACK);
    //printf("cn_buffer: %s\n",cn_buffer);
  }else{
    lcd_str(POS_DOW_X, POS_DOW_Y, week[theme_pos][t.dotw], &TFONT, colors[0], BLACK);
  }

  sprintf(dbuf,"%02d",t.day);
  lcd_str(POS_DATE_X+0*TFW, POS_DATE_Y, dbuf, &TFONT, colors[1], BLACK);
  lcd_str(POS_DATE_X+2*TFW, POS_DATE_Y, ".", &TFONT, WHITE, BLACK);
  sprintf(dbuf,"%02d",t.month);
  lcd_str(POS_DATE_X+3*TFW, POS_DATE_Y, dbuf, &TFONT, colors[2], BLACK);
  lcd_str(POS_DATE_X+5*TFW, POS_DATE_Y, ".", &TFONT, WHITE, BLACK);
  sprintf(dbuf,"%04d",t.year);
  lcd_str(POS_DATE_X+6*TFW, POS_DATE_Y, dbuf, &TFONT, colors[3], BLACK);

  sprintf(dbuf,"%02d",t.hour);
  lcd_str(POS_TIME_X, POS_TIME_Y, dbuf, &TFONT, colors[4], BLACK);
  lcd_str(POS_TIME_X+2*TFW, POS_TIME_Y, ":", &TFONT, WHITE, BLACK);
  sprintf(dbuf,"%02d",t.min);
  lcd_str(POS_TIME_X+3*TFW, POS_TIME_Y, dbuf, &TFONT, colors[5], BLACK);
  lcd_str(POS_TIME_X+5*TFW, POS_TIME_Y, ":", &TFONT, WHITE, BLACK);
  sprintf(dbuf,"%02d",t.sec);
  lcd_str(POS_TIME_X+6*TFW, POS_TIME_Y, dbuf, &TFONT, colors[6], BLACK);
}




int main(void)
{
    stdio_init_all();
    puts("stdio init");
    lcd_init();
    lcd_set_brightness(30);
    puts("lcd init");
    uint8_t* b0 = malloc(LCD_SZ);
    uint32_t o = 0;
    lcd_setimg((uint16_t*)b0);

    colt[0]=&colt1;
    colt[1]=&colt2;
    colt[2]=&colt3;
    colt[3]=&colt4;

    bool o_clk;
    bool o_dt;
    bool o_sw;


    //uint32_t bm = 0b00000000000010110000000000000000;
    gpio_set_irq_enabled_with_callback(CCLK, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled(CDT, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(CSW, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_dir(CBUT,GPIO_IN);
    gpio_pull_up(CBUT);
    gpio_set_irq_enabled(CBUT, GPIO_IRQ_LEVEL_LOW, true);
    //gpio_pull_up(CBUT);
    rtc_init();
    rtc_set_datetime(&t);
    if(!(t.year%4)){last[2]=29;}else{last[2]=28;}

    //set_colt_colors();
    set_dcolors(); // are set from dcolors so set em first
    copy_pos_matrix(theme_pos);

    QMI8658_init();
    printf("QMI8658_init\r\n");

    for(uint16_t i=0;i<360;i++){
      float f = (float)i;
      sincosf(to_rad(f-90),&tsin[i],&tcos[i]);
    }
    float ff=0.0f;
    for(int i=0;i<600;++i){
      sincosf(to_rad(ff-90),&tfsin[i],&tfcos[i]);
      ff+=0.6f;
    }
    print_font_table();
    acc[0]=0.0f;
    acc[1]=0.0f;
    acc[2]=0.0f;

    while(true){
      QMI8658_read_xyz(acc, gyro, &tim_count);
      //check if not moving
      #define GYRMAX 300.0f
      #define ACCMAX 500.0f
      bool moveshake = false;
      if(theme_bg_dynamic_mode==1){
        if((gyro[0]>-ACCMAX&&gyro[0]<ACCMAX)&&(gyro[1]>-ACCMAX&&gyro[1]<ACCMAX)&&(gyro[2]>-ACCMAX&&gyro[2]<ACCMAX)){            moveshake =true;          }
      }else{
        if((acc[0]>-GYRMAX&&acc[0]<GYRMAX)&&(acc[1]>-GYRMAX&&acc[1]<GYRMAX)){            moveshake =true;          }

      }
      if(moveshake){
        if(!is_sleeping && cmode==CM_None && !usb_loading){
          screensaver--;
          if(screensaver<=0){
            if(bg_dynamic[theme_pos]){
              theme_bg_dynamic_mode++;
              if(theme_bg_dynamic_mode==1){
                screensaver=SCRSAV2;
                continue;}
            }
            is_sleeping=true;
            screensaver=SCRSAV;
            lcd_set_brightness(0);
            lcd_sleepon();
          }
        }
      }else{
        if(is_sleeping){
          is_sleeping=false;
          lcd_set_brightness(BRIGHTD);
          lcd_sleepoff();
        }
        if(theme_bg_dynamic_mode){theme_bg_dynamic_mode--;}
      }

      if(is_sleeping){
        sleep_ms(1000);
        continue;
      }

      if(bg_dynamic[theme_pos]){ // dynamic background
        for(int i=0;i<LCD_SZ;i++){b0[i]=0x00;}
        if(acc[1]>1024){acc[1]=1024;}
        if(acc[1]<-1024){acc[1]=-1024;}
        if(acc[0]>1024){acc[0]=1024;}
        if(acc[0]<-1024){acc[0]=-1024;}
        int8_t xa = (int8_t)(acc[1]/50.0f);
        int8_t ya = (int8_t)(acc[0]/50.0f);
        xa&=0xff;
        ya&=0xff;
        if(xa>EYE_MAX){xa=EYE_MAX;}
        if(xa<-EYE_MAX){xa=-EYE_MAX;}
        if(ya>EYE_MAX){ya=EYE_MAX;}
        if(ya<-EYE_MAX){ya=-EYE_MAX;}
        lcd_blit(EYE_X+xa,EYE_Y-ya,EYE_SZ,EYE_SZ,BLACK,backgrounds[theme_pos]);
      }else{
          mcpy(b0,backgrounds[theme_pos],LCD_SZ);
      }
      uint8_t save_sec = t.sec;
      uint8_t save_min = t.min;
      if(cmode!=CM_Editpos){
        rtc_get_datetime(&t);
      }
      //if(!fire && cmode==CM_Editpos && (editpos==6||editpos==5)){
      //  t.sec = save_sec;
      //  t.min = save_min;
      //}
      result = adc_read();
      usb_loading = ((result * conversion_factor)>=4.15);

      if(fire){
        theme_bg_dynamic_mode=0;
        dir_x = D_NONE;
        dir_y = D_NONE;

        if(cmode==CM_None){
          puts("CM_Config");
          cmode=CM_Config;
          hgx = (int)acc[0];
          hgy = (int)acc[1];
          puts("CM_Changepos");
          cmode=CM_Changepos;
          colors[editpos]=edit_colors[theme_pos];
        }else if(cmode==CM_Config){
          //puts("CM_Changepos");
          //cmode=CM_Changepos;
          //colors[editpos]=edit_colors[theme_pos];
        }else if(cmode==CM_Changepos){
          puts("CM_Editpos");
          cmode=CM_Editpos;
          hgx = (int)acc[0];
          hgy = (int)acc[1];
          tcw = false;
          tccw = false;
          //colors[editpos]=change_colors[theme_pos];
          colors[editpos]=changecol;
        }else if(cmode==CM_Editpos){
          puts("CM_None");
          cmode=CM_None;
          colors[editpos]=dcolors[editpos];
          rtc_set_datetime(&t);
          if(!(t.year%4)){last[2]=29;}else{last[2]=28;}
          //puts("CM_Changetheme");
          //cmode=CM_Changetheme;
        }
        //else if(cmode==CM_Changetheme){
        //  puts("CM_None");
        //  cmode=CM_None;
        //}

        fire=false;
      }
      if(cmode == CM_Config){
        float cx = acc[0]-hgx;
        float cy = acc[1]-hgy;
        float r;
        cx/=1000;
        cy/=1000;
        r = sqrt(cx*cx+cy*cy);
        printf("cxy: %0.3f %0.3f  r=%0.3f\n",cx,cy,r);
        cx/=r;
        cy/=r;
        printf("cxy: %0.3f %0.3f\n",cx,cy);
        lcd_line(120,120, 120+cy*110, 120-cx*110, CYAN, 1);
        lcd_line(121,120, 121+cy*110, 120-cx*110, CYAN, 1);
        lcd_line(120,121, 120+cy*110, 121-cx*110, CYAN, 1);


      }
      if(cmode==CM_Changepos || cmode==CM_Editpos){
        // wrist-control (arm==x-axis)
        int asx = (int)acc[0];
        asx-=hgx;
        if( asx > HOURGLASSBORDER || asx < -HOURGLASSBORDER ){
          int a = asx;
          if(a<0){a=-a;}
          hourglass_x -= a;
          a>>=2;
          //printf("hgx a: %d\n",a);
          if( hourglass_x <=0 ){
            hourglass_x=HOURGLASS;
            //if(asx>0){tcw=true;}else{tccw=true;}
            if(asx>0){ dir_y=D_MINUS;tcw=true;}
            if(a==0){dir_y=D_NONE;}
            if(asx<0){ dir_y=D_PLUS;tccw=true;}
          }
        }
        int asy = (int)acc[1];
        asy-=hgy;
        if( asy > HOURGLASSBORDER || asy < -HOURGLASSBORDER ){
          int a = asy;
          if(a<0){a=-a;}
          hourglass_y -= a;
          a>>=2;
          //printf("hgy a: %d\n",a);
          if( hourglass_y <=0 ){
            hourglass_y=HOURGLASS;
            //if(asy>0){tcw=true;}else{tccw=true;}
            if(asy>0){ dir_x=D_PLUS;tcw=true;}
            if(a==0){dir_x=D_NONE;}
            if(asy<0){ dir_x=D_MINUS;tccw=true;}
          }
        }
      }

      if(cmode==CM_Changepos){
        if(NO_POS_MODE){
          if(dir_x==D_PLUS){
            puts("Dright");
            if(pos_matrix_x<2)++pos_matrix_x;
          }
          if(dir_x==D_MINUS){
            puts("Dleft");
            if(pos_matrix_x>0)--pos_matrix_x;
          }

          if(dir_y==D_PLUS){
            if(pos_matrix_y<2)++pos_matrix_y;
            puts("Ddown");
          }
          if(dir_y==D_MINUS){
            puts("Dup");
            if(pos_matrix_y>0)--pos_matrix_y;
          }
          colors[editpos]=dcolors[editpos];
          editpos=pos_matrix[pos_matrix_y*3+pos_matrix_x];
          //printf("posM: %d %d [%d]\n",pos_matrix_x, pos_matrix_y, pos_matrix[pos_matrix_y*3+pos_matrix_x]);
          dir_x = D_NONE;
          dir_y = D_NONE;

        }else{
          if(tcw){
            colors[editpos]=dcol;
            if(editpos==7){editpos=0;}else{++editpos;}
            colors[editpos]=editcol;
            tcw=false;
          }else if(tccw){
            colors[editpos]=dcol;
            if(editpos==0){editpos=7;}else{--editpos;}
            colors[editpos]=editcol;
            tccw=false;
          }
        }
      }

      if(cmode==CM_Editpos){
        bool set=false;
        if(tcw){
          //colors[editpos]=dcolors[editpos];
          colors[editpos]=changecol;
          switch(editpos){
            case 0: (t.dotw==6)?t.dotw=0:++t.dotw;break;
            case 1: (t.day==last[t.month])?t.day=1:++t.day;break;
            case 2: (t.month==12)?t.month=1:++t.month;break;
            case 3: (t.year==2099)?t.year=2022:++t.year;break;
            case 4: (t.hour==23)?t.hour=0:++t.hour;break;
            case 5: (t.min==59)?t.min=0:++t.min;break;
            case 6: (t.sec==59)?t.sec=0:++t.sec;break;
            case 7: {
              (theme_pos==THEMES-1)?theme_pos=0:++theme_pos;
              copy_pos_matrix(theme_pos);
              set_dcolors();
              break;
            }
          }
          tcw=false;
          //set=true;
        }
        if(tccw){
          //colors[editpos]=dcolors[editpos];
          colors[editpos]=changecol;
          switch(editpos){
            case 0: (t.dotw==0)?t.dotw=6:--t.dotw;break;
            case 1: (t.day==1)?t.day=last[t.month]:--t.day;break;
            case 2: (t.month==1)?t.month=12:--t.month;break;
            case 3: (t.year==2099)?t.year=2022:--t.year;break;
            case 4: (t.hour==0)?t.hour=23:--t.hour;break;
            case 5: (t.min==0)?t.min=59:--t.min;break;
            case 6: (t.sec==0)?t.sec=59:--t.sec;break;
            case 7: {
              (theme_pos==0)?theme_pos=THEMES-1:--theme_pos;
              copy_pos_matrix(theme_pos);
              set_dcolors();
              break;
            }
          }
          tccw=false;
          //set=true;
        }
        if(set){
          rtc_set_datetime(&t);
          if(!(t.year%4)){last[2]=29;}else{last[2]=28;}
        }
      }
      if(cmode==CM_Editpos || cmode==CM_Changepos){
        uint16_t cmode_color = GREEN;
        if(cmode==CM_Changepos){
          blink_counter++;
          if(blink_counter==5){ bmode=!bmode;blink_counter=0; }
          cmode_color = blinker[bmode];
        }
        if(editpos==0){
          if(theme_pos==0){
            lcd_rect(POS_CNDOW_X-6,POS_CNDOW_Y,POS_CNDOW_X+CNFONT.w+3,POS_CNDOW_Y+CNFONT.h*3+1,cmode_color,3);
          }else{
            lcd_circle(tpos[editpos].x+18,tpos[editpos].y+8,25,cmode_color,3,0);
          }
        }else if(editpos==3){
          lcd_circle(tpos[editpos].x+12,tpos[editpos].y+5,30,cmode_color,3,0);
        }else if(editpos==7){
          lcd_circle(tpos[editpos].x,tpos[editpos].y,19,cmode_color,3,0);
        }else{
          lcd_circle(tpos[editpos].x+12,tpos[editpos].y+5,20,cmode_color,3,0);
        }
      }

      if(!theme_bg_dynamic_mode){
        if(draw_gfx_first||cmode==CM_Editpos){
          draw_gfx();
          draw_text();
        }else{
          draw_text();
          draw_gfx();
        }
      }
      lcd_display(b0);
      sleep_ms((analog_seconds)?1:50);

    }
    return 0;
}
