
#include "jimk.h"

/* blit.c - bitplane oriented blit.  In C.  It's slow.  But gets the
   job done for MacIntosh, ST, Amiga, PCPaintBrush, ... bitplane oriented
   file formats. */

unsigned char 
#ifdef LATER
		lmasks[8] = {0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff},
		rmasks[8] = {0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01},
#endif /* LATER */
		irmasks[8] = {	~0x80, ~0xc0, ~0xe0, ~0xf0, 
						~0xf8, ~0xfc, ~0xfe, ~0xff},
		ilmasks[8] = {	~0xff, ~0x7f, ~0x3f, ~0x1f, 
						~0x0f, ~0x07, ~0x03, ~0x01}
		;

blit_box(w, h, sx, sy, s, sbpr, dx, dy, d, dbpr)
int w, h;
int sx,sy,sbpr,dx,dy,dbpr;
unsigned char *s,*d;
{
int ssbit, dsbit;
int debit;
int sstart;
int dstart, dend;
unsigned char c;
unsigned char lmask, rmask;
int dcount;
unsigned shacc;
int shift;
int dl, sl;
int dct;


ssbit = sx&7;
dsbit = dx&7;
sstart = sx>>3;
dstart = dx>>3;
debit = dx + w - 1;
dend = (debit)>>3;
debit &= 7;
lmask = ilmasks[dsbit];
rmask = irmasks[debit];
dcount = dend - dstart;
shift = dsbit - ssbit;
s += sbpr*sy;
d += dbpr*dy;
if (shift == 0)		/* don't shift */
	{
	if (dcount == 0)
		{
		lmask |= rmask;
		while (--h >= 0)
			{
			d[dstart] = ((d[dstart]&lmask) | (s[sstart]&(~lmask)));
			dstart += dbpr;
			sstart += sbpr;
			}
		}
	else
		{
		while (--h >= 0)
			{
			dl = dstart;
			sl = sstart;
			dct = dcount;
			d[dl] = ((d[dl]&lmask) | (s[sl]&(~lmask)));
			dl++;
			sl++;
			while (--dct > 0)	/* loop dcount - 1 times */
				{
				d[dl++] = s[sl++];
				}
			d[dl] = ((d[dl]&rmask) | (s[sl]&(~rmask)));
			dstart += dbpr;
			sstart += sbpr;
			}
		}
	}
else if (shift > 0)	/* shift right */
	{
	if (dcount == 0)
		{
		lmask |= rmask;
		while (--h >= 0)
			{
			shacc = (s[sstart++]<<8);
			shacc += s[sstart];
			shacc >>= shift;
			d[dstart] = ((d[dstart]&lmask) | (shacc&(~lmask)));
			dstart += dbpr;
			sstart += sbpr;
			}
		}
	else
		{
		while (--h >= 0)
			{
			sl = sstart;
			dl = dstart;
			dct  = dcount;
			/*
			shacc = (s[sl++]<<8);
			shacc += s[sl];
			*/
			shacc = s[sl];
			shacc >>= shift;
			d[dl] = ((d[dl]&lmask) | (shacc&(~lmask)));
			dl++;
			while (--dct > 0)	/* loop dcount - 1 times */
				{
				shacc = (s[sl++]<<8);
				shacc += s[sl];
				shacc >>= shift;
				d[dl++] = shacc;
				}
			shacc = (s[sl++]<<8);
			shacc += s[sl];
			shacc >>= shift;
			d[dl] = ((d[dl]&rmask) | (shacc&(~rmask)));
			sstart += sbpr;
			dstart += dbpr;
			}
		}
	}
else					/* shift left */
	{
	if (dcount == 0)
		{
		lmask |= rmask;
		while (--h >= 0)
			{
			shacc = (s[sstart]<<8);
			shacc += s[sstart+1];
			shacc <<= shift;
			shacc >>= 8;
			d[dstart] = (((d[dstart]&lmask) | (shacc&(~lmask))));
			sstart += sbpr;
			dstart += dbpr;
			}
		}
	else
		{
		while (--h >= 0)
			{
			sl = sstart;
			dl = dstart;
			dct = dcount;
			shacc = (s[sl++]<<8);
			shacc += s[sl];
			shift = -shift;
			shacc <<= shift;
			shacc >>= 8;
			d[dl] = (((d[dl]&lmask) | (shacc&(~lmask))));
			dl++;
			while (--dct > 0)	/* loop dcount - 1 times */
				{
				shacc = ((unsigned)s[sl++]<<8);
				shacc += s[sl];
				shacc <<= shift;
				d[dl++] = (shacc>>8);
				}
			shacc = (s[sl++]<<8);
			shacc += s[sl];
			shacc <<= shift;
			shacc >>= 8;
			d[dl] = (((d[dl]&rmask) | (shacc&(~rmask))));
			sstart += sbpr;
			dstart += dbpr;
			}
		}
	}
}

