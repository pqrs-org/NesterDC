/* KallistiOS 1.1.8

   sndoggvorbis.h
   (c)2001 Thorsten Titze

   An Ogg/Vorbis player library using sndstream and the official Xiphophorus
   libogg and libvorbis libraries.

   sndoggvorbis.h,v 1.1.1.1 2002/10/09 13:59:15 tekezo Exp
   
*/

#ifndef __SNDOGGVORBIS_H
#define __SNDOGGVORBIS_H

#include <sys/cdefs.h>
__BEGIN_DECLS

int sndoggvorbis_init();
int sndoggvorbis_start(char *filename,int loop);
void sndoggvorbis_stop();
void sndoggvorbis_shutdown();

int sndoggvorbis_isplaying();

void sndoggvorbis_volume(int vol);

void sndoggvorbis_mainloop();
void sndoggvorbis_wait_start();

void sndoggvorbis_setbitrateinterval(int interval);
long sndoggvorbis_getbitrate();
long sndoggvorbis_getposition();

char *sndoggvorbis_getcommentbyname(char *commentfield);
char *sndoggvorbis_getartist();
char *sndoggvorbis_gettitle();
char *sndoggvorbis_getgenre();

/* Enable/disable queued waiting */
void sndoggvorbis_queue_enable();
void sndoggvorbis_queue_disable();

/* Wait for the song to be queued */
void sndoggvorbis_queue_wait();

/* Queue the song to start if it's in QUEUED */
void sndoggvorbis_queue_go();

__END_DECLS

#endif	/* __SNDOGGVORBIS_H */

