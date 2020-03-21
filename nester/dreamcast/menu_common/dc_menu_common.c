#include "dc_utils.h"
#include "dc_menu_common.h"

dc_menu_controller_status_t dc_menu_controller_status[DC_MAPLE_INFO_SIZE];


void
dc_menu_controller_status_init()
{
  memset(dc_menu_controller_status, 0, sizeof(dc_menu_controller_status));
}


static int 
dc_menu_keyvalid(uint8 key)
{
  return (0 < key) && (key < 255);
}


void
dc_menu_controller_status_scan()
{
  dc_menu_controller_status_t *dmcs;
  dc_maple_info_t *p;
  cont_state_t *stat;
  
  dmcs = dc_menu_controller_status;
  p = dc_maple_controller_info;
  while (p->dev)
  {
    if (dmcs->dev != p->dev)
    {
      /* new controllers inserted */
      memset(dmcs, 0, sizeof(dc_menu_controller_status_t));
      dmcs->dev = p->dev;
    }
    
    stat = maple_dev_status (p->dev);
    
    /* ------------------------------------------------------------ */
#define DMCSS_SETBUTTON(BUTTON) \
    { \
      if (stat->buttons & CONT_##BUTTON) \
      { \
        if (dc_menu_keyvalid(dmcs->BUTTON)) ++(dmcs->BUTTON); \
      } \
      else \
        dmcs->BUTTON = DC_MENU_CONTROLLER_STATUS_RELEASE; \
    } 
    
    DMCSS_SETBUTTON(B);
    DMCSS_SETBUTTON(A);
    DMCSS_SETBUTTON(START);
    DMCSS_SETBUTTON(DPAD_UP);
    DMCSS_SETBUTTON(DPAD_DOWN);
    DMCSS_SETBUTTON(DPAD_LEFT);
    DMCSS_SETBUTTON(DPAD_RIGHT);
    DMCSS_SETBUTTON(Y);
    DMCSS_SETBUTTON(X);
    
    /* ------------------------------------------------------------ */
#define DMCSS_SETTRIGGER(TRIGGER) \
    { \
      if (stat->TRIGGER > 128) \
      { \
        if (dc_menu_keyvalid(dmcs->TRIGGER)) ++(dmcs->TRIGGER); \
      } \
      else \
        dmcs->TRIGGER = DC_MENU_CONTROLLER_STATUS_RELEASE; \
    }
    
    DMCSS_SETTRIGGER(rtrig);
    DMCSS_SETTRIGGER(ltrig);
    
    /* ------------------------------------------------------------ */
    if (stat->joyy < -120)
    {
      if (dc_menu_keyvalid(dmcs->analog_up)) ++(dmcs->analog_up);
    }
    else
      dmcs->analog_up = DC_MENU_CONTROLLER_STATUS_RELEASE;
    
    if (stat->joyy > 120)
    {
      if (dc_menu_keyvalid(dmcs->analog_down)) ++(dmcs->analog_down);
    }
    else
      dmcs->analog_down = DC_MENU_CONTROLLER_STATUS_RELEASE;
    
    if (stat->joyx < -120) 
    {
      if (dc_menu_keyvalid(dmcs->analog_left)) ++(dmcs->analog_left);
    }
    else
      dmcs->analog_left = DC_MENU_CONTROLLER_STATUS_RELEASE;
    
    if (stat->joyx > 120) 
    {
      if (dc_menu_keyvalid(dmcs->analog_right)) ++(dmcs->analog_right);
    }
    else
      dmcs->analog_right = DC_MENU_CONTROLLER_STATUS_RELEASE;
    
    /* ------------------------------------------------------------ */
    ++dmcs;
    ++p;
  }
}


int
dc_menu_keypress(uint8 key)
{
  return key == 2;
}

int
dc_menu_keypressing(uint8 key)
{
  return key >= 2;
}

