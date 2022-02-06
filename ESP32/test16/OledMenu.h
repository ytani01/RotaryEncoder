/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _OLED_MENU_H_
#define _OLED_MENU_H_

#include "common.h"
#include "Oled.h"

constexpr int MENU_TITLE_TEXT_SIZE = 2;
constexpr int MENU_ENT_TEXT_SIZE = 2;
constexpr int OLED_MENU_DISP_ENT_N
= (OLED_DISP_H - (OLED_CH_H * MENU_TITLE_TEXT_SIZE))
  / (OLED_CH_H * MENU_ENT_TEXT_SIZE);

static constexpr int TITLE_LEN = 16;

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
  std::vector<OledMenuEnt_t> ent;
  uint8_t cur;
} OledMenu_t;

class OledMenu; // 不完全型: 相互参照のために必要

/** XXX ToDo: OledMenuEnt_tのクラス化
 *
 */
class OledMenuEnt {
public:
  String title;
  OledMenuEntType_t type;
  union {
    void (*func)();
    OledMenu *menu;
  } dst;

  OledMenuEnt(String title, void (*func)()=NULL) {
    this->title = title;
    this->dst.func = func;
    this->type = OLED_MENU_ENT_TYPE_FUNC;
  };
  OledMenuEnt(String title, OledMenu *menu=NULL) {
    this->title = title;
    this->dst.menu = menu;
    this->type = OLED_MENU_ENT_TYPE_MENU;
  };

  const char *title_str() {
    return this->title.c_str();
  };
}; // class OledMEnuEnt

/**
 *
 */
class OledMenu {
 public:
  OledMenu_t top;
  OledMenu_t cur;
  int cur_ent;
  int disp_top_ent;
  
  std::vector<OledMenuEnt> ent; // XXX

  OledMenu(OledMenu_t top);

  void cursor_up();
  void cursor_down();
  void display(Display_t *disp);
}; // class OledMenu

#endif // _OLED_MENU_H_
