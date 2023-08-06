#include "w.h"
extern font_t Font12;
extern font_t Font16;
W wroot;

void init_root(){
  wroot.p = NULL;
  wroot.x = 0;
  wroot.y = 0;
  wroot.w = LCD_W;
  wroot.h = LCD_H;
  wroot.t = wt_root;
  wroot.st = st_none;
  wroot.ws = ws_shown;
  wroot.d = NULL;
  W_box* wb = (W_box*)malloc(sizeof(W_box));
  if(wb){
    //printf("init_root_wb\n");
    wb->nc = 0;
    wb->size = 0;
    wb->ch = NULL;
  }
  wroot.d = (void*)wb;
}


bool resize(W* box, uint16_t new_size){
  W_box* wb = (W_box*)box->d;
  W** rch = (W**)malloc(sizeof(W*)*new_size);
  if(!rch){
    printf("rch malloc error\n");
    return false;
  }  // out of memory?
  if(!wb->ch){  // empty array
    //printf("ch NULL\n");
    wb->ch = rch;
    wb->size = new_size;
    return true;
  }else{
    if(wb->nc){
      //printf("resize (%d) [%d]\n",new_size,wb->nc);
      for(uint16_t nci=0; nci<wb->nc; nci++){ rch[nci] = wb->ch[nci]; } // copy children
      free(wb->ch);
      wb->ch = rch;
      wb->size = new_size;
    }
  }
  return true;
}

W* wfindxy(W* w, int16_t x, int16_t y){
  //printf("wfxy:  %d %d  0x%08x [%d] {%d–%d %d–%d}\n",x,y,w,w->t,w->x,w->x+w->w,w->y,w->y+w->h);
  if(w->ws == ws_hidden){return NULL;}
  if(x >= w->x && x <= w->x+w->w  &&  y >= w->y && y <= w->y+w->h){
    if(w->t == wt_root || w->t == wt_box){
      W_box* wb = (W_box*)w->d;
      if(wb && wb->nc){
        W* wfound = NULL;
        for(uint16_t nci=0; nci<wb->nc; nci++){
          wfound = wfindxy(wb->ch[nci], x,y);
          if(wfound){ return wfound; }
        }
      }
      return NULL;
    }else{
      //printf("*wfxy:  %d %d  0x%08x [%d] {%d–%d %d–%d}\n",x,y,w,w->t,w->x,w->x+w->w,w->y,w->y+w->h);
      if(w->ws == ws_hidden)return NULL;
      if(w->t == wt_blinker_circle || w->t == wt_blinker_rect)return NULL;
      if(w->t == wt_text && w->st == st_text_ghost)return NULL;
      return w;
    }
  }else{
    return NULL;
  }
}

void whide(W* w){w->ws = ws_hidden;}
void wshow(W* w){w->ws = ws_shown;}
void whidem(W** w, uint16_t n){
  //printf("whidem: %08x %d\n",w,n);
  for(uint16_t i=0;i<n;i++){ w[i]->ws = ws_hidden;}
}
void wshowm(W** w, uint16_t n){
  //printf("wshowm: %08x %d\n",w,n);
  for(uint16_t i=0;i<n;i++){ w[i]->ws = ws_shown; }
}


W* wnew(){
  W* w = malloc(sizeof(W));
  return w;
}

bool wadd(W* p, W* ch){
  if(p && ch && ( p->t == wt_root || p->t == wt_box) ){
    W_box* wb = (W_box*)p->d;
    if(wb->nc == wb->size){
      if(!wb->size){
        wb->size = W_BOX_CHILDREN;
      } // default size 4
      bool rok = resize(p,wb->size<<1);
      if(!rok){ return false; } // out of mem
    }
    wb->ch[wb->nc] = ch;
    //printf("WADD: 0x%08x [%d]\n",ch,ch->t);
    ++wb->nc;
    return true;
  }
  return false;
}


