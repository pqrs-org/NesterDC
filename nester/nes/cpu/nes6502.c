/*
** Nofrendo (c) 1998-2000 Matthew Conte (matt@conte.com)
**
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of version 2 of the GNU Library General 
** Public License as published by the Free Software Foundation.
**
** This program is distributed in the hope that it will be useful, 
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
** Library General Public License for more details.  To obtain a 
** copy of the GNU Library General Public License, write to the Free 
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
**
**
** nes6502.c
**
** NES custom 6502 (2A03) CPU implementation
** nes6502.c,v 1.3.2.6 2002/11/07 13:00:04 tekezo Exp
*/

#include "stdlib.h"
#include "types.h"
#include "nes6502.h"

#define ASSERT(CONDITION)
#define INLINE inline

#if defined(__GNUC__) && !defined(NES6502_DISASM)
#define  NES6502_JUMPTABLE
#endif /* __GNUC__ */ 

#define  ADD_CYCLES(x) \
{ \
  remaining_cycles -= (x); \
  cpu.total_cycles += (x); \
}

/*
** Check to see if an index reg addition overflowed to next page
*/
#define PAGE_CROSS_CHECK(addr, reg) \
{ \
  ADD_CYCLES((reg) > (addr & 0xff)); \
}

#define EMPTY_READ(value)  /* empty */

/*
** Addressing mode macros
*/

/* Immediate */
#define IMMEDIATE_BYTE(value) \
{ \
  value = bank_readbyte(REG_PC++); \
}

/* Absolute */
#define ABSOLUTE_ADDR(address) \
{ \
  address = bank_readword(REG_PC); \
  REG_PC += 2; \
}

#define ABSOLUTE(address, value) \
{ \
  ABSOLUTE_ADDR(address); \
  value = mem_readbyte(address); \
}

#define ABSOLUTE_BYTE(value) \
{ \
  uint32 addr_tmp_absolutebyte; \
  ABSOLUTE(addr_tmp_absolutebyte, value); \
}

/* Absolute indexed X */
#define ABS_IND_X_ADDR(address) \
{ \
  ABSOLUTE_ADDR(address); \
  address = (address + REG_X) & 0xffff; \
  PAGE_CROSS_CHECK(address, REG_X); \
}

#define ABS_IND_X(address, value) \
{ \
  ABS_IND_X_ADDR(address); \
  value = mem_readbyte(address); \
}

#define ABS_IND_X_BYTE(value) \
{ \
  uint32 addr_tmp_absindxbyte; \
  ABS_IND_X(addr_tmp_absindxbyte, value); \
}

/* Absolute indexed Y */
#define ABS_IND_Y_ADDR(address) \
{ \
  ABSOLUTE_ADDR(address); \
  address = (address + REG_Y) & 0xffff; \
  PAGE_CROSS_CHECK(address, REG_Y); \
}

#define ABS_IND_Y(address, value) \
{ \
  ABS_IND_Y_ADDR(address); \
  value = mem_readbyte(address); \
}

#define ABS_IND_Y_BYTE(value) \
{ \
  uint32 addr_tmp_absindybyte; \
  ABS_IND_Y(addr_tmp_absindybyte, value); \
}

/* Zero-page */
#define ZERO_PAGE_ADDR(address) \
{ \
  IMMEDIATE_BYTE(address); \
}

#define ZERO_PAGE(address, value) \
{ \
  ZERO_PAGE_ADDR(address); \
  value = ZP_READBYTE(address); \
}

#define ZERO_PAGE_BYTE(value) \
{ \
  uint32 addr_tmp_zeropagebyte; \
  ZERO_PAGE(addr_tmp_zeropagebyte, value); \
}

/* Zero-page indexed X */
#define ZP_IND_X_ADDR(address) \
{ \
  ZERO_PAGE_ADDR(address); \
  address += REG_X; \
}

#define ZP_IND_X(address, value) \
{ \
  ZP_IND_X_ADDR(address); \
  value = ZP_READBYTE(address); \
}

#define ZP_IND_X_BYTE(value) \
{ \
  uint32 addr_tmp_zpindxbyte; \
  ZP_IND_X(addr_tmp_zpindxbyte, value); \
}

/* Zero-page indexed Y */
/* Not really an adressing mode, just for LDx/STx */
#define ZP_IND_Y_ADDR(address) \
{ \
  ZERO_PAGE_ADDR(address); \
  address += REG_Y; \
}

#define ZP_IND_Y_BYTE(value) \
{ \
  uint32 addr_tmp_zpindybyte; \
  ZP_IND_Y_ADDR(addr_tmp_zpindybyte); \
  value = ZP_READBYTE(addr_tmp_zpindybyte); \
}

/* Indexed indirect */
#define INDIR_X_ADDR(address) \
{ \
  uint32 addr_tmp_indirxaddr; \
  IMMEDIATE_BYTE(addr_tmp_indirxaddr); \
  address = zp_readword(addr_tmp_indirxaddr + REG_X); \
}

#define INDIR_X(address, value) \
{ \
  INDIR_X_ADDR(address); \
  value = mem_readbyte(address); \
} 

#define INDIR_X_BYTE(value) \
{ \
  uint32 addr_tmp_indirxbyte; \
  INDIR_X(addr_tmp_indirxbyte, value); \
}

/* Indirect indexed */
#define INDIR_Y_ADDR(address) \
{ \
  uint32 addr_tmp_indiryaddr; \
  IMMEDIATE_BYTE(addr_tmp_indiryaddr); \
  address = (zp_readword(addr_tmp_indiryaddr) + REG_Y) & 0xffff; \
  PAGE_CROSS_CHECK(address, REG_Y); \
}

#define INDIR_Y(address, value) \
{ \
  INDIR_Y_ADDR(address); \
  value = mem_readbyte(address); \
}

#define INDIR_Y_BYTE(value) \
{ \
  uint32 addr_tmp_indirybyte; \
  INDIR_Y(addr_tmp_indirybyte, value); \
}


/* Stack push/pull */
#define  PUSH(value)             stack[REG_S--] = (value)
#define  PULL()                  stack[++REG_S]


/*
** flag register helper macros
*/

/* Theory: Z and N flags are set in just about every
** instruction, so we will just store the value in those
** flag variables, and mask out the irrelevant data when
** we need to check them (branches, etc).  This makes the
** zero flag only really be 'set' when flag_z == 0.
** The rest of the flags are stored as true booleans.
*/

/* Scatter flags to separate variables */
#define  SCATTER_FLAGS(value) \
{ \
  flag_n = (value) & N_FLAG; \
  flag_v = (value) & V_FLAG; \
  flag_b = (value) & B_FLAG; \
  flag_d = (value) & D_FLAG; \
  flag_i = (value) & I_FLAG; \
  flag_z = !(value & Z_FLAG); \
  flag_c = (value) & C_FLAG; \
}

/* Combine flags into flag register */
#define  COMBINE_FLAGS() \
( \
  (flag_n & N_FLAG) | \
  flag_v | \
  R_FLAG | \
  flag_b | \
  flag_d | \
  flag_i | \
  ((flag_z == 0) << 1) | \
  flag_c \
)

/* Set N and Z flags based on given value */
/* this code is called many many times! */
#define  SET_NZ_FLAGS(value) { \
  flag_n = flag_z = (value); \
}

/* For BCC, BCS, BEQ, BMI, BNE, BPL, BVC, BVS */
#define RELATIVE_BRANCH(condition) \
{ \
  if (condition) \
  { \
    uint32 addr_tmp_relativebranch; \
    IMMEDIATE_BYTE(addr_tmp_relativebranch); \
    ADD_CYCLES(3 + ((((int8)addr_tmp_relativebranch + (REG_PC & 0xff)) >> 8) & 0x1)); \
    REG_PC += ((int8)addr_tmp_relativebranch); \
  } \
  else \
  { \
    REG_PC++; \
    ADD_CYCLES(2); \
  } \
}

#define JUMP(address) \
{ \
  REG_PC = bank_readword((address)); \
}

/*
** Interrupt macros
*/
#define NMI_PROC() \
{ \
  PUSH(REG_PC >> 8); \
  PUSH(REG_PC & 0xff); \
  flag_b = 0; \
  PUSH(COMBINE_FLAGS()); \
  flag_i = I_FLAG; \
  JUMP(NMI_VECTOR); \
}

#define IRQ_PROC() \
{ \
  PUSH(REG_PC >> 8); \
  PUSH(REG_PC & 0xFF); \
  flag_b = 0; \
  PUSH(COMBINE_FLAGS()); \
  flag_i = I_FLAG; \
  JUMP(IRQ_VECTOR); \
}

#define NMI() \
{ \
  NMI_PROC(); \
  ADD_CYCLES(INT_CYCLES); \
}

#define IRQ() \
{ \
  IRQ_PROC(); \
  ADD_CYCLES(INT_CYCLES); \
}

/*
** Instruction macros
*/

