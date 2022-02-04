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
  this->top_ent = 0;
} // OledMenu::OledMenu()

/**
 *
 */
int OledMenu::get_ent_n(OledMenu_t menu) {
  int ent_n = 0;
  for (int i=0; i < ENT_N; i++) {
    if ( strlen(menu.ent[i].title) == 0 ) {
      break;
    }
    ent_n++;
  }
  return ent_n;
} // OledMenu::get_ent_n()

/**
 *
 */
void OledMenu::cursor_up() {
  OledMenu_t m = this->cur;
  int ent_n = this->get_ent_n(m);

  this->cur_ent = (this->cur_ent - 1 + ent_n) % ent_n;
  if ( this->cur_ent < this->top_ent ) {
    this->top_ent = this->cur_ent;
  }
  if ( this->cur_ent > this->top_ent + 5 ) {
    this->top_ent = this->cur_ent - 5;
  }
  log_i("ent=%d/%d top=%d", this->cur_ent, ent_n, this->top_ent);
} // OledMenu::cursor_up()

/**
 *
 */
void OledMenu::cursor_down() {
  OledMenu_t m = this->cur;
  int ent_n = this->get_ent_n(m);

  this->cur_ent = (this->cur_ent + 1) % ent_n;
  if ( this->cur_ent < this->top_ent ) {
    this->top_ent = this->cur_ent;
  }
  if ( this->cur_ent > this->top_ent + 5 ) {
    this->top_ent = this->cur_ent - 5;
  }
  log_i("ent=%d/%d top=%d", this->cur_ent, ent_n, this->top_ent);
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
  for (int i=top_ent; i <= top_ent+5; i++) {
    if ( strlen(m.ent[i].title) == 0 ) {
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
