/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _OLED_MENU_H_
#define _OLED_MENU_H_

#include "Oled.h"

static constexpr int TITLE_LEN = 16;
static constexpr int ENT_N = 16;

typedef enum {
              OLED_MENU_ENT_TYPE_FUNC,
              OLED_MENU_ENT_TYPE_MENU,
              OLED_MENU_ENT_TYPE_N
} OledMenuEntType_t;

typedef struct oled_menu_ent {
  char title[TITLE_LEN];
  OledMenuEntType_t type;
  union _dst {
    void (*func)();
    struct oled_menu *menu;
  } dst;
} OledMenuEnt_t;

typedef struct oled_menu {
  char title[TITLE_LEN];
  OledMenuEnt_t ent[ENT_N];
  uint8_t cur;
} OledMenu_t;

/**
 *
 */
class OledMenu {
 public:
  OledMenu_t top;
  OledMenu_t cur;
  int cur_ent;
  int top_ent;
  
  OledMenu(OledMenu_t top);

  int get_ent_n(OledMenu_t menu);
  void cursor_up();
  void cursor_down();
  void display(Display_t *disp);
}; // class OledMenu

#endif // _OLED_MENU_H_
