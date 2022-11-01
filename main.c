static __attribute__((section (".noinit")))char losabuf[4096];

#include "stdio.h"
#include "pico/stdlib.h"
#include "stdlib.h"
#include "string.h"
#include "pico/time.h"
#include <math.h>
#include "pico/util/datetime.h"
#include "hardware/adc.h"
#include "hardware/rtc.h"
#include "hardware/gpio.h"
#include <hardware/flash.h>
#include "hardware/watchdog.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/binary_info.h"
#include <float.h>
#include "pico/types.h"
#include "pico/bootrom/sf_table.h"
#include <stdio.h>
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"

#include "lcd.h"
#include "QMI8658.h"
//#include "lib/bme280.h"
//#include "lib/Fonts/fonts.h"
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

//textures
#include "img/w2.h"
#include "img/mo1.h"
#include "img/flow.h"

typedef enum {
  GFX_NORMAL,
  GFX_DYNABG,
  GFX_ROTOZOOM,
  GFX_ROTATE,
} GFX_MODE;

typedef enum {
  PS_NORMAL,
  PS_ALPHA,
  PS_TEXTURE,
  PS_BENDER
} PSTYLE; //Clock Pointer Style


typedef struct Battery_t {
  char mode[8];
  float mA;
  float load;
  float max;
  float min;
  float dif;
  float read;
} Battery_t;



typedef struct {
  char mode[8];
  datetime_t dt;
  Battery_t bat;
  uint8_t theme_pos;
  uint8_t editpos;
  uint8_t BRIGHTNESS;

  bool SENSORS;
  bool GYROCROSS;
  bool SECOND_BENDER;
  bool SMOOTH_BACKGROUND;
  bool INSOMNIA;
  bool DYNAMIC_CIRCLES;
  bool DEEPSLEEP;
  bool is_sleeping;
  bool highpointer;
  bool alphapointer;
  bool clock;
  bool pointerdemo;
  GFX_MODE gfxmode;
  PSTYLE pstyle;
  int16_t spin;
  uint8_t texture;
} LOSA_t;

static LOSA_t* plosa=(LOSA_t*)losabuf;


datetime_t default_time = {
  .year  = 2022,
  .month = 10,
  .day   = 22,
  .dotw  = 6, // 0 is Sunday, so 5 is Friday
  .hour  = 22,
  .min   = 55,
  .sec   = 0
};

int16_t flagdeg=90;
int16_t flagdeg1=30;
int16_t flagdeg2=60;
int16_t flagdeg1a=30;
int16_t flagdeg2a=60;
int16_t flagdeg1b=130;
int16_t flagdeg2b=260;
#define DEFAULT_THEME 0
// NO_POS_MODE 1 : gyroscope+button control
#define NO_POS_MODE 1
#define SHELL_ENABLED 1
// NO_SENSORS 1 : don't show sensor values [gyro,acc,bat]

// DEEPSLEEP : increases sleep_frame by SLEEP_FRAME_ADD till SLEEP_FRAME_END
// so at max, pico is only able to awake every 10th second
//#define DEEPSLEEP 0
#define SLEEP_FRAME_START 1000
#define SLEEP_FRAME_END   30000
#define SLEEP_FRAME_ADD   100

Battery_t bat0 = {"GOOD\0", 150.0f, 4.16f, 3.325781f, 4.158838f, 0.0f, 0.0f}; // 150mA
Battery_t bat1 = {"GOOD\0",1100.0f, 4.19f, 3.346729f, 4.189453f, 0.0f, 0.0f}; //1100ma

// add new battery:
// change the XX below to your value when the battery is loading [the least one]
// set bat_default = &bat2;
//Battery_t bat2 = {"GOOD\0",1100.0f, 4.XXf, 0.0f, 5.55f, 0.0f, 0.0f};

Battery_t* bat_default= &bat0;


const float conversion_factor = 3.3f / (1 << 12) * 2;
#define BAT_NUMRES 16
uint16_t resulti = BAT_NUMRES-1; // pre incremented – so becomes 0 in first run
float result[BAT_NUMRES] = {0.0f};

float resultsum(){
  float sum=0.0f;
  for(uint8_t i=0;i<BAT_NUMRES;i++){sum+=result[i];}
  return sum;
}
float resultsummid(){
  float sum=0.0f;
  for(uint8_t i=0;i<BAT_NUMRES;i++){sum+=result[i];}
  return sum/BAT_NUMRES;

}
float resultminmaxmid(){
  float min=99.0f, max=0.0f;
  for(uint8_t i=0;i<BAT_NUMRES;i++){
    if(result[i]<min){min=result[i];}
    if(result[i]>max){max=result[i];}
  }
  return (min+max)/2.0f;
}

void dosave();
void draw_pointer(Vec2 vs, Vec2 vts, int16_t tu, uint16_t color, const uint8_t* sr, uint16_t alpha);
void draw_pointer_mode(Vec2 vs, Vec2 vts, int16_t tu, uint16_t color, const uint8_t* sr, uint16_t alpha, PSTYLE cps);

float read_battery(){
  plosa->bat.read = adc_read()*conversion_factor;
  if(plosa->bat.read<plosa->bat.load){
    //printf("battery_read: %f\n",plosa->bat.read);
    if(plosa->bat.read>plosa->bat.max){
      plosa->bat.max=plosa->bat.read;
      //printf("battery_max: %f\n",plosa->bat.max);
    }
    if(plosa->bat.read<plosa->bat.min){
      plosa->bat.min=plosa->bat.read;
      if(plosa->is_sleeping && plosa->BRIGHTNESS==0){
        dosave();
      }
      //printf("battery_min: %f\n",plosa->bat.min);
    }
    if(plosa->bat.dif != (plosa->bat.max-plosa->bat.min)){
      //printf("battery_dif: %f\n",plosa->bat.dif);
    }
    plosa->bat.dif = plosa->bat.max-plosa->bat.min;
  }
  return plosa->bat.read;
}



#define BATTERY_MAKFOC_1100mA 4.18f
#define BATTERY_BERBAS_150mA_loading 4.16f
#define BATTERY_BERBAS_150mA_max 4.10f
#define BATTERY_BERBAS_150mA_min 3.80f
#define BATTERY_V_load     BATTERY_BERBAS_150mA_loading
#define BATTERY_V_max BATTERY_BERBAS_150mA_max
#define BATTERY_V_min BATTERY_BERBAS_150mA_min
#define BATTERY_V_dif BATTERY_V_max-BATTERY_V_min

#define TFONT Font20
#define CNFONT Font30


#define mcpy(d,s,sz) for(int i=0;i<sz;i++){d[i]=s[i];}
#define THEMES 4

#define EYE irisa190


#define FRAME_DELAY 50
#define LOOPWAIT 50

#define DRAW_GFX_FIRST true //1 == text floating above clock
#define HOURGLASSBORDER 200 // minimum rise/fall of acc_x
#define HOURGLASS 1000*(100/LOOPWAIT)  // rise/fall of acc_x border till switch (cw/ccw)
#define BUTTONGLASSC 300
#define BUTTONGLASS 1400
#define SCRSAV 15*(100/LOOPWAIT)
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
#define POS_DATE_Y 66

#define POS_DOW_X 20
#define POS_DOW_Y 111
#define POS_CNDOW_X 190
#define POS_CNDOW_Y 72

#define POS_TIME_X 64
#define POS_TIME_Y 156

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
  uint16_t bat_level;
  uint16_t bat_level_low;
  uint16_t bat_level_critical;
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

#define EDITPOSITIONS 7
  PXY_t tpos[EDITPOSITIONS+1] =
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

/* Position Matrices
it's like a gear shifter
[ | / \] : up/down movement ways
[ -    ] : left/right movement ways

example us [3*3]:
DAY-MON-YEAR
 |   |  /
DOW-FLAG
 |   |  \
H  - M - S

example cn [4*3]:
DAY-MON-YEAR-DOW
  \  |  /     |
   FLAG    - DOW
  /  |  \     |
H  - M - S - DOW

*/

// pos points to position matrix, containing possible 'editpos' (positions)
  typedef struct PosMat_t{
    uint8_t dim_x;
    uint8_t dim_y;
    uint8_t* pos;
  } PosMat_t;

  int8_t pos_matrix_CN[] =
  {
    1,2,3,0,
    7,7,7,0,
    4,5,6,0
  };


  int8_t pos_matrix_US[] =
  {
    1,2,3,
    0,7,7,
    4,5,6
  };

  PosMat_t p_us = {3,3,pos_matrix_US};
  PosMat_t p_cn = {4,3,pos_matrix_CN};


#define USA_Old_Glory_Red  0xB0C8 //0xB31942
#define USA_Old_Glory_Blue 0x098C //0x0A3161

#define CN_Red 0xE8E4 //0xee1c25
#define CN_Gold 0xFFE0 //0xffff00

#define GER_Gold 0xFE60
#define GER_Red 0xF800

#define TR_Red 0xF800
#define THEMES 4