W* wadd_box(W* p, int16_t x, int16_t y, int16_t w, int16_t h){
  W* wn = wnew();
  if(wn){
    //printf("wn 0x%08x\n",wn);
    wn->p = p;
    wn->x = x;
    wn->y = y;
    wn->w = w;
    wn->h = h;
    wn->t = wt_box;
    wn->st = st_none;
    wn->ws = ws_shown;
    wn->d = NULL;
    wn->lt = 0;
    W_box* wb = (W_box*)malloc(sizeof(W_box));
    if(wb){
      wb->nc = 0;
      wb->size = 0;
      wb->ch = NULL;
      wn->d = (void*)wb;
    }else{
      free(wn);
      return NULL;
    }
    if(p){ wadd(p,wn); }
  }
  return wn;
}


W* wadd_image(W* p, uint16_t* data, int16_t x, int16_t y, int16_t w, int16_t h)
{
  W* wn = wnew();
  if(wn){
    //printf("wn 0x%08x\n",wn);
    wn->p = p;
    wn->x = x;
    wn->y = y;
    wn->w = w;
    wn->h = h;
    wn->t = wt_image;
    wn->st = st_none;
    wn->ws = ws_shown;
    wn->d = (void*)data;
    wn->lt = 0;
    if(p){ wadd(p,wn); }
  }
  return wn;
}

W* wadd_imager(W* p, uint16_t* (*get_image)(), int16_t x, int16_t y, int16_t w, int16_t h)
{
  W* wn = wnew();
  if(wn){
    //printf("wn 0x%08x\n",wn);
    wn->p = p;
    wn->x = x;
    wn->y = y;
    wn->w = w;
    wn->h = h;
    wn->t = wt_imager;
    wn->st = st_none;
    wn->ws = ws_shown;
    wn->d = (void*)get_image;
    wn->lt = 0;
    if(p){ wadd(p,wn); }
  }
  return wn;
}

W* wadd_imagef(W* p, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t* data, uint8_t (*image_function)(), uint8_t* index, uint8_t max_index){
  W* wn = wnew();
  if(wn){
    //printf("wn 0x%08x\n",wn);
    W_imagef* wif = malloc(sizeof(W_imagef));
    if(!wif){free(wn);return NULL;}
    wn->p = p;
    wn->x = x;
    wn->y = y;
    wn->w = w;
    wn->h = h;
    wn->t = wt_imagef;
    wn->st = st_none;
    wn->ws = ws_shown;
    wn->lt = 0;
    wn->d = wif;
    wif->image_data = data;
    wif->image_function = image_function;
    wif->index = index;
    wif->max_index = max_index;
    if(p){ wadd(p,wn); }
  }
  return wn;
}


W* wadd_text(W* p, int16_t x, int16_t y, int16_t w, int16_t h,
  char* text, font_t** fonts, uint16_t cf, uint16_t cb, uint16_t cfr,uint8_t fs, uint8_t fid, uint8_t o)
{
  W* wn = wnew();
  if(wn){
    wn->p = p;
    wn->x = x;
    wn->y = y;
    wn->w = w;
    wn->h = h;
    wn->t = wt_text;
    wn->st = st_none;
    wn->ws = ws_shown;
    wn->d = NULL;
    wn->lt = 0;
    W_text* wt = malloc(sizeof(W_text));
    if(wt){
      wt->text = text;
      wt->fonts = fonts;
      wt->cf = cf;
      wt->cb = cb;
      wt->cfr = cfr;
      wt->fs = fs;
      wt->marge = 0;
      wt->pad = 0;
      wt->fid = fid;
      wt->o = o;
      wn->d = (void*)wt;
      if(p){ wadd(p,wn); }
    }else{
      free(wn);
      return NULL;
    }
  }
  return wn;
}

W* wadd_textr(W* p, int16_t x, int16_t y, int16_t w, int16_t h,
    char* (*get_value)(), font_t** fonts, uint16_t cf, uint16_t cb, uint16_t cfr,uint8_t fs, uint8_t fid, uint8_t o)
{
  W* wn = wnew();
  if(wn){
    wn->p = p;
    wn->x = x;
    wn->y = y;
    wn->w = w;
    wn->h = h;
    wn->t = wt_textr;
    wn->st = st_none;
    wn->ws = ws_shown;
    wn->d = NULL;
    wn->lt = 0;
    W_textr* wt = malloc(sizeof(W_textr));
    if(wt){
      wt->get_text = get_value;
      wt->fonts = fonts;
      wt->cf = cf;
      wt->cb = cb;
      wt->cfr = cfr;
      wt->fs = fs;
      wt->marge = 0;
      wt->pad = 0;
      wt->fid = fid;
      wt->o = o;
      wn->d = (void*)wt;
      if(p){ wadd(p,wn); }
    }else{
      free(wn);
      return NULL;
    }
  }
  return wn;
}

