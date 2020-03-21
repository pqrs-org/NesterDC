/* KallistiOS 1.1.8

   asic.c
   (c)2000-2002 Dan Potter
*/

/* 
   This module contains low-level ASIC handling. Right now this is just for
   ASIC interrupts, but it will eventually include DMA as well.

   The DC's System ASIC is integrated with the 3D chip and serves as the
   Grand Central Station for the interaction of all the various peripherals
   (which really means that it allows the SH-4 to control all of them =).
   The basic block diagram looks like this:

   +-----------+    +--------+    +-----------------+
   | 16MB Ram  |    |        |----| 8MB Texture Ram |
   +-----------+    | System |    +-----------------+
      |             |  ASIC  |    +--------------------+  +-------------+
      +-------------+        +-+--+    AICA SPU        |--+ 2MB SPU RAM |
      |A            | PVR2DC | |  +-------------------++  +-------------+
   +-------+        |        | |C +-----------------+ |
   | SH-4  |        |        | \--+ Expansion Port  | |
   +-------+        +---+----+    +-----------------+ |
                        |B        +------------+      |D
                        +---------+   GD-Rom   +------/
                        |         +------------+
                        |         +----------------------+
                        \---------+ 2MB ROM + 256K Flash |
                                  +----------------------+

   A: Main system bus -- connects the SH-4 to the ASIC and its main RAM
   B: "G1" bus -- connects the ASIC to the GD-Rom and the ROM/Flash
   C: "G2" bus -- connects the ASIC to the SPU and Expansion port
   D: Not entirely verified connection for streaming audio data

   All buses can simultaneously transmit data via PIO and/or DMA. This
   is where the ridiculous bandwidth figures come from that you see in
   marketing literature. In reality, each bus isn't terribly fast on
   its own -- they tend to have high bandwidth but high latency.

   The "G2" bus is notoriously flaky. Specifically, one should ensure
   to write the proper data size for the peripheral you are accessing
   (32-bits for SPU, 8-bits for 8-bit peripherals, etc). Every 8
   32-bit words written to the SPU must be followed by a g2_fifo_wait().
   Additionally, if SPU or Expansion Port DMA is being used, only one
   of these may proceed at once and any PIO access _must_ pause the
   DMA and disable interrupts. Any other treatment may cause serious
   data corruption between the ASIC and the G2 peripherals.

   For more information on all of this see:

   http://www.segatech.com/technical/dcblock/index.html
   http://mc.pp.se/dc/

 */

/* Small interrupt listing (from the two Marcus's =)
   For now, everything is mapped thru IRQ 9

  691x -> irq 13
  692x -> irq 11
  693x -> irq 9

  69x0
    bit 2	render complete
        3	scanline 1
        4	scanline 2
        5	vsync
        7	opaque polys accepted
        8	opaque modifiers accepted
        9	translucent polys accepted
        10	translucent modifiers accepted
        12	maple dma complete
        13	maple error(?)
        14	gd-rom dma complete
        15	aica dma complete
        16	external dma 1 complete
        17	external dma 2 complete
        18	??? dma complete
        21	punch-thru polys accepted

  69x4
    bit 0	gd-rom command status
        1	AICA
        2       modem?
        3       expansion port (PCI bridge)

  69x8
    bit 2	out of primitive memory
    bit 3	out of matrix memory

 */

#include <string.h>
#include <stdio.h>
#include <arch/irq.h>
#include <dc/asic.h>
#include <arch/spinlock.h>

CVSID("asic.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp");

/* Exception table -- this table matches each potential G2 event to a function
   pointer. If the pointer is null, then nothing happens. Otherwise, the
   function will handle the exception. */
static asic_evt_handler asic_evt_handlers[0x4][0x20];

/* Set a handler, or remove a handler */
int asic_evt_set_handler(uint32 code, asic_evt_handler hnd) {
	uint32 evtreg, evt;
	
	evtreg = (code >> 8) & 0xff;
	evt = code & 0xff;
	
	if (evtreg > 0x4)
		return -1;
	if (evt > 0x20)
		return -1;

	asic_evt_handlers[evtreg][evt] = hnd;
	
	return 0;
}

/* The ASIC event handler; this is called from the global IRQ handler
   to handle external IRQ 9. */
static void handler_irq9(irq_t source, irq_context_t *context) {
	int	reg, i;
	vuint32	*asicint = (vuint32*)0xa05f6900;
	uint32	mask;

	/* Go through each event register and look for pending events */
	for (reg=0; reg<4; reg+=2) {
		/* Read the event mask and clear pending */
		mask = asicint[4*reg]; asicint[4*reg] = mask;
		
		/* Search for relevant handlers */
		for (i=0; i<32; i++) {
			if (mask & (1 << i)) {
				if (asic_evt_handlers[reg][i] != NULL) {
					asic_evt_handlers[reg][i]( (reg << 8) | i );
				}
			}
		}
	}
}


/* Disable all G2 events */
void asic_evt_disable_all() {
	vuint32	*asicint = (vuint32*)0xa05f6900;
	
	asicint[0x30/4] = 0;
	asicint[0x30/4] = 0;
	asicint[0x30/4] = 0;
	asicint[0x30/4] = 0;
}

/* Disable a particular G2 event */
void asic_evt_disable(uint32 code) {
	vuint32 *asicint = (vuint32*)0xa05f6900;
	asicint[0x30/4 + ((code >> 8) & 0xff)] &= ~(1 << (code & 0xff));
}

/* Enable a particular G2 event */
void asic_evt_enable(uint32 code) {
	vuint32 *asicint = (vuint32*)0xa05f6900;
	asicint[0x30/4 + ((code >> 8) & 0xff)] |= (1 << (code & 0xff));
}

/* Initialize events */
void asic_evt_init() {
	vuint32 *asic;

	asic = (vuint32*)0xa05f0000;

	/* Clear any pending interrupts and disable all events */
	asic[0x6884/4] = 0;
	asic[0x6910/4] = 0;
	asic[0x6914/4] = 0;
	asic[0x6918/4] = 0;
	asic[0x6920/4] = 0;
	asic[0x6924/4] = 0;
	asic[0x6928/4] = 0;
	asic[0x6930/4] = 0;
	asic[0x6934/4] = 0;
	asic[0x6938/4] = 0;
	asic[0x6900/4] = 0xffffffff;
	asic[0x6904/4] = 0xffffffff;
	asic[0x6908/4] = 0xffffffff;
	
	/* Clear out the event table */
	memset(asic_evt_handlers, 0, sizeof(asic_evt_handlers));
	
	/* Hook IRQ9 */
	irq_set_handler(EXC_IRQ9, handler_irq9);
}

/* Shutdown events */
void asic_evt_shutdown() {
	/* Disable all events */
	asic_evt_disable_all();
	
	/* Unhook handler */
	irq_set_handler(EXC_IRQ9, NULL);
	
}

/* Init routine */
void asic_init() {
	asic_evt_init();
}


void asic_shutdown() {
	asic_evt_shutdown();
}