CMode cmode = CM_None;
int8_t xold,xoldt;
int8_t yold,yoldt;

//uint8_t theme_pos = DEFAULT_THEME;
const PosMat_t* positions[THEMES] = {&p_cn,&p_us,&p_us,&p_us};
const uint8_t* flags[THEMES] = {cn32,usa32,ger32,tr32};
const uint8_t* stars[THEMES] = {cn16,usa16,ger16,tr16};

uint16_t tex = 0;
const char* textures[3] ={w2,flow,mo1};
Vec2 psize_h[3] = {65,5,75,20,75,20};
Vec2 psize_m[3] = {102,4,102,10,102,10};
const char* backgrounds[THEMES] = {earth190,irisa190,bega,sand};
const int16_t bgsz[THEMES] = {190,190,240,240};

const bool bg_dynamic[THEMES] = {true,true,false,false};

const uint16_t edit_colors[THEMES] = {ORANGE,YELLOW,ORANGE,ORANGE};
const uint16_t change_colors[THEMES] = {YELLOW,YELLOW,YELLOW,YELLOW};

uint8_t theme_bg_dynamic_mode = 0;

ColorTheme_t colt1={BLACK,CN_Red,CN_Red,CN_Gold,CN_Red,CN_Gold,WHITE,WHITE,WHITE,WHITE,YELLOW,RED};
ColorTheme_t colt2={BLACK,USA_Old_Glory_Red,USA_Old_Glory_Blue,NWHITE,USA_Old_Glory_Red,WHITE,WHITE,WHITE,WHITE,WHITE,YELLOW,RED};
ColorTheme_t colt3={BLACK,GER_Red,BLACK,GER_Gold,GER_Red,GER_Gold,WHITE,WHITE,WHITE,WHITE,YELLOW,RED};
ColorTheme_t colt4={BLACK,TR_Red,TR_Red,WHITE,NWHITE,TR_Red,WHITE,WHITE,WHITE,WHITE,YELLOW,RED};

ColorTheme_t* colt[THEMES];

void update_pos_matrix(){
  if(pos_matrix_x>=positions[plosa->theme_pos]->dim_x){
    pos_matrix_x=positions[plosa->theme_pos]->dim_x-1;
  }
  if(pos_matrix_y>=positions[plosa->theme_pos]->dim_y){
    pos_matrix_y=positions[plosa->theme_pos]->dim_y-1;
  }
}

Vec2 vO = {120,120};
Vec2 v0 = {0,0};



uint16_t dcol = WHITE;
uint16_t editcol = YELLOW;
uint16_t changecol = YELLOW;
uint16_t acol=WHITE;
uint16_t colors[8]  = {WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE};
uint16_t dcolors[8] = {WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE};

uint16_t blinker[2] = {BLUE,RED};

typedef enum Dir_t {
  D_NONE = 0,
  D_PLUS = 1,
  D_MINUS = 2
} Dir_t;


Dir_t dir_x;
Dir_t dir_y;
uint8_t no_pos_x=0;
uint8_t no_pos_y=0;

int16_t gyrox=0;
int16_t gyroy=0;

uint32_t sleep_frame = SLEEP_FRAME_START;

char timebuffer[16] = {0};
char* ptimebuffer=timebuffer;
bool h24=true;

uint16_t comi=0;
char combufa[256]={0};
int16_t comt;
uint8_t comc;

uint8_t* b0=NULL;

//shell vars!
int16_t bcx0 = 80;
int16_t bcy0 = 80;
int16_t bcx1 = 80;
int16_t bcy1 = 80;

int16_t tpoy = 30;
int16_t tpox = 22;

int16_t tpol = -22;
int16_t tpor = 22;


//Bez2_t* bezt[8] = {NULL};
//
//Bez2_t* tbez = NULL;
//int16_t bfc = 0;

//char datetime_buf[256];
//char *datetime_str = &datetime_buf[0];
//char* dt_date;
//char* dt_time;

//uint32_t dps=0;
//uint32_t dpsc=0;

//ky-040
#define CCLK 16
#define CDT 17
#define CSW 19

//one button /
#define CBUT0 22
#define CBUT1 3
uint32_t nopvar;
bool fire_pressed=false;
bool analog_seconds=false;
uint32_t fire_counter=0;
bool fire=false;
bool ceasefire=false;
bool tcw=false;
bool tccw=false;
bool clk,dt,sw,oclk,odt,osw;
bool temp_read=false;
int gc=0;
char gch;
char gbuf[2] = {'c','d'};
uint32_t last_wait;
uint32_t stime;
uint8_t tseco;
int16_t hourglass_x=HOURGLASS;
int16_t hourglass_y=HOURGLASS;

bool hg_enabled=false;
int16_t hgx=0;
int16_t hgy=0;

int16_t buttonglass=BUTTONGLASS;
int16_t screensaver=SCRSAV;

int flagsdelay = SWITCH_THEME_DELAY;
int blink_counter = 0;
bool bmode = false;

float tsin[360];
float tcos[360];
float tfsin[600];
float tfcos[600];


bool edittime=false;
bool changetime=false;
char dbuf[8];
float temperature = -99.99f;

float mag[3];
bool usb_loading = false;
bool draw_gfx_first = DRAW_GFX_FIRST;

float acc[3], gyro[3];
unsigned int tim_count = 0;
float last_z = 0.0f;

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

bool do_reset = false;
bool force_no_load = false;
bool is_flashed = false;



//objdump -x main.elf | grep binary_info_end
//1006f7d8 g       .binary_info   00000000 __binary_info_end
// 0x90000 == xip_offset (must be bigger then the above value from objdump)

void __no_inline_not_in_flash_func(flash_data_load)(){
	uint32_t xip_offset = 0x90000;
	char *p = (char *)XIP_BASE+xip_offset;
	for(size_t i=0;i<FLASH_SECTOR_SIZE;i++){ losabuf[i]=p[i];	}
}

void __no_inline_not_in_flash_func(flash_data)(){
	printf("FLASHING SAVE (c%d)\n",get_core_num());
	uint32_t xip_offset = 0x90000;
	char *p = (char *)XIP_BASE+xip_offset;
	uint32_t ints = save_and_disable_interrupts();
	flash_range_erase (xip_offset, FLASH_SECTOR_SIZE);
	flash_range_program (xip_offset, (uint8_t*)losabuf, FLASH_SECTOR_SIZE);
	for(size_t i=0;i<FLASH_SECTOR_SIZE;i++){ losabuf[i]=p[i];	}
	restore_interrupts(ints);
	printf("FLASHED!\n");
}

void check_save_data(){
  if(strstr((char*)plosa->bat.mode,"GOOD")!=plosa->bat.mode){
    plosa->bat.mA = bat_default->mA;
    plosa->bat.load = bat_default->load;
    plosa->bat.max = bat_default->max;
    plosa->bat.min = bat_default->min;
    plosa->bat.dif = bat_default->dif;
    plosa->bat.read = bat_default->read;
    sprintf(plosa->bat.mode,"GOOD\0");
    printf("bat mode reset to defaults='%s'\n",plosa->bat.mode);
  }
  if(strstr((char*)plosa->mode,"LOAD")!=plosa->mode){
    plosa->dt.year  = default_time.year ;
    plosa->dt.month = default_time.month;
    plosa->dt.day   = default_time.day  ;
    plosa->dt.dotw  = default_time.dotw ;
    plosa->dt.hour  = default_time.hour ;
    plosa->dt.min   = default_time.min  ;
    plosa->dt.sec   = default_time.sec  ;
    plosa->theme_pos = DEFAULT_THEME;
    plosa->editpos = EDITPOSITIONS; //center
    plosa->is_sleeping = false;
    plosa->highpointer = false;
    plosa->alphapointer = true;
    plosa->pointerdemo = false;
    plosa->pstyle = PS_NORMAL;
    plosa->clock = true;
    plosa->spin = 1;
    plosa->texture = 1;
    sprintf(plosa->mode,"LOAD\0");
    printf("settings reset to defaults");
  }else{ // do a few sanity checks
    plosa->dt.sec+=1;
    if(plosa->dt.month > 12 ){plosa->dt.month = default_time.month;}
    if(plosa->dt.day   > 31) {plosa->dt.day   = default_time.day  ;}
    if(plosa->dt.dotw  > 6)  {plosa->dt.dotw  = default_time.dotw ;}
    if(plosa->dt.hour  > 23) {plosa->dt.hour  = default_time.hour ;}
    if(plosa->dt.min   > 59) {plosa->dt.min   = default_time.min  ;}
    if(plosa->dt.sec   > 59) {plosa->dt.sec   = default_time.sec  ;}
    if(plosa->theme_pos>=THEMES){plosa->theme_pos=0;}
    if(plosa->editpos>EDITPOSITIONS){plosa->editpos=EDITPOSITIONS;}
    plosa->pointerdemo = false;
    plosa->pstyle = PS_NORMAL;
    plosa->clock = true;
    plosa->spin = 1;
    plosa->texture = 1;
    rtc_set_datetime(&plosa->dt);
    printf("mode reset to defaults='%s'\n",plosa->mode);
  }



}


