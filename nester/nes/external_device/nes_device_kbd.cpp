#include "nes.h"
#include "nes_device_kbd.h"


static uint8 nes_kbd_register_table0[] = { 
  /* kb_scan 0 */
  NES_device_kbd::NES_KEY_VoidSymbol,
  NES_device_kbd::NES_KEY_VoidSymbol,
  NES_device_kbd::NES_KEY_VoidSymbol,
  NES_device_kbd::NES_KEY_VoidSymbol,
  
  /* kb_scan 1 */
  NES_device_kbd::NES_KEY_Kana,
  NES_device_kbd::NES_KEY_Shift_R,
  NES_device_kbd::NES_KEY_backslash,
  NES_device_kbd::NES_KEY_Stop,
  
  /* kb_scan 2 */
  NES_device_kbd::NES_KEY_apostrophe,
  NES_device_kbd::NES_KEY_slash,
  NES_device_kbd::NES_KEY_minus,
  NES_device_kbd::NES_KEY_caret,
  
  /* kb_scan 3 */
  NES_device_kbd::NES_KEY_period,
  NES_device_kbd::NES_KEY_comma,
  NES_device_kbd::NES_KEY_P,
  NES_device_kbd::NES_KEY_0,
  
  /* kb_scan 4 */
  NES_device_kbd::NES_KEY_M,
  NES_device_kbd::NES_KEY_N,
  NES_device_kbd::NES_KEY_9,
  NES_device_kbd::NES_KEY_8,
  
  /* kb_scan 5 */
  NES_device_kbd::NES_KEY_B,
  NES_device_kbd::NES_KEY_V,
  NES_device_kbd::NES_KEY_7,
  NES_device_kbd::NES_KEY_6,
  
  /* kb_scan 6 */
  NES_device_kbd::NES_KEY_F,
  NES_device_kbd::NES_KEY_C,
  NES_device_kbd::NES_KEY_5,
  NES_device_kbd::NES_KEY_4,
  
  /* kb_scan 7 */
  NES_device_kbd::NES_KEY_X,
  NES_device_kbd::NES_KEY_Z,
  NES_device_kbd::NES_KEY_E,
  NES_device_kbd::NES_KEY_3,
  
  /* kb_scan 8 */
  NES_device_kbd::NES_KEY_Shift_L,
  NES_device_kbd::NES_KEY_Grph,
  NES_device_kbd::NES_KEY_1,
  NES_device_kbd::NES_KEY_2,
  
  /* kb_scan 9 */
  NES_device_kbd::NES_KEY_Down,
  NES_device_kbd::NES_KEY_Space,
  NES_device_kbd::NES_KEY_Delete,
  NES_device_kbd::NES_KEY_Insert,
};


static uint8 nes_kbd_register_table1[] = {
  /* kb_scan 0 */
  NES_device_kbd::NES_KEY_VoidSymbol,
  NES_device_kbd::NES_KEY_VoidSymbol,
  NES_device_kbd::NES_KEY_VoidSymbol,
  NES_device_kbd::NES_KEY_VoidSymbol,
  
  /* kb_scan 1 */
  NES_device_kbd::NES_KEY_F8,
  NES_device_kbd::NES_KEY_Return,
  NES_device_kbd::NES_KEY_bracketleft,
  NES_device_kbd::NES_KEY_bracketright,
  
  /* kb_scan 2 */
  NES_device_kbd::NES_KEY_F7,
  NES_device_kbd::NES_KEY_at,
  NES_device_kbd::NES_KEY_colon,
  NES_device_kbd::NES_KEY_semicolon,
  
  /* kb_scan 3 */
  NES_device_kbd::NES_KEY_F6,
  NES_device_kbd::NES_KEY_O,
  NES_device_kbd::NES_KEY_L,
  NES_device_kbd::NES_KEY_K,
  
  /* kb_scan 4 */
  NES_device_kbd::NES_KEY_F5,
  NES_device_kbd::NES_KEY_I,
  NES_device_kbd::NES_KEY_U,
  NES_device_kbd::NES_KEY_J,
  
  /* kb_scan 5 */
  NES_device_kbd::NES_KEY_F4,
  NES_device_kbd::NES_KEY_Y,
  NES_device_kbd::NES_KEY_G,
  NES_device_kbd::NES_KEY_H,
  
  /* kb_scan 6 */
  NES_device_kbd::NES_KEY_F3,
  NES_device_kbd::NES_KEY_T,
  NES_device_kbd::NES_KEY_R,
  NES_device_kbd::NES_KEY_D,
  
  /* kb_scan 7 */
  NES_device_kbd::NES_KEY_F2,
  NES_device_kbd::NES_KEY_W,
  NES_device_kbd::NES_KEY_S,
  NES_device_kbd::NES_KEY_A,
  
  /* kb_scan 8 */
  NES_device_kbd::NES_KEY_F1,
  NES_device_kbd::NES_KEY_Escape,
  NES_device_kbd::NES_KEY_Q,
  NES_device_kbd::NES_KEY_Control_L,
  
  /* kb_scan 9 */
  NES_device_kbd::NES_KEY_Clear,
  NES_device_kbd::NES_KEY_Up,
  NES_device_kbd::NES_KEY_Right,
  NES_device_kbd::NES_KEY_Left,
};


