/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "OledMenu.h"

OledMenu *OledMenu_curMenu = NULL;

/**
 *
 */
OledMenuEnt::OledMenuEnt(String title) {
  this->title = title;
  this->type = OLED_MENU_ENT_TYPE_NULL;
  log_i("type=%s", OLED_MENU_ENT_TYPE_STR[this->type].c_str());
} // OledMenuEnt::OledMenuEnt()

/**
 *
 */
OledMenuEnt::OledMenuEnt(String title, void (*func)()) {
  this->title = title;
  this->dst.func = func;
  this->type = OLED_MENU_ENT_TYPE_FUNC;
  log_i("type=%s", OLED_MENU_ENT_TYPE_STR[this->type].c_str());
} // OledMenuEnt::OledMenuEnt()

/**
 *
 */
OledMenuEnt::OledMenuEnt(String title, OledMenu *menu) {
  this->title = title;
  this->dst.menu = menu;
  this->type = OLED_MENU_ENT_TYPE_MENU;
  log_i("type=%s", OLED_MENU_ENT_TYPE_STR[this->type].c_str());
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
 *
 */
bool OledMenu::select() {
  OledMenuEnt ment = this->ent[this->cur_ent];

  if ( ment.type == OLED_MENU_ENT_TYPE_FUNC && ment.dst.func != NULL ) {
    log_i("[%s] call func()", ment.title_str());
    ment.dst.func();
    return true;
  }
  if ( ment.type == OLED_MENU_ENT_TYPE_MENU && ment.dst.menu != NULL ) {
    log_i("ment[%s] ==> dst.menu:[%s]",
          ment.title_str(), ment.dst.menu->title_str());
    OledMenu_curMenu = ment.dst.menu;
    OledMenu_curMenu->init();
    return true;
  }
  return false;
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

  if ( this->ent[this->cur_ent].type == OLED_MENU_ENT_TYPE_NULL ) {
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
  if ( this->cur_ent < ent_n - 1 ) {
    this->cur_ent++;
  }
  
  int disp_ent_n
    = (DISPLAY_H - (DISPLAY_CH_H * MENU_TITLE_TEXT_SIZE))
    / (DISPLAY_CH_H * this->menu_ent_text_size);
  if ( this->cur_ent > this->disp_top_ent + (disp_ent_n - 1) ) {
    this->disp_top_ent = this->cur_ent - (disp_ent_n - 1);
  }
  log_i("ent=%d/%d top=%d", this->cur_ent, ent_n - 1, this->disp_top_ent);

  if ( this->ent[this->cur_ent].type == OLED_MENU_ENT_TYPE_NULL ) {
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
