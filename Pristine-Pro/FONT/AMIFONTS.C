
#include "stdtypes.h"
#include "errcodes.h"
#include "memory.h"
#include "jfile.h"
#include "rastcall.h"
#include "rastext.h"
#include "fontdev.h"
#include "amifonts.h"


static void free_ami_font(Vfont *vfont)
{
Afcb *fcb;

if ((fcb = vfont->font) != NULL)
	{
	pj_gentle_free(fcb->image);
	pj_gentle_free(fcb->loc);
	pj_gentle_free(fcb->spacing);
	pj_gentle_free(fcb->kerning);
	pj_free(fcb);
	}
}

static Errcode verify_ami_head(Afont_head *h)
{
if (h->h1.ln_type != AMIF_TYPE)
	return(Err_bad_magic);
if (h->h2.fln_type != AMIF_TYPE)
	return(Err_bad_magic);
intel_swap(&h->h1.dfh_id, 2);
if (h->h1.dfh_id != 0x0f80)	/* not yet swapped */
	return(Err_bad_magic);
intel_swap(&h->h2.mn_length, 2);
intel_swap(&h->h2.tf_xsize, 4);
intel_swap(&h->h2.tf_modulo, 1);
intel_dswap(&h->h2.tf_chardata, 1);
intel_dswap(&h->h2.tf_charloc, 3);
return(Success);
}

static Errcode check_ami_font(char *name)
{
Jfile f;
Afont_head ah;
Errcode err;

if ((f = pj_open(name, JREADONLY)) == JNONE)
	return(pj_ioerr());
if (pj_read(f, &ah, sizeof(ah)) != sizeof(ah))
	{
	err = Err_truncated;
	goto OUT;
	}
err = verify_ami_head(&ah);
OUT:
pj_close(f);
return(err);
}



static ami_char_width(Vfont *v, UBYTE *s)
{
Afcb *fcb = v->font;
UBYTE c = s[0];

if (fcb->is_prop)
	{
	c = oem_to_ansi[c];
	if (c < fcb->head.h2.tf_lochar || c > fcb->head.h2.tf_hichar)
		c = fcb->head.h2.tf_hichar+1;
	c -= fcb->head.h2.tf_lochar;
	return(fcb->spacing[c] + v->spacing);
	}
else
	return(fcb->head.h2.tf_xsize + v->spacing);
}

static Boolean ami_in_font(Vfont *v, int c)
{
Afcb *fcb = v->font;

c = oem_to_ansi[c];
return(c >= fcb->head.h2.tf_lochar && c <= fcb->head.h2.tf_hichar);
}

static Errcode ami_gftext(Raster *rast,
			Vfont *vfont,
			register unsigned char *s,
			int x,int y,
			Pixel color,Text_mode tmode,
			Pixel bcolor)
{
Afcb *fcb = vfont->font;
unsigned char c;
int x1;
unsigned char hi,lo;
int maxx;

	maxx = rast->width;

hi = fcb->head.h2.tf_hichar;
lo = fcb->head.h2.tf_lochar;
while ((c = oem_to_ansi[*s++]) != 0)
	{
	if (c == '\t')
		{
		x += vfont->tab_width;
		continue;
		}
	if (c < lo || c > hi)
		c = hi+1;
	c -= lo;
	x1 = x;
	if (fcb->is_kerned)
		x1 += fcb->kerning[c];

	if(x > maxx)
		break;
	(*blit_for_mode[tmode])(fcb->image, fcb->head.h2.tf_modulo, fcb->loc[c].startx, 0,
		rast, x1, y,
		fcb->loc[c].width, fcb->head.h2.tf_ysize, color, bcolor);
	if (fcb->is_prop)
		x += fcb->spacing[c] + vfont->spacing;
	else
		x += fcb->head.h2.tf_xsize + vfont->spacing;
	}
return(Success);
}

#define AMIF_SEEK_OFF 0x20		/* Sometime they added 20 bytes to the header */

