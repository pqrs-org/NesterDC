#ifndef __NNNESTERJ_CHEAT_H__
#define __NNNESTERJ_CHEAT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if 0
}
#endif


typedef struct {
  uint8 comment[36]; 
  uint32 address;
  uint32 data;
  uint8 check_flag;
  uint8 flag;
  uint8 size;
} nnnesterj_cheatinfo_t;


typedef struct {
  nnnesterj_cheatinfo_t cheatinfo;
  uint8 *mem_base;
  uint32 mem_mask;
} nes_nnnesterj_cheatinfo_t;


int nnnesterj_cheat_load (nes_nnnesterj_cheatinfo_t *nes_nnnesterj_cheatinfo, int *nes_nnnesterj_cheatinfo_size, uint8 *RAM_base, uint8 *SRAM_base, const char *filename);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
