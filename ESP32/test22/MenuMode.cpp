/**
 * Copyright (c) 2022 Yoichi Tanibayashi
 */
#include "MenuMode.h"

/** constructor
 *
 */
MenuMode::MenuMode(String name, CommonData_t *common_data,
                   void (*cb)(String text))
  : ModeBase(name, common_data) {
  this->cb = cb;

} // MenuMode::MenuMode()

/**
 *
 */
void MenuMode::setup() {
  this->topMenu = new OledMenu("TopMenu");
  this->subMenu = new OledMenu("SubMenu");

  OledMenuEnt *ment_mode_main = new OledMenuEnt("<< Clock", MODE_MAIN);
  OledMenuEnt *ment_mode_sto = new OledMenuEnt(" * Temp Offset",
                                               MODE_SET_TEMP_OFFSET);

  OledMenuEnt *ment_menu_top = new OledMenuEnt(" < Top Menu", topMenu);
  OledMenuEnt *ment_menu_sub = new OledMenuEnt(" > Sub Menu", subMenu);

  OledMenuEnt *ment_text_clear_ssid = new OledMenuEnt(" @ clear SSID",
                                                      "clear_ssid");
  OledMenuEnt *ment_text_restart_wifi = new OledMenuEnt(" @ restart WiFi",
                                                        "restart_wifi");
  OledMenuEnt *ment_text_reboot = new OledMenuEnt("!@ reboot", "reboot");


  OledMenuEnt *ment_line = new OledMenuEnt("----------");

  this->topMenu->addEnt(ment_mode_main);
  this->topMenu->addEnt(ment_menu_sub);
  this->topMenu->addEnt(ment_mode_sto);
  this->topMenu->addEnt(ment_line);
  this->topMenu->addEnt(ment_text_clear_ssid);
  this->topMenu->addEnt(ment_text_restart_wifi);
  this->topMenu->addEnt(ment_text_reboot);

  this->subMenu->addEnt(ment_menu_top);
  this->subMenu->addEnt(ment_mode_main);
  this->subMenu->addEnt(ment_line);
  this->subMenu->addEnt(ment_text_restart_wifi);
  this->subMenu->addEnt(ment_text_reboot);

  this->curMenu = this->topMenu;
  this->curMenu->init();
}

/**
 *
 */
bool MenuMode::enter(Mode_t prev_mode) {
  ModeBase::enter(prev_mode);

  if ( prev_mode == MODE_MAIN ) {
    this->curMenu = this->topMenu;
    this->curMenu->init();
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

  if ( bi->click_count > 1 ) {
    return MODE_MAIN;
  }

  /*
   * bi->click_count == 1
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
    log_i("call dst.obj.func(dst.param)");
    dst.obj.func(dst.param);
    dst_mode = MODE_MAIN;
    break;

  case OLED_MENU_DST_TYPE_TEXT:
    log_i("dst.obj.text=\"%s\"", dst.obj.text);

    if ( this->cb != NULL  ) {
      log_i("call this->cb(\"%s\")", dst.obj.text);
      (*(this->cb))(String(dst.obj.text));
    }
    break;

  default:
    log_e("dst.type=%d ??", dst.type);
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