xor_blit_box(w, h, sx, sy, s, sbpr, dx, dy, d, dbpr)
int w, h;
int sx,sy,sbpr,dx,dy,dbpr;
unsigned char *s,*d;
{
int ssbit, dsbit;
int debit;
int sstart;
int dstart, dend;
unsigned char c;
unsigned char lmask, rmask;
int dcount;
unsigned shacc;
int shift;
int dl, sl;
int dct;


ssbit = sx&7;
dsbit = dx&7;
sstart = sx>>3;
dstart = dx>>3;
debit = dx + w - 1;
dend = (debit)>>3;
debit &= 7;
lmask = ilmasks[dsbit];
rmask = irmasks[debit];
dcount = dend - dstart;
shift = dsbit - ssbit;
s += sbpr*sy;
d += dbpr*dy;
if (shift == 0)		/* don't shift */
	{
	if (dcount == 0)
		{
		lmask |= rmask;
		while (--h >= 0)
			{
			d[dstart] = ((d[dstart]) ^ (s[sstart]&(~lmask)));
			dstart += dbpr;
			sstart += sbpr;
			}
		}
	else
		{
		while (--h >= 0)
			{
			dl = dstart;
			sl = sstart;
			dct = dcount;
			d[dl] = ((d[dl]) ^ (s[sl]&(~lmask)));
			dl++;
			sl++;
			while (--dct > 0)	/* loop dcount - 1 times */
				{
				d[dl++] ^= s[sl++];
				}
			d[dl] = ((d[dl]) ^ (s[sl]&(~rmask)));
			dstart += dbpr;
			sstart += sbpr;
			}
		}
	}
else if (shift > 0)	/* shift right */
	{
	if (dcount == 0)
		{
		lmask |= rmask;
		while (--h >= 0)
			{
			shacc = (s[sstart++]<<8);
			shacc += s[sstart];
			shacc >>= shift;
			d[dstart] = ((d[dstart]) ^ (shacc&(~lmask)));
			dstart += dbpr;
			sstart += sbpr;
			}
		}
	else
		{
		while (--h >= 0)
			{
			sl = sstart;
			dl = dstart;
			dct  = dcount;
			/*
			shacc = (s[sl++]<<8);
			shacc += s[sl];
			*/
			shacc = s[sl];
			shacc >>= shift;
			d[dl] = ((d[dl]) ^ (shacc&(~lmask)));
			dl++;
			while (--dct > 0)	/* loop dcount - 1 times */
				{
				shacc = (s[sl++]<<8);
				shacc += s[sl];
				shacc >>= shift;
				d[dl++] ^= shacc;
				}
			shacc = (s[sl++]<<8);
			shacc += s[sl];
			shacc >>= shift;
			d[dl] = ((d[dl]) ^ (shacc&(~rmask)));
			sstart += sbpr;
			dstart += dbpr;
			}
		}
	}
else					/* shift left */
	{
	if (dcount == 0)
		{
		lmask |= rmask;
		while (--h >= 0)
			{
			shacc = (s[sstart]<<8);
			shacc += s[sstart+1];
			shacc <<= shift;
			shacc >>= 8;
			d[dstart] = (((d[dstart]) ^ (shacc&(~lmask))));
			sstart += sbpr;
			dstart += dbpr;
			}
		}
	else
		{
		while (--h >= 0)
			{
			sl = sstart;
			dl = dstart;
			dct = dcount;
			shacc = (s[sl++]<<8);
			shacc += s[sl];
			shift = -shift;
			shacc <<= shift;
			shacc >>= 8;
			d[dl] = (((d[dl]) ^ (shacc&(~lmask))));
			dl++;
			while (--dct > 0)	/* loop dcount - 1 times */
				{
				shacc = ((unsigned)s[sl++]<<8);
				shacc += s[sl];
				shacc <<= shift;
				d[dl++] ^= (shacc>>8);
				}
			shacc = (s[sl++]<<8);
			shacc += s[sl];
			shacc <<= shift;
			shacc >>= 8;
			d[dl] = (((d[dl]) ^ (shacc&(~rmask))));
			sstart += sbpr;
			dstart += dbpr;
			}
		}
	}
}