static Errcode load_ami_font(char *title, Vfont *vfont, SHORT height)
{
Jfile f = 0;
Errcode err;
Afcb *fcb;
long image_size;
char buf[16];
int char_count;
long rsz;

clear_struct(vfont);
if ((fcb = pj_zalloc(sizeof(*fcb))) == NULL)
	{
	err = Err_no_memory;
	goto ERROR;
	}
vfont->font = fcb;
if ((f = pj_open(title, JREADONLY)) == JNONE)
	{
	err = pj_ioerr();
	goto ERROR;
	}
if (pj_read(f, &fcb->head, sizeof(fcb->head)) != sizeof(fcb->head))
	{
	err = Err_truncated;
	goto ERROR;
	}
if ((err = verify_ami_head(&fcb->head)) < Success)
	goto ERROR;
image_size = fcb->head.h2.tf_ysize*fcb->head.h2.tf_modulo;
if ((fcb->image = pj_malloc(image_size)) == NULL)
	{
	err = Err_no_memory;
	goto ERROR;
	}
pj_seek(f, fcb->head.h2.tf_chardata+AMIF_SEEK_OFF, JSEEK_START);
if (pj_read(f, fcb->image, image_size) != image_size)
	{
	err = Err_truncated;
	goto ERROR;
	}
pj_seek(f, fcb->head.h2.tf_charloc+AMIF_SEEK_OFF, JSEEK_START);
char_count = fcb->head.h2.tf_hichar - fcb->head.h2.tf_lochar + 2;
rsz = (long)char_count * sizeof(Font_loc);
if ((fcb->loc = pj_malloc(rsz)) == NULL)
	{
	err = Err_no_memory;
	goto ERROR;
	}
if (pj_read(f, fcb->loc, rsz) != rsz)
	{
	err = Err_truncated;
	goto ERROR;
	}
intel_swap(fcb->loc, (int)rsz>>1);
if (fcb->head.h2.tf_charspace != 0)	/* got spacing data */
	{
	fcb->is_prop = TRUE;
	pj_seek(f, fcb->head.h2.tf_charspace+AMIF_SEEK_OFF, JSEEK_START);
	rsz = (long)char_count * sizeof(SHORT);
	if ((fcb->spacing = pj_malloc(rsz)) == NULL)
		{
		err = Err_no_memory;
		goto ERROR;
		}
	if (pj_read(f, fcb->spacing, rsz) != rsz)
		{
		err = Err_truncated;
		goto ERROR;
		}
	intel_swap(fcb->spacing, (int)rsz>>1);
	}
if (fcb->head.h2.tf_charkern != 0)
	{
	fcb->is_kerned = TRUE;
	pj_seek(f, fcb->head.h2.tf_charkern+AMIF_SEEK_OFF, JSEEK_START);
	rsz = (long)char_count * sizeof(SHORT);
	if ((fcb->kerning = pj_malloc(rsz)) == NULL)
		{
		err = Err_no_memory;
		goto ERROR;
		}
	if (pj_read(f, fcb->kerning, rsz) != rsz)
		{
		err = Err_truncated;
		goto ERROR;
		}
	intel_swap(fcb->kerning, (int)rsz>>1);
	}
pj_close(f);     
vfont->type = AMIFONT;
vfont->gftext = ami_gftext;
vfont->close_vfont = free_ami_font;
vfont->char_width = ami_char_width;
vfont->in_font = ami_in_font;
vfont->tab_width = 
	vfont->widest_image = vfont->widest_char = fcb->head.h2.tf_xsize;
vfont->tab_width *= TABEXP;
vfont->line_spacing = vfont->image_height = (fcb->head.h2.tf_ysize);
vfont->leading = vfont->default_leading = vfont->image_height/3;
vfont->line_spacing += vfont->leading;
scan_init_vfont(vfont);
return(Success);
ERROR:
if (f != 0)
	pj_close(f);     
free_ami_font(vfont);
return(err);
}

Font_dev ami_font_dev = {
NULL,
"Amiga Mono",
"*.*",
check_ami_font,
load_ami_font,
AMIFONT,
};
