#include "types.h"
#include "nes.h"
#include "nes_mapper.h"
#include "188.h"

/////////////////////////////////////////////////////////////////////
// Mapper 188
void NES_mapper188::Reset()
{
  // set CPU bank pointers
  if(num_8k_ROM_banks > 16)
  {
    set_CPU_banks(0,1,14,15);
  }
  else
  {
    set_CPU_banks(0,1,num_8k_ROM_banks-2,num_8k_ROM_banks-1);
  }
  
  (nes6502_get_current_context ())->mem_page[3] = dummy;
  nes6502_setcontext();
  
  dummy[0] = 0x03; // ?
}

// Coded by rinao
void NES_mapper188::MemoryWrite(uint32 addr, uint8 data)
{
  if (data)
  {
    if (data & 0x10)
    {
      data &= 0x07;
      set_CPU_bank4(data*2);
      set_CPU_bank5(data*2+1);
    }
    else
    {
      set_CPU_bank4(data*2+16);
      set_CPU_bank5(data*2+17);
    }
  }
  else
  {
    if (num_8k_ROM_banks == 0x10)
    {
      set_CPU_bank4(14);
      set_CPU_bank5(15);
    }
    else
    {
      set_CPU_bank4(16);
      set_CPU_bank5(17);
    }
  }
  //data &= num_8k_ROM_banks-1;
  //set_CPU_banks(data*2,(data*2)+1,num_8k_ROM_banks-2,num_8k_ROM_banks-1);
}
/////////////////////////////////////////////////////////////////////