void
NES_device_kbd::kbd_table_init(uint8 *register_table, int kb_out)
{
  int i;
  int shift = kb_out ? 4 : 0;
  uint8 *p = register_table;
  
  for (i = 0; i < 10; ++i)
  {
    if (*p != NES_KEY_VoidSymbol) 
    {
      kbd_table[*p].register_index = i;
      kbd_table[*p].flag = 0x01 << shift;
    }
    ++p;
    
    if (*p != NES_KEY_VoidSymbol)
    {
      kbd_table[*p].register_index = i;
      kbd_table[*p].flag = 0x02 << shift;
    }
    ++p;
    
    if (*p != NES_KEY_VoidSymbol)
    {
      kbd_table[*p].register_index = i;
      kbd_table[*p].flag = 0x04 << shift;
    }
    ++p;
    
    if (*p != NES_KEY_VoidSymbol)
    {
      kbd_table[*p].register_index = i;
      kbd_table[*p].flag = 0x08 << shift;
    }
    ++p;
  }
}


bool
NES_device_kbd::initialize()
{
  int i;
  kbd_table_t *kt;
  
  kt = kbd_table;
  for (i = 0; i < 256; ++i)
  {
    kt->register_index = 0;
    kt->flag = 0;
    ++kt;
  }
  
  clear_register ();
  
  kbd_table_init (nes_kbd_register_table0, 0);
  kbd_table_init (nes_kbd_register_table1, 1);
  
  kb_scan = 0;
  kb_out = 0;
  
  return true;
}


void
NES_device_kbd::clear_register()
{
  int i;
  
  for (i = 0; i < 10; ++i)
    kbd_register[i] = 0;
}

void
NES_device_kbd::setkey(uint8 key)
{
  kbd_table_t *p = kbd_table + key;
  
  kbd_register[p->register_index] |= p->flag;
}


uint8
NES_device_kbd::ReadReg4016()
{
#if 0
  if(tape_status == 1 && tape_in) 
    return 0x02;
#endif
  
  return 0;
}


uint8
NES_device_kbd::ReadReg4017()
{
  uint8 rv;
  
  if (kb_out)
    rv = (kbd_register[kb_scan] >> 4) & 0x0f;
  else
    rv = kbd_register[kb_scan] & 0x0f;
  
  return ~(rv << 1);
}


void
NES_device_kbd::WriteReg4016(uint8 data)
{
  if (data == 0x05)
  {
    kb_out = 0;
    kb_scan = 0;
  }
  else if (data  == 0x04)
  {
    kb_scan++;
    if (kb_scan > 9) kb_scan = 0;
    kb_out ^= 1;
  }
  else if (data == 0x06)
  {
    kb_out ^= 1;
  }
  
#if 0
  if (tape_status == 2)
  {
    tape_out = data & 0x02;
  }
#endif
}



