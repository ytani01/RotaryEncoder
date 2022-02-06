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

  this->cur_ent = (this->cur_ent - 1 + ent_n) % ent_n;
  if ( this->cur_ent < this->disp_top_ent ) {
    this->disp_top_ent = this->cur_ent;
  }
  if ( this->cur_ent > this->disp_top_ent + 5 ) {
    this->disp_top_ent = this->cur_ent - 5;
  }
  log_i("ent=%d/%d top=%d", this->cur_ent, ent_n, this->disp_top_ent);
} // OledMenu::cursor_up()

/**
 *
 */
void OledMenu::cursor_down() {
  OledMenu_t m = this->cur;
  int ent_n = m.ent.size();

  this->cur_ent = (this->cur_ent + 1) % ent_n;
  if ( this->cur_ent < this->disp_top_ent ) {
    this->disp_top_ent = this->cur_ent;
  }
  if ( this->cur_ent > this->disp_top_ent + 5 ) {
    this->disp_top_ent = this->cur_ent - 5;
  }
  log_i("ent=%d/%d top=%d", this->cur_ent, ent_n, this->disp_top_ent);
} // OledMenu::cursor_down()

/**
 *
 */
void OledMenu::display(Display_t *disp) {
  disp->clearDisplay();
  disp->setCursor(0,0);
  disp->setTextSize(2);
  
  OledMenu_t m = this->cur;
  disp->printf("%s\n", m.title);
  
  disp->setTextSize(1);
  for (int i=disp_top_ent; i <= disp_top_ent+5; i++) {
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
