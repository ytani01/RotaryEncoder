/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "OledMenu.h"

/** constructor
 *
 */
OledMenuEnt::OledMenuEnt(String title) {
  this->title = title;
  this->dst.type = OLED_MENU_DST_TYPE_NULL;
  log_i("type=%s", OLED_MENU_DST_TYPE_STR[this->dst.type]);
} // OledMenuEnt::OledMenuEnt()

/** constructor
 *
 */
OledMenuEnt::OledMenuEnt(String title,
                         void (*func)(void *param), void *func_param) {
  this->title = title;
  this->dst.type = OLED_MENU_DST_TYPE_FUNC;
  this->dst.obj.func = func;
  this->dst.param = func_param;
  log_i("type=%s", OLED_MENU_DST_TYPE_STR[this->dst.type]);
} // OledMenuEnt::OledMenuEnt()

/** constructor
 *
 */
OledMenuEnt::OledMenuEnt(String title, OledMenu *menu) {
  this->title = title;
  this->dst.type = OLED_MENU_DST_TYPE_MENU;
  this->dst.obj.menu = menu;
  log_i("type=%s(%s)",
        OLED_MENU_DST_TYPE_STR[this->dst.type],
        this->dst.obj.menu->title_str());
} // OledMenuEnt::OledMenuEnt()

/** constructor
 *
 */
OledMenuEnt::OledMenuEnt(String title, Mode_t mode) {
  this->title = title;
  this->dst.type = OLED_MENU_DST_TYPE_MODE;
  this->dst.obj.mode = mode;
  log_i("type=%s(%s)", OLED_MENU_DST_TYPE_STR[this->dst.type],
        MODE_T_STR[this->dst.obj.mode]);
} // OledMenuEnt::OledMenuEnt()

/**
 *
 */
const char *OledMenuEnt::title_str() {
    return this->title.c_str();
} // OledMenuEnt::title_str()


/**
 *
 */
OledMenu::OledMenu(String title, int menu_ent_text_size) {
  this->title = title;
  this->init();
  this->menu_ent_text_size = menu_ent_text_size;
} // OledMenu::OledMenu()

/**
 *
 */
void OledMenu::init() {
  this->cur_ent = 0;
  this->disp_top_ent = 0;
} // OledMenu::init()

/**
 *
 */
const char *OledMenu::title_str() {
  return this->title.c_str();
} // OledMenu::title_str()

/**
 *
 */
int OledMenu::addEnt(OledMenuEnt *ment) {
    this->ent.push_back(*ment);
    return this->ent.size();
} // OledMenu::addEnt()

/**
 *
 */
int OledMenu::change_text_size(int text_size) {
  if ( text_size == 1 || text_size == 2 ) {
    this->menu_ent_text_size = text_size;
    return this->menu_ent_text_size;
  }

  if ( this->menu_ent_text_size == 1 ) {
    this->menu_ent_text_size = 2;
  } else {
    this->menu_ent_text_size = 1;
  }
  return this->menu_ent_text_size;
} // OledMenu::change_text_size()

/**
 * @return  destination menu
 */
OledMenuDst_t OledMenu::select() {
  OledMenuEnt ment = this->ent[this->cur_ent];

  if ( ment.dst.type == OLED_MENU_DST_TYPE_FUNC
       && ment.dst.obj.func != NULL ) {
    log_i("[%s] call func()", ment.title_str());
    ment.dst.obj.func(ment.dst.param);
    return ment.dst;
  }
  if ( ment.dst.type == OLED_MENU_DST_TYPE_MENU
       && ment.dst.obj.menu != NULL ) {
    log_i("ment[%s] ==> dst.obj.menu:[%s]",
          ment.title_str(), ment.dst.obj.menu->title_str());
    ment.dst.obj.menu->init();
    return ment.dst;
  }
  if ( ment.dst.type == OLED_MENU_DST_TYPE_MODE
       && ment.dst.obj.mode < MODE_N ) {
    log_i("ment[%s] ==> dst.obj.mode:[%s]",
          ment.title_str(), MODE_T_STR[ment.dst.obj.mode]);
    return ment.dst;
  }
  return ment.dst;
} // OledMenu::select()

/**
 *
 */
void OledMenu::cursor_up() {
  int ent_n = this->ent.size();

  // XXX 循環させる場合
  // this->cur_ent = (this->cur_ent - 1 + ent_n) % ent_n;
  if ( this->cur_ent > 0 ) {
    this->cur_ent--;
  }
  
  if ( this->cur_ent < this->disp_top_ent ) {
    this->disp_top_ent = this->cur_ent;
  }
  log_i("ent=%d/%d top=%d", this->cur_ent, ent_n - 1, this->disp_top_ent);

  if ( this->ent[this->cur_ent].dst.type == OLED_MENU_DST_TYPE_NULL
       && this->cur_ent > 0 ) {
    this->cursor_up();
  }
} // OledMenu::cursor_up()

/**
 *
 */
void OledMenu::cursor_down() {
  int ent_n = this->ent.size();

  // XXX 循環させる場合
  // this->cur_ent = (this->cur_ent - 1 + ent_n) % ent_n;
  if ( this->cur_ent < (ent_n - 1) ) {
    this->cur_ent++;
  }
  
  int disp_ent_n
    = (DISPLAY_H - (DISPLAY_CH_H * MENU_TITLE_TEXT_SIZE))
    / (DISPLAY_CH_H * this->menu_ent_text_size);
  if ( this->cur_ent > this->disp_top_ent + (disp_ent_n - 1) ) {
    this->disp_top_ent = this->cur_ent - (disp_ent_n - 1);
  }
  log_i("ent=%d/%d top=%d", this->cur_ent, ent_n - 1, this->disp_top_ent);

  if ( this->ent[this->cur_ent].dst.type == OLED_MENU_DST_TYPE_NULL
       && this->cur_ent < (ent_n - 1) ) {
    this->cursor_down();
  }
} // OledMenu::cursor_down()

/**
 *
 */
void OledMenu::display(Display_t *disp) {
  disp->clearDisplay();
  disp->setCursor(0,0);
  disp->setTextWrap(false);
  
  disp->setTextSize(MENU_TITLE_TEXT_SIZE);
  disp->printf("%s\n", this->title_str());
  
  disp->drawFastHLine(0, DISPLAY_CH_H * MENU_TITLE_TEXT_SIZE - 1,
                      DISPLAY_W, WHITE);
  
  disp->setTextSize(this->menu_ent_text_size);
  int disp_ent_n
    = (DISPLAY_H - (DISPLAY_CH_H * MENU_TITLE_TEXT_SIZE))
    / (DISPLAY_CH_H * this->menu_ent_text_size);

  for (int i=this->disp_top_ent;
       i <= this->disp_top_ent+disp_ent_n;
       i++) {
    if ( i >= this->ent.size() ) {
      break;
    }
    
    disp->setTextColor(WHITE, BLACK);
    if ( i == this->cur_ent ) {
      disp->setTextColor(BLACK, WHITE);
    }
    disp->printf(" %-16s\n", this->ent[i].title_str());
    disp->setTextColor(WHITE, BLACK);
  } // for (i)
  
  disp->display();
} // OledMenu::display()