W* wadd_none(W* p, void (*do_func)()){
  W* wn = wnew();
  if(wn){
    wn->p = p;
    wn->x = 0;
    wn->y = 0;
    wn->w = 0;
    wn->h = 0;
    wn->t = wt_none;
    wn->st = st_none;
    wn->ws = ws_shown;
    wn->d = (void*)do_func;
    wn->lt = 0;
    if(p){ wadd(p,wn); }
  }
  return wn;
}

W* wadd_blinker(W* p, int16_t x, int16_t y, int16_t w, int16_t h, int16_t ps,
  uint16_t ceven, uint16_t codd, uint8_t max_count, bool mode_rect)
{
  W* wn = wnew();
  if(wn){
    wn->p = p;
    wn->x = x;wn->y = y;
    wn->w = w;wn->h = h;
    wn->t = mode_rect?wt_blinker_rect:wt_blinker_circle;
    wn->st = st_none; wn->ws = ws_shown;
    wn->d = NULL;
    wn->lt = 0;
    W_blinker* wb = malloc(sizeof(W_blinker));
    if(wb){
      wb->ps = ps;
      wb->ceven = ceven;
      wb->codd = codd;
      wb->count = 0;
      wb->max_count = max_count;
      wb->bmode = false;
      wn->d = (void*)wb;
      if(p){ wadd(p,wn); }
    }else{
      free(wn);
      return NULL;
    }
  }
  return wn;
}

W* wadd_spinner(W* p, int16_t x, int16_t y, int16_t w, int16_t h, W_st st,
 uint8_t pos, uint8_t nc, void** e, char* (*get_text)(uint8_t i), font_t** fonts,
 uint8_t fid, uint16_t cf, uint16_t cb, uint16_t cfr)
{
  W* wn = wnew();
  if(wn){
    wn->p = p;
    wn->x = x;wn->y = y;
    wn->w = w;wn->h = h;
    wn->t = wt_spinner;
    wn->st = st;
    wn->ws = ws_shown;
    wn->d = NULL;
    wn->lt = 0;
    W_spinner* wsp = malloc(sizeof(W_spinner));
    if(wsp){
      wsp->pos = pos;
      wsp->nc = nc;
      wsp->e = e;
      wsp->get_text = get_text;
      wsp->fonts = fonts;
      wsp->cf = cf;
      wsp->cb = cb;
      wsp->cfr = cfr;
      wsp->fpos = (int16_t) fonts[0]->h*pos;
      wsp->velocity = 0;
      wsp->moved = false;
      wsp->fixed = false;
      wsp->fid = fid;
      //wsp->fpos = 0;
      wn->d = (void*)wsp;
      if(p){ wadd(p,wn); }
    }else{
      free(wn);
      return NULL;
    }
    return wn;
  }
}


uint16_t wblink_counter = 0;
bool wbmode = false;

W_text* wdwt;
W_textr* wdwtr;