#define ADC(cycles, read_func) \
{ \
  read_func(data); /* read_func == byteread */ \
  { \
    uint32 temp; \
    temp = REG_A + data + flag_c; \
    flag_c = (temp > 0xff); \
    flag_v = ((~(REG_A ^ data)) & (REG_A ^ temp) & 0x80) >> 1; \
    REG_A = temp & 0xff; \
  } \
  SET_NZ_FLAGS(REG_A); \
  ADD_CYCLES(cycles); \
}

/* undocumented */
#define ANC(cycles, read_func) \
{ \
  read_func(data); /* read_func == byteread */ \
  REG_A &= data; \
  SET_NZ_FLAGS(REG_A); \
  flag_c = (flag_n >> 7) & 0x1; \
  ADD_CYCLES(cycles); \
}

#define AND(cycles, read_func) \
{ \
  read_func(data); /* read_func == byteread */ \
  REG_A &= data; \
  SET_NZ_FLAGS(REG_A); \
  ADD_CYCLES(cycles); \
}

/* undocumented */
#define ANE(cycles, read_func) \
{ \
  read_func(data); /* read_func == byteread */ \
  REG_A = (REG_A | 0xee) & REG_X & data; \
  SET_NZ_FLAGS(REG_A); \
  ADD_CYCLES(cycles); \
}

/* undocumented */
#define ARR(cycles, read_func) \
{ \
  read_func(data); /* read_func == byteread */ \
  data &= REG_A; \
  REG_A = (data >> 1) | (flag_c << 7); \
  SET_NZ_FLAGS(REG_A); \
  flag_c = (REG_A >> 6) & 0x1; \
  flag_v = (REG_A ^ (REG_A << 1)) & 0x40; \
  ADD_CYCLES(cycles); \
}

#define ASL(cycles, read_func, write_func) \
{ \
  uint32 addr_tmp_asl; \
  read_func(addr_tmp_asl, data); /* read_func: wordread(addr), byteread(data) */ \
  data <<= 1; \
  flag_c = (data >> 8) & 0x1; \
  data &= 0xff; \
  write_func(addr_tmp_asl, data); \
  SET_NZ_FLAGS(data); \
  ADD_CYCLES(cycles); \
}

#define ASL_A() \
{ \
  uint32 A_tmp = REG_A << 1; \
  flag_c = (A_tmp >> 8) & 0x1; \
  REG_A = (A_tmp) & 0xff; \
  SET_NZ_FLAGS(REG_A); \
  ADD_CYCLES(2); \
}

/* undocumented */
#define ASR(cycles, read_func) \
{ \
  read_func(data); /* read_func == byteread */ \
  data &= REG_A; \
  flag_c = data & 0x1; \
  REG_A = data >> 1; \
  SET_NZ_FLAGS(REG_A); \
  ADD_CYCLES(cycles); \
}

#define BCC() \
{ \
  RELATIVE_BRANCH(!(flag_c)); \
}

#define BCS() \
{ \
  RELATIVE_BRANCH(flag_c); \
}

#define BEQ() \
{ \
  RELATIVE_BRANCH(!(flag_z)); \
}

#define BIT(cycles, read_func) \
{ \
  read_func(data); /* read_func == byteread */ \
  flag_z = data & REG_A; \
  /* move bit 7/6 of data into N/V flags */ \
  flag_n = data; \
  flag_v = data & V_FLAG; \
  ADD_CYCLES(cycles); \
}

#define BMI() \
{ \
  RELATIVE_BRANCH(flag_n & N_FLAG); \
}

#define BNE() \
{ \
  RELATIVE_BRANCH(flag_z); \
}

#define BPL() \
{ \
  RELATIVE_BRANCH(!(flag_n & N_FLAG)); \
}

/* Software interrupt type thang */
#define BRK() \
{ \
  REG_PC++; \
  PUSH(REG_PC >> 8); \
  PUSH(REG_PC & 0xff); \
  flag_b = B_FLAG; \
  PUSH(COMBINE_FLAGS()); \
  flag_i = I_FLAG; \
  JUMP(IRQ_VECTOR); \
  ADD_CYCLES(7); \
}

#define BVC() \
{ \
  RELATIVE_BRANCH(!(flag_v)); \
}

#define BVS() \
{ \
  RELATIVE_BRANCH(flag_v); \
}

#define CLC() \
{ \
  flag_c = 0; \
  ADD_CYCLES(2); \
}

#define CLD() \
{ \
  flag_d = 0; \
  ADD_CYCLES(2); \
}

#define CLI() \
{ \
  flag_i = 0; \
  ADD_CYCLES(2); \
  if (cpu.int_pending & (remaining_cycles > 0)) \
  { \
    IRQ(); \
    cpu.int_pending = 0; \
  } \
}

#define CLV() \
{ \
  flag_v = 0; \
  ADD_CYCLES(2); \
}

#define _COMPARE(reg, value) \
{ \
  uint32 temp = (reg) - (value); \
  /* C is clear when data > A */ \
  flag_c = (((temp >> 8) & 0x1) ^ 0x1); \
  SET_NZ_FLAGS(temp & 0xff); \
}

#define CMP(cycles, read_func) \
{ \
  read_func(data); /* read_func == byteread */ \
  _COMPARE(REG_A, data); \
  ADD_CYCLES(cycles); \
}

#define CPX(cycles, read_func) \
{ \
  read_func(data); /* read_func == byteread */ \
  _COMPARE(REG_X, data); \
  ADD_CYCLES(cycles); \
}

#define CPY(cycles, read_func) \
{ \
  read_func(data); /* read_func == byteread */ \
  _COMPARE(REG_Y, data); \
  ADD_CYCLES(cycles); \
}

/* undocumented */
#define DCP(cycles, read_func, write_func) \
{ \
  uint32 addr_tmp_dcp; \
  read_func(addr_tmp_dcp, data); /* read_func: wordread(addr), byteread(data) */ \
  data = (data - 1) & 0xff; \
  write_func(addr_tmp_dcp, data); \
  CMP(cycles, EMPTY_READ); \
}

#define DEC(cycles, read_func, write_func) \
{ \
  uint32 addr_tmp_dec; \
  read_func(addr_tmp_dec, data); /* read_func: wordread(addr), byteread(data) */\
  data = (data - 1) & 0xff; \
  write_func(addr_tmp_dec, data); \
  SET_NZ_FLAGS(data); \
  ADD_CYCLES(cycles); \
}

#define DEX() \
{ \
  REG_X = (REG_X - 1) & 0xff; \
  SET_NZ_FLAGS(REG_X); \
  ADD_CYCLES(2); \
}

#define DEY() \
{ \
  REG_Y = (REG_Y - 1) & 0xff; \
  SET_NZ_FLAGS(REG_Y); \
  ADD_CYCLES(2); \
}

/* undocumented (double-NOP) */
#define DOP(cycles) \
{ \
  REG_PC++; \
  ADD_CYCLES(cycles); \
}

#define EOR(cycles, read_func) \
{ \
  read_func(data); /* read_func == byteread */ \
  REG_A ^= data; \
  SET_NZ_FLAGS(REG_A); \
  ADD_CYCLES(cycles); \
}

#define INC(cycles, read_func, write_func) \
{ \
  uint32 addr_tmp_inc; \
  read_func(addr_tmp_inc, data); /* read_func: wordread(addr), byteread(data) */ \
  data = (data + 1) & 0xff; \
  write_func(addr_tmp_inc, data); \
  SET_NZ_FLAGS(data); \
  ADD_CYCLES(cycles); \
}

#define INX() \
{ \
  REG_X = (REG_X + 1) & 0xff; \
  SET_NZ_FLAGS(REG_X); \
  ADD_CYCLES(2); \
}

#define INY() \
{ \
  REG_Y = (REG_Y + 1) & 0xff; \
  SET_NZ_FLAGS(REG_Y); \
  ADD_CYCLES(2); \
}

/* undocumented */
#define ISB(cycles, read_func, write_func) \
{ \
  uint32 addr_tmp_isb; \
  read_func(addr_tmp_isb, data); /* read_func: wordread(addr), byteread(data) */\
  data = (data + 1) & 0xff; \
  write_func(addr_tmp_isb, data); \
  SBC(cycles, EMPTY_READ); \
}

#ifdef NES6502_TESTOPS
#define JAM() \
{ \
   cpu_Jam(); \
}
#else /* !NES6502_TESTOPS */
#define JAM() \
{ \
   REG_PC--; \
   cpu.jammed = TRUE; \
   cpu.int_pending = 0; \
   ADD_CYCLES(2); \
}
#endif /* !NES6502_TESTOPS */

#define JMP_INDIRECT() \
{ \
  uint32 temp = bank_readword(REG_PC); \
  /* bug in crossing page boundaries */ \
  if (0xff == (temp & 0xff)) \
    REG_PC = (bank_readbyte(temp & 0xff00) << 8) | bank_readbyte(temp); \
  else \
    JUMP(temp); \
  ADD_CYCLES(5); \
}

#define JMP_ABSOLUTE() \
{ \
  JUMP(REG_PC); \
  ADD_CYCLES(3); \
}

