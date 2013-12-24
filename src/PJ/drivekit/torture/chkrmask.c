#include "torture.h"

#define	MASK1_TIME_ITCOUNT  30
#define	MASK2_TIME_ITCOUNT  20

#define MBPH 14
#define MBPW 48
#define MBPR (MBPW/8)
static UBYTE markBitmap[MBPH][MBPR] =
			   {
			   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
			   0xC2, 0x03, 0xC0, 0x03, 0xC2, 0x03,
			   0xA1, 0x05, 0xA0, 0x05, 0xA1, 0x05,
			   0x90, 0x89, 0x90, 0x09, 0x90, 0x89,
			   0x88, 0x51, 0x88, 0x11, 0x88, 0x51,
			   0x87, 0xE1, 0x87, 0xE1, 0x87, 0xE1,
			   0x84, 0x21, 0x84, 0x21, 0x84, 0x21,
			   0x84, 0x21, 0x84, 0x21, 0x84, 0x21,
			   0x87, 0xE1, 0x87, 0xE1, 0x87, 0xE1,
			   0x88, 0x51, 0x88, 0x11, 0x88, 0x51,
			   0x90, 0x89, 0x90, 0x09, 0x90, 0x89,
			   0xA1, 0x05, 0xA0, 0x05, 0xA1, 0x05,
			   0xC2, 0x03, 0xC0, 0x03, 0xC2, 0x03,
			   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
			   };

      /* 111111111111111111111111111111111111111111111111 */
      /* 110000100000001111000000000000111100001000000011 */
      /* 101000010000010110100000000001011010000100000101 */
      /* 100100001000100110010000000010011001000010001001 */
      /* 100010000101000110001000000100011000100001010001 */
      /* 100001111110000110000111111000011000011111100001 */
      /* 100001000010000110000100001000011000010000100001 */
      /* 100001000010000110000100001000011000010000100001 */
      /* 100001111110000110000111111000011000011111100001 */
      /* 100010000101000110001000000100011000100001010001 */
      /* 100100001000100110010000000010011001000010001001 */
      /* 101000010000010110100000000001011010000100000101 */
      /* 110000100000001111000000000000111100001000000011 */
      /* 111111111111111111111111111111111111111111111111 */

void test_mask1blit(Raster *r)
/*****************************************************************************
 *
 ****************************************************************************/
{
Rastlib		*rlib = r->lib;
Raster		*vbr = &tcb.verification_raster;
Coor		x;
Coor		y;
Coor		xstart;
Coor		width;
Coor		height;
Pixel		color;
short		iteration_count;
short		iteration_limit;

	width = r->width;
	height = r->height;

	/*-----------------------------------------------------------------------
	 * mask1blit testing...
	 *   if the driver didn't provide a routine, and we're not testing via
	 *   generics, skip the testing.
	 *---------------------------------------------------------------------*/
	
	if (is_generic(rlib,mask1blit) && !tcb.test_via_generics)
		{
		log_bypass("mask1blit()");
		goto M1BLIT_DONE;
		}

	/*-----------------------------------------------------------------------
	 * test mask1blit...
	 *---------------------------------------------------------------------*/

	iteration_limit = (tcb.timing_only_run) ? MASK1_TIME_ITCOUNT : 1;

	log_start("Testing mask1blit()...\n");

	iteration_count = iteration_limit;

	pj_set_rast(r,  0);
	pj_set_rast(vbr,0);
	for (x = 0; x+MBPW/3 <= width; x += 2*(MBPW/3))
		{
		pj_set_rect(r,   1, x, 0, MBPW/3, height);
		pj_set_rect(vbr, 1, x, 0, MBPW/3, height);
		}

	rlib->wait_vsync(r);
	time_start();
	while (iteration_count--)
		{
		color = iteration_count * 3;
		for (y = -MBPH+3, xstart = -MBPW; y < height+MBPH; y += MBPH, xstart +=1)
			{
			for (x=xstart; x < width+MBPW; x+=MBPW)
				{
				++color;
				pj_mask1blit(markBitmap,  MBPR,
							 0, 0, r, x, y, MBPW, MBPH, color);
				}
			}
		}
	time_end();

	/* build the verification raster... */

	color = 0;
	for (y = -MBPH+3, xstart = -MBPW; y < height+MBPH; y += MBPH, xstart +=1)
		{
		for (x=xstart; x < width+MBPW; x+=MBPW)
			{
			++color;
			pj_mask1blit(markBitmap, MBPR,
						 0, 0, vbr, x, y, MBPW, MBPH, color);
			}
		}

	verify_raster(r, vbr, TRUE);

M1BLIT_EXIT:

	log_end("...testing of mask1blit() complete.\n\n");

	single_step();

M1BLIT_DONE:

	return;

}

void test_mask2blit(Raster *r)
/*****************************************************************************
 *
 ****************************************************************************/
{
Rastlib		*rlib = r->lib;
Raster		*vbr = &tcb.verification_raster;
Coor		x;
Coor		y;
Coor		xstart;
Coor		width;
Coor		height;
Pixel		color;
short		iteration_count;
short		iteration_limit;

	width = r->width;
	height = r->height;

	/*-----------------------------------------------------------------------
	 * mask2blit testing...
	 *   if the driver didn't provide a routine, and we're not testing via
	 *   generics, skip the testing.
	 *---------------------------------------------------------------------*/
	
	if (is_generic(rlib,mask2blit) && !tcb.test_via_generics)
		{
		log_bypass("mask2blit()");
		goto M2BLIT_DONE;
		}

	/*-----------------------------------------------------------------------
	 * test mask2blit...
	 *---------------------------------------------------------------------*/


	iteration_limit = (tcb.timing_only_run) ? MASK1_TIME_ITCOUNT : 1;

	log_start("Testing mask2blit()...\n");

	iteration_count = iteration_limit;

	pj_set_rast(r,  0);
	pj_set_rast(vbr,0);
	for (x = 0; x+MBPW/3 <= width; x += 2*(MBPW/3))
		{
		pj_set_rect(r,   1, x, 0, MBPW/3, height);
		pj_set_rect(vbr, 1, x, 0, MBPW/3, height);
		}

	rlib->wait_vsync(r);
	time_start();
	while (iteration_count--)
		{
		color = iteration_count * 3;
		for (y = -MBPH+3, xstart = -MBPW; y < height+MBPH; y += MBPH, xstart +=1)
			{
			for (x=xstart; x < width+MBPW; x+=MBPW)
				{
				++color;
				pj_mask2blit(markBitmap, MBPR,
						  0, 0, r, x, y, MBPW, MBPH, color, ~color);
				}
			}
		}
	time_end();

	/* build the verification raster... */

	color = 0;
	for (y = -MBPH+3, xstart = -MBPW; y < height+MBPH; y += MBPH, xstart +=1)
		{
		for (x=xstart; x < width+MBPW; x+=MBPW)
			{
			++color;
			pj_mask2blit(markBitmap, MBPR,
					  0, 0, vbr, x, y, MBPW, MBPH, color, ~color);
			}
		}

	verify_raster(r, vbr, TRUE);

	log_end("...testing of mask2blit() complete.\n\n");

	single_step();

M2BLIT_DONE:

	return;

}