void wdraw(W* w){
  if(w->ws == ws_hidden){ return; }
  if(w->t == wt_root || w->t == wt_box){
    W_box* wb = (W_box*)w->d;
    if(wb){
      for(uint16_t nci=0; nci<wb->nc; nci++){
        wdraw(wb->ch[nci]);
      }
    }
  }
  //printf("w: %d %d\n",w->t, w->st);
  if(w->t == wt_none && w->d){
    ((void (*)())w->d)(); // call function
  }else if(w->t ==wt_image){
      lcd_blit((uint8_t)w->x,(uint8_t)w->y, (uint8_t)w->w,(uint8_t)w->h, BLACK,(uint8_t*)w->d);
  }else if(w->t == wt_imager){
      //uint16_t* (*get_image)() = ((uint16_t* (*)())w->d);
      //lcd_blit((uint8_t)w->x,(uint8_t)w->y, (uint8_t)w->w,(uint8_t)w->h, BLACK,(uint8_t*)get_image());
      lcd_blit((uint8_t)w->x,(uint8_t)w->y, (uint8_t)w->w,(uint8_t)w->h, BLACK,(uint8_t*)((uint16_t* (*)())w->d)());
  }else if(w->t == wt_imagef){
    W_imagef* wif = (W_imagef*)w->d;
    if(wif->max_index){
      int16_t wifdegg = DEGS/(wif->max_index);
      int16_t deg = wifdegg*wif->max_index-10;
      int16_t ww = (w->w>>1)+10;
      Vec2 vi;
      if(wif->max_index==1){
        vi = gvdl(deg, ww);
        lcd_circle(w->x+(w->w>>1)+vi.x , w->y+(w->h>>1)+vi.y, 3, (*wif->index==1)?BLUE:RED, 1, *wif->index);
      }else{
        for(uint16_t i=0;i<wif->max_index;++i){
          vi = gvdl(deg, ww);
          lcd_circle(w->x+(w->w>>1)+vi.x , w->y+(w->h>>1)+vi.y, 3, (*wif->index==i)?BLUE:RED, 1, (*wif->index==i)?true:false);
          deg += wifdegg;
          if(deg>=DEGS){deg-=DEGS;}
          if(deg<0){deg+=DEGS;}
        }
      }
    }
    lcd_blit((uint8_t)w->x,(uint8_t)w->y, (uint8_t)w->w,(uint8_t)w->h, BLACK,(uint8_t*)wif->image_data);
  }else if(w->t == wt_text){
    wdwt = (W_text*)w->d;
    lcd_stringm((uint8_t)w->x,(uint8_t)w->y,wdwt->text,wdwt->fonts,wdwt->cf,wdwt->cb,wdwt->o);
  }else if(w->t == wt_textr){
    wdwtr = (W_textr*)w->d;
    char* t = wdwtr->get_text();
    lcd_stringm((uint8_t)w->x,(uint8_t)w->y,t,wdwtr->fonts,wdwtr->cf,wdwtr->cb,wdwtr->o);
  }else if(w->t == wt_blinker_circle){
    W_blinker* wb = (W_blinker*)w->d;
    ++wb->count;
    if(wb->count>=wb->max_count){
      wb->bmode=!wb->bmode;
      wb->count=0;
    }
    lcd_circle(w->x,w->y,w->w,((wb->bmode)?wb->codd:wb->ceven),wb->ps,0);
  }else if(w->t == wt_blinker_rect){
    W_blinker* wb = (W_blinker*)w->d;
    ++wb->count;
    if(wb->count>=wb->max_count){
      wb->bmode=!wb->bmode;
      wb->count=0;
    }
    lcd_frame(w->x,w->y,w->x+w->w,w->y+w->h,((wb->bmode)?wb->codd:wb->ceven),wb->ps-1);
  }else if(w->t == wt_spinner){
    if(w->st == st_spinner_char_v){
      wspinner_draw(w);
    }else{
      wspinner_draw_h(w);
    }
  }
}


void wset_st(W* w, W_st st){
  w->st = st;
}


