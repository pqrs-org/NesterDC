/*
** Nintendo FDS ExSound by TAKEDA, toshiya
**
** original: s_fds.c in nezp0922
*/

/* modifyed by stun 2000.10.24 */
/* bug fix  by stun 2000.11.27 */


static int32 FDSSoundRender(void)
{
  FDS_FMOP *op0, *op1;
  FDSSOUND *fds;
  
  fds = &(apu->fdssound);
  op0 = fds->op;
  op1 = fds->op + 1;
  
  if (op0->timer >= 0)
    --(op0->timer);
  
  if (op0->timer == 0)
  {
    op1->sweep = op0->last_spd;
    op0->envmode = 1;
  }
  
  if (op0->envmode == 1)
    op1->spd = op1->sweep;
  else if (op0->envmode & 0x80)
  {
    op0->envphase++;
    if (!(op0->envmode & 0x40))
    {
      if (op0->envphase == op0->envspd)
      {
	op0->envphase = 0;
	if (op1->spd < op1->sweep)
	{
	  op1->spd += op0->volume;
	  if (op1->spd > op1->sweep)
	    op1->spd = op1->sweep;
	}
      }
    }
    else
    {
      if (op0->envphase == op0->envspd)
      {
	op0->envphase = 0;
	if (op1->spd > op1->sweep / 2)
	{
	  op1->spd -= op0->volume; 
	  if (op1->spd < op1->sweep)
	    op1->spd = op1->sweep;
	}
      }
    }
  }
  
  {
    uint32 vol; 
    
    op0->envout = LinearToLog(0);
    
    if (op1->envmode && fds->fade)
    {
      op1->envphase -= fds->cps >> (11 - 1);
      if (op1->envmode & 0x40)
	while (op1->envphase < 0)
	{
	  op1->envphase += op1->envspd;
	  op1->volume += (op1->volume < 0x1f);
	}
      else
	while (op1->envphase < 0)
	{
	  op1->envphase += op1->envspd;
	  op1->volume -= (op1->volume > 0x00);
	}
    }
    vol = op1->volume;
    op1->envout = LinearToLog(vol) + fds->mastervolume;
  }
  
  {
    fds->cycles -= fds->cps;
    fds->output = 0;
    
    if (fds->cycles < 0)
    {
      int cycles = - fds->cycles;
      int count;
      
      if ((cycles & 0x7ff) == 0)
	count = cycles >> 23;
      else
	count = (cycles >> 23) + 1;
      
      fds->cycles += (count << 23); 
      
      if (op0->spd && op0->enable)
      {
	op0->phase += op0->spd * count;
	if (!(op1->spd) || !(op1->enable))
	{
	  fds->output = 
	    LogToLinear(op0->envout + op0->wave[(op0->phase >> (23 - 1)) & 0x3f], 
			op0->outlvl);
	}
      }
      if (op1->spd && op1->enable)
      {
	op1->phase += op1->spd * count;
	fds->output = 
	  LogToLinear(op1->envout + op1->wave[(op1->phase >> (23 - 1)) & 0x3f], 
		      op1->outlvl);
      }
    }
  }
  
  if (fds->mute) return 0;
  return fds->output >> 8;
}

static void FDSSoundVolume(uint32 volume)
{
	apu->fdssound.mastervolume = (volume << (LOG_BITS - 8)) << 1;
}