#define JSR() \
{ \
  REG_PC++; \
  PUSH(REG_PC >> 8); \
  PUSH(REG_PC & 0xff); \
  JUMP(REG_PC - 1); \
  ADD_CYCLES(6); \
}

/* undocumented */
#define LAS(cycles, read_func) \
{ \
  read_func(data); /* read_func == byteread */ \
  REG_A = REG_X = REG_S = (REG_S & data); \
  SET_NZ_FLAGS(REG_A); \
  ADD_CYCLES(cycles); \
}

/* undocumented */
#define LAX(cycles, read_func) \
{ \
  read_func(REG_A); /* read_func == byteread */ \
  REG_X = REG_A; \
  SET_NZ_FLAGS(REG_A); \
  ADD_CYCLES(cycles); \
}

#define LDA(cycles, read_func) \
{ \
  read_func(REG_A); /* read_func == byteread */ \
  SET_NZ_FLAGS(REG_A); \
  ADD_CYCLES(cycles); \
}

#define LDX(cycles, read_func) \
{ \
  read_func(REG_X); /* read_func == byteread */ \
  SET_NZ_FLAGS(REG_X);\
  ADD_CYCLES(cycles); \
}

#define LDY(cycles, read_func) \
{ \
  read_func(REG_Y); /* read_func == byteread */ \
  SET_NZ_FLAGS(REG_Y);\
  ADD_CYCLES(cycles); \
}

#define LSR(cycles, read_func, write_func) \
{ \
  uint32 addr_tmp_lsr; \
  read_func(addr_tmp_lsr, data); /* read_func: wordread(addr), byteread(data) */\
  flag_c = data & 0x1; \
  data >>= 1; \
  write_func(addr_tmp_lsr, data); \
  SET_NZ_FLAGS(data); \
  ADD_CYCLES(cycles); \
}

#define LSR_A() \
{ \
  flag_c = REG_A & 0x1; \
  REG_A >>= 1; \
  SET_NZ_FLAGS(REG_A); \
  ADD_CYCLES(2); \
}

/* undocumented */
#define LXA(cycles, read_func) \
{ \
  read_func(data); \
  REG_A = REG_X = ((REG_A | 0xee) & data); \
  SET_NZ_FLAGS(REG_A); \
  ADD_CYCLES(cycles); \
}

#define NOP() \
{ \
   ADD_CYCLES(2); \
}

#define ORA(cycles, read_func) \
{ \
  read_func(data); /* read_func == byteread */ \
  REG_A |= data; \
  SET_NZ_FLAGS(REG_A);\
  ADD_CYCLES(cycles); \
}

#define PHA() \
{ \
  PUSH(REG_A); \
  ADD_CYCLES(3); \
}

#define PHP() \
{ \
  /* B flag is pushed on stack as well */ \
  PUSH(COMBINE_FLAGS() | B_FLAG); \
  ADD_CYCLES(3); \
}

#define PLA() \
{ \
  REG_A = PULL(); \
  SET_NZ_FLAGS(REG_A); \
  ADD_CYCLES(4); \
}

#define PLP() \
{ \
  uint32 temp; \
  temp = PULL(); \
  SCATTER_FLAGS(temp); \
  ADD_CYCLES(4); \
}

/* undocumented */
#define RLA(cycles, read_func, write_func) \
{ \
  uint32 addr_tmp_rla; \
  read_func(addr_tmp_rla, data); /* read_func: wordread(addr), byteread(data) */\
  { \
    uint32 data_tmp = data << 1; \
    data = (data_tmp | flag_c) & 0xff; \
    flag_c = (data_tmp >> 8) & 0x1; \
  } \
  write_func(addr_tmp_rla, data); \
  REG_A &= data; \
  SET_NZ_FLAGS(REG_A); \
  ADD_CYCLES(cycles); \
}

/* 9-bit rotation (carry flag used for rollover) */
#define ROL(cycles, read_func, write_func) \
{ \
  uint32 addr_tmp_rol; \
  read_func(addr_tmp_rol, data); /* read_func: wordread(addr), byteread(data) */ \
  { \
    uint32 data_tmp = data << 1; \
    data = (data_tmp | flag_c) & 0xff; \
    flag_c = (data_tmp >> 8) & 0x1; \
  } \
  write_func(addr_tmp_rol, data); \
  SET_NZ_FLAGS(data); \
  ADD_CYCLES(cycles); \
}

#define ROL_A() \
{ \
  { \
    uint32 A_tmp = REG_A << 1; \
    REG_A = (A_tmp | flag_c) & 0xff; \
    flag_c = (A_tmp >> 8) & 0x1; \
  } \
  SET_NZ_FLAGS(REG_A); \
  ADD_CYCLES(2); \
}

#define ROR(cycles, read_func, write_func) \
{ \
  uint32 addr_tmp_ror; \
  read_func(addr_tmp_ror, data); /* read_func: wordread(addr), byteread(data) */ \
  { \
    uint8 c_tmp = flag_c; \
    flag_c = data & 0x1; \
    data = (data >> 1) | (c_tmp << 7); \
  } \
  write_func(addr_tmp_ror, data); \
  SET_NZ_FLAGS(data); \
  ADD_CYCLES(cycles); \
}

#define ROR_A() \
{ \
  { \
    uint8 c_tmp = flag_c; \
    flag_c = REG_A & 0x1; \
    REG_A = (REG_A >> 1) | (c_tmp << 7); \
  } \
  SET_NZ_FLAGS(REG_A); \
  ADD_CYCLES(2); \
}

/* undocumented */
#define RRA(cycles, read_func, write_func) \
{ \
  uint32 addr_tmp_rra; \
  read_func(addr_tmp_rra, data); /* read_func: wordread(addr), byteread(data) */ \
  { \
    uint8 c_tmp = flag_c; \
    flag_c = data & 0x1; \
    data = (data >> 1) | (c_tmp << 7); \
  } \
  write_func(addr_tmp_rra, data); \
  ADC(cycles, EMPTY_READ); \
}

#define RTI() \
{ \
  uint32 temp; \
  temp = PULL(); \
  SCATTER_FLAGS(temp); \
  REG_PC = PULL(); \
  REG_PC |= PULL() << 8; \
  ADD_CYCLES(6); \
  if (((flag_i >> 2) ^ 0x1) & cpu.int_pending & (remaining_cycles > 0)) \
  { \
    cpu.int_pending = 0; \
    IRQ(); \
  } \
}

#define RTS() \
{ \
  REG_PC = PULL(); \
  REG_PC = (REG_PC | (PULL() << 8)) + 1; \
  ADD_CYCLES(6); \
}

/* undocumented */
#define SAX(cycles, read_func, write_func) \
{ \
  uint32 addr_tmp_sax; \
  read_func(addr_tmp_sax); \
  data = REG_A & REG_X; \
  write_func(addr_tmp_sax, data); \
  ADD_CYCLES(cycles); \
}


#define SBC(cycles, read_func) \
{ \
  read_func(data); /* read_func == byteread */ \
  { \
    uint32 temp = REG_A - data - (flag_c ^ 0x1); \
    flag_v = ((REG_A ^ data) & (REG_A ^ temp) & 0x80) >> 1; \
    flag_c = (temp < 0x100); \
    REG_A = temp & 0xff; \
  } \
  SET_NZ_FLAGS(REG_A); \
  ADD_CYCLES(cycles); \
}

/* undocumented */
#define SBX(cycles, read_func) \
{ \
  read_func(data); \
  { \
    uint32 temp = (REG_A & REG_X) - data; \
    flag_c = (temp < 0x100); \
    REG_X = temp & 0xff; \
  } \
  SET_NZ_FLAGS(REG_X); \
  ADD_CYCLES(cycles); \
}

#define SEC() \
{ \
  flag_c = 1; \
  ADD_CYCLES(2); \
}

#define SED() \
{ \
  flag_d = D_FLAG; \
  ADD_CYCLES(2); \
}

#define SEI() \
{ \
  flag_i = I_FLAG; \
  ADD_CYCLES(2); \
}

/* undocumented */
#define SHA(cycles, read_func, write_func) \
{ \
  uint32 addr_tmp_sha; \
  read_func(addr_tmp_sha); \
  data = REG_A & REG_X & ((addr_tmp_sha >> 8) + 1); \
  write_func(addr_tmp_sha, data); \
  ADD_CYCLES(cycles); \
}

/* undocumented */
#define SHS(cycles, read_func, write_func) \
{ \
  uint32 addr_tmp_shs; \
  read_func(addr_tmp_shs); \
  REG_S = REG_A & REG_X; \
  data = REG_S & ((addr_tmp_shs >> 8) + 1); \
  write_func(addr_tmp_shs, data); \
  ADD_CYCLES(cycles); \
}

/* undocumented */
#define SHX(cycles, read_func, write_func) \
{ \
  uint32 addr_tmp_shx; \
  read_func(addr_tmp_shx); \
  data = REG_X & ((addr_tmp_shx >> 8) + 1); \
  write_func(addr_tmp_shx, data); \
  ADD_CYCLES(cycles); \
}

