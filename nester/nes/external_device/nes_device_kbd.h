#ifndef _NES_DEVICE_KBD_H_
#define _NES_DEVICE_KBD_H_

class NES_device_kbd
{
public:
  bool initialize();
  
  uint8 ReadReg4016();
  uint8 ReadReg4017();
  
  void WriteReg4016(uint8 data);
  
  void clear_register();
  void setkey(uint8 key);
  
  typedef struct {
    int register_index;
    int flag;
  } kbd_table_t;
  
  enum {
    NES_KEY_VoidSymbol, /* 0x00 */
    
    NES_KEY_A,          /* 0x01 */
    NES_KEY_B,
    NES_KEY_C,
    NES_KEY_D,
    NES_KEY_E,
    NES_KEY_F,
    NES_KEY_G,
    NES_KEY_H,
    NES_KEY_I,
    NES_KEY_J,
    NES_KEY_K,
    NES_KEY_L,
    NES_KEY_M,
    NES_KEY_N,
    NES_KEY_O,
    NES_KEY_P,
    NES_KEY_Q,
    NES_KEY_R,
    NES_KEY_S,
    NES_KEY_T,
    NES_KEY_U,
    NES_KEY_V,
    NES_KEY_W,
    NES_KEY_X,
    NES_KEY_Y,
    NES_KEY_Z,
    
    NES_KEY_0,          /* 0x1b */
    NES_KEY_1,
    NES_KEY_2,
    NES_KEY_3,
    NES_KEY_4,
    NES_KEY_5,
    NES_KEY_6,
    NES_KEY_7,
    NES_KEY_8,
    NES_KEY_9,
    
    NES_KEY_F1,         /* 0x25 */
    NES_KEY_F2,
    NES_KEY_F3,
    NES_KEY_F4,
    NES_KEY_F5,
    NES_KEY_F6,
    NES_KEY_F7,
    NES_KEY_F8,
    
    NES_KEY_Kana,
    NES_KEY_Grph,
    NES_KEY_Stop,
    NES_KEY_Return,     /* 0x2e */
    NES_KEY_Escape,
    NES_KEY_Control_L,
    NES_KEY_Clear,
    NES_KEY_Shift_L,
    NES_KEY_Shift_R,
    NES_KEY_Up,
    NES_KEY_Down,
    NES_KEY_Right,
    NES_KEY_Left,
    
    NES_KEY_Space,      /* 0x3a */
    NES_KEY_Delete,
    NES_KEY_Insert,
    
    NES_KEY_at,             /* '@' */ /* 0x3d */
    NES_KEY_colon,          /* ':' */
    NES_KEY_semicolon,      /* ';' */
    NES_KEY_bracketleft,    /* '[' */
    NES_KEY_bracketright,   /* ']' */
    NES_KEY_backslash,      /* '\' */
    NES_KEY_apostrophe,     /* ''' */
    NES_KEY_slash,          /* '/' */
    NES_KEY_minus,          /* '-' */
    NES_KEY_caret,          /* '^' */
    NES_KEY_period,         /* '.' */
    NES_KEY_comma,          /* ',' */
  };
  
private:
  /* 
     kbd_register, kbd_table contain 2 different data at HI,LO.
     
     kbd_table: HHHH LLLL (8bit) 
        - HHHH is 4017 register which kb_out == 1
        - LLLL is 4017 register which kb_out == 0
  */
  
  uint8 kbd_register[10];
  kbd_table_t kbd_table[256];
  int kb_scan;
  int kb_out;
  
  void kbd_table_init(uint8 *register_table, int kb_out);
};

#endif