static void FDSSoundWrite(uint32 address, uint8 value)
{
	if (0x4040 <= address && address <= 0x407F)
	{
		apu->fdssound.op[1].wave[address - 0x4040] = LinearToLog(((int32)value & 0x3f) - 0x20);
	}
	else if (0x4080 <= address && address <= 0x408F)
	{
		int ch = (address < 0x4084);
		FDS_FMOP *pop = &apu->fdssound.op[ch];
		apu->fdssound.reg[address - 0x4080] = value;
		switch (address & 15)
		{
			case 0:
				if (value & 0x80)
				{
					pop->volume = (value & 0x3f);
					pop->envmode = 0;
				}
				else
				{
					pop->envspd = ((value & 0x3f) + 1) << 23;
					pop->envmode = 0x80 | value;
				}
				break;
			case 4:
				if(value & 0x80)
				{
					int32 a=apu->fdssound.op[1].spd;
					int32 b=apu->fdssound.op[1].sweep;
					pop->timer=(0x3f-(value & 0x3f)) << 10;
					if(pop->timer==0) pop->timer=1;
					pop->last_spd=a*(0x3f-(value & 0x3f))/0x3f+
								  b*(value & 0x3f)/0x3f;
				}
				else if(apu->fdssound.op[1].sweep)
				{
					pop->envspd = (value & 0x3f) << 5;
					if((value & 0x3f)==0) pop->envspd=1;
					pop->envphase = 0;
					pop->envmode = 0x80 | (value & 0x40) ;
					pop->volume=abs(apu->fdssound.op[1].sweep - apu->fdssound.op[1].spd);
					pop->volume/=pop->envspd;
					if((value & 0x3f)==0) pop->envmode=1;
				}
				apu->fdssound.waveaddr = 0;
				break;
			case 1:
				if ((value & 0x7f) < 0x60)
					apu->fdssound.op[0].sweep = value & 0x7f;
				else
					apu->fdssound.op[0].sweep = ((int32)value & 0x7f) - 0x80;
				break;
			case 5:
				if (!value) break;
				if ((value & 0x7f) < 0x60)
				{
					apu->fdssound.op[1].sweep = (int32)apu->fdssound.op[1].spd+
						((apu->fdssound.op[1].spd * (value & 0x7f))>>5);
				}
				else
				{
					apu->fdssound.op[1].sweep = (int32)apu->fdssound.op[1].spd-
						(((apu->fdssound.op[1].spd) * (((int32)value & 0x7f) - 0x80)) >> 5);
				}
				break;
			case 2:
				pop->spd &= 0x00000F00 << 7;
				pop->spd |= (value & 0xFF) << 7;
				apu->fdssound.op[0].envmode = 0;
				apu->fdssound.op[0].timer=0;
				break;
			case 6:
				pop->spd &= 0x00000F00 << 7;
				pop->spd |= (value & 0xFF) << 7;
				pop->envmode = 0;
				break;
			case 3:
				pop->spd &= 0x000000FF << 7;
				pop->spd |= (value & 0x0F) << (7 + 8);
				pop->enable = !(value & 0x80);
				apu->fdssound.op[0].envmode = 0;
				apu->fdssound.op[0].timer=0;
				break;
			case 7:
				pop->spd &= 0x000000FF << 7;
				pop->spd |= (value & 0x0F) << (7 + 8);
				pop->enable = !(value & 0x80);
				apu->fdssound.waveaddr = 0;
				break;
			case 8:
				{
					static int8 lfotbl[8] = { 0,1,2,3,-4,-3,-2,-1 };
					uint32 v = LinearToLog(lfotbl[value & 7]);
					apu->fdssound.op[0].wave[apu->fdssound.waveaddr++] = v;
					apu->fdssound.op[0].wave[apu->fdssound.waveaddr++] = v;
					if (apu->fdssound.waveaddr == 0x40)
					{
						apu->fdssound.waveaddr = 0;
					}
				}
				break;
			case 9:
				apu->fdssound.op[0].outlvl = LOG_LIN_BITS - LIN_BITS - LIN_BITS - 10 - (value & 3);
				if(value & 0x80) apu->fdssound.mute=1;
				else             apu->fdssound.mute=0;
				break;
			case 10:
				apu->fdssound.op[1].outlvl = LOG_LIN_BITS - LIN_BITS - LIN_BITS - 10 - (value & 3);
				if(value & 0x80) apu->fdssound.fade=1;
				else             apu->fdssound.fade=0;
				break;
		}
	}
}

static void FDSSoundWriteCurrent(uint32 address, uint8 value)
{
	if (0x4080 <= address && address <= 0x408F)
	{
		apu->fdssound.reg_cur[address - 0x4080] = value;
	}
}

static uint8 FDSSoundRead(uint32 address)
{
	if (0x4090 <= address && address <= 0x409F)
	{
		return apu->fdssound.reg_cur[address - 0x4090];
	}
	return 0;
}

static void FDSSoundReset(void)
{
  int i;
  FDS_FMOP *pop;
  
  memset(&apu->fdssound, 0, sizeof(FDSSOUND));
  apu->fdssound.cps = DivFix(NES_BASECYCLES, 12 * (1 << 1) * SAMPLE_RATE, 23);
  for (pop = &apu->fdssound.op[0]; pop < &apu->fdssound.op[2]; pop++)
  {
    pop->enable = 1;
  }
  apu->fdssound.op[0].outlvl = LOG_LIN_BITS - LIN_BITS - LIN_BITS - 10;
  apu->fdssound.op[1].outlvl = LOG_LIN_BITS - LIN_BITS - LIN_BITS - 10;
  
  for (i = 0; i < 0x40; i++)
  {
    apu->fdssound.op[1].wave[i] = LinearToLog((i < 0x20) ? 0x1f : -0x20);
  }
}

