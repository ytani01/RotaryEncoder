/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "MenuMode.h"

/** constructor
 *
 */
MenuMode::MenuMode(String name, CommonData_t *common_data)
  : ModeBase(name, common_data) {

  // this->dst_mode = MODE_MENU;
  
  this->topMenu = new OledMenu("TopMenu");
  this->subMenu = new OledMenu("SubMenu");

  OledMenuEnt *ment_reboot = new OledMenuEnt("! Reboot", MODE_RESTART);
  OledMenuEnt *ment_mode_main = new OledMenuEnt("<< Clock", MODE_MAIN);
  OledMenuEnt *ment_menu_top = new OledMenuEnt("< Top Menu", topMenu);
  OledMenuEnt *ment_menu_sub = new OledMenuEnt("> Sub Menu", subMenu);
  OledMenuEnt *ment_line = new OledMenuEnt("----------");

  this->topMenu->addEnt(ment_mode_main);
  this->topMenu->addEnt(ment_menu_sub);
  this->topMenu->addEnt(ment_line);
  this->topMenu->addEnt(ment_reboot);

  this->subMenu->addEnt(ment_menu_top);
  this->subMenu->addEnt(ment_mode_main);
  this->subMenu->addEnt(ment_line);
  this->subMenu->addEnt(ment_reboot);
} // MenuMode::MenuMode()

/**
 *
 */
void MenuMode::setup() {
  this->curMenu = this->topMenu;
  this->curMenu->cur_ent = 0;
  this->curMenu->disp_top_ent = 0;
}

/**
 *
 */
bool MenuMode::enter(Mode_t prev_mode) {
  ModeBase::enter(prev_mode);

  if ( prev_mode == MODE_MAIN ) {
    this->setup();
  }
  
  return true;
} // MenuMode()

/**
 * @return  destination mode
 */
Mode_t MenuMode::reBtn_cb(Esp32ButtonInfo_t *bi) {
  Mode_t dst_mode = MODE_N;

  if ( bi->click_count == 0 ) {
    return dst_mode;
  }

  /*
   * bi->click_count > 0
   */
  OledMenuDst_t dst = this->curMenu->select();
  log_i("dst.type=%s", OLED_MENU_DST_TYPE_STR[dst.type]);

  switch ( dst.type ) {
  case OLED_MENU_DST_TYPE_MENU:
    log_i("dst.obj.menu=%s", dst.obj.menu->title_str());
    this->curMenu = dst.obj.menu;
    break;
  case OLED_MENU_DST_TYPE_MODE:
    log_i("dst.obj.mode=%s", MODE_T_STR[dst.obj.mode]);
    dst_mode = dst.obj.mode;
    break;
  case OLED_MENU_DST_TYPE_FUNC:
    // XXX
    break;
  default:
    break;
  } // switch

  return dst_mode;
} // MenuMode::reBtn_cb()

/**
 *
 */
Mode_t MenuMode::obBtn_cb(Esp32ButtonInfo_t *bi) {
  if ( bi->click_count > 0 ) {
    common_data->msg = " Onboard Btn\n";
    common_data->msg += " click:" + String(bi->click_count);
  }
  return MODE_N;
} // MenuMode::obBtn_cb()

/**
 *
 */
Mode_t MenuMode::re_cb(Esp32RotaryEncoderInfo_t *ri) {
  if ( ri->d_angle < 0 ) {
    this->curMenu->cursor_down();
    return MODE_N;
  }
  if ( ri->d_angle > 0 ) {
    this->curMenu->cursor_up();
    return MODE_N;
  }
  return MODE_N;
} // MenuMode::re_cb()

/**
 *
 */
void MenuMode::display(Display_t *disp, float fps) {
  this->curMenu->display(disp);
} // MenuMode::display()

/** protected
 *
 */
