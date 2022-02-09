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
  OledMenuEnt *ment_to_top = new OledMenuEnt("< Top", topMenu);
  OledMenuEnt *ment_to_sub = new OledMenuEnt("> Sub", subMenu);
  OledMenuEnt *ment_line = new OledMenuEnt("----------");

  this->topMenu->addEnt(ment_mode_main);
  this->topMenu->addEnt(ment_to_sub);
  this->topMenu->addEnt(ment_line);
  this->topMenu->addEnt(ment_to_sub);
  this->topMenu->addEnt(ment_to_sub);
  this->topMenu->addEnt(ment_to_sub);
  this->topMenu->addEnt(ment_mode_main);
  this->topMenu->addEnt(ment_reboot);

  this->subMenu->addEnt(ment_line);
  this->subMenu->addEnt(ment_to_top);
  this->subMenu->addEnt(ment_line);
  this->subMenu->addEnt(ment_to_top);
  this->subMenu->addEnt(ment_to_top);
  this->subMenu->addEnt(ment_to_top);
  this->subMenu->addEnt(ment_to_top);
  this->subMenu->addEnt(ment_line);
  this->subMenu->addEnt(ment_mode_main);
  this->subMenu->addEnt(ment_line);
  this->subMenu->addEnt(ment_reboot);
  this->subMenu->addEnt(ment_line);
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
  if ( bi->click_count > 0 ) {
    OledMenuEnt ment = this->curMenu->ent[this->curMenu->cur_ent];

    this->curMenu = this->curMenu->select();
    log_i("%s", this->curMenu->title_str());
    
    if ( ment.type == OLED_MENU_ENT_TYPE_MODE ) {
      return ment.dst.mode;
    }
  }
  return MODE_N;
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