void wspinner_draw(W* w){
  W_spinner* wsp = (W_spinner*)w->d;
  uint8_t pos = (uint8_t)(wsp->fpos/wsp->fonts[wsp->fid]->h);
  wsp->pos = pos;
  //printf("pos = %d\n",pos);
  uint16_t offset = pos*wsp->fonts[wsp->fid]->h;
  lcd_frame(w->x-4,w->y-4,w->x+w->w+4,w->y+w->h+4,wsp->cfr,2);
  if(wsp->pos >= wsp->nc)wsp->pos=0;

  if((wsp->fpos%wsp->fonts[wsp->fid]->h) == 0){
    pos = (!pos)?wsp->nc-1:pos-1;
    lcd_stringm((uint8_t)w->x,(uint8_t)w->y+4,wsp->e?((char**)wsp->e)[pos]:wsp->get_text(pos),wsp->fonts,wsp->cf,wsp->cb,0);
    if(++pos>=wsp->nc)pos-=(wsp->nc);
    lcd_stringm((uint8_t)w->x,(uint8_t)w->y+wsp->fonts[wsp->fid]->h+4,wsp->e?((char**)wsp->e)[pos]:wsp->get_text(pos),wsp->fonts,wsp->cf,wsp->cb,0);
    if(wsp->pos>=wsp->nc)wsp->pos-=wsp->nc;
    if(++pos>=wsp->nc)pos-=(wsp->nc);
    lcd_stringm((uint8_t)w->x,(uint8_t)w->y+2*wsp->fonts[wsp->fid]->h+4,wsp->e?((char**)wsp->e)[pos]:wsp->get_text(pos),wsp->fonts,wsp->cf,wsp->cb,0);
  }else{
    uint16_t o = (wsp->fpos%wsp->fonts[wsp->fid]->h);
    printf("o=%d fpos=%d fposmax=%d f:%08x [%d] ",o,wsp->fpos,wsp->fonts[wsp->fid]->w*wsp->nc-1,wsp->fonts[wsp->fid],wsp->fid);
    pos = (!pos)?wsp->nc-1:pos-1;
    lcd_stringmo((uint8_t)w->x,(uint8_t)w->y+4,wsp->e?((char**)wsp->e)[pos]:wsp->get_text(pos),wsp->fonts,wsp->cf,wsp->cb,0, 0,0 ,o,0);
    if(++pos>=wsp->nc)pos-=(wsp->nc);
    lcd_stringm((uint8_t)w->x,(uint8_t)w->y-o+wsp->fonts[wsp->fid]->h+4,wsp->e?((char**)wsp->e)[pos]:wsp->get_text(pos),wsp->fonts,wsp->cf,wsp->cb,0);
    if(++pos>=wsp->nc)pos-=(wsp->nc);
    lcd_stringm((uint8_t)w->x,(uint8_t)w->y-o+2*wsp->fonts[wsp->fid]->h+4,wsp->e?((char**)wsp->e)[pos]:wsp->get_text(pos),wsp->fonts,wsp->cf,wsp->cb,0);
    ++pos;if(pos>=wsp->nc)pos-=(wsp->nc);
    lcd_stringmo((uint8_t)w->x,(uint8_t)w->y-o+3*wsp->fonts[wsp->fid]->h+4,wsp->e?((char**)wsp->e)[pos]:wsp->get_text(pos),wsp->fonts,wsp->cf,wsp->cb,0, 0,0 ,0,wsp->fonts[wsp->fid]->h-o);

    if(wsp->velocity){
      wsp->fpos += wsp->velocity;
      if(wsp->velocity>0){
        --wsp->velocity;
      }else if(wsp->velocity < 0){
        ++wsp->velocity;
      }
    }else if(!wsp->fixed && o && ((time_us_32()-w->lt)/1000)>200){   //
      uint16_t od;
      if(o>(wsp->fonts[wsp->fid]->h>>1)){
        od = o - (wsp->fonts[wsp->fid]->h>>1);
        if(!od&&o)od=5;
        if(od>4){o+=3;}
        else if(od>2){o+=2;}
        else if(od>0){o+=1;}
        if(o>wsp->fonts[wsp->fid]->h)o=wsp->fonts[wsp->fid]->h;
        wsp->fpos = wsp->pos * wsp->fonts[wsp->fid]->h+o;
      }else{
        od = (wsp->fonts[wsp->fid]->h>>1) - o;
        if(!od&&o)od=5;
        if(od>4){o-=3;}
        else if(od>2){o-=2;}
        else if(od>0){o-=1;}
        wsp->fpos = wsp->pos * wsp->fonts[wsp->fid]->h+o;
      }
      //printf("o %d %d [%d]\n",o,od,wsp->fonts[wsp->fid]->h);

      if(wsp->fpos >= wsp->fonts[wsp->fid]->h*wsp->nc){wsp->fpos-=wsp->fonts[wsp->fid]->h*wsp->nc;}
      if(wsp->fpos < 0){wsp->fpos+=wsp->fonts[wsp->fid]->h*wsp->nc;}
    }
  }
  lcd_number((uint8_t)w->x,(uint8_t)w->y-2*Font16.h,(uint32_t)wsp->fpos,&Font16,GREEN,wsp->cb);
  lcd_xline((uint8_t)w->x,(uint8_t)w->y+wsp->fonts[wsp->fid]->h, w->w-8,__builtin_bswap16(GREEN),2);
  lcd_xline((uint8_t)w->x,(uint8_t)w->y+2*wsp->fonts[wsp->fid]->h+4, w->w-8,__builtin_bswap16(GREEN),2);
}


