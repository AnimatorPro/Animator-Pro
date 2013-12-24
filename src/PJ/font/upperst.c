#include <stdio.h>
#include "jimk.h"
#include "rastext.h"
#include "font\\fontdev.h"
#include "errcodes.h"
#include "font\\stfont.h"
#include "memory.h"

static void move_image(UBYTE inc, UBYTE *data,int sx,int dx,int wd, 
					   int ht, int bpr)
{
UBYTE *src;
UBYTE *dst;
UBYTE dmask[2];
USHORT line;

	/* only does 6 by 6 with upper and lower case at same bit offset in byte */
	if(wd > 8)
		return;
	src = data + sx/8;
	dst = data + dx/8;

	dmask[0] = 0xFC >> (dx%8);
	dmask[1] = 0xFC << (8-(dx%8));

	boxf("char |%c| ht %d wd %d\n"
	     "sx %d dx %d sx/8 %d dx/8 %d sx%%8 %d dx%%8 %d\n"
		 "abcdefghijklmnopqrstuvwxyz\n"
		 "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 
		 inc, ht, wd, sx, dx ,sx/8, dx/8, sx%8, dx%8);

	while(ht-- > 0)
	{
		line = *((USHORT *)src);
		*((USHORT *)dst) &= ~(*((USHORT *)dmask));
		*((USHORT *)dst) |=  (line & *((USHORT *)dmask));
		src += bpr;
		dst += bpr;
	}
}
static void st_copy_images(Vfont *vfont,UBYTE *in,UBYTE *out)
{
register Font_hdr *f = vfont->font;
UBYTE in_c, out_c, lo, hi;
int sx, dx, imageWid;
SHORT *off, wd, ht, *data;
int missChar;
int font_type;

	lo = f->ADE_lo;
	hi = f->ADE_hi;
	off = f->ch_ofst;
	wd = f->frm_wdt;
	ht = f->frm_hgt,
	data = f->fnt_dta;
	font_type = f->id;

	if (font_type != MFIXED)
		return;

	while((in_c = *in++)!=0 && (out_c = *out++) != 0)
	{
		if (in_c == '\t')
			continue;
		if (in_c > f->ADE_hi)
			in_c = f->ADE_hi;
		in_c -= lo;

		if (out_c == '\t')
			continue;
		if (out_c > f->ADE_hi)
			out_c = f->ADE_hi;
		out_c -= lo;

		if(in_c == out_c)
			continue;

		missChar=0;
		sx = off[in_c];
		imageWid = off[in_c+1]-sx;
		dx = off[out_c];
		if(off[out_c+1]-dx != imageWid)
		{
			boxf("different width");
			continue;
		}
		move_image(in_c,(UBYTE *)data,sx,dx,imageWid,ht,wd);
	}
}
test()
{
	boxf("setting font to upper case only");
	st_copy_images(get_sys_font(),
				   "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
				   "abcdefghijklmnopqrstuvwxyz" );
	write_sixhi_data();
}