/* undocumented */
#define SHY(cycles, read_func, write_func) \
{ \
  uint32 addr_tmp_shy; \
  read_func(addr_tmp_shy); \
  data = REG_Y & ((addr_tmp_shy >> 8 ) + 1); \
  write_func(addr_tmp_shy, data); \
  ADD_CYCLES(cycles); \
}

/* undocumented */
#define SLO(cycles, read_func, write_func) \
{ \
  uint32 addr_tmp_slo; \
  read_func(addr_tmp_slo, data); /* read_func: wordread(addr), byteread(data) */ \
  { \
    uint32 data_tmp = data << 1; \
    flag_c = (data_tmp >> 8) & 0x1; \
    data = data_tmp & 0xff; \
  } \
  write_func(addr_tmp_slo, data); \
  REG_A |= data; \
  SET_NZ_FLAGS(REG_A); \
  ADD_CYCLES(cycles); \
}

/* undocumented */
#define SRE(cycles, read_func, write_func) \
{ \
  uint32 addr_tmp_sre; \
  read_func(addr_tmp_sre, data); /* read_func: wordread(addr), byteread(data) */ \
  flag_c = data & 0x1; \
  data >>= 1; \
  write_func(addr_tmp_sre, data); \
  REG_A ^= data; \
  SET_NZ_FLAGS(REG_A); \
  ADD_CYCLES(cycles); \
}

#define STA(cycles, read_func, write_func) \
{ \
  uint32 addr_tmp_sta; \
  read_func(addr_tmp_sta); /* read_func == byteread */ \
  write_func(addr_tmp_sta, REG_A); \
  ADD_CYCLES(cycles); \
}

#define STX(cycles, read_func, write_func) \
{ \
  uint32 addr_tmp_stx; \
  read_func(addr_tmp_stx); /* read_func == byteread */ \
  write_func(addr_tmp_stx, REG_X); \
  ADD_CYCLES(cycles); \
}

#define STY(cycles, read_func, write_func) \
{ \
  uint32 addr_tmp_sty; \
  read_func(addr_tmp_sty); /* read_func == byteread */ \
  write_func(addr_tmp_sty, REG_Y); \
  ADD_CYCLES(cycles); \
}

#define TAX() \
{ \
  REG_X = REG_A; \
  SET_NZ_FLAGS(REG_X);\
  ADD_CYCLES(2); \
}

#define TAY() \
{ \
  REG_Y = REG_A; \
  SET_NZ_FLAGS(REG_Y);\
  ADD_CYCLES(2); \
}

/* undocumented (triple-NOP) */
#define TOP() \
{ \
  REG_PC += 2; \
  ADD_CYCLES(4); \
}

#define TSX() \
{ \
  REG_X = REG_S; \
  SET_NZ_FLAGS(REG_X);\
  ADD_CYCLES(2); \
}

#define TXA() \
{ \
  REG_A = REG_X; \
  SET_NZ_FLAGS(REG_A);\
  ADD_CYCLES(2); \
}

#define TXS() \
{ \
  REG_S = REG_X; \
  ADD_CYCLES(2); \
}

#define TYA() \
{ \
  REG_A = REG_Y; \
  SET_NZ_FLAGS(REG_A); \
  ADD_CYCLES(2); \
}



/* internal CPU context */
static nes6502_context cpu;

/* memory region pointers */
static uint8 *ram = NULL, *stack = NULL;

/*
** Zero-page helper macros
*/

#define  ZP_READBYTE(addr)          ram[(addr)]
#define  ZP_WRITEBYTE(addr, value)  ram[(addr)] = (value)

INLINE uint32 zp_readword(register uint8 address)
{
#ifdef HOST_LITTLE_ENDIAN
  return (ram[address] | (ram[address + 1] << 8));
#else
#ifdef TARGET_CPU_PPC
  return __lhbrx(ram, address);
#else
  uint32 x = (uint32) *(uint16 *)(ram + address);
  return (x << 8) | (x >> 8);
#endif /* TARGET_CPU_PPC */
#endif /* HOST_LITTLE_ENDIAN */
}

INLINE uint8 bank_readbyte(register uint32 address)
{
  return cpu.mem_page[address >> NES6502_BANKSHIFT][address & NES6502_BANKMASK];
}

INLINE uint32 bank_readword(register uint32 address)
{
#ifdef HOST_LITTLE_ENDIAN
  /* TODO: this fails if src address is $xFFF */
  uint8 *p = cpu.mem_page[address >> NES6502_BANKSHIFT] + (address & NES6502_BANKMASK);
  uint8 x1 = *p++;
  uint8 x2 = *p;
  return x1 | (x2 << 8);
#else
#ifdef TARGET_CPU_PPC
  return __lhbrx(cpu.mem_page[address >> NES6502_BANKSHIFT], address & NES6502_BANKMASK);
#else
  uint32 x = (uint32) *(uint16 *)(cpu.mem_page[address >> NES6502_BANKSHIFT] + (address & NES6502_BANKMASK));
  return (x << 8) | (x >> 8);
#endif /* TARGET_CPU_PPC */
#endif /* HOST_LITTLE_ENDIAN */
}

INLINE void bank_writebyte(register uint32 address, register uint8 value)
{
   cpu.mem_page[address >> NES6502_BANKSHIFT][address & NES6502_BANKMASK] = value;
}

/* read a byte of 6502 memory */
/* note: address <= 0xffff */
static INLINE uint8 mem_readbyte(uint32 address)
{
  /* TODO: following cases are N2A03-specific */
  /* RAM */
  if (address < 0x800)
    return ram[address];
  /* always paged memory */
  else if (address >= 0x8000)
    return bank_readbyte(address);
  else
    return cpu.read_handler (address);
}

/* write a byte of data to 6502 memory */
/* note: address <= 0xffff */
static INLINE void mem_writebyte(uint32 address, uint8 value)
{
  /* RAM */
  if (address < 0x800)
    ram[address] = value;
  else
    cpu.write_handler (address, value);
}

nes6502_context *
nes6502_get_current_context ()
{
  return &cpu;
}


void 
nes6502_set_zeropage()
{
  ram = cpu.mem_page[0];
  stack = ram + STACK_OFFSET;
}


void
nes6502_setcontext()
{
  cpu.jammed = FALSE;
}


/* DMA a byte of data from ROM */
uint8 nes6502_getbyte(uint32 address)
{
   return bank_readbyte(address);
}


/* get number of elapsed cycles */
uint32 nes6502_getcycles()
{
  return cpu.total_cycles;
}


#define  GET_GLOBAL_REGS() \
{ \
   REG_PC = cpu.pc_reg; \
   REG_A = cpu.a_reg; \
   REG_X = cpu.x_reg; \
   REG_Y = cpu.y_reg; \
   SCATTER_FLAGS(cpu.p_reg); \
   REG_S = cpu.s_reg; \
}


#define  STORE_LOCAL_REGS() \
{ \
   cpu.pc_reg = REG_PC; \
   cpu.a_reg = REG_A; \
   cpu.x_reg = REG_X; \
   cpu.y_reg = REG_Y; \
   cpu.p_reg = COMBINE_FLAGS(); \
   cpu.s_reg = REG_S; \
}

#define  MIN(a,b)    (((a) < (b)) ? (a) : (b))

/* Execute instructions until count expires
**
** Returns the number of cycles *actually* executed, which will be
** anywhere from remaining_cycles to remaining_cycles + 6
*/