void wspinner_draw_h(W* w){
  W_spinner* wsp = (W_spinner*)w->d;
  //printf("fpos = %d ",wsp->fpos);
  //if(wsp->fpos >= wsp->fonts[wsp->fid]->w*wsp->nc){wsp->fpos-=wsp->fonts[wsp->fid]->w*wsp->nc;}
  //if(wsp->fpos < 0){wsp->fpos+=wsp->fonts[wsp->fid]->w*wsp->nc;}
  uint8_t pos = (uint8_t)(wsp->fpos/wsp->fonts[wsp->fid]->w);
  if(pos>=wsp->nc)pos-=wsp->nc;
  wsp->pos = pos;
  uint16_t offset = pos*wsp->fonts[wsp->fid]->w;
  lcd_frame(w->x-6,w->y-4,w->x+w->w-6,w->y+w->h+4,wsp->cfr,2);
  if(wsp->pos >= wsp->nc)wsp->pos=0;

  if((wsp->fpos%wsp->fonts[wsp->fid]->w) == 0){
    pos = (!pos)?wsp->nc-1:pos-1;
    lcd_stringm((uint8_t)w->x,(uint8_t)w->y,wsp->e?((char**)wsp->e)[pos]:wsp->get_text(pos),wsp->fonts,wsp->cf,wsp->cb,1);
    if(++pos>=wsp->nc)pos-=(wsp->nc);
    lcd_stringm((uint8_t)w->x+wsp->fonts[wsp->fid]->w,(uint8_t)w->y,wsp->e?((char**)wsp->e)[pos]:wsp->get_text(pos),wsp->fonts,wsp->cf,wsp->cb,1);
    if(wsp->pos>=wsp->nc)wsp->pos-=wsp->nc;
    if(++pos>=wsp->nc)pos-=(wsp->nc);
    lcd_stringm((uint8_t)w->x+2*wsp->fonts[wsp->fid]->w,(uint8_t)w->y,wsp->e?((char**)wsp->e)[pos]:wsp->get_text(pos),wsp->fonts,wsp->cf,wsp->cb,1);
  }else{
    uint16_t o = (wsp->fpos%wsp->fonts[wsp->fid]->w);
    printf("o=%d fpos=%d fposmax=%d f:%08x [%d] ",o,wsp->fpos,wsp->fonts[wsp->fid]->w*(wsp->nc-1),wsp->fonts[wsp->fid],wsp->fid);
    if(o>=wsp->fonts[wsp->fid]->w*wsp->nc){
      o -= wsp->fonts[wsp->fid]->w*(wsp->nc);
    }
    printf(" {%d}\n",o);
    pos = (!pos)?wsp->nc-1:pos-1;
    lcd_stringmo((uint8_t)w->x,(uint8_t)w->y,wsp->e?((char**)wsp->e)[pos]:wsp->get_text(pos),wsp->fonts,wsp->cf,wsp->cb,1,o,0,0,0);
    if(++pos>=wsp->nc)pos-=(wsp->nc);
    lcd_stringm((uint8_t)w->x-o+1*wsp->fonts[wsp->fid]->w,(uint8_t)w->y,wsp->e?((char**)wsp->e)[pos]:wsp->get_text(pos),wsp->fonts,wsp->cf,wsp->cb,1);
    if(++pos>=wsp->nc)pos-=(wsp->nc);
    lcd_stringm((uint8_t)w->x-o+2*wsp->fonts[wsp->fid]->w,(uint8_t)w->y,wsp->e?((char**)wsp->e)[pos]:wsp->get_text(pos),wsp->fonts,wsp->cf,wsp->cb,1);
    ++pos;if(pos>=wsp->nc)pos-=(wsp->nc);
    lcd_stringmo((uint8_t)w->x-o+3*wsp->fonts[wsp->fid]->w,(uint8_t)w->y,wsp->e?((char**)wsp->e)[pos]:wsp->get_text(pos),wsp->fonts,wsp->cf,wsp->cb,1,0,o,0,0);

    if(wsp->velocity){
      wsp->fpos += wsp->velocity;
      if(wsp->velocity>0){
        --wsp->velocity;
      }else if(wsp->velocity < 0){
        ++wsp->velocity;
      }
    }else if(!wsp->fixed && o && ((time_us_32()-w->lt)/1000)>200){   //
      uint16_t od;
      if(o>(wsp->fonts[wsp->fid]->w>>1)){
        od = o - (wsp->fonts[wsp->fid]->w>>1);
        if(!od&&o)od=5;
        if(od>4){o+=3;}
        else if(od>2){o+=2;}
        else if(od>0){o+=1;}
        if(o>wsp->fonts[wsp->fid]->w)o=wsp->fonts[wsp->fid]->w;
        wsp->fpos = wsp->pos * wsp->fonts[wsp->fid]->w+o;
      }else{
        od = (wsp->fonts[wsp->fid]->w>>1) - o;
        if(!od&&o)od=5;
        if(od>4){o-=3;}
        else if(od>2){o-=2;}
        else if(od>0){o-=1;}
        wsp->fpos = wsp->pos * wsp->fonts[wsp->fid]->w+o;
      }
      printf("o %d %d [%d]\n",o,od,wsp->fonts[wsp->fid]->w);

      if(wsp->fpos >= wsp->fonts[wsp->fid]->w*wsp->nc){wsp->fpos-=wsp->fonts[wsp->fid]->w*wsp->nc;}
      if(wsp->fpos < 0){wsp->fpos+=wsp->fonts[wsp->fid]->w*wsp->nc;}
    }
  }
  lcd_number((uint8_t)w->x,(uint8_t)w->y-2*Font16.h,(uint32_t)wsp->fpos,&Font16,GREEN,wsp->cb);
  lcd_yline((uint8_t)w->x+wsp->fonts[wsp->fid]->w-1,(uint8_t)w->y+8, w->h-8,__builtin_bswap16(GREEN),2);
  lcd_yline((uint8_t)w->x+2*wsp->fonts[wsp->fid]->w+1,(uint8_t)w->y+8, w->h-8,__builtin_bswap16(GREEN),2);
}


