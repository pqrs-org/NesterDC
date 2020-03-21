/*
** Namco 106 ExSound by TAKEDA, toshiya
**
** original: s_n106.c in nezp0922
*/

static int32 N106SoundRender(void)
{
  N106_WM *chp;
  int32 accum = 0;
  uint32 cyclesspd = apu->n106s.chinuse << 20;
  uint8 chinuse = apu->n106s.chinuse;
  
  chp = apu->n106s.ch + 8 - apu->n106s.chinuse;
  while (chinuse--)
  {
    chp->cycles -= apu->n106s.cps;
    while (chp->cycles < 0)
    {
      chp->cycles += cyclesspd;
      chp->phase += chp->spd;
    }
    while (chp->phase >= chp->tlen)
      chp->phase -= chp->tlen;
    
    accum += LogToLinear(apu->n106s.tone[((chp->phase >> 18) + chp->tadr) & 0xff] + chp->logvol + apu->n106s.mastervolume, LOG_LIN_BITS - LIN_BITS - LIN_BITS - 10);
    
    ++chp;
  }
  return accum >> 8;
}

static void N106SoundVolume(uint32 volume)
{
  apu->n106s.mastervolume = (volume << (LOG_BITS - 8)) << 1;
}

static uint8 N106SoundReadData(uint32 address)
{
  uint8 ret = apu->n106s.data[apu->n106s.address];
  if (apu->n106s.addressauto)
    apu->n106s.address = (apu->n106s.address + 1) & 0x7f;
  return ret;
}

static uint8 N106SoundRead(uint32 address)
{
  if (address == 0x4800)
  {
    return N106SoundReadData(address);
  }
  else
  {
    return 0x00;
  }
}

static void N106SoundWriteAddr(uint32 address, uint8 value)
{
  apu->n106s.address     = value & 0x7f;
  apu->n106s.addressauto = (value & 0x80) ? 1 : 0;
}

static void N106SoundWriteData(uint32 address, uint8 value)
{
  apu->n106s.data[apu->n106s.address] = value;
  apu->n106s.tone[apu->n106s.address * 2]     = LinearToLog(((int)(value & 0xf) << 2) - 0x20);
  apu->n106s.tone[apu->n106s.address * 2 + 1] = LinearToLog(((int)(value >>  4) << 2) - 0x20);
  if (apu->n106s.address >= 0x40)
  {
    N106_WM *chp = &apu->n106s.ch[(apu->n106s.address - 0x40) >> 3];
    switch (apu->n106s.address & 7)
    {
      case 0:
	chp->spd = (chp->spd & 0xffff00) | value;
	break;
      case 2:
	chp->spd = (chp->spd & 0xff00ff) | (value << 8);
	break;
      case 4:
	chp->spd = (chp->spd & 0x00ffff) | ((value & 0x3) << 16);
	
	{
	  uint32 tlen;
	  
	  tlen = (0x20 - (value & 0x1c)) << 18;
	  if (chp->tlen != tlen)
	  {
	    chp->tlen = tlen;
	    chp->phase = 0;
	  }
	}
	break;
      case 6:
	chp->tadr = value & 0xff;
	break;
      case 7:
	if (chp == &apu->n106s.ch[7])
	  apu->n106s.chinuse = 1 + ((value >> 4) & 0x07);
	
	chp->logvol = LinearToLog((value & 0x0f) << 2);
	
	break;
    }
  }
  if (apu->n106s.addressauto)
  {
    apu->n106s.address = (apu->n106s.address + 1) & 0x7f;
  }
}

static void N106SoundWrite(uint32 address, uint8 value)
{
  if (address & 0x10000)
  {
    /* dummy read */
    N106SoundRead(address & 0xffff);
  }
  else
  {
    if (address == 0x4800)
      N106SoundWriteData(address, value);
    else if (address == 0xf800)
      N106SoundWriteAddr(address, value);
  }
}

static void N106SoundReset(void)
{
  int i;
  
  apu_t *apu;
  apu = apu_getcontext ();
  
  memset (&apu->n106s, 0, sizeof(N106SOUND));
  for (i = 0; i < 8; i++)
  {
    apu->n106s.ch[i].tlen = 0x10 << 18;
    apu->n106s.ch[i].logvol = LinearToLog(0);
  }
  
  apu->n106s.addressauto = 1;
  apu->n106s.chinuse = 8;
  apu->n106s.cps = DivFix(NES_BASECYCLES, 45 * SAMPLE_RATE, 20);
}

