/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "OledMenu.h"

/** constructor
 *
 */
OledMenu::OledMenu(OledMenu_t top) {
  this->top = top;
  this->cur = top;
  this->cur_ent = 0;
  this->disp_top_ent = 0;
} // OledMenu::OledMenu()

/**
 *
 */
void OledMenu::cursor_up() {
  OledMenu_t m = this->cur;
  int ent_n = m.ent.size();

  // XXX 循環させる場合
  // this->cur_ent = (this->cur_ent - 1 + ent_n) % ent_n;
  if ( this->cur_ent > 0 ) {
    this->cur_ent--;
  }

  if ( this->cur_ent < this->disp_top_ent ) {
    this->disp_top_ent = this->cur_ent;
  }
  log_i("ent=%d/%d top=%d", this->cur_ent, ent_n - 1, this->disp_top_ent);
} // OledMenu::cursor_up()

/**
 *
 */
void OledMenu::cursor_down() {
  OledMenu_t m = this->cur;
  int ent_n = m.ent.size();

  // XXX 循環させる場合
  // this->cur_ent = (this->cur_ent - 1 + ent_n) % ent_n;
  if ( this->cur_ent < ent_n - 1 ) {
    this->cur_ent++;
  }

  if ( this->cur_ent > this->disp_top_ent + (OLED_MENU_DISP_ENT_N - 1) ) {
    this->disp_top_ent = this->cur_ent - (OLED_MENU_DISP_ENT_N - 1);
  }
  log_i("ent=%d/%d top=%d", this->cur_ent, ent_n - 1, this->disp_top_ent);
} // OledMenu::cursor_down()

/**
 *
 */
void OledMenu::display(Display_t *disp) {
  disp->clearDisplay();
  disp->setCursor(0,0);
  disp->setTextWrap(false);

  disp->setTextSize(MENU_TITLE_TEXT_SIZE);
  OledMenu_t m = this->cur;
  disp->printf("%s\n", m.title);

  disp->drawFastHLine(0, OLED_CH_H * MENU_TITLE_TEXT_SIZE - 1,
                      OLED_DISP_W, WHITE);
  
  disp->setTextSize(MENU_ENT_TEXT_SIZE);
  for (int i=disp_top_ent; i <= disp_top_ent+OLED_MENU_DISP_ENT_N; i++) {
    if ( i >= m.ent.size() ) {
      break;
    }
    
    disp->setTextColor(WHITE, BLACK);
    if ( i == this->cur_ent ) {
      disp->setTextColor(BLACK, WHITE);
    }
    disp->printf(" %-16s \n", m.ent[i].title);
    disp->setTextColor(WHITE, BLACK);
  } // for (i)
  
  disp->display();
} // OledMenu::display();