void wspinner_set(W* w, uint8_t i){
  W_spinner* wsp = (W_spinner*)w->d;
  if(wsp){
    wsp->pos = i;
    wsp->fpos = wsp->pos * wsp->fonts[wsp->fid]->h;
  }
}

void wspinner_set_h(W* w, uint8_t i){
  W_spinner* wsp = (W_spinner*)w->d;
  if(wsp){
    wsp->pos = i;
    wsp->fpos = wsp->pos * wsp->fonts[wsp->fid]->w;
  }
}

void wspinner_set_max(W* w, uint8_t i){
  W_spinner* wsp = (W_spinner*)w->d;
  if(wsp){ wsp->nc = i; }
}

void wspinner_setfp(W* w){
  W_spinner* wsp = (W_spinner*)w->d;
  if(wsp){    wsp->pos = (wsp->fpos/wsp->fonts[wsp->fid]->h);  }
}

void wspinner_setfp_h(W* w){
  W_spinner* wsp = (W_spinner*)w->d;
  if(wsp){    wsp->pos = (wsp->fpos/wsp->fonts[wsp->fid]->w);  }
}

void wspinner_adjust(W_spinner* wsp){
  if(wsp->fpos >= wsp->fonts[wsp->fid]->h*wsp->nc){
    wsp->fpos -= wsp->fonts[wsp->fid]->h*wsp->nc;
  }else if(wsp->fpos < 0){
    wsp->fpos += wsp->fonts[wsp->fid]->h*wsp->nc;
  }
  wsp->moved = true;
}

