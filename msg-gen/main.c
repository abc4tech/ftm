#include "display.h"

volatile unsigned int* leds = (unsigned int*)0x100400;



void _irq_entry(void) {
  /* Currently only triggered by DMA completion */
}


const char mytext[] = "Hallo Welt!...\n\n";

void main(void) {
  int i, j, xinc, yinc, x, y;

unsigned int time = 0;

  unsigned int addr_raw = 0x223;
	unsigned int addr_raw_off;

	char color = 0xFF;

  disp_reset();	
  disp_put_c('\f');
	x = 0;
	y = 9;
	yinc = -1;
 	xinc = 1;
	addr_raw_off = 0;
	
  while (1) {
    /* Rotate the LEDs */
    for (i = 0; i < 8; ++i) {
      *leds = 1 << i;
	//disp_put_raw( 0xFF, addr_raw);
	/*	
	disp_put_raw( 0x03, addr_raw+addr_raw_off);
	if(addr_raw_off < 0x500) addr_raw_off += 1<<8;
	else addr_raw_off= 0;

	//disp_put_raw( 0x30, addr_raw + 0x500);
	//disp_put_raw( 0x10, addr_raw + 0x500 + 0x3F);
	
*/
	disp_put_raw( get_pixcol_val((unsigned char)y), get_pixcol_addr((unsigned char)x, (unsigned char)y), color);


	if(x == 63) xinc = -1;
	if(x == 0)  xinc = 1;

	if(y == 47) yinc = -1;
	if(y == 0)  yinc = 1;

	x += xinc;
	y += yinc;

	//disp_put_raw(1 << i, addr_raw+addr_raw_off);	
	//addr_raw_off++;
	
	

      /* Each loop iteration takes 4 cycles.
       * It runs at 125MHz.
       * Sleep 0.2 second.
       */
      for (j = 0; j < 125000000/160; ++j) {
        asm("# noop"); /* no-op the compiler can't optimize away */
      }

	if(time++ > 500) {time = 0; color = ~color; }
	
    }
  }
}
