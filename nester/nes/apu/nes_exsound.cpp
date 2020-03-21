
/*
** ExSound by TAKEDA, toshiya
**
** original: nezp0922
*/

#include <math.h>

// ----------------------------------------------------------------------------
// common

// ex_chip	=  0; none
//		=  1; VRC6
//		=  2; VRC7
//		=  4; FDS
//		=  8; MMC5
//		= 16; N106
//		= 32; FME7
//		= 64; J106 (reserved)

//#define SAMPLE_RATE 44100
#define SAMPLE_RATE apu->sample_rate
#define NES_BASECYCLES (21477270)
#define M_PI 3.14159265358979323846

#include "sound_tables/tbl_param.h"

static const uint32 lineartbl[] = {
#include "sound_tables/tbl/lineartable.h"
};
static const uint32 logtbl[] = {
#include "sound_tables/tbl/logtable.h"
};

uint32 LinearToLog(int32 l)
{
	return (l < 0) ? (lineartbl[-l] + 1) : lineartbl[l];
}

int32 LogToLinear(uint32 l, uint32 sft)
{
	int32 ret;
	uint32 ofs;
	sft += (l >> 1) >> LOG_BITS;
	if (sft >= LOG_LIN_BITS) return 0;
	ofs = (l >> 1) & ((1 << LOG_BITS) - 1);
	ret = logtbl[ofs] >> sft;
	
	return (l & 1) ? -ret : ret;
}


#include "debug.h"
void LogTableInitialize(void)
{
  ASSERT(sizeof(lineartbl) == sizeof(uint32) * lineartable_len);
  ASSERT(sizeof(logtbl) == sizeof(uint32) * logtable_len);
}


static uint32 DivFix(uint32 p1, uint32 p2, uint32 fix)
{
	uint32 ret;
	ret = p1 / p2;
	p1  = p1 % p2;/* p1 = p1 - p2 * ret; */
	while (fix--)
	{
		p1 += p1;
		ret += ret;
		if (p1 >= p2)
		{
			p1 -= p2;
			ret++;
		}
	}
	return ret;
}

#include "nes_vrc6.cpp"
#include "nes_vrc7.cpp"
//#include "nes_fds_new.cpp"
#include "nes_fds.cpp"
#include "nes_mmc5.cpp"
#include "nes_n106.cpp"
#include "nes_fme7.cpp"

