/*****************************************************************************
* | File      	:   LCD_1in28_test.c
* | Author      :   Waveshare team
* | Function    :   1.3inch LCD  test demo
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2021-08-20
* | Info        :
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#include "LCD_Test.h"
#include "LCD_1in28.h"
#include "QMI8658.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/util/datetime.h"
#include "hardware/adc.h"
#include "hardware/rtc.h"
#include "hardware/gpio.h"
#include <math.h>
#include <float.h>
#include "pico/types.h"
#include "pico/bootrom/sf_table.h"
#include "stdlib.h"
#include "Font32ALL.h"
#include "Font30.h"
#include "cn16.h"
#include "usa16.h"
#include "ger16.h"
#include "tr16.h"
#include "cn32.h"
#include "usa32.h"
#include "ger32.h"
#include "tr32.h"

// Start on Friday 5th of June 2020 15:45:00
datetime_t t = {
  .year  = 2022,
  .month = 10,
  .day   = 13,
  .dotw  = 4, // 0 is Sunday, so 5 is Friday
  .hour  = 7,
  .min   = 10,
  .sec   = 0
};


#define DRAW_GFX_FIRST true //1 == text floating above clock
#define to_rad(angleInDegrees) ((angleInDegrees) * M_PI / 180.0)
#define to_deg(angleInRadians) ((angleInRadians) * 180.0 / M_PI)
#define HOURGLASSBORDER 200 // minimum rise/fall of acc_x
#define HOURGLASS 600  // rise/fall of acc_x border till switch (cw/ccw)
#define BUTTONGLASSC 300
#define BUTTONGLASS 1400
#define SCRSAV 60

#define BRIGHTD 20
#define FLAGS_DELAY 4

#define ACCX 60
#define ACCY 40

#define BATX 70
#define BATY 30
#define BATYS 4
#define TFONT Font20
#define TFW 14
#define DATIX 2
#define DATIY 86
#define HXST 64
#define HYST 136


void sincosf(float,float*,float*);

typedef struct ColorTheme_t{
  uint16_t alpha;
  uint16_t col1;
  uint16_t col2;
  uint16_t col3;
} ColorTheme_t;

#define USA_Old_Glory_Red 0xB0C8 //0xB31942
#define USA_Old_Glory_Blue 0x098C //0x0A3161

#define CN_Red 0xE8E4 //0xee1c25
#define CN_Gold 0xFFE0 //0xffff00

#define GER_Gold 0xFE60

#define TR_Red 0xF800

#define THEMES 4
//#define FLAGS_MAX 4


uint8_t theme_pos = 0;
uint8_t* flags[] = {cn32,usa32,ger32,tr32};
uint8_t* stars[] = {cn16,usa16,ger16,tr16};
uint16_t alpha[] = {BLACK,BLACK};



ColorTheme_t colt1={BLACK,CN_Red,CN_Red,CN_Gold};
ColorTheme_t colt2={BLACK,USA_Old_Glory_Red,USA_Old_Glory_Blue,WHITE};
ColorTheme_t colt3={BLACK,0xF800,0x0001,GER_Gold};
ColorTheme_t colt4={BLACK,TR_Red,TR_Red,WHITE};

//ColorTheme_t* colt[2] = [&colt1,&colt2];
ColorTheme_t* colt[THEMES];

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
int hourglass=HOURGLASS;
int hgb;
int hgy;
int buttonglass=BUTTONGLASS;
int screensaver=SCRSAV;
int flagsdelay = FLAGS_DELAY;


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
uint16_t dcol = WHITE;
uint16_t editcol = YELLOW;
uint16_t changecol = ORANGE;
uint16_t acol=WHITE;
uint16_t colors[7] = {WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE};

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

uint8_t find_cc(uint8_t a, uint8_t b, uint8_t c){
  uint fo=0;
  for(int i=0; i<cn_chars+1;i++){
    //printf("[%02x%02x%02x] %02x %02x %02x\n",a,b,c,ftst[fo],ftst[fo+1],ftst[fo+2]);
    if( (ftst[fo+0]==a) && (ftst[fo+1]==b) && (ftst[fo+2]==c) ){
      printf("find_cc: %d %d\n",i,i+228);
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


void blit(int x, int y,int sx, int sy,char* data, uint16_t alpha){
  int iso=0;
  for(int iy=0;iy<sx;iy++){
    for(int ix=0;ix<sy;ix++){
      if(((uint16_t*)data)[iso]!=alpha){
        Paint_SetPixel(x+ix,y+iy,((uint16_t*)data)[iso]); }
      ++iso;
    }
  }
}


void draw_gfx(){
  uint8_t x1,y1,xt,yt;
  uint8_t x0=120;
  uint8_t y0=120;

  Paint_DrawLine(BATX    ,BATY,   BATX+102, BATY, BLUE , 1, LINE_STYLE_SOLID);//top
  Paint_DrawLine(BATX    ,BATY+BATYS, BATX+102, BATY+BATYS, BLUE , 1, LINE_STYLE_SOLID);//bottom
  Paint_DrawLine(BATX    ,BATY,   BATX,     BATY+BATYS, BLUE , 1, LINE_STYLE_SOLID);//left
  Paint_DrawLine(BATX+102,BATY,   BATX+102, BATY+BATYS, BLUE , 1, LINE_STYLE_SOLID);//right
  uint16_t bat = (uint16_t)(((result * conversion_factor)/4.17)*100);
  //printf("bat :  %03d\n",bat);
  if(bat>100){bat=100;}
  Paint_DrawLine(BATX+1    ,BATY+1,   BATX+bat-1, BATY+1, WHITE , 1, LINE_STYLE_SOLID);//top
  Paint_DrawLine(BATX+1    ,BATY+2,   BATX+bat-1, BATY+2, WHITE , 1, LINE_STYLE_SOLID);//top
  Paint_DrawLine(BATX+3    ,BATY+3,   BATX+bat-1, BATY+3, WHITE , 1, LINE_STYLE_SOLID);//top
  if(!usb_loading){
    sprintf(dbuf,"  %02d%%",bat);
  }else{
    sprintf(dbuf,"LOADING",bat);
  }
  Paint_DrawString_EN(94, 12, dbuf, &Font12, WHITE, RED);

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
      Paint_DrawLine((uint8_t)x0+xi,(uint8_t)y0+yi, x1, y1, colt[theme_pos]->col1 , 1, LINE_STYLE_SOLID);
    }else{
      xi = (int)(tcos[i*6]*115);
      yi = (int)(tsin[i*6]*115);
      Paint_DrawLine((uint8_t)x0+xi,(uint8_t)y0+yi, x1, y1, colt[theme_pos]->col2 , 1, LINE_STYLE_SOLID);
    }
  }
  xi = (int)(tcos[t.min*6]*105);
  yi = (int)(tsin[t.min*6]*105);
  x1 = (uint8_t)x0+xi;
  y1 = (uint8_t)y0+yi;
  Paint_DrawLine(x0,y0, x1, y1, colt[theme_pos]->col3 , 2, LINE_STYLE_SOLID);

  int th=(int)t.hour;
  if(th>=12){th-=12;}
  th*=30;
  th+=(int)(t.min>>1);
  xi = (int)(tcos[th]*64);
  yi = (int)(tsin[th]*64);
  x1 = (uint8_t)x0+xi;
  y1 = (uint8_t)y0+yi;
  Paint_DrawLine(x0,y0, x1, y1, colt[theme_pos]->col1 , 3, LINE_STYLE_SOLID);

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
    Paint_DrawLine(x0,y0, x1, y1, colt[theme_pos]->col2 , 1, LINE_STYLE_SOLID);
    blit((int)(x0-8+tcos[t.sec*6]*102),(int)(y0-8+tsin[t.sec*6]*102),16,16,stars[theme_pos],colt[theme_pos]->alpha);
  }else{
    // 'analog' seconds
    xi = (int)(tfcos[t.sec*10+st]*114);
    yi = (int)(tfsin[t.sec*10+st]*114);
    x1 = (uint8_t)x0+xi;
    y1 = (uint8_t)y0+yi;
    Paint_DrawLine(x0,y0, x1, y1, colt[theme_pos]->col2 , 1, LINE_STYLE_SOLID);
    blit((int)(x0-8+tfcos[t.sec*10+st]*102),(int)(y0-8+tfsin[t.sec*10+st]*102),16,16,stars[theme_pos],colt[theme_pos]->alpha);

  }

  blit(120-16,120-16,32,32,flags[theme_pos],colt[theme_pos]->alpha); // center

}


void draw_text(){
  Paint_DrawString_EN(ACCX, ACCY    ,     "ACC_X = ", &Font12, WHITE,  CYAN);
  Paint_DrawString_EN(ACCX, ACCY+16 , "ACC_Y = ", &Font12, WHITE,  CYAN);
  Paint_DrawString_EN(ACCX, ACCY+32 , "ACC_Z = ", &Font12, WHITE, CYAN);
  Paint_DrawString_EN(ACCX, ACCY+114 , "GYR_X = ", &Font12, WHITE, CYAN);
  Paint_DrawString_EN(ACCX, ACCY+128, "GYR_Y = ", &Font12, WHITE, CYAN);
  Paint_DrawString_EN(ACCX, ACCY+142, "GYR_Z = ", &Font12, WHITE, CYAN);
  Paint_DrawNum(130, ACCY    , acc[0], &Font12, 2,   YELLOW,   WHITE);
  Paint_DrawNum(130, ACCY+16 , acc[1], &Font12, 2,   YELLOW,   WHITE);
  Paint_DrawNum(130, ACCY+32 , acc[2], &Font12, 2,  YELLOW,  WHITE);
  Paint_DrawNum(130, ACCY+114 , gyro[0], &Font12, 2, YELLOW, WHITE);
  Paint_DrawNum(130, ACCY+128, gyro[1], &Font12, 2, YELLOW, WHITE);
  Paint_DrawNum(130, ACCY+142, gyro[2], &Font12, 2, YELLOW, WHITE);
  Paint_DrawString_EN(70, 194, "TEMP = ", &Font12, WHITE, CYAN);
  Paint_DrawNum(120, 194, temperature, &Font12, 2, YELLOW, WHITE);
  Paint_DrawString_EN(50, 208, "BAT(V)=", &Font16, WHITE, BLACK);
  Paint_DrawNum(130, 208, result * conversion_factor, &Font16, 2, BLACK, WHITE);

  if(!theme_pos){
    convert_cs(week[theme_pos][t.dotw],cn_buffer);
    Paint_ext=true;
    Paint_DrawString_EN(200, 72, cn_buffer, &Font30, WHITE, colors[0]);
    Paint_ext=false;
    printf("cn_buffer: %s\n",cn_buffer);
  }else{
    Paint_DrawString_EN(20, 111, week[theme_pos][t.dotw], &TFONT, WHITE, colors[0]);
  }

  sprintf(dbuf,"%02d",t.day);
  Paint_DrawString_EN(DATIX+3*TFW, DATIY, dbuf, &TFONT, WHITE, colors[1]);
  Paint_DrawString_EN(DATIX+5*TFW, DATIY, ".", &TFONT, WHITE, WHITE);
  sprintf(dbuf,"%02d",t.month);
  Paint_DrawString_EN(DATIX+6*TFW, DATIY, dbuf, &TFONT, WHITE, colors[2]);
  Paint_DrawString_EN(DATIX+8*TFW, DATIY, ".", &TFONT, WHITE, WHITE);
  sprintf(dbuf,"%04d",t.year);
  Paint_DrawString_EN(DATIX+9*TFW, DATIY, dbuf, &TFONT, WHITE, colors[3]);

  //Paint_DrawString_EN(15, 104, dt_date, &TFONT, WHITE, WHITE);
  sprintf(dbuf,"%02d",t.hour);
  Paint_DrawString_EN(HXST, HYST, dbuf, &TFONT, WHITE, colors[4]);
  Paint_DrawString_EN(HXST+2*TFW, HYST, ":", &TFONT, WHITE, WHITE);
  sprintf(dbuf,"%02d",t.min);
  Paint_DrawString_EN(HXST+3*TFW, HYST, dbuf, &TFONT, WHITE, colors[5]);
  Paint_DrawString_EN(HXST+5*TFW, HYST, ":", &TFONT, WHITE, WHITE);
  sprintf(dbuf,"%02d",t.sec);
  Paint_DrawString_EN(HXST+6*TFW, HYST, dbuf, &TFONT, WHITE, colors[6]);
}


int LCD_1in28_test(void)
{
    if (DEV_Module_Init() != 0)    {        return -1;    }
    printf("LCD_1in28_test piclo\r\n");
    adc_init();
    adc_gpio_init(29);
    adc_select_input(3);
    /* LCD Init */
    LCD_1IN28_Init(HORIZONTAL);
    LCD_1IN28_Clear(WHITE);
    DEV_SET_PWM(BRIGHTD);
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
    UDOUBLE Imagesize = LCD_1IN28_HEIGHT * LCD_1IN28_WIDTH * 2;
    UWORD *BlackImage;
    if ((BlackImage = (UWORD *)malloc(Imagesize)) == NULL)
    {
        printf("Failed to apply for black memory...\r\n");
        exit(0);
    }
    // /*1.Create a new image cache named IMAGE_RGB and fill it with white*/
    Paint_NewImage((UBYTE *)BlackImage, LCD_1IN28.WIDTH, LCD_1IN28.HEIGHT, 0, WHITE);
    Paint_SetScale(65);
    Paint_Clear(WHITE);
    Paint_SetRotate(ROTATE_0);
    Paint_Clear(WHITE);
    LCD_1IN28_Display(BlackImage);
    Paint_DrawImage(gImage_1inch3_1, 0, 0, 240, 240);
    LCD_1IN28_Display(BlackImage);

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
    DEV_Delay_ms(1000);
    print_font_table();
    while (true)    {
        QMI8658_read_xyz(acc, gyro, &tim_count);
        //check if not moving
        if((gyro[0]>-10.0f&&gyro[0]<10.0f)&&(gyro[1]>-10.0f&&gyro[1]<10.0f)&&(gyro[2]>-10.0f&&gyro[2]<10.0f)){
          if(!is_sleeping && !edittime && !changetime && !usb_loading){
            screensaver--;
            if(screensaver<=0){
              is_sleeping=true;
              screensaver=SCRSAV;
              DEV_SET_PWM(0);
              LCD_1IN28_SLEEPON();
            }
          }
        }else{
          if(is_sleeping){
            is_sleeping=false;
            DEV_SET_PWM(BRIGHTD);
            LCD_1IN28_SLEEPOFF();
          }
        }

        if(is_sleeping){
          DEV_Delay_ms(1000);
          continue;
        }

        rtc_get_datetime(&t);
        result = adc_read();
        // (v>4.15)?true:false
        usb_loading = ((result * conversion_factor)>=4.15);
        Paint_DrawImage(gImage_1inch3_1, 0, 0, 240, 240);


        if(fire){
          hgb = acc[0];
          hgy = acc[1];
          if(!edittime && !changetime){edittime=true;colors[editpos]=editcol;}
          else if(!changetime){edittime=false;changetime=true;colors[editpos]=changecol;}
          else if(!edittime){changetime=false;colors[editpos]=dcol;}
          fire=false;
        }
        if(edittime || changetime){
          // wrist-control (arm==x-axis)
          int as = acc[0];
          as-=hgb;
          //printf("hgb: %d as: %d\n",hgb,as);
          if( as > HOURGLASSBORDER || as < -HOURGLASSBORDER ){
            int a = as;
            if(a<0){a*=-1;}
            hourglass -= a;
            //printf("hg: %d\n",hourglass);
            if( hourglass <=0 ){
              hourglass=HOURGLASS;
              if(as>0){tcw=true;}else{tccw=true;}
            }
          }
        }

        if(edittime){
          if(tcw){
            colors[editpos]=dcol;
            if(editpos==6){editpos=0;}else{++editpos;}
            colors[editpos]=editcol;
            tcw=false;
          }else if(tccw){
            colors[editpos]=dcol;
            if(editpos==0){editpos=6;}else{--editpos;}
            colors[editpos]=editcol;
            tccw=false;
          }
        }

        if(changetime){
          bool set=false;
          if(tcw){
            colors[editpos]=dcol;
            switch(editpos){
              case 0: (t.dotw==6)?t.dotw=0:++t.dotw;break;
              case 1: (t.day==last[t.month])?t.day=1:++t.day;break;
              case 2: (t.month==12)?t.month=1:++t.month;break;
              case 3: (t.year==2099)?t.year=2022:++t.year;break;
              case 4: (t.hour==23)?t.hour=0:++t.hour;break;
              case 5: (t.min==59)?t.min=0:++t.min;break;
              case 6: (t.sec==59)?t.sec=0:++t.sec;break;
            }
            colors[editpos]=changecol;
            tcw=false;
            set=true;
          }
          if(tccw){
            colors[editpos]=dcol;
            switch(editpos){
              case 0: (t.dotw==0)?t.dotw=6:--t.dotw;break;
              case 1: (t.day==1)?t.day=last[t.month]:--t.day;break;
              case 2: (t.month==1)?t.month=12:--t.month;break;
              case 3: (t.year==2099)?t.year=2022:--t.year;break;
              case 4: (t.hour==0)?t.hour=23:--t.hour;break;
              case 5: (t.min==0)?t.min=59:--t.min;break;
              case 6: (t.sec==0)?t.sec=59:--t.sec;break;
            }
            colors[editpos]=changecol;
            tccw=false;
            set=true;
          }
          if(set){
            rtc_set_datetime(&t);
            if(!(t.year%4)){last[2]=29;}else{last[2]=28;}
          }
        }



        if(draw_gfx_first){
          draw_gfx();
          draw_text();
        }else{
          draw_text();
          draw_gfx();
        }
        LCD_1IN28_Display(BlackImage);

#define THRS 12
#define THRLY 30
        int as = acc[1];  // y-axis
        as-=hgy;

				if((as>THRLY) && edittime){
          printf("right\n");
          if(!flagsdelay){
            theme_pos++;
            if(theme_pos==THEMES){theme_pos=0;}
            flagsdelay=FLAGS_DELAY;
          }else{
            --flagsdelay;
          }
        }
				if((as<-THRLY) && edittime){
          printf("left\n");
          if(!flagsdelay){
            if(theme_pos==0){theme_pos=THEMES;}
            theme_pos--;
            flagsdelay=FLAGS_DELAY;
          }else{
            --flagsdelay;
          }
        }
        if(gyro[1]>THRS){printf("up\n");} // shake up
        if(gyro[1]<-THRS){printf("down\n");} //shake down
        //printf("%d\n",st/100000);
        //printf("[%d] {as%d} fd=%d\n",theme_pos,as,flagsdelay);

        DEV_Delay_ms((analog_seconds)?1:100);
    }

    /* Module Exit */
    //free(BlackImage);
    //BlackImage = NULL;

    DEV_Module_Exit();
    return 0;
}