void empty_deinit(){
  //printf("REBOOTING...\n");
}

void doreset(){
  watchdog_reboot((uint32_t)&empty_deinit,0,1);
  watchdog_enable(1, 1);
}

void dosave(){
  // do other stuff here
  sprintf((char*)&plosa->mode,"SAVE");
  doreset();
}

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
  dcolors[0]=colt[plosa->theme_pos]->col_dotw;
  dcolors[1]=colt[plosa->theme_pos]->col_date;
  dcolors[2]=colt[plosa->theme_pos]->col_date;
  dcolors[3]=colt[plosa->theme_pos]->col_date;
  dcolors[4]=colt[plosa->theme_pos]->col_time;
  dcolors[5]=colt[plosa->theme_pos]->col_time;
  dcolors[6]=colt[plosa->theme_pos]->col_time;
  dcolors[7]=colt[plosa->theme_pos]->col_time;
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


bool reserved_addr(uint8_t addr) {
  return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

#define AHT15 0x38
#define I2C0 i2c0
#define I2C_SDA 4
#define I2C_SCL 5


void i2c_scan(){

  i2c_init(I2C0, 100 * 1000);
	gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
	gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
	gpio_pull_up(I2C_SDA);
	gpio_pull_up(I2C_SCL);
  bi_decl(bi_2pins_with_func(I2C_SDA, I2C_SCL, GPIO_FUNC_I2C));
	printf("\nI2C Bus Scan\n");
	printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
	for (int addr = 0; addr < (1 << 7); ++addr) {
			if (addr % 16 == 0) {					printf("%02x ", addr);			}
			int ret;
			uint8_t rxdata=0;
			if (reserved_addr(addr))
					ret = PICO_ERROR_GENERIC;
			else
					ret = i2c_read_blocking(I2C0, addr, &rxdata, 1, false);

			printf(ret < 0 ? "." : "@");
			printf(addr % 16 == 15 ? "\n" : "  ");
			//if(ret>=0 && addr==HIH71){has_hih=true;}
	}
}

void i2c_read(){
  uint8_t cmd=0;
  uint8_t data[6];

  //cmd = 0b11100001; // init
  cmd = 0xE1; // reset
  i2c_write_blocking(I2C0, AHT15, &cmd,1, true);
  sleep_ms(100);

  //cmd = 0b10101100; // read
  cmd = 0xAC;
  i2c_write_blocking(I2C0, AHT15, &cmd,1, true);
  sleep_ms(100);
  i2c_read_blocking( I2C0, AHT15, &data[0],6, false);
  printf("0x%02x 0x%02x\n",data[0],data[1]);
  printf("0x%02x 0x%02x\n",data[2],data[3]);
  printf("0x%02x 0x%02x\n",data[4],data[5]);

  //uint32_t humidity   = data[1];                          //20-bit raw humidity data
  //         humidity <<= 8;
  //         humidity  |= data[2];
  //         humidity <<= 4;
  //         humidity  |= data[3] >> 4;
  //if (humidity > 0x100000) {humidity = 0x100000;}             //check if RH>100, no need to check for RH<0 since "humidity" is "uint"
  //float hum = ((float)humidity / 0x100000) * 100;

  //uint32_t temperature   = data[3] & 0x0F;                //20-bit raw temperature data
  //         temperature <<= 8;
  //         temperature  |= data[4];
  //         temperature <<= 8;
  //         temperature  |= data[5];

  //float tem = ((float)temperature / 0x100000) * 200 - 50;
  //printf("%f %f\n",hum,tem);

}

#define MS 1000
#define US 1000000
#define BUTD 500  // delay between possible button presses (default: 500, half of a second)
#define REBOOT US*3 // 30 second/10
uint32_t rebootcounter = 0;
uint32_t rebootcounterold = 0;
uint32_t button0_time=0;
uint32_t button1_time=0;
uint32_t button0_dif=0;
uint32_t button1_dif=0;

void gpio_callback(uint gpio, uint32_t events) {
    if(events&GPIO_IRQ_EDGE_RISE){
      if(gpio==CSW){        osw=true;      }
      if(gpio==CCLK){        gch='c';      }
      if(gpio==CDT){        gch='d';      }
      if(gpio==CBUT0){ceasefire=true;fire_pressed=false;rebootcounter=0;} // puts("erise");
      if(gpio==CBUT1){ceasefire=true;fire_pressed=false;rebootcounter=0;} // puts("erise");

      gbuf[0]=gbuf[1];
      gbuf[1]=gch;
    }

    if(events&GPIO_IRQ_EDGE_FALL){
      if(gpio==CSW){        sw=true;      }
      if(gpio==CCLK){        gch='C';      }
      if(gpio==CDT) {        gch='D';      }
      //if(gpio==CBUT0){
      //  printf("tus: %d\n",time_us_32());
      //  printf("b0t: %d\n",button0_time);
      //}
      if(gpio==CBUT0 && !fire && (((time_us_32()-button0_time)/MS)>=BUTD)){ceasefire=false;fire=true;button0_time = time_us_32();fire_pressed=true;} // puts("efall");
      if(gpio==CBUT1 && !fire && (((time_us_32()-button1_time)/MS)>=BUTD)){ceasefire=false;fire=true;button1_time = time_us_32();fire_pressed=true;} // puts("efall");
      gbuf[0]=gbuf[1];
      gbuf[1]=gch;
    }

    //if(events&GPIO_IRQ_LEVEL_LOW && gpio==CBUT0){
    //  buttonglass-=BUTTONGLASSC;
    //  if(buttonglass<=0){
    //    fire=true;
    //    if(plosa->editpos == 0){rebootcounter++;}
    //    buttonglass=BUTTONGLASS;
    //  }
    //}

    //if(events&GPIO_IRQ_LEVEL_LOW && gpio==CBUT1){
    //  buttonglass-=BUTTONGLASSC;
    //  if(buttonglass<=0){
    //    fire=true;
    //    if(plosa->editpos == 0){rebootcounter++;}
    //    buttonglass=BUTTONGLASS;
    //  }
    //}

    if(gbuf[0]=='C'&&gbuf[1]=='D'){tcw=true;}
    if(gbuf[0]=='D'&&gbuf[1]=='C'){tccw=true;}
    if(sw){sw=false;fire=true;}
    if(osw){osw=false;ceasefire=true;}


}
char C_SET[4]="set ";
char C_GET[4]="get ";

void command(char* c){
    bool tc=false;  // time changed
    char* left=c;
    if(strstr(left," ")){
      char* space = strstr(left," ");
      space[0] = 0;
      char* right = space+1;
      if(strstr(left,"sensors")){   plosa->SENSORS = (bool)atoi(right);}
      if(strstr(left,"gyro")){ plosa->GYROCROSS = (bool)atoi(right);}
      if(strstr(left,"bender")){    plosa->SECOND_BENDER = (bool)atoi(right);}
      if(strstr(left,"smooth")){  plosa->SMOOTH_BACKGROUND = (bool)atoi(right);}
      if(strstr(left,"insomnia")){  plosa->INSOMNIA = (bool)atoi(right);}
      if(strstr(left,"circle")){ plosa->DYNAMIC_CIRCLES = (bool)atoi(right);}
      if(strstr(left,"light")){     plosa->BRIGHTNESS = (uint8_t)atoi(right);lcd_set_brightness(plosa->BRIGHTNESS);}
      if(strstr(left,"deep")){ plosa->DEEPSLEEP = (bool)atoi(right);}
      if(strstr(left,"high")){ plosa->highpointer = (bool)atoi(right);}
      if(strstr(left,"alpha")){ plosa->alphapointer = (bool)atoi(right);}
      if(strstr(left,"pstyle")){ plosa->pstyle = (int16_t)atoi(right);}
      if(strstr(left,"texture")){ plosa->texture = (int16_t)atoi(right);}
      if(strstr(left,"spin")){ plosa->spin = (int16_t)atoi(right);}
      if(strstr(left,"clock")){ plosa->clock = (bool)atoi(right);}
      if(strstr(left,"pointerdemo")){ plosa->pointerdemo = (bool)atoi(right);}
      if(strstr(left,"theme")){
        plosa->theme_pos = (uint8_t)atoi(right);
        if(plosa->theme_pos>=THEMES){
          plosa->theme_pos=THEMES-1;
        }
      }
      if(strstr(left,"deg")){
        flagdeg=(int16_t)atoi(right);
      }
      if(strstr(left,"blit")){
        int16_t d = (int16_t)atoi(right);
        Vec2 dp1 = {128,20};
        Vec2 dp0 = {102,20};
        draw_pointer_mode(dp0,dp1,d, colt[plosa->theme_pos]->col_m,textures[plosa->texture],BLACK,PS_TEXTURE);
      }
      if(strstr(left,"hour")){ uint8_t tv = (uint8_t)atoi(right);if(tv>=0&&tv<24){plosa->dt.hour = tv;tc=true;}}
      if(strstr(left,"min")){  uint8_t tv = (uint8_t)atoi(right);if(tv>=0&&tv<60){plosa->dt.min  = tv;tc=true;}}
      if(strstr(left,"sec")){  uint8_t tv = (uint8_t)atoi(right);if(tv>=0&&tv<60){plosa->dt.sec  = tv;tc=true;}}

      if(strstr(left,"day")){   uint8_t tv = (uint8_t)atoi(right);if(tv>0&&tv<=last[plosa->dt.month]){plosa->dt.day = tv;tc=true;}}
      if(strstr(left,"mon")){ uint8_t tv = (uint8_t)atoi(right);if(tv>0&&tv<13){plosa->dt.month  = tv;tc=true;}}
      if(strstr(left,"year")){  uint16_t tv = (uint16_t)atoi(right);if(tv>=0){plosa->dt.year  = tv;tc=true;}}
      if(strstr(left,"dotw")||strstr(left,"WDAY")){ uint8_t tv = (uint8_t)atoi(right);if(tv>=0&&tv<7){plosa->dt.dotw  = tv;tc=true;}}
      if(tc){rtc_set_datetime(&plosa->dt);}
      return;
    }
    if(strstr(left,"cir0")){plosa->DYNAMIC_CIRCLES=false;printf("DYCI0\n");return;}
    if(strstr(left,"cir1")){plosa->DYNAMIC_CIRCLES=true;printf("DYCI1\n");return;}
    if(strstr(left,"batmax")){ printf("BATMAX: %f\n", plosa->bat.max); return; }
    if(strstr(left,"batmin")){ printf("BATMIN: %f\n", plosa->bat.min); return; }
    if(strstr(left,"save")){ dosave(); }
    //if(strstr(left,"read")){ uint8_t data[4]; i2c_read((uint8_t*)&data); }
    //if(strstr(left,"scan")){ i2c_scan(); }
    if(strstr(left,"rota")){ plosa->gfxmode=GFX_ROTATE;}
    if(strstr(left,"roto")){ plosa->gfxmode=GFX_ROTOZOOM;}
    if(strstr(left,"norm")){ plosa->gfxmode=GFX_NORMAL;}
    if(strstr(left,"stat")){
      if(plosa->theme_pos>=THEMES){plosa->theme_pos=0;}
      if(plosa->editpos>7){plosa->editpos=0;}
      if(plosa->dt.dotw>6){plosa->dt.dotw=0;}

      printf("\n- STATUS -\n\nmode[8]: %s\n",plosa->mode);
      printf("dt: %02d:%02d:%04d\n",plosa->dt.day,plosa->dt.month,plosa->dt.year);
      printf("dt: %s %02d:%02d:%02d\n",week[plosa->theme_pos][plosa->dt.dotw],plosa->dt.hour,plosa->dt.min,plosa->dt.sec);
      printf("bat: %s %fmA [%d] %fmax %fmin\n", plosa->bat.mode,plosa->bat.mA,(plosa->bat.load)?1:0,plosa->bat.max,plosa->bat.min);
      printf("editpos: %d\n",plosa->editpos);
      printf("theme_pos: %d\n",plosa->theme_pos);
      printf("BRIGHTNESS : %d\n",plosa->BRIGHTNESS);

      printf("is_sleeping: %s\n",(plosa->is_sleeping)?"1":"0");
      printf("SENSORS: %s\n",plosa->SENSORS?"1":"0");
      printf("GYROCROSS: %s\n",plosa->GYROCROSS?"1":"0");
      printf("SECOND_BENDER: %s\n",plosa->SECOND_BENDER?"1":"0");
      printf("SMOOTH_BACKGROUND: %s\n",plosa->SMOOTH_BACKGROUND?"1":"0");
      printf("INSOMNIA : %s\n",plosa->INSOMNIA?"1":"0");
      printf("DYNAMIC_CIRCLES: %s\n",plosa->DYNAMIC_CIRCLES?"1":"0");
      printf("DEEPSLEEP: %s\n",plosa->DEEPSLEEP?"1":"0");
      printf("HIGHPOINTER: %s\n",plosa->highpointer?"1":"0");
    }
    if(strstr(left,"SNAPSHOT")){
      //printf("-----------------------> CUT HERE <---------------------\n\nuint8_t imagedata[138+  240*240*2] = {\n");
      if(b0==NULL){return;}
      uint8_t snaphead[] = {
      //  ID   ||  SIZE  |                                                                     | WIDTH   |          | HEIGHT |
      0x4d,0x42,0xc2,0x8a,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x8a,0x00,0x00,0x00,0x7c,0x00,0x00,0x00,0xf0,0x00,0x00,0x00,0xf0,
      0x00,0x00,0x00,0x01,0x00,0x10,0x00,0x03,0x00,0x00,0xc2,0x00,0x00,0x01,0x0b,0x12,0x00,0x00,0x0b,0x12,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0xf8,0x00,0x00,0x00,0x07,0xe0,0x00,0x00,0x00,0x1f,0x00,0x00,0x00,0x00,0x00,0x00,0x47,0x42,
      0x73,0x52,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
      for(uint32_t i=0;i<138;i+=2){
        putchar_raw(snaphead[i+1]);
        putchar_raw(snaphead[i]);
      }

      for(uint32_t i=0;i<LCD_SZ;i+=2){
        putchar_raw(b0[i+1]);
        putchar_raw(b0[i]);
      }
      stdio_flush();
      //stdio_usb_init();
    }
}

void shell(){
  // shell
  comt=getchar_timeout_us(100);
  while(comt!=PICO_ERROR_TIMEOUT){
    comc=comt&0xff;
    //putchar(comc);
    combufa[comi++]=comc;
    if(comc=='\n'){
      combufa[comi]=0;
      combufa[comi-1]=0;
      //printf("CMD: %s\n",combufa);
      command(&combufa[0]);
      comi=0;
    }
    if(comi==254){comi=0;}
    comt=getchar_timeout_us(100);
  }

}

void fx_circle(uint16_t x, uint16_t y, uint16_t r, uint16_t c, uint16_t ps, uint16_t xo, uint16_t yo){
  if(plosa->DYNAMIC_CIRCLES){
    lcd_bez3circ(x,y,r,c,ps,xo,yo);
  }else{
    lcd_circle(x,y,r,c,ps,0);
  }
}

void draw_pointer_mode(Vec2 vs, Vec2 vts, int16_t tu, uint16_t color, const uint8_t* sr, uint16_t alpha, PSTYLE cps){
  switch(cps){
    case PS_NORMAL:
      lcd_line_deg(vO, tu, vs.x, color, vs.y);
      break;
    case PS_ALPHA:
      lcd_alpha_line_deg(vO, tu, vs.x, color, vs.y);
      break;
    case PS_TEXTURE:
      lcd_blit_deg(vO,vs,vts,tu,sr,alpha,false);
      break;
    case PS_BENDER:
      lcd_blit_deg(vO,vs,vts,90,sr,alpha,true);
      break;
  };
}

void draw_pointer(Vec2 vs, Vec2 vts, int16_t tu, uint16_t color, const uint8_t* sr, uint16_t alpha){
  draw_pointer_mode(vs,vts,tu,color,sr,alpha,plosa->pstyle);
}


void draw_gfx(){
  uint8_t x1,y1,xt,yt;
  uint8_t x0=120;
  uint8_t y0=120;
  if(!plosa->clock){return;}
  // battery display
  lcd_frame(POS_BAT_X    ,POS_BAT_Y,   POS_BAT_X+102+(POS_BAT_PS<<1), POS_BAT_Y+POS_BAT_YS, BLUE, POS_BAT_PS); // frame
  lcd_line(POS_BAT_X-1    ,POS_BAT_Y+1, POS_BAT_X-1    ,POS_BAT_Y+POS_BAT_YS-2,BLUE,1);// round end
  lcd_line(POS_BAT_X+102+(POS_BAT_PS<<1)    ,POS_BAT_Y+1, POS_BAT_X+102+(POS_BAT_PS<<1)    ,POS_BAT_Y+POS_BAT_YS-2,BLUE,1); //round end
  lcd_yline(POS_BAT_X+103+(POS_BAT_PS<<1)    ,POS_BAT_Y+POS_BAT_YS/2-2,4,__builtin_bswap16(BLUE),2);  //+
  //printf("bat: %f %f %f %f\n",plosa->bat.read,plosa->bat.max, plosa->bat.min, plosa->bat.dif);
  float bat_dif = ( plosa->bat.dif - (plosa->bat.max - plosa->bat.read ) );
  if(bat_dif<0.0f){bat_dif=0.0f;}
  //printf("(%f)  %f %f [%f]\n",(resultsummid()*conversion_factor),plosa->bat.dif,bat_dif,(bat_dif/plosa->bat.dif)*100.0f);

  uint16_t bat =  (uint16_t)((bat_dif/plosa->bat.dif)*100.0f);
  //printf("bat :  %03d\n",bat);
  if(bat>100){bat=100;}
  uint16_t level_color = colt[plosa->theme_pos]->bat_level;
  if(bat>10&&bat<30){level_color = colt[plosa->theme_pos]->bat_level_low;}
  if(bat<10){level_color = colt[plosa->theme_pos]->bat_level_critical;}
  lcd_xline(POS_BAT_X+POS_BAT_PS    ,POS_BAT_Y+POS_BAT_PS,   bat+1, __builtin_bswap16(level_color), POS_BAT_YS-(POS_BAT_PS<<1)); // battery level
  if(!usb_loading){
    sprintf(dbuf,"  %02d%%",bat);
  }else{
    sprintf(dbuf,"LOADING",bat);
  }
  lcd_str(94, 12, dbuf, &Font12, level_color, BLACK);

  if(plosa->SENSORS){
    if((plosa->dt.sec==0||plosa->dt.sec==30)&&(!temp_read)){
      temperature = QMI8658_readTemp();
      temp_read=true;
    }else{
      temp_read=false;
    }
  }
  //printf("acc_x   = %4.3fmg , acc_y  = %4.3fmg , acc_z  = %4.3fmg\r\n", acc[0], acc[1], acc[2]);
  //printf("gyro_x  = %4.3fdps, gyro_y = %4.3fdps, gyro_z = %4.3fdps\r\n", gyro[0], gyro[1], gyro[2]);
  //printf("tim_count = %d\r\n", tim_count);
  int xi,yi;
  Vec2 vc_s,vc_e;
  for(int16_t i=0;i<60;i++){
    vc_e = gvdl(i*6,119);
    vc_e = vadd(vc_e,vO);
    if(!(i%5)){
      vc_s = gvdl(i*6,110);
      vc_s = vadd(vc_s,vO);
      lcd_linev2(vc_s,vc_e, colt[plosa->theme_pos]->col_cs, 1);

    }else{
      vc_s = gvdl(i*6,115);
      vc_s = vadd(vc_s,vO);
      lcd_linev2(vc_s,vc_e, colt[plosa->theme_pos]->col_cs5, 1);

    }
  }

  Vec2 dp1 = {128,20};
  // draw minute pointer
  int16_t tu=plosa->dt.min*6;
  Vec2 dp0 = {102,10};
  draw_pointer(dp0,dp1,tu,colt[plosa->theme_pos]->col_m,textures[plosa->texture],BLACK);
  //dp0.x=102;dp0.y=20;draw_pointer_mode(dp0,dp1,tu, colt[plosa->theme_pos]->col_m,textures[1],BLACK,PS_TEXTURE);

  if(plosa->pointerdemo){
    dp0.x=102;dp0.y=6; draw_pointer_mode(dp0,dp1,flagdeg1, colt[plosa->theme_pos]->col_m,textures[plosa->texture],BLACK,PS_NORMAL);
    dp0.x=102;dp0.y=4; draw_pointer_mode(dp0,dp1,flagdeg1a, colt[plosa->theme_pos]->col_m,textures[plosa->texture],BLACK,PS_ALPHA);
    dp0.x=102;dp0.y=10;draw_pointer_mode(dp0,dp1,flagdeg1b, colt[plosa->theme_pos]->col_m,textures[1],BLACK,PS_TEXTURE);
  }
  tu=(int16_t)plosa->dt.hour;
  if(tu>=12){tu-=12;}
  tu*=30;
  tu+=(int16_t)(plosa->dt.min>>1);

  dp0=vset(75, 20);
  draw_pointer(dp0,dp1,tu, colt[plosa->theme_pos]->col_h,textures[plosa->texture],BLACK);
  //dp0.x=75;dp0.y=20; draw_pointer_mode(dp0,dp1,tu, colt[plosa->theme_pos]->col_h,textures[1],BLACK,PS_TEXTURE);
  if(plosa->pointerdemo){
    dp0=vset(65,3);draw_pointer_mode(dp0,dp1,flagdeg2, colt[plosa->theme_pos]->col_h,textures[plosa->texture],BLACK,PS_NORMAL);
    dp0.x=65;dp0.y=5; draw_pointer_mode(dp0,dp1,flagdeg2a, colt[plosa->theme_pos]->col_h,textures[plosa->texture],BLACK,PS_ALPHA);
    dp0.x=75;dp0.y=20; draw_pointer_mode(dp0,dp1,flagdeg2b, colt[plosa->theme_pos]->col_h,textures[1],BLACK,PS_TEXTURE);
    //draw_pointer_mode(65,10,flagdeg2, colt[plosa->theme_pos]->col_h,textures[plosa->texture],BLACK,PS_NORMAL);
    //draw_pointer_mode(65,10,flagdeg2a, colt[plosa->theme_pos]->col_h,textures[plosa->texture],BLACK,PS_ALPHA);
    //draw_pointer_mode(75,20,flagdeg2b, colt[plosa->theme_pos]->col_h,textures[plosa->texture],BLACK,PS_TEXTURE);
  }

  if(tseco!=plosa->dt.sec){
    tseco=plosa->dt.sec;
    stime = time_us_32();
  }
  tu=plosa->dt.sec*6;

  if(!analog_seconds){
    // 'jump' seconds
    xi = (int8_t)(tcos[plosa->dt.sec*6]*114);
    yi = (int8_t)(tsin[plosa->dt.sec*6]*114);
    x1 = (uint8_t)x0+xi;
    y1 = (uint8_t)y0+yi;
    if(plosa->SECOND_BENDER){
        int16_t xit=x1;
        int16_t yit=y1;
        xi = (int8_t)(tcos[plosa->dt.sec*6]*106);
        yi = (int8_t)(tsin[plosa->dt.sec*6]*106);
        x1 = (uint8_t)x0+xi;
        y1 = (uint8_t)y0+yi;
        Vec2 v1={x1,y1};
        lcd_alpha_on();
        lcd_bez2curve(0,0,(int8_t)(xi/2)+(int8_t)(acc[1]/25.0f),(int8_t)(yi/2)-(int8_t)(acc[0]/25.0f),xi,yi,114,colt[plosa->theme_pos]->col_s,2);
        lcd_alpha_off();
        if(plosa->pstyle == PS_ALPHA){
          //lcd_alpha_line(x1,y1, xit, yit, colt[plosa->theme_pos]->col_s, 1);
          lcd_alpha_line_deg(v1, tu, 7, colt[plosa->theme_pos]->col_s, 1);

        }else{
          lcd_line_deg(v1, tu, 7, colt[plosa->theme_pos]->col_s, 1);
          //lcd_line(x1,y1, xit, yit, colt[plosa->theme_pos]->col_s, 1);
        }
    }else{
      dp0.x=106;dp0.y=1;
      draw_pointer_mode(vO,dp0,tu, colt[plosa->theme_pos]->col_s,textures[plosa->texture],BLACK,PS_NORMAL);

    }
    lcd_blit((int)(x0-8+tcos[plosa->dt.sec*6]*100),(int)(y0-8+tsin[plosa->dt.sec*6]*100),16,16,colt[plosa->theme_pos]->alpha,stars[plosa->theme_pos]);
  }
  //else{
  //  uint32_t st = time_us_32();
  //  st-=stime;
  //  st=st/US; //micro-seconds
  //  // 'analog' seconds
  //  xi = (int)(tfcos[plosa->dt.sec*10+st]*114);
  //  yi = (int)(tfsin[plosa->dt.sec*10+st]*114);
  //  x1 = (uint8_t)x0+xi;
  //  y1 = (uint8_t)y0+yi;
  //  lcd_line(x0,y0, x1, y1, colt[plosa->theme_pos]->col_s, 1);
  //
  //  lcd_blit((int)(x0-9+tfcos[plosa->dt.sec*10+st]*100),(int)(y0-9+tfsin[plosa->dt.sec*10+st]*100),16,16,colt[plosa->theme_pos]->alpha,stars[plosa->theme_pos]);
  //}

  if(plosa->alphapointer){lcd_alpha_off();}

  lcd_blit(120-16,120-16,32,32,colt[plosa->theme_pos]->alpha, flags[plosa->theme_pos]); // center

  //draw_pointer_mode(120,20,90, colt[plosa->theme_pos]->col_h,textures[plosa->texture],BLACK,PS_BENDER);

  // looks
  //Vec2 vflag_pos = {120-16,120-16};
  //Vec2 vflag = {32,32};
  //lcd_blit_deg(vflag_pos,vflag,flagdeg,flags[plosa->theme_pos],colt[plosa->theme_pos]->alpha); // center
  if(plosa->spin!=0){
    flagdeg = gdeg(flagdeg+plosa->spin);
    flagdeg1  = gdeg(flagdeg1 +plosa->spin+(gyrox>>3));
    flagdeg2  = gdeg(flagdeg2 -plosa->spin*7);
    flagdeg1a = gdeg(flagdeg1a-plosa->spin*2);
    flagdeg2a = gdeg(flagdeg2a+plosa->spin*5);
    flagdeg1b = gdeg(flagdeg1b-plosa->spin);
    flagdeg2b = gdeg(flagdeg2b+plosa->spin*7);
  }
  // graphical view of x/y gyroscope
  if(plosa->GYROCROSS){
    #define GSPX 120
    #define GSPY 200
    #define GSPS 4
    #define GSPSZ 20

    lcd_frame(GSPX-GSPS , GSPY-GSPSZ, GSPX+GSPS, GSPY+GSPSZ,WHITE,1); //vert |
    lcd_frame(GSPX-GSPSZ, GSPY-GSPS, GSPX+GSPSZ,GSPY+GSPS, WHITE,1); //horz –
    float fx = (acc[1]/25.0f); // -20 – 20
    float fy = (acc[0]/25.0f);

    if(hg_enabled){
      fy -= (int8_t)(hgx/25.0f);
      fx -= (int8_t)(hgy/25.0f);
    }
    //printf("gxy: %f %f\n",fx,fy);
    int8_t gx = (int8_t)fx;
    int8_t gy = (int8_t)fy;
    if(gx>  (GSPSZ-GSPS)){ gx= (GSPSZ-GSPS); }
    if(gx< -(GSPSZ-GSPS)){ gx=-(GSPSZ-GSPS); }
    if(gy>  (GSPSZ-GSPS)){ gy= (GSPSZ-GSPS); }
    if(gy< -(GSPSZ-GSPS)){ gy=-(GSPSZ-GSPS); }
    //printf("gxy: %d %d\n",gx,gy);
    uint16_t gdx = (uint16_t)GSPX+gx;
    uint16_t gdy = (uint16_t)GSPY-gy;
    uint16_t gcoly = __builtin_bswap16(WHITE);
    uint16_t gcolx = __builtin_bswap16(YELLOW);
    lcd_pixel_rawps(GSPX,gdy,gcoly,GSPS);
    lcd_pixel_rawps(gdx,GSPY,gcolx,GSPS);
    if(hg_enabled){
      gy = (int8_t)(hgx/25.0f);
      gx = (int8_t)(hgy/25.0f);
      if(gx>  (GSPSZ-GSPS)){ gx= (GSPSZ-GSPS); }
      if(gx< -(GSPSZ-GSPS)){ gx=-(GSPSZ-GSPS); }
      if(gy>  (GSPSZ-GSPS)){ gy= (GSPSZ-GSPS); }
      if(gy< -(GSPSZ-GSPS)){ gy=-(GSPSZ-GSPS); }
      //printf("gxy: %d %d\n",gx,gy);
      gdx = (uint16_t)GSPX+gx;
      gdy = (uint16_t)GSPY-gy;
      int16_t GSPSs = GSPS-1;
      lcd_frame(GSPX-GSPSs,gdy -GSPSs,GSPX + GSPSs,gdy  +GSPSs,LGRAY,1);
      lcd_frame(gdx -GSPSs,GSPY-GSPSs,gdx  + GSPSs,GSPY +GSPSs,ORANGE,1);
    }
  }
}


void draw_text(){
  if(plosa->SENSORS){
    lcd_str(POS_ACC_X, POS_ACC_Y+  1, "GYR_X =", &Font12, WHITE, BLACK);
    lcd_str(POS_ACC_X, POS_ACC_Y+ 15, "GYR_Y =", &Font12, WHITE, BLACK);
    lcd_str(POS_ACC_X, POS_ACC_Y+ 48, "GYR_Z =", &Font12, WHITE, BLACK);
    lcd_str(POS_ACC_X, POS_ACC_Y+ 98, "ACC_X =", &Font12, WHITE, BLACK);
    lcd_str(POS_ACC_X, POS_ACC_Y+130, "ACC_Y =", &Font12, WHITE, BLACK);
    lcd_str(POS_ACC_X, POS_ACC_Y+144, "ACC_Z =", &Font12, WHITE, BLACK);
    lcd_str(POS_ACC_X, POS_ACC_Y+156, "TEMP", &Font12, WHITE, BLACK);
    lcd_float(POS_ACC_X+70, POS_ACC_Y+  1,  acc[0], &Font12, YELLOW, BLACK);
    lcd_float(POS_ACC_X+70, POS_ACC_Y+ 15,  acc[1], &Font12, YELLOW, BLACK);
    lcd_float(POS_ACC_X+70, POS_ACC_Y+ 48,  acc[2], &Font12, YELLOW, BLACK);
    lcd_float(POS_ACC_X+70, POS_ACC_Y+ 98, gyro[0], &Font12, YELLOW, BLACK);
    lcd_float(POS_ACC_X+70, POS_ACC_Y+130, gyro[1], &Font12, YELLOW, BLACK);
    lcd_float(POS_ACC_X+70, POS_ACC_Y+144, gyro[2], &Font12, YELLOW, BLACK);
    lcd_float(POS_ACC_X+70, POS_ACC_Y+156, temperature, &Font12,  YELLOW, BLACK);
    lcd_str(50, 208, "BAT(V)", &Font16, WHITE, BLACK);
    lcd_floatshort(130, 208, resultsummid(), &Font16, ORANGE, BLACK);
  }
  //sprintf(dbuf, "DPS: %02d",dpsc);
  //lcd_str(120, 220    , dbuf , &Font12, YELLOW,  CYAN);
  if(!plosa->theme_pos){
    convert_cs(week[plosa->theme_pos][plosa->dt.dotw],cn_buffer);
    lcd_strc(POS_CNDOW_X, POS_CNDOW_Y, cn_buffer, &CNFONT, colors[0], BLACK);
    //printf("cn_buffer: %s\n",cn_buffer);
  }else{
    lcd_str(POS_DOW_X, POS_DOW_Y, week[plosa->theme_pos][plosa->dt.dotw], &TFONT, colors[0], BLACK);
  }
  uint8_t yoff_date = POS_DATE_Y;
  uint8_t yoff_time = POS_TIME_Y;
  if(plosa->SENSORS){
    //yoff_date+=20;
    //yoff_time-=20;
  }
  sprintf(dbuf,"%02d",plosa->dt.day);
  lcd_str(POS_DATE_X+0*TFW, yoff_date, dbuf, &TFONT, colors[1], BLACK);
  lcd_str(POS_DATE_X+2*TFW, yoff_date, ".", &TFONT, WHITE, BLACK);
  sprintf(dbuf,"%02d",plosa->dt.month);
  lcd_str(POS_DATE_X+3*TFW, yoff_date, dbuf, &TFONT, colors[2], BLACK);
  lcd_str(POS_DATE_X+5*TFW, yoff_date, ".", &TFONT, WHITE, BLACK);
  sprintf(dbuf,"%04d",plosa->dt.year);
  lcd_str(POS_DATE_X+6*TFW, yoff_date, dbuf, &TFONT, colors[3], BLACK);

  sprintf(dbuf,"%02d",plosa->dt.hour);
  lcd_str(POS_TIME_X,       yoff_time, dbuf, &TFONT, colors[4], BLACK);
  lcd_str(POS_TIME_X+2*TFW, yoff_time, ":", &TFONT, WHITE, BLACK);
  sprintf(dbuf,"%02d",plosa->dt.min);
  lcd_str(POS_TIME_X+3*TFW, yoff_time, dbuf, &TFONT, colors[5], BLACK);
  lcd_str(POS_TIME_X+5*TFW, yoff_time, ":", &TFONT, WHITE, BLACK);
  sprintf(dbuf,"%02d",plosa->dt.sec);
  lcd_str(POS_TIME_X+6*TFW, yoff_time, dbuf, &TFONT, colors[6], BLACK);
}

int main(void)
{
    if(strstr((char*)plosa->mode,"SAVE")){
    		sprintf((char*)plosa->mode,"LOAD");
    		flash_data();
    }else{
    		if(!force_no_load && !strstr((char*)plosa->mode,"LOAD")){
    			flash_data_load();
    		}else{
    			//plosa->rules[0]=0;
          //if(!force_no_load){
          //  save_config();
          //  sprintf((char*)plosa->mode,"LOAD");
          //  flash_data();
          //}
        }
    }
    stdio_init_all();

    check_save_data(); // init
    //bool init=false;
    //bool fixed=false;
    sleep_ms(400);  // reboot takes about 1.6 sec. -> increase time by 2sec, wait 0.4sec
    //plosa->spin=1;
    //plosa->gfxmode=GFX_ROTATE;
    plosa->pointerdemo=false;
    plosa->pstyle=2;
    plosa->theme_pos=0;
    plosa->texture=1;
    lcd_init();
    lcd_make_cosin();
    //printf("sin0=%f\n",gcosin(0));
    puts("stdio init");
    lcd_set_brightness(plosa->BRIGHTNESS);
    puts("lcd init");
    printf("%02d-%02d-%04d %02d:%02d:%02d [%d]\n",plosa->dt.day,plosa->dt.month,plosa->dt.year,plosa->dt.hour,plosa->dt.min,plosa->dt.sec,plosa->dt.dotw);
    printf("mode='%s'\n",plosa->mode);

    b0 = malloc(LCD_SZ);
    if(b0==0){printf("b0==0!\n");}
    uint32_t o = 0;
    lcd_setimg((uint16_t*)b0);
    //printf("INIT: %b FIXED: %b [%08x] mode='%s'\n",init,fixed,plosa,plosa->mode);

    colt[0]=&colt1;
    colt[1]=&colt2;
    colt[2]=&colt3;
    colt[3]=&colt4;

    bool o_clk;
    bool o_dt;
    bool o_sw;

    //uint32_t bm = 0b00000000000010110000000000000000;
    gpio_set_dir(CBUT0,GPIO_IN);
    gpio_pull_up(CBUT0);
    //gpio_set_irq_enabled(CBUT0, GPIO_IRQ_LEVEL_LOW, true);
    gpio_set_dir(CBUT1,GPIO_IN);
    gpio_pull_up(CBUT1);
    //gpio_set_irq_enabled(CBUT1, GPIO_IRQ_LEVEL_LOW, true);
    gpio_set_irq_enabled_with_callback(CCLK, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled(CDT, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(CSW, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(CBUT0, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(CBUT1, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    //gpio_pull_up(CBUT);
    //i2c_scan();
    rtc_init();
    printf("init realtime clock\n");
    rtc_set_datetime(&plosa->dt);
    printf("init realtime clock done\n");
    if(!(plosa->dt.year%4)){last[2]=29;}else{last[2]=28;}
    QMI8658_init();
    printf("QMI8658_init\r\n");

    //set_colt_colors();
    //set_dcolors(); // are set from dcolors so set em first
    //copy_pos_matrix(theme_pos);
    //command("scan");
    //command("stat");

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

    command("stat");
    bool qmis = false;
    while(true){
      qmis = !qmis;
      if(qmis){
        QMI8658_read_xyz(acc, gyro, &tim_count);
      }
      //check if not moving
      #define GYRMAX 300.0f
      #define ACCMAX 500.0f
      bool no_moveshake = false;
      if(theme_bg_dynamic_mode==1){
        if((gyro[0]>-ACCMAX&&gyro[0]<ACCMAX)&&(gyro[1]>-ACCMAX&&gyro[1]<ACCMAX)&&(gyro[2]>-ACCMAX&&gyro[2]<ACCMAX)){            no_moveshake = true;          }
      }else{
        if((acc[0]>-GYRMAX&&acc[0]<GYRMAX)&&(acc[1]>-GYRMAX&&acc[1]<GYRMAX)){            no_moveshake = true;          }
      }
      if(acc[2]>=0.0f){//force awake!
        no_moveshake=true;
        screensaver=SCRSAV;
        plosa->is_sleeping=false;
        theme_bg_dynamic_mode = 0;
        lcd_set_brightness(plosa->BRIGHTNESS);
        lcd_sleepoff();
      }
      //if((acc[2]>0&&last_z<0)||(acc[2]<0&&last_z>0)){no_moveshake=true;}  // coin-flipped
      last_z=acc[2];
      if(no_moveshake){
        if(!plosa->is_sleeping && cmode==CM_None && !(usb_loading|plosa->INSOMNIA)){
          screensaver--;
          if(screensaver<=0){
            if(bg_dynamic[plosa->theme_pos]){
              theme_bg_dynamic_mode++;
              if(theme_bg_dynamic_mode==1){
                screensaver=SCRSAV2;
                continue;}
            }
            plosa->is_sleeping=true;
            screensaver=SCRSAV;
            lcd_set_brightness(0);
            lcd_sleepon();
          }
        }
      }else{
        if(plosa->is_sleeping){
          plosa->is_sleeping=false;
          lcd_set_brightness(plosa->BRIGHTNESS);
          lcd_sleepoff();
          sleep_frame=SLEEP_FRAME_START;
        }
        if(theme_bg_dynamic_mode){theme_bg_dynamic_mode--;}
      }

      if(plosa->is_sleeping){
        sleep_ms(sleep_frame);
        if(plosa->DEEPSLEEP){
          if(sleep_frame<SLEEP_FRAME_END){sleep_frame+=SLEEP_FRAME_ADD;}
        }
        continue;
      }


      if(fire_pressed){
        uint32_t t = time_us_32();
        if(button0_time){ button0_dif = t-button0_time; }
        if(button1_time){ button1_dif = t-button1_time; }
        if((button0_dif||button1_dif)&&(plosa->editpos==7)){ // only from central position (flag)
          //printf("%d %d %d\n",t,button0_time,button0_dif);
          if(button0_dif>=US){ printf("REBOOT: [%d->%d] s\n", button0_dif/MS, REBOOT/MS); }
          if(button1_dif>=US){ printf("REBOOT: [%d->%d] s\n", button1_dif/MS, REBOOT/MS); }
          if(button0_dif>=REBOOT || button1_dif >= REBOOT){
            printf("SAVING...\n");
            dosave();
          }
        }
      }

      for(int i=0;i<LCD_SZ;i++){b0[i]=0x00;}

      if(plosa->gfxmode==GFX_NORMAL||plosa->gfxmode==GFX_ROTATE){
        if(bg_dynamic[plosa->theme_pos]){ // dynamic background
          int8_t xa = (int8_t)(acc[1]/50.0f);
          int8_t ya = (int8_t)(acc[0]/50.0f);
          if(xa>EYE_MAX){xa=EYE_MAX;}
          if(xa<-EYE_MAX){xa=-EYE_MAX;}
          if(ya>EYE_MAX){ya=EYE_MAX;}
          if(ya<-EYE_MAX){ya=-EYE_MAX;}
          if(plosa->SMOOTH_BACKGROUND){
            xoldt = xa;
            yoldt = ya;
            xa+=xold;
            ya+=yold;
            xa>>=1;
            ya>>=1;
            xold = xoldt;
            yold = yoldt;
          }
          if(xa >18){xa= 18;}
          if(ya >18){ya= 18;}
          if(xa<-18){xa=-18;}
          if(ya<-18){ya=-18;}
          gyrox=xa;
          gyroy=ya;
          //printf("xya: %d %d\n",xa,ya);
          if(plosa->gfxmode==GFX_ROTATE){
            //printf("XAYA: %d %d\n",xa,ya);
            Vec2 vbs = {120-95+xa,120-95-ya};
            //Vec2 vbs = {0,0};

            Vec2 vbe = {190,190};
            lcd_blit_deg(vbs,vbe,vbe,flagdeg,backgrounds[plosa->theme_pos],colt[plosa->theme_pos]->alpha,true);
          }else{
            lcd_blit(EYE_X+xa,EYE_Y-ya,EYE_SZ,EYE_SZ,BLACK,backgrounds[plosa->theme_pos]);
          }
        }else{
          mcpy(b0,backgrounds[plosa->theme_pos],LCD_SZ);
        }
      }else if(plosa->gfxmode==GFX_ROTOZOOM){
        lcd_roto(backgrounds[plosa->theme_pos],bgsz[plosa->theme_pos],bgsz[plosa->theme_pos]);
        lcd_rotoa();
      }


      uint8_t save_sec = plosa->dt.sec;
      uint8_t save_min = plosa->dt.min;
      if(cmode!=CM_Editpos || plosa->editpos==7 ){
        rtc_get_datetime(&plosa->dt);
      }
      //if(!fire && cmode==CM_Editpos && (plosa->editpos==6||plosa->editpos==5)){
      //  plosa->dt.sec = save_sec;
      //  plosa->dt.min = save_min;
      //}
      ++resulti;
      resulti&=0x0f;
      result[resulti] = read_battery();
      usb_loading = (resultsummid()>=plosa->bat.load);

      if(fire==true){
        fire_counter++;
        //printf("fire! [%08x] (%d %d %d) {%d}\n",fire_counter,time_us_32()/MS,button0_time/MS,button1_time/MS,(time_us_32()-button0_time)/MS);

        sleep_frame = SLEEP_FRAME_START;
        plosa->is_sleeping = false;
        theme_bg_dynamic_mode=0;
        dir_x = D_NONE;
        dir_y = D_NONE;

        if(cmode==CM_None){
          //puts("CM_Config");
          //cmode=CM_Config;
          hgx = (int)acc[0];
          hgy = (int)acc[1];
          hg_enabled = true;
          //puts("CM_Changepos");
          cmode=CM_Changepos;
          colors[plosa->editpos]=edit_colors[plosa->theme_pos];
        }else if(cmode==CM_Config){
          //puts("CM_Changepos");
          //cmode=CM_Changepos;
          //colors[plosa->editpos]=edit_colors[plosa->theme_pos];
        }else if(cmode==CM_Changepos){
          //puts("CM_Editpos");
          cmode=CM_Editpos;
          hgx = (int)acc[0];
          hgy = (int)acc[1];
          hg_enabled = true;
          tcw = false;
          tccw = false;
          //colors[plosa->editpos]=change_colors[plosa->theme_pos];
          colors[plosa->editpos]=changecol;
        }else if(cmode==CM_Editpos){
          //puts("CM_None");
          cmode=CM_None;
          colors[plosa->editpos]=dcolors[plosa->editpos];
          rtc_set_datetime(&plosa->dt);
          if(!(plosa->dt.year%4)){last[2]=29;}else{last[2]=28;}
          hg_enabled = false;
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
        //printf("cxy: %0.3f %0.3f  r=%0.3f\n",cx,cy,r);
        cx/=r;
        cy/=r;
        //printf("cxy: %0.3f %0.3f\n",cx,cy);
        lcd_line(120,120, 120+cy*110, 120-cx*110, CYAN, 3);
      }
      if(cmode==CM_Changepos || cmode==CM_Editpos){
        // wrist-control (arm==x-axis)
        int asx = (int)acc[0];
        asx-=hgx;
        asx>>1;asx<<1;
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
        asy>>1;asy<<1;
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
            //puts("Dright");
            if(pos_matrix_x<positions[plosa->theme_pos]->dim_x-1)++pos_matrix_x;
//            if(pos_matrix_x<2)++pos_matrix_x;
          }
          if(dir_x==D_MINUS){
            //puts("Dleft");
            if(pos_matrix_x>0)--pos_matrix_x;
          }

          if(dir_y==D_PLUS){
            //if(pos_matrix_y<2)++pos_matrix_y;
            if(pos_matrix_y<positions[plosa->theme_pos]->dim_y-1)++pos_matrix_y;
            //puts("Ddown");
          }
          if(dir_y==D_MINUS){
            //puts("Dup");
            if(pos_matrix_y>0)--pos_matrix_y;
          }
          colors[plosa->editpos]=dcolors[plosa->editpos];
          plosa->editpos=positions[plosa->theme_pos]->pos[pos_matrix_y*(positions[plosa->theme_pos]->dim_x)+pos_matrix_x];
          //printf("posM: %d %d [%d]\n",pos_matrix_x, pos_matrix_y, pos_matrix[pos_matrix_y*3+pos_matrix_x]);
          dir_x = D_NONE;
          dir_y = D_NONE;

        }else{
          // change editposition (l/r or u/d) [ur+](tcw) [dl-](tccw)
          if(tcw){
            colors[plosa->editpos]=dcol;
            if(plosa->editpos==EDITPOSITIONS){plosa->editpos=0;}else{++plosa->editpos;}
            colors[plosa->editpos]=editcol;
            tcw=false;
          }else if(tccw){
            colors[plosa->editpos]=dcol;
            if(plosa->editpos==0){plosa->editpos=EDITPOSITIONS;}else{--plosa->editpos;}
            colors[plosa->editpos]=editcol;
            tccw=false;
          }
        }
      }

      if(cmode==CM_Editpos){
        bool set=false;
        if(tcw){
          //colors[plosa->editpos]=dcolors[plosa->editpos];
          colors[plosa->editpos]=changecol;
          switch(plosa->editpos){
            case 0: (plosa->dt.dotw==6)?plosa->dt.dotw=0:plosa->dt.dotw++;break;
            case 1: (plosa->dt.day==last[plosa->dt.month])?plosa->dt.day=1:plosa->dt.day++;break;
            case 2: (plosa->dt.month==12)?plosa->dt.month=1:plosa->dt.month++;break;
            case 3: (plosa->dt.year==2099)?plosa->dt.year=2022:plosa->dt.year++;break;
            case 4: (plosa->dt.hour==23)?plosa->dt.hour=0:plosa->dt.hour++;break;
            case 5: (plosa->dt.min==59)?plosa->dt.min=0:plosa->dt.min++;break;
            case 6: (plosa->dt.sec==59)?plosa->dt.sec=0:plosa->dt.sec++;break;
            case 7: {
              (plosa->theme_pos==THEMES-1)?plosa->theme_pos=0:plosa->theme_pos++;
              //copy_pos_matrix(plosa->theme_pos);
              update_pos_matrix();

              set_dcolors();
              break;
            }
          }
          tcw=false;
          //set=true;
        }
        if(tccw){
          //colors[plosa->editpos]=dcolors[plosa->editpos];
          colors[plosa->editpos]=changecol;
          switch(plosa->editpos){
            case 0: (plosa->dt.dotw==0)?plosa->dt.dotw=6:plosa->dt.dotw--;break;
            case 1: (plosa->dt.day==1)?plosa->dt.day=last[plosa->dt.month]:plosa->dt.day--;break;
            case 2: (plosa->dt.month==1)?plosa->dt.month=12:plosa->dt.month--;break;
            case 3: (plosa->dt.year==2099)?plosa->dt.year=2022:plosa->dt.year--;break;
            case 4: (plosa->dt.hour==0)?plosa->dt.hour=23:plosa->dt.hour--;break;
            case 5: (plosa->dt.min==0)?plosa->dt.min=59:plosa->dt.min--;break;
            case 6: (plosa->dt.sec==0)?plosa->dt.sec=59:plosa->dt.sec--;break;
            case 7: {
              (plosa->theme_pos==0)?plosa->theme_pos=THEMES-1:plosa->theme_pos--;
              //copy_pos_matrix(plosa->theme_pos);
              update_pos_matrix();
              set_dcolors();
              break;
            }
          }
          tccw=false;
          //set=true;
        }
        if(set){
          rtc_set_datetime(&plosa->dt);
          if(!(plosa->dt.year%4)){last[2]=29;}else{last[2]=28;}
        }
      }
      if(cmode==CM_Editpos || cmode==CM_Changepos){
        uint16_t cmode_color = GREEN;
        if(cmode==CM_Changepos){
          blink_counter++;
          if(blink_counter==5){ bmode=!bmode;blink_counter=0; }
          cmode_color = blinker[bmode];
        }
        if(plosa->editpos==0){
          if(plosa->theme_pos==0){
            lcd_frame(POS_CNDOW_X-6,POS_CNDOW_Y,POS_CNDOW_X+CNFONT.w+3,POS_CNDOW_Y+CNFONT.h*3+1,cmode_color,3);
          }else{
            fx_circle(tpos[plosa->editpos].x+18,tpos[plosa->editpos].y+8,25,cmode_color,3,xold,yold);
            //if(plosa->DYNAMIC_CIRCLES){lcd_bez3circ(tpos[plosa->editpos].x+18,tpos[plosa->editpos].y+8,25,cmode_color,3,xold,yold);}
            //else{lcd_circle(tpos[plosa->editpos].x+18,tpos[plosa->editpos].y+8,25,cmode_color,3,0);}
          }
        }else if(plosa->editpos==3){
          fx_circle(tpos[plosa->editpos].x+12,tpos[plosa->editpos].y+5,30,cmode_color,3,xold,yold);
          //if(plosa->DYNAMIC_CIRCLES){lcd_bez3circ(tpos[plosa->editpos].x+12,tpos[plosa->editpos].y+5,30,cmode_color,3,xold,yold);}
          //else{lcd_circle(tpos[plosa->editpos].x+12,tpos[plosa->editpos].y+5,30,cmode_color,3,0);}
        }else if(plosa->editpos==7){
          fx_circle(tpos[plosa->editpos].x,tpos[plosa->editpos].y,19,cmode_color,3,xold,yold);
          //if(plosa->DYNAMIC_CIRCLES){lcd_bez3circ(tpos[plosa->editpos].x,tpos[plosa->editpos].y,19,cmode_color,3,xold,yold);}
          //else{lcd_circle(tpos[plosa->editpos].x,tpos[plosa->editpos].y,19,cmode_color,3,0);}
        }else{
          fx_circle(tpos[plosa->editpos].x+12,tpos[plosa->editpos].y+5,20,cmode_color,3,xold,yold);
          //if(plosa->DYNAMIC_CIRCLES){lcd_bez3circ(tpos[plosa->editpos].x+12,tpos[plosa->editpos].y+5,20,cmode_color,3,xold,yold);}
          //else{lcd_circle(tpos[plosa->editpos].x+12,tpos[plosa->editpos].y+5,20,cmode_color,3,0);}
        }
      }

      if(!theme_bg_dynamic_mode){
        if(!plosa->highpointer||cmode==CM_Editpos){
          draw_gfx();
          draw_text();
        }else{
          draw_text();
          draw_gfx();
        }
      }

      lcd_display(b0);
      if(SHELL_ENABLED){
        shell();
      }
      //Vec2 vs = {120,120};
      //Vec2 ve = {32,32};
      //lcd_blit_deg(vs,ve,deg,usa32);
      //uint32_t atime = time_us_32();
      //uint32_t wtime = ((atime-last_wait)/100000);
      //last_wait = atime;
      ////printf("wt= %d [%d]\n",wtime,FRAME_DELAY-wtime);
      //if(wtime>=FRAME_DELAY){wtime=FRAME_DELAY-1;}
      //sleep_ms(FRAME_DELAY-wtime);
    }
    return 0;
}
