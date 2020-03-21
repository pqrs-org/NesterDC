#ifndef _DC_MENU_COMMON_H_
#define _DC_MENU_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#if 0
}
#endif

typedef struct {
  /* 
   * dc_menu_controller_status_t value status:
   * 
   * 0: invalid (set when dc_menu_controller_status_init)
   * 1: release
   * 2-255: pressed
   * 
   */
  uint8 B;
  uint8 A;
  uint8 START;
  uint8 DPAD_UP;
  uint8 DPAD_DOWN;
  uint8 DPAD_LEFT;
  uint8 DPAD_RIGHT;
  uint8 Y;
  uint8 X;
  
  uint8 rtrig;
  uint8 ltrig;
  
  uint8 analog_up;
  uint8 analog_down;
  uint8 analog_left;
  uint8 analog_right;
  
  uint8 enable;
  maple_device_t *dev;
} dc_menu_controller_status_t;

enum {
  DC_MENU_CONTROLLER_STATUS_INVALID,
  DC_MENU_CONTROLLER_STATUS_RELEASE,
  DC_MENU_CONTROLLER_STATUS_PRESS,
};

extern dc_menu_controller_status_t dc_menu_controller_status[DC_MAPLE_INFO_SIZE];

extern void dc_menu_controller_status_init();
extern void dc_menu_controller_status_scan();
extern int dc_menu_keypress(uint8 key);
extern int dc_menu_keypressing(uint8 key);

#ifdef __cplusplus
}
#endif

#endif