int nes6502_execute(int remaining_cycles)
{
  int old_cycles = cpu.total_cycles;
  
  uint32 data;
  
  uint32 REG_PC;
  uint32 REG_A;
  uint32 REG_X;
  uint32 REG_Y;
  uint32 REG_S;
  
  uint32 flag_n;
  uint32 flag_v;
  uint32 flag_b;
  uint32 flag_d;
  uint32 flag_i;
  uint32 flag_z;
  uint32 flag_c;
  
#ifdef NES6502_JUMPTABLE
  
#define  OPCODE_BEGIN(xx)  op##xx:
#define  OPCODE_END \
  if (remaining_cycles <= 0) \
    goto end_execute; \
  goto *opcode_table[bank_readbyte(REG_PC++)];
  
  static const void *opcode_table[256] = {
    &&op00, &&op01, &&op02, &&op03, &&op04, &&op05, &&op06, &&op07,
    &&op08, &&op09, &&op0A, &&op0B, &&op0C, &&op0D, &&op0E, &&op0F,
    &&op10, &&op11, &&op12, &&op13, &&op14, &&op15, &&op16, &&op17,
    &&op18, &&op19, &&op1A, &&op1B, &&op1C, &&op1D, &&op1E, &&op1F,
    &&op20, &&op21, &&op22, &&op23, &&op24, &&op25, &&op26, &&op27,
    &&op28, &&op29, &&op2A, &&op2B, &&op2C, &&op2D, &&op2E, &&op2F,
    &&op30, &&op31, &&op32, &&op33, &&op34, &&op35, &&op36, &&op37,
    &&op38, &&op39, &&op3A, &&op3B, &&op3C, &&op3D, &&op3E, &&op3F,
    &&op40, &&op41, &&op42, &&op43, &&op44, &&op45, &&op46, &&op47,
    &&op48, &&op49, &&op4A, &&op4B, &&op4C, &&op4D, &&op4E, &&op4F,
    &&op50, &&op51, &&op52, &&op53, &&op54, &&op55, &&op56, &&op57,
    &&op58, &&op59, &&op5A, &&op5B, &&op5C, &&op5D, &&op5E, &&op5F,
    &&op60, &&op61, &&op62, &&op63, &&op64, &&op65, &&op66, &&op67,
    &&op68, &&op69, &&op6A, &&op6B, &&op6C, &&op6D, &&op6E, &&op6F,
    &&op70, &&op71, &&op72, &&op73, &&op74, &&op75, &&op76, &&op77,
    &&op78, &&op79, &&op7A, &&op7B, &&op7C, &&op7D, &&op7E, &&op7F,
    &&op80, &&op81, &&op82, &&op83, &&op84, &&op85, &&op86, &&op87,
    &&op88, &&op89, &&op8A, &&op8B, &&op8C, &&op8D, &&op8E, &&op8F,
    &&op90, &&op91, &&op92, &&op93, &&op94, &&op95, &&op96, &&op97,
    &&op98, &&op99, &&op9A, &&op9B, &&op9C, &&op9D, &&op9E, &&op9F,
    &&opA0, &&opA1, &&opA2, &&opA3, &&opA4, &&opA5, &&opA6, &&opA7,
    &&opA8, &&opA9, &&opAA, &&opAB, &&opAC, &&opAD, &&opAE, &&opAF,
    &&opB0, &&opB1, &&opB2, &&opB3, &&opB4, &&opB5, &&opB6, &&opB7,
    &&opB8, &&opB9, &&opBA, &&opBB, &&opBC, &&opBD, &&opBE, &&opBF,
    &&opC0, &&opC1, &&opC2, &&opC3, &&opC4, &&opC5, &&opC6, &&opC7,
    &&opC8, &&opC9, &&opCA, &&opCB, &&opCC, &&opCD, &&opCE, &&opCF,
    &&opD0, &&opD1, &&opD2, &&opD3, &&opD4, &&opD5, &&opD6, &&opD7,
    &&opD8, &&opD9, &&opDA, &&opDB, &&opDC, &&opDD, &&opDE, &&opDF,
    &&opE0, &&opE1, &&opE2, &&opE3, &&opE4, &&opE5, &&opE6, &&opE7,
    &&opE8, &&opE9, &&opEA, &&opEB, &&opEC, &&opED, &&opEE, &&opEF,
    &&opF0, &&opF1, &&opF2, &&opF3, &&opF4, &&opF5, &&opF6, &&opF7,
    &&opF8, &&opF9, &&opFA, &&opFB, &&opFC, &&opFD, &&opFE, &&opFF
  };
  
#else /* !NES6502_JUMPTABLE */
#define  OPCODE_BEGIN(xx)  case 0x##xx:
#define  OPCODE_END        break;
#endif /* !NES6502_JUMPTABLE */
  
  GET_GLOBAL_REGS();
  
  if (cpu.int_pending & (remaining_cycles > 0))
  {
    if (0 == flag_i)
    {
      cpu.int_pending = 0;
      IRQ();
    }
  }
  
  /* check for DMA cycle burning */
  if ((cpu.burn_cycles > 0) & (remaining_cycles > 0))
  {
    int burn_for;

    burn_for = MIN(remaining_cycles, cpu.burn_cycles);
    ADD_CYCLES(burn_for);
    cpu.burn_cycles -= burn_for;
  }

#ifdef NES6502_JUMPTABLE
  /* fetch first instruction */
  OPCODE_END
  
#else /* !NES6502_JUMPTABLE */

  /* Continue until we run out of cycles */
  while (remaining_cycles > 0)
  {
#ifdef NES6502_DISASM
    log_printf(nes6502_disasm(REG_PC, COMBINE_FLAGS(), A, X, Y, S));
#endif /* NES6502_DISASM */
    
    /* Fetch and execute instruction */
    switch (bank_readbyte(REG_PC++))
    {
#endif /* !NES6502_JUMPTABLE */

      OPCODE_BEGIN(00)  /* BRK */
      BRK();
      OPCODE_END
      
      OPCODE_BEGIN(01)  /* ORA ($nn,X) */
      ORA(6, INDIR_X_BYTE);
      OPCODE_END
      
      OPCODE_BEGIN(02)  /* JAM */
      OPCODE_BEGIN(12)  /* JAM */
      OPCODE_BEGIN(22)  /* JAM */
      OPCODE_BEGIN(32)  /* JAM */
      OPCODE_BEGIN(42)  /* JAM */
      OPCODE_BEGIN(52)  /* JAM */
      OPCODE_BEGIN(62)  /* JAM */
      OPCODE_BEGIN(72)  /* JAM */
      OPCODE_BEGIN(92)  /* JAM */
      OPCODE_BEGIN(B2)  /* JAM */
      OPCODE_BEGIN(D2)  /* JAM */
      OPCODE_BEGIN(F2)  /* JAM */
         JAM();
         /* kill the CPU */
         remaining_cycles = 0;
         OPCODE_END

      OPCODE_BEGIN(03)  /* SLO ($nn,X) */
         SLO(8, INDIR_X, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(04)  /* NOP $nn */
      OPCODE_BEGIN(44)  /* NOP $nn */
      OPCODE_BEGIN(64)  /* NOP $nn */
         DOP(3);
         OPCODE_END

      OPCODE_BEGIN(05)  /* ORA $nn */
         ORA(3, ZERO_PAGE_BYTE); 
         OPCODE_END

      OPCODE_BEGIN(06)  /* ASL $nn */
         ASL(5, ZERO_PAGE, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(07)  /* SLO $nn */
         SLO(5, ZERO_PAGE, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(08)  /* PHP */
         PHP(); 
         OPCODE_END

      OPCODE_BEGIN(09)  /* ORA #$nn */
         ORA(2, IMMEDIATE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(0A)  /* ASL A */
         ASL_A();
         OPCODE_END

      OPCODE_BEGIN(0B)  /* ANC #$nn */
      OPCODE_BEGIN(2B)  /* ANC #$nn */
         ANC(2, IMMEDIATE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(0C)  /* NOP $nnnn */
      OPCODE_BEGIN(1C)  /* NOP $nnnn,X */
      OPCODE_BEGIN(3C)  /* NOP $nnnn,X */
      OPCODE_BEGIN(5C)  /* NOP $nnnn,X */
      OPCODE_BEGIN(7C)  /* NOP $nnnn,X */
      OPCODE_BEGIN(DC)  /* NOP $nnnn,X */
      OPCODE_BEGIN(FC)  /* NOP $nnnn,X */
         TOP(); 
         OPCODE_END

      OPCODE_BEGIN(0D)  /* ORA $nnnn */
         ORA(4, ABSOLUTE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(0E)  /* ASL $nnnn */
         ASL(6, ABSOLUTE, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(0F)  /* SLO $nnnn */
         SLO(6, ABSOLUTE, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(10)  /* BPL $nnnn */
         BPL();
         OPCODE_END

      OPCODE_BEGIN(11)  /* ORA ($nn),Y */
         ORA(5, INDIR_Y_BYTE);
         OPCODE_END
      
      OPCODE_BEGIN(13)  /* SLO ($nn),Y */
         SLO(8, INDIR_Y, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(14)  /* NOP $nn,X */
      OPCODE_BEGIN(34)  /* NOP */
      OPCODE_BEGIN(54)  /* NOP $nn,X */
      OPCODE_BEGIN(74)  /* NOP $nn,X */
      OPCODE_BEGIN(D4)  /* NOP $nn,X */
      OPCODE_BEGIN(F4)  /* NOP ($nn,X) */
         DOP(4);
         OPCODE_END

      OPCODE_BEGIN(15)  /* ORA $nn,X */
         ORA(4, ZP_IND_X_BYTE);
         OPCODE_END

      OPCODE_BEGIN(16)  /* ASL $nn,X */
         ASL(6, ZP_IND_X, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(17)  /* SLO $nn,X */
         SLO(6, ZP_IND_X, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(18)  /* CLC */
         CLC();
         OPCODE_END

      OPCODE_BEGIN(19)  /* ORA $nnnn,Y */
         ORA(4, ABS_IND_Y_BYTE);
         OPCODE_END
      
      OPCODE_BEGIN(1A)  /* NOP */
      OPCODE_BEGIN(3A)  /* NOP */
      OPCODE_BEGIN(5A)  /* NOP */
      OPCODE_BEGIN(7A)  /* NOP */
      OPCODE_BEGIN(DA)  /* NOP */
      OPCODE_BEGIN(EA)  /* NOP */
      OPCODE_BEGIN(FA)  /* NOP */
         NOP();
         OPCODE_END

      OPCODE_BEGIN(1B)  /* SLO $nnnn,Y */
         SLO(7, ABS_IND_Y, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(1D)  /* ORA $nnnn,X */
         ORA(4, ABS_IND_X_BYTE);
         OPCODE_END

      OPCODE_BEGIN(1E)  /* ASL $nnnn,X */
         ASL(7, ABS_IND_X, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(1F)  /* SLO $nnnn,X */
         SLO(7, ABS_IND_X, mem_writebyte);
         OPCODE_END
      
      OPCODE_BEGIN(20)  /* JSR $nnnn */
         JSR();
         OPCODE_END

      OPCODE_BEGIN(21)  /* AND ($nn,X) */
         AND(6, INDIR_X_BYTE);
         OPCODE_END

      OPCODE_BEGIN(23)  /* RLA ($nn,X) */
         RLA(8, INDIR_X, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(24)  /* BIT $nn */
         BIT(3, ZERO_PAGE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(25)  /* AND $nn */
         AND(3, ZERO_PAGE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(26)  /* ROL $nn */
         ROL(5, ZERO_PAGE, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(27)  /* RLA $nn */
         RLA(5, ZERO_PAGE, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(28)  /* PLP */
         PLP();
         OPCODE_END

      OPCODE_BEGIN(29)  /* AND #$nn */
         AND(2, IMMEDIATE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(2A)  /* ROL A */
         ROL_A();
         OPCODE_END

      OPCODE_BEGIN(2C)  /* BIT $nnnn */
         BIT(4, ABSOLUTE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(2D)  /* AND $nnnn */
         AND(4, ABSOLUTE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(2E)  /* ROL $nnnn */
         ROL(6, ABSOLUTE, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(2F)  /* RLA $nnnn */
         RLA(6, ABSOLUTE, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(30)  /* BMI $nnnn */
         BMI();
         OPCODE_END

      OPCODE_BEGIN(31)  /* AND ($nn),Y */
         AND(5, INDIR_Y_BYTE);
         OPCODE_END

      OPCODE_BEGIN(33)  /* RLA ($nn),Y */
         RLA(8, INDIR_Y, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(35)  /* AND $nn,X */
         AND(4, ZP_IND_X_BYTE);
         OPCODE_END

      OPCODE_BEGIN(36)  /* ROL $nn,X */
         ROL(6, ZP_IND_X, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(37)  /* RLA $nn,X */
         RLA(6, ZP_IND_X, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(38)  /* SEC */
         SEC();
         OPCODE_END

      OPCODE_BEGIN(39)  /* AND $nnnn,Y */
         AND(4, ABS_IND_Y_BYTE);
         OPCODE_END

      OPCODE_BEGIN(3B)  /* RLA $nnnn,Y */
         RLA(7, ABS_IND_Y, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(3D)  /* AND $nnnn,X */
         AND(4, ABS_IND_X_BYTE);
         OPCODE_END

      OPCODE_BEGIN(3E)  /* ROL $nnnn,X */
         ROL(7, ABS_IND_X, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(3F)  /* RLA $nnnn,X */
         RLA(7, ABS_IND_X, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(40)  /* RTI */
         RTI();
         OPCODE_END

      OPCODE_BEGIN(41)  /* EOR ($nn,X) */
         EOR(6, INDIR_X_BYTE);
         OPCODE_END

      OPCODE_BEGIN(43)  /* SRE ($nn,X) */
         SRE(8, INDIR_X, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(45)  /* EOR $nn */
         EOR(3, ZERO_PAGE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(46)  /* LSR $nn */
         LSR(5, ZERO_PAGE, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(47)  /* SRE $nn */
         SRE(5, ZERO_PAGE, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(48)  /* PHA */
         PHA();
         OPCODE_END

      OPCODE_BEGIN(49)  /* EOR #$nn */
         EOR(2, IMMEDIATE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(4A)  /* LSR A */
         LSR_A();
         OPCODE_END

      OPCODE_BEGIN(4B)  /* ASR #$nn */
         ASR(2, IMMEDIATE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(4C)  /* JMP $nnnn */
         JMP_ABSOLUTE();
         OPCODE_END

      OPCODE_BEGIN(4D)  /* EOR $nnnn */
         EOR(4, ABSOLUTE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(4E)  /* LSR $nnnn */
         LSR(6, ABSOLUTE, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(4F)  /* SRE $nnnn */
         SRE(6, ABSOLUTE, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(50)  /* BVC $nnnn */
         BVC();
         OPCODE_END

      OPCODE_BEGIN(51)  /* EOR ($nn),Y */
         EOR(5, INDIR_Y_BYTE);
         OPCODE_END

      OPCODE_BEGIN(53)  /* SRE ($nn),Y */
         SRE(8, INDIR_Y, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(55)  /* EOR $nn,X */
         EOR(4, ZP_IND_X_BYTE);
         OPCODE_END

      OPCODE_BEGIN(56)  /* LSR $nn,X */
         LSR(6, ZP_IND_X, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(57)  /* SRE $nn,X */
         SRE(6, ZP_IND_X, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(58)  /* CLI */
         CLI();
         OPCODE_END

      OPCODE_BEGIN(59)  /* EOR $nnnn,Y */
         EOR(4, ABS_IND_Y_BYTE);
         OPCODE_END

      OPCODE_BEGIN(5B)  /* SRE $nnnn,Y */
         SRE(7, ABS_IND_Y, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(5D)  /* EOR $nnnn,X */
         EOR(4, ABS_IND_X_BYTE);
         OPCODE_END

      OPCODE_BEGIN(5E)  /* LSR $nnnn,X */
         LSR(7, ABS_IND_X, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(5F)  /* SRE $nnnn,X */
         SRE(7, ABS_IND_X, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(60)  /* RTS */
         RTS();
         OPCODE_END

      OPCODE_BEGIN(61)  /* ADC ($nn,X) */
         ADC(6, INDIR_X_BYTE);
         OPCODE_END

      OPCODE_BEGIN(63)  /* RRA ($nn,X) */
         RRA(8, INDIR_X, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(65)  /* ADC $nn */
         ADC(3, ZERO_PAGE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(66)  /* ROR $nn */
         ROR(5, ZERO_PAGE, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(67)  /* RRA $nn */
         RRA(5, ZERO_PAGE, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(68)  /* PLA */
         PLA();
         OPCODE_END

      OPCODE_BEGIN(69)  /* ADC #$nn */
         ADC(2, IMMEDIATE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(6A)  /* ROR A */
         ROR_A();
         OPCODE_END

      OPCODE_BEGIN(6B)  /* ARR #$nn */
         ARR(2, IMMEDIATE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(6C)  /* JMP ($nnnn) */
         JMP_INDIRECT();
         OPCODE_END

      OPCODE_BEGIN(6D)  /* ADC $nnnn */
         ADC(4, ABSOLUTE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(6E)  /* ROR $nnnn */
         ROR(6, ABSOLUTE, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(6F)  /* RRA $nnnn */
         RRA(6, ABSOLUTE, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(70)  /* BVS $nnnn */
         BVS();
         OPCODE_END

      OPCODE_BEGIN(71)  /* ADC ($nn),Y */
         ADC(5, INDIR_Y_BYTE);
         OPCODE_END

      OPCODE_BEGIN(73)  /* RRA ($nn),Y */
         RRA(8, INDIR_Y, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(75)  /* ADC $nn,X */
         ADC(4, ZP_IND_X_BYTE);
         OPCODE_END

      OPCODE_BEGIN(76)  /* ROR $nn,X */
         ROR(6, ZP_IND_X, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(77)  /* RRA $nn,X */
         RRA(6, ZP_IND_X, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(78)  /* SEI */
         SEI();
         OPCODE_END

      OPCODE_BEGIN(79)  /* ADC $nnnn,Y */
         ADC(4, ABS_IND_Y_BYTE);
         OPCODE_END

      OPCODE_BEGIN(7B)  /* RRA $nnnn,Y */
         RRA(7, ABS_IND_Y, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(7D)  /* ADC $nnnn,X */
         ADC(4, ABS_IND_X_BYTE);
         OPCODE_END

      OPCODE_BEGIN(7E)  /* ROR $nnnn,X */
         ROR(7, ABS_IND_X, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(7F)  /* RRA $nnnn,X */
         RRA(7, ABS_IND_X, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(80)  /* NOP #$nn */
      OPCODE_BEGIN(82)  /* NOP #$nn */
      OPCODE_BEGIN(89)  /* NOP #$nn */
      OPCODE_BEGIN(C2)  /* NOP #$nn */
      OPCODE_BEGIN(E2)  /* NOP #$nn */
         DOP(2);
         OPCODE_END

      OPCODE_BEGIN(81)  /* STA ($nn,X) */
         STA(6, INDIR_X_ADDR, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(83)  /* SAX ($nn,X) */
         SAX(6, INDIR_X_ADDR, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(84)  /* STY $nn */
         STY(3, ZERO_PAGE_ADDR, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(85)  /* STA $nn */
         STA(3, ZERO_PAGE_ADDR, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(86)  /* STX $nn */
         STX(3, ZERO_PAGE_ADDR, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(87)  /* SAX $nn */
         SAX(3, ZERO_PAGE_ADDR, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(88)  /* DEY */
         DEY();
         OPCODE_END

      OPCODE_BEGIN(8A)  /* TXA */
         TXA();
         OPCODE_END

      OPCODE_BEGIN(8B)  /* ANE #$nn */
         ANE(2, IMMEDIATE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(8C)  /* STY $nnnn */
         STY(4, ABSOLUTE_ADDR, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(8D)  /* STA $nnnn */
         STA(4, ABSOLUTE_ADDR, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(8E)  /* STX $nnnn */
         STX(4, ABSOLUTE_ADDR, mem_writebyte);
         OPCODE_END
      
      OPCODE_BEGIN(8F)  /* SAX $nnnn */
         SAX(4, ABSOLUTE_ADDR, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(90)  /* BCC $nnnn */
         BCC();
         OPCODE_END

      OPCODE_BEGIN(91)  /* STA ($nn),Y */
         STA(6, INDIR_Y_ADDR, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(93)  /* SHA ($nn),Y */
         SHA(6, INDIR_Y_ADDR, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(94)  /* STY $nn,X */
         STY(4, ZP_IND_X_ADDR, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(95)  /* STA $nn,X */
         STA(4, ZP_IND_X_ADDR, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(96)  /* STX $nn,Y */
         STX(4, ZP_IND_Y_ADDR, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(97)  /* SAX $nn,Y */
         SAX(4, ZP_IND_Y_ADDR, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(98)  /* TYA */
         TYA();
         OPCODE_END

      OPCODE_BEGIN(99)  /* STA $nnnn,Y */
         STA(5, ABS_IND_Y_ADDR, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(9A)  /* TXS */
         TXS();
         OPCODE_END

      OPCODE_BEGIN(9B)  /* SHS $nnnn,Y */
         SHS(5, ABS_IND_Y_ADDR, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(9C)  /* SHY $nnnn,X */
         SHY(5, ABS_IND_X_ADDR, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(9D)  /* STA $nnnn,X */
         STA(5, ABS_IND_X_ADDR, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(9E)  /* SHX $nnnn,Y */
         SHX(5, ABS_IND_Y_ADDR, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(9F)  /* SHA $nnnn,Y */
         SHA(5, ABS_IND_Y_ADDR, mem_writebyte);
         OPCODE_END
      
      OPCODE_BEGIN(A0)  /* LDY #$nn */
         LDY(2, IMMEDIATE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(A1)  /* LDA ($nn,X) */
         LDA(6, INDIR_X_BYTE);
         OPCODE_END

      OPCODE_BEGIN(A2)  /* LDX #$nn */
         LDX(2, IMMEDIATE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(A3)  /* LAX ($nn,X) */
         LAX(6, INDIR_X_BYTE);
         OPCODE_END

      OPCODE_BEGIN(A4)  /* LDY $nn */
         LDY(3, ZERO_PAGE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(A5)  /* LDA $nn */
         LDA(3, ZERO_PAGE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(A6)  /* LDX $nn */
         LDX(3, ZERO_PAGE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(A7)  /* LAX $nn */
         LAX(3, ZERO_PAGE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(A8)  /* TAY */
         TAY();
         OPCODE_END

      OPCODE_BEGIN(A9)  /* LDA #$nn */
         LDA(2, IMMEDIATE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(AA)  /* TAX */
         TAX();
         OPCODE_END

      OPCODE_BEGIN(AB)  /* LXA #$nn */
         LXA(2, IMMEDIATE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(AC)  /* LDY $nnnn */
         LDY(4, ABSOLUTE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(AD)  /* LDA $nnnn */
         LDA(4, ABSOLUTE_BYTE);
         OPCODE_END
      
      OPCODE_BEGIN(AE)  /* LDX $nnnn */
         LDX(4, ABSOLUTE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(AF)  /* LAX $nnnn */
         LAX(4, ABSOLUTE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(B0)  /* BCS $nnnn */
         BCS();
         OPCODE_END

      OPCODE_BEGIN(B1)  /* LDA ($nn),Y */
         LDA(5, INDIR_Y_BYTE);
         OPCODE_END

      OPCODE_BEGIN(B3)  /* LAX ($nn),Y */
         LAX(5, INDIR_Y_BYTE);
         OPCODE_END

      OPCODE_BEGIN(B4)  /* LDY $nn,X */
         LDY(4, ZP_IND_X_BYTE);
         OPCODE_END

      OPCODE_BEGIN(B5)  /* LDA $nn,X */
         LDA(4, ZP_IND_X_BYTE);
         OPCODE_END

      OPCODE_BEGIN(B6)  /* LDX $nn,Y */
         LDX(4, ZP_IND_Y_BYTE);
         OPCODE_END

      OPCODE_BEGIN(B7)  /* LAX $nn,Y */
         LAX(4, ZP_IND_Y_BYTE);
         OPCODE_END

      OPCODE_BEGIN(B8)  /* CLV */
         CLV();
         OPCODE_END

      OPCODE_BEGIN(B9)  /* LDA $nnnn,Y */
         LDA(4, ABS_IND_Y_BYTE);
         OPCODE_END

      OPCODE_BEGIN(BA)  /* TSX */
         TSX();
         OPCODE_END

      OPCODE_BEGIN(BB)  /* LAS $nnnn,Y */
         LAS(4, ABS_IND_Y_BYTE);
         OPCODE_END

      OPCODE_BEGIN(BC)  /* LDY $nnnn,X */
         LDY(4, ABS_IND_X_BYTE);
         OPCODE_END

      OPCODE_BEGIN(BD)  /* LDA $nnnn,X */
         LDA(4, ABS_IND_X_BYTE);
         OPCODE_END

      OPCODE_BEGIN(BE)  /* LDX $nnnn,Y */
         LDX(4, ABS_IND_Y_BYTE);
         OPCODE_END

      OPCODE_BEGIN(BF)  /* LAX $nnnn,Y */
         LAX(4, ABS_IND_Y_BYTE);
         OPCODE_END

      OPCODE_BEGIN(C0)  /* CPY #$nn */
         CPY(2, IMMEDIATE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(C1)  /* CMP ($nn,X) */
         CMP(6, INDIR_X_BYTE);
         OPCODE_END

      OPCODE_BEGIN(C3)  /* DCP ($nn,X) */
         DCP(8, INDIR_X, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(C4)  /* CPY $nn */
         CPY(3, ZERO_PAGE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(C5)  /* CMP $nn */
         CMP(3, ZERO_PAGE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(C6)  /* DEC $nn */
         DEC(5, ZERO_PAGE, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(C7)  /* DCP $nn */
         DCP(5, ZERO_PAGE, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(C8)  /* INY */
         INY();
         OPCODE_END

      OPCODE_BEGIN(C9)  /* CMP #$nn */
         CMP(2, IMMEDIATE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(CA)  /* DEX */
         DEX();
         OPCODE_END

      OPCODE_BEGIN(CB)  /* SBX #$nn */
         SBX(2, IMMEDIATE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(CC)  /* CPY $nnnn */
         CPY(4, ABSOLUTE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(CD)  /* CMP $nnnn */
         CMP(4, ABSOLUTE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(CE)  /* DEC $nnnn */
         DEC(6, ABSOLUTE, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(CF)  /* DCP $nnnn */
         DCP(6, ABSOLUTE, mem_writebyte);
         OPCODE_END
      
      OPCODE_BEGIN(D0)  /* BNE $nnnn */
         BNE();
         OPCODE_END

      OPCODE_BEGIN(D1)  /* CMP ($nn),Y */
         CMP(5, INDIR_Y_BYTE);
         OPCODE_END

      OPCODE_BEGIN(D3)  /* DCP ($nn),Y */
         DCP(8, INDIR_Y, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(D5)  /* CMP $nn,X */
         CMP(4, ZP_IND_X_BYTE);
         OPCODE_END

      OPCODE_BEGIN(D6)  /* DEC $nn,X */
         DEC(6, ZP_IND_X, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(D7)  /* DCP $nn,X */
         DCP(6, ZP_IND_X, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(D8)  /* CLD */
         CLD();
         OPCODE_END

      OPCODE_BEGIN(D9)  /* CMP $nnnn,Y */
         CMP(4, ABS_IND_Y_BYTE);
         OPCODE_END

      OPCODE_BEGIN(DB)  /* DCP $nnnn,Y */
         DCP(7, ABS_IND_Y, mem_writebyte);
         OPCODE_END                  

      OPCODE_BEGIN(DD)  /* CMP $nnnn,X */
         CMP(4, ABS_IND_X_BYTE);
         OPCODE_END

      OPCODE_BEGIN(DE)  /* DEC $nnnn,X */
         DEC(7, ABS_IND_X, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(DF)  /* DCP $nnnn,X */
         DCP(7, ABS_IND_X, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(E0)  /* CPX #$nn */
         CPX(2, IMMEDIATE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(E1)  /* SBC ($nn,X) */
         SBC(6, INDIR_X_BYTE);
         OPCODE_END

      OPCODE_BEGIN(E3)  /* ISB ($nn,X) */
         ISB(8, INDIR_X, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(E4)  /* CPX $nn */
         CPX(3, ZERO_PAGE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(E5)  /* SBC $nn */
         SBC(3, ZERO_PAGE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(E6)  /* INC $nn */
         INC(5, ZERO_PAGE, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(E7)  /* ISB $nn */
         ISB(5, ZERO_PAGE, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(E8)  /* INX */
         INX();
         OPCODE_END

      OPCODE_BEGIN(E9)  /* SBC #$nn */
      OPCODE_BEGIN(EB)  /* USBC #$nn */
         SBC(2, IMMEDIATE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(EC)  /* CPX $nnnn */
         CPX(4, ABSOLUTE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(ED)  /* SBC $nnnn */
         SBC(4, ABSOLUTE_BYTE);
         OPCODE_END

      OPCODE_BEGIN(EE)  /* INC $nnnn */
         INC(6, ABSOLUTE, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(EF)  /* ISB $nnnn */
         ISB(6, ABSOLUTE, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(F0)  /* BEQ $nnnn */
         BEQ();
         OPCODE_END

      OPCODE_BEGIN(F1)  /* SBC ($nn),Y */
         SBC(5, INDIR_Y_BYTE);
         OPCODE_END

      OPCODE_BEGIN(F3)  /* ISB ($nn),Y */
         ISB(8, INDIR_Y, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(F5)  /* SBC $nn,X */
         SBC(4, ZP_IND_X_BYTE);
         OPCODE_END

      OPCODE_BEGIN(F6)  /* INC $nn,X */
         INC(6, ZP_IND_X, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(F7)  /* ISB $nn,X */
         ISB(6, ZP_IND_X, ZP_WRITEBYTE);
         OPCODE_END

      OPCODE_BEGIN(F8)  /* SED */
         SED();
         OPCODE_END

      OPCODE_BEGIN(F9)  /* SBC $nnnn,Y */
         SBC(4, ABS_IND_Y_BYTE);
         OPCODE_END

      OPCODE_BEGIN(FB)  /* ISB $nnnn,Y */
         ISB(7, ABS_IND_Y, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(FD)  /* SBC $nnnn,X */
         SBC(4, ABS_IND_X_BYTE);
         OPCODE_END

      OPCODE_BEGIN(FE)  /* INC $nnnn,X */
         INC(7, ABS_IND_X, mem_writebyte);
         OPCODE_END

      OPCODE_BEGIN(FF)  /* ISB $nnnn,X */
         ISB(7, ABS_IND_X, mem_writebyte);
         OPCODE_END

#ifdef NES6502_JUMPTABLE
end_execute:

#else /* !NES6502_JUMPTABLE */
      }
   }
#endif /* !NES6502_JUMPTABLE */

   /* store local copy of regs */
   STORE_LOCAL_REGS();

   /* Return our actual amount of executed cycles */
   return (cpu.total_cycles - old_cycles);
}


#if 0
void nes6502_init(void)
{
  cpu.a_reg = cpu.x_reg = cpu.y_reg = 0;
  cpu.s_reg = 0xff;                         /* Stack grows down */
  cpu.burn_cycles = 0;
}
#endif

/* Issue a CPU Reset */
void nes6502_reset(void)
{
  cpu.p_reg = Z_FLAG | R_FLAG | I_FLAG;     /* Reserved bit always 1 */
  cpu.int_pending = 0;                      /* No pending interrupts */
  cpu.pc_reg = bank_readword(RESET_VECTOR); /* Fetch reset vector */
  cpu.burn_cycles = RESET_CYCLES;
  cpu.jammed = FALSE;
}

/* Non-maskable interrupt */
void nes6502_nmi(void)
{
  uint32 REG_PC;
  uint32 REG_A;
  uint32 REG_X;
  uint32 REG_Y;
  uint32 REG_S;
  
  uint32 flag_n;
  uint32 flag_v;
  uint32 flag_b;
  uint32 flag_d;
  uint32 flag_i;
  uint32 flag_z;
  uint32 flag_c;
   
  if (FALSE == cpu.jammed)
  {
    GET_GLOBAL_REGS();
    NMI_PROC();
    cpu.burn_cycles += INT_CYCLES;
    STORE_LOCAL_REGS();
  }
}

/* Interrupt request */
void nes6502_irq(void)
{
  uint32 REG_PC;
  uint32 REG_A;
  uint32 REG_X;
  uint32 REG_Y;
  uint32 REG_S;
  
  uint32 flag_n;
  uint32 flag_v;
  uint32 flag_b;
  uint32 flag_d;
  uint32 flag_i;
  uint32 flag_z;
  uint32 flag_c;
  
  if (FALSE == cpu.jammed)
  {
    GET_GLOBAL_REGS();
    if (0 == flag_i)
    {
      IRQ_PROC();
      cpu.burn_cycles += INT_CYCLES;
    }
    //else
    //   cpu.int_pending = 1;
    STORE_LOCAL_REGS();
  }
}

void nes6502_pending_irq(void)
{
  uint32 REG_PC;
  uint32 REG_A;
  uint32 REG_X;
  uint32 REG_Y;
  uint32 REG_S;
  
  uint32 flag_n;
  uint32 flag_v;
  uint32 flag_b;
  uint32 flag_d;
  uint32 flag_i;
  uint32 flag_z;
  uint32 flag_c;
  
  if (FALSE == cpu.jammed)
  {
    GET_GLOBAL_REGS();
    if (0 == flag_i)
    {
      IRQ_PROC();
      cpu.burn_cycles += INT_CYCLES;
    }
    else
      cpu.int_pending = 1;
    STORE_LOCAL_REGS();
  }
}

/* Set dead cycle period */
void nes6502_burn(int cycles)
{
  cpu.burn_cycles += cycles;
}

/*
** nes6502.c,v
** Revision 1.3.2.6  2002/11/07 13:00:04  tekezo
** *** empty log message ***
**
** Revision 1.2  2002/02/05 10:41:41  tekezo
** *** empty log message ***
**
** Revision 1.1  2001/12/11 05:43:07  tekezo
** *** empty log message ***
**
** Revision 1.7  2001/10/08 17:39:33  tekezo
** fds pre-support
**
** Revision 1.6  2001/10/03 10:09:37  tekezo
** support save snapshot
**
** Revision 1.5  2001/09/29 13:07:53  tekezo
** brushup
**
** Revision 1.4  2001/09/23 17:28:48  tekezo
** bug fix for gradius2
**
** Revision 1.3  2001/09/23 12:23:57  tekezo
** now support SNSS(real)
**
** Revision 1.2  2001/09/23 10:53:54  tekezo
** now support SNSS
**
** Revision 1.1.1.1  2001/09/02 17:03:19  tekezo
** reimport
**
** Revision 1.23  2000/09/11 01:45:45  matt
** flag optimizations.  this thing is fast!
**
** Revision 1.22  2000/09/08 13:29:25  matt
** added switch()-less execution for gcc
**
** Revision 1.21  2000/09/08 11:54:48  matt
** optimize
**
** Revision 1.20  2000/09/07 21:58:18  matt
** api change for nes6502_burn, optimized core
**
** Revision 1.19  2000/09/07 13:39:01  matt
** resolved a few conflicts
**
** Revision 1.18  2000/09/07 01:34:55  matt
** nes6502_init deprecated, moved flag regs to separate vars
**
** Revision 1.17  2000/08/31 13:26:35  matt
** added DISASM flag, to sync with asm version
**
** Revision 1.16  2000/08/29 05:38:00  matt
** removed faulty failure note
**
** Revision 1.15  2000/08/28 12:53:44  matt
** fixes for disassembler
**
** Revision 1.14  2000/08/28 04:32:28  matt
** naming convention changes
**
** Revision 1.13  2000/08/28 01:46:15  matt
** moved some of them defines around, cleaned up jamming code
**
** Revision 1.12  2000/08/16 04:56:37  matt
** accurate CPU jamming, added dead page emulation
**
** Revision 1.11  2000/07/30 04:32:00  matt
** now emulates the NES frame IRQ
**
** Revision 1.10  2000/07/17 01:52:28  matt
** made sure last line of all source files is a newline
**
** Revision 1.9  2000/07/11 04:27:18  matt
** new disassembler calling convention
**
** Revision 1.8  2000/07/10 05:26:38  matt
** cosmetic
**
** Revision 1.7  2000/07/06 17:10:51  matt
** minor (er, spelling) error fixed
**
** Revision 1.6  2000/07/04 04:50:07  matt
** minor change to includes
**
** Revision 1.5  2000/07/03 02:18:16  matt
** added a few notes about potential failure cases
**
** Revision 1.4  2000/06/09 15:12:25  matt
** initial revision
**
*/
