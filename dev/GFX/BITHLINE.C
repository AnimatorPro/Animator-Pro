
static unsigned char lmasks[] = {0xff,0x7f,0x3f,0x1f,0x0f,0x07,0x03,0x01};
static unsigned char rmasks[] = {0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe,0xff};
unsigned char bit_masks[] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};


void set_bit_hline(unsigned char *buf, 
	unsigned int bpr, unsigned int y, unsigned int x1, unsigned int x2)
/* Set a horizontal line on a bitplane.  No clipping, and x1 better
 * be less than or equal to x2! */
{
register unsigned char *pt;
unsigned int xbyte;
int bcount;

	xbyte = (x1>>3);
	pt = buf + ((y*bpr) + xbyte);
	if((bcount = (x2>>3)-xbyte) == 0)
	{
		*pt |= (lmasks[x1&7] & rmasks[x2&7]);
	}
	else
	{
		*pt++ |= lmasks[x1&7];
		while (--bcount > 0)
			*pt++ = 0xff;
		*pt |= rmasks[x2&7];
	}
}