void wspinner_adjust_h(W_spinner* wsp){
  if(wsp->fpos >= wsp->fonts[wsp->fid]->w*wsp->nc){
    wsp->fpos -= wsp->fonts[wsp->fid]->w*wsp->nc; printf("*fp-\n");
  }else if(wsp->fpos < 0){
    wsp->fpos += wsp->fonts[wsp->fid]->w*wsp->nc; printf("*fp+\n");
  }
  wsp->moved = true;
}


WMove_t* wmove_make(WBez2_t* b, W* w){
  WMove_t* wmt = malloc(sizeof(WMove_t));
  if(wmt){
    wmt->b = b;
    wmt->w = w;
    wmt->xo = w->x;
    wmt->yo = w->y;
  }
  return wmt;
}


WBez2_t* wbez2_make(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
    int16_t x2, int16_t y2, int16_t fr)
{
    WBez2_t * bez=malloc(sizeof(WBez2_t));
    if(!bez)return NULL;
    bez->x = malloc(sizeof(int16_t)*fr);
    if(!bez->x){ free(bez); return NULL; }
    bez->y = malloc(sizeof(int16_t)*fr);
    if(!bez->y){ free(bez->x);free(bez); return NULL; }

    bez->frame = 0;
    bez->frames = fr;
    bez->x0 = x0;
    bez->y0 = y0;
    bez->x1 = x1;
    bez->y1 = y1;
    bez->x2 = x2;
    bez->y2 = y2;
    bez->ax = bez->x2-bez->x0;
    bez->ay = bez->y2-bez->y0;
    bez->bx = bez->x2-bez->x1;
    bez->by = bez->y2-bez->y1;
    bez->done = false;
    wbez2_loop(bez);
    return bez;
}

void wbez2_del(WBez2_t* b){
//  printf("wbez2_del: %08x\n",b);
  if(b){
    if(b->y)free(b->y);
    if(b->x)free(b->x);
    free(b);
  }
  b = NULL;
}

void wbez2_reset(WBez2_t* b){
  b->frame = 0;
  b->done = false;
}

void wbez2_loop(WBez2_t* b){
  int16_t tf = 0;
  while(tf<b->frames){
    int16_t t;
    int16_t txa = ((int32_t)(b->x0+((b->ax*tf)/(b->frames-1))));
    int16_t tya = ((int32_t)(b->y0+((b->ay*tf)/(b->frames-1))));
    int16_t txb = ((int32_t)(b->x1+((b->bx*tf)/(b->frames-1))));
    int16_t tyb = ((int32_t)(b->y1+((b->by*tf)/(b->frames-1))));
    int16_t cx = txb - txa;
    int16_t cy = tyb - tya;
    b->x[tf] = (int16_t)((int32_t)(txa+((cx*tf)/(b->frames-1))));
    b->y[tf] = (int16_t)((int32_t)(tya+((cy*tf)/(b->frames-1))));
    tf++;
  }
}

bool wbez2_next(WBez2_t* b){
  if(!b->done){
    b->frame++;
    if(b->frame > b->frames){
      b->done = true;
    }
  }
  return b->done;
}


void wbez2_movem(WBez2_t* b, W* w)
{
  if(!b->done){
    w->x = b->x[b->frame];
    w->y = b->y[b->frame];
  }
}


bool wbez2_move(WBez2_t* b, W* w)
{
  if(!b->done){
    w->x = b->x[b->frame];
    w->y = b->y[b->frame];
    b->frame++;
    if(b->frame >= b->frames){
      b->done = true;
      b->frame = 0;
    }
    //printf("mov: %d %d %d\n",w->x,w->y,b->done);
  }
  return b->done;
}

bool wbez2_mover(WBez2_t* b, W* w)
{
  if(!b->done){
    w->x = b->x[(b->frames-1)-b->frame];
    w->y = b->y[(b->frames-1)-b->frame];
    b->frame++;
    if(b->frame >= b->frames){
      b->done = true;
      b->frame = 0;
    }
    //printf("movr: %d %d %d\n",w->x,w->y,b->done);
  }
  return b->done;
}
