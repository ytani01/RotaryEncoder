/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#ifndef _OLED_MENU_H_
#define _OLED_MENU_H_

#include <vector>
#include <Arduino.h>
#include <esp32-hal-log.h>
#include "Display.h"

constexpr int MENU_TITLE_TEXT_SIZE = 2;
constexpr int MENU_ENT_TEXT_SIZE = 1;

static constexpr int TITLE_LEN = 16;

typedef enum {
              OLED_MENU_ENT_TYPE_FUNC,
              OLED_MENU_ENT_TYPE_MENU,
              OLED_MENU_ENT_TYPE_NULL,
              OLED_MENU_ENT_TYPE_N
} OledMenuEntType_t;
const String OLED_MENU_ENT_TYPE_STR[] = {"FUNC", "MENU", "NULL"};

class OledMenu; // 不完全型: OledMenuEntからの相互参照のために必要

extern OledMenu *OledMenu_curMenu;

/**
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

  OledMenuEnt(String title);
  OledMenuEnt(String title, void (*func)());
  OledMenuEnt(String title, OledMenu *menu);

  const char *title_str();
}; // class OledMEnuEnt

/**
 *
 */
class OledMenu {
public:
  String title;
  std::vector<OledMenuEnt> ent;
  int cur_ent;
  int disp_top_ent;
  int menu_ent_text_size;

  OledMenu(String title, int menu_ent_text_size=MENU_ENT_TEXT_SIZE);

  void init();
  const char *title_str();
  int addEnt(OledMenuEnt *ment);
  int change_text_size(int text_size=0);

  bool select();
  void cursor_up();
  void cursor_down();

  void display(Display_t *disp);
}; // OledMenu

#endif // _OLED_MENU_H_
