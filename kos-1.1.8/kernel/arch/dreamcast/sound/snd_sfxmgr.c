/* KallistiOS 1.1.8

   snd_sfxmgr.c
   (c)2000-2002 Dan Potter

   Sound effects management system; this thing loads and plays sound effects
   during game operation.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <kos/fs.h>
#include <dc/spu.h>
#include <dc/sound/sound.h>
#include <dc/sound/sfxmgr.h>

#include "arm/aica_cmd_iface.h"

CVSID("snd_sfxmgr.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp");

typedef struct {
	uint32	loc, len;
	uint32	rate;
	uint32	used;
} snd_effect;

#define SFX_CNT 16
static snd_effect sfx[SFX_CNT] = { {0,0,0,0} };

static int sfx_lastchan = 2;

static int sfx_find_open() {
	int i;
	for (i=0; i<SFX_CNT; i++)
		if (sfx[i].used == 0)
			return i;
			
	return -1;
}

/* Unload all loaded samples and free their SPU RAM */
void snd_sfx_unload_all() {
	int i;

	for (i=0; i<SFX_CNT; i++) {
		snd_mem_free(sfx[i].loc);
		sfx[i].loc = sfx[i].len = sfx[i].rate = sfx[i].used = 0;
	}
}

/* Unload a single sample */
void snd_sfx_unload(int idx) {
	assert( idx < SFX_CNT );
	snd_mem_free(sfx[idx].loc);
	sfx[idx].loc = sfx[idx].len = sfx[idx].rate = sfx[idx].used = 0;
}

/* WAV header:
	0x08	-- "WAVE"
	0x16	-- short num channels (1/2)
	0x18	-- long  HZ
	0x22	-- short 8 or 16 (bits)
	0x28	-- long  data length
	0x2c	-- data start

 */

/* Load a sound effect from a WAV file and return a handle to it */
int snd_sfx_load(const char *fn) {
	uint32	fd, len, hz;
	uint16	*tmp, chn, fmt;
	int	idx;

	idx = sfx_find_open();
	if (idx < 0) {
		dbglog(DBG_WARNING, "snd_sfx: no more sfx slots\n");
		return -1;
	}
	
	dbglog(DBG_DEBUG, "snd_sfx: loading effect %s as slot %d\n", fn, idx);

	fd = fs_open(fn, O_RDONLY);
	if (fd == 0) {
		dbglog(DBG_WARNING, "snd_sfx: can't open sfx %s\n", fn);
		return -1;
	}

	/* Check file magic */
	hz = 0;
	fs_seek(fd, 8, SEEK_SET);
	fs_read(fd, &hz, 4);
	if (strncmp((char*)&hz, "WAVE", 4)) {
		dbglog(DBG_WARNING, "snd_sfx: file is not RIFF WAVE\n");
		fs_close(fd);
		return -1;
	}
	
	
	/* Read WAV header info */
	fs_seek(fd, 0x16, SEEK_SET);
	fs_read(fd, &chn, 2);
	fs_read(fd, &hz, 4);
	fs_seek(fd, 0x22, SEEK_SET);
	fs_read(fd, &fmt, 2);
	
	// printf("WAVE file is %s, %dHZ, %d bits\r\n", chn==1 ? "mono" : "stereo", hz, fmt);
	
	/* Read WAV data */
	fs_seek(fd, 0x28, SEEK_SET);
	fs_read(fd, &len, 4);
	// printf("Reading %d bytes of wave data\r\n", len);
	tmp = malloc(len);
	fs_read(fd, tmp, len);
	fs_close(fd);

	sfx[idx].len = len;
	sfx[idx].rate = hz;
	sfx[idx].used = 1;
	sfx[idx].loc = snd_mem_malloc(len);
	if (sfx[idx].loc)
		spu_memload(sfx[idx].loc, tmp, len);
	free(tmp);
	
	return idx;
}

/* Play a sound effect with the given volume and panning */
void snd_sfx_play(int idx, int vol, int pan) {
	int size;
	AICA_CMDSTR_CHANNEL(tmp, cmd, chan);

	size = sfx[idx].len/2;
	if (size > 65536) size = 65535;
	
	/* printf("sndstream: playing effect %d on chan %d, loc %x, rate %d, size %d, vol %d, pan %d\r\n",
		idx, sfx_lastchan, sfx[idx].loc, sfx[idx].rate, size, vol, pan); */
	/* snd_play_effect(sfx_lastchan, sfx[idx].loc, sfx[idx].rate, size, vol, pan); */
	cmd->cmd = AICA_CMD_CHAN;
	cmd->timestamp = 0;
	cmd->size = sizeof(tmp)/4;
	cmd->cmd_id = sfx_lastchan;
	chan->cmd = AICA_CH_CMD_START;
	chan->base = sfx[idx].loc;
	chan->type = AICA_SM_16BIT;
	chan->length = size;
	chan->loop = 0;
	chan->loopstart = 0;
	chan->loopend = size;
	chan->freq = sfx[idx].rate;
	chan->vol = vol;
	chan->pan = pan;
	snd_sh4_to_aica(tmp, sizeof(tmp)/4);
	
	sfx_lastchan++;
	if (sfx_lastchan >= 64)
		sfx_lastchan = 2;
}

