/* stfont.c - Load and display Atari ST style GEM raster fonts.
 */

#include "rastext.h"
#include "fontdev.h"
#include "errcodes.h"
#include "stfont.h"
#include "memory.h"

typedef struct stfcb
/*
 * ST Font Control block.  
 * You need one of these for each active font.
 */
	{
	struct font_hdr cfont;	/* code depends on this being 1st field */
	long cf_offset_size;
	long cf_data_size;
	} Stfcb;

#ifdef USEFUL
int print_font(char *title, Stfcb *fcb)
/* 
 * Print out font in memory as a C structure
 * (in case you want to compile it in.)
 */
{
struct font_hdr *font = &fcb->cfont;
FILE *f;
long i;
int j, offsets;

if ((f = fopen(title, "w")) == NULL)
	{
	continu_line("Couldn't open sixhi.c!");
	return(0);
	}
fprintf(f, "sixhi_data[] = {\n");
for (i=0; i<fcb->cf_data_size; i++)
	{
	fprintf(f, "0x%x,", cf_data[i]);
	if ((i&7)==7)
		fprintf(f, "\n");
	}
fprintf(f, "};\n");
offsets = fcb->cf_offset_size/sizeof(SHORT);
fprintf(f, "sixhi_ch_ofst[%d] = {\n",offsets);
for (j=0; j<offsets; j++)
	{
	fprintf(f, "%d,", fcb->cf_offsets[j]);
	if ((j&7)==7)
		fprintf(f, "\n");
	}
fprintf(f, "};\n");
fprintf(f, "struct font_hdr sixhi_font = {\n");
fprintf(f, "0x%x, %d, \"%s\", %d, %d,\n",
	fcb->cfont.id, fcb->cfont.size, fcb->cfont.facename, fcb->cfont.ADE_lo, fcb->cfont.ADE_hi);
fprintf(f, "%d, %d, %d, %d, %d,\n",
	fcb->cfont.top_dist, fcb->cfont.asc_dist, fcb->cfont.hlf_dist, fcb->cfont.des_dist,
	fcb->cfont.bot_dist);
fprintf(f, "%d, %d, %d, %d,\n",
	fcb->cfont.wchr_wdt, fcb->cfont.wcel_wdt, fcb->cfont.lft_ofst, fcb->cfont.rgt_ofst);
fprintf(f, "%d, %d, 0x%x, 0x%x,\n",
	fcb->cfont.thckning, fcb->cfont.undrline, fcb->cfont.lghtng_m, fcb->cfont.skewng_m);
fprintf(f, "0x%x, NULL, sixhi_ch_ofst, (SHORT *)sixhi_data,\n",
	fcb->cfont.flags);
fprintf(f, "%d, %d,\n", fcb->cfont.frm_wdt, fcb->cfont.frm_hgt);
fprintf(f, "NULL,};\n");
fclose(f);
return(1);
}
#endif /* USEFUL */

#ifdef USEFUL
int save_font(char *title, Stfcb *fcb)
/*
 * Save font in memory to disk.  (Untested lately.)
 */
{
struct font_hdr *font = &fcb->cfont;
int file;
long offset_size;
long data_size;

if ((fd = pj_create(title,JWRITEONLY)) == 0)
	{
	cant_create(title);
	return(0);
	}
if (pj_write(fd, font, (long)sizeof(*font)) < sizeof(*font))
	{
	truncated(title);
	goto BADEND;
	}
offset_size = font->ADE_hi - font->ADE_lo + 2;
offset_size*= sizeof(SHORT);
if (pj_write(fd, font->ch_ofst, offset_size) < offset_size )
	{
	truncated(title);
	goto BADEND;
	}
data_size = font->frm_wdt;
data_size *= font->frm_hgt;
if (pj_write(fd, font->fnt_dta, data_size) < data_size )
	{
	truncated(title);
	goto BADEND;
	}
pj_close(fd);
return(1);
BADEND:
pj_close(fd);
return(0);
}
#endif /* USEFUL */

static void _free_st_font(Stfcb *fcb)
/*
 * Release memory associated with font.
 */
{
if (fcb != NULL)
	{
	pj_gentle_free(fcb->cfont.ch_ofst);
	pj_gentle_free(fcb->cfont.fnt_dta);
	pj_free(fcb);
	}
}

static void free_st_font(Vfont *v)
/*
 * Free font and do a little bookkeeping to make sure
 * it doesn't get freed twice. 
 */
{
if (v != NULL)
	{
	_free_st_font(v->font);
	v->font = NULL;
	}
}

static Errcode verify_st_head(struct font_hdr *head)
/*
 * Make sure that recently read in font data structure
 * has plausible values.  (Alas there is no single signature
 * field.)
 */
{
int offset_size;

offset_size = head->ADE_hi - head->ADE_lo + 2;
if (offset_size > 300 || offset_size <= 2 || 
	head->wchr_wdt <= 0 || head->wchr_wdt > 1000 ||
	head->wcel_wdt < head->wchr_wdt || head->wcel_wdt > 1000 ||
	head->frm_wdt <= 0 || head->frm_hgt <= 0 || head->frm_hgt > 1000)
	{
	return(Err_bad_magic);
	}
return(Success);
}

static Errcode check_st_font(char *title)
/*
 * Given a file name try to decide whether it refers to an
 * ST type font file.
 */
{
Jfile f;
struct font_hdr h;
Errcode err = Success;

if (!suffix_in(title, ".FNT"))
	return(Err_bad_magic);
if ((f = pj_open(title, 0)) == JNONE)
	return(pj_ioerr());
if (pj_read(f, &h, sizeof(h)) == sizeof(h))
	err = verify_st_head(&h);
else
	err = Err_bad_magic;
pj_close(f);
return(err);
}

static Errcode load_st_font(char *title, Vfont *vfont, SHORT height)
/* 
 * Load up ST style font from disk into memory.
 * Height request parameter is ignored since this not
 * a scalable font.
 */
{
Errcode err;
Jfile fd;
Stfcb *fcb = NULL;
SHORT *cf_offsets;
UBYTE *cf_data;

if ((fd = pj_open(title, 0)) == JNONE)
	{
	return(pj_ioerr());
	}
if ((fcb = pj_zalloc(sizeof(*fcb))) == NULL)
	{
	err = Err_no_memory;
	goto BADEND;
	}
if (pj_read(fd, &fcb->cfont, (long)sizeof(fcb->cfont)) < sizeof(fcb->cfont))
	{
	err = Err_truncated;
	goto BADEND;
	}
if ((err = verify_st_head(&fcb->cfont)) < Success)
	goto BADEND;
fcb->cf_offset_size = fcb->cfont.ADE_hi - fcb->cfont.ADE_lo + 2;
if (fcb->cf_offset_size > 300 || fcb->cf_offset_size <= 2)
	{
	err = Err_bad_magic;
	goto BADEND;
	}
fcb->cf_offset_size*= sizeof(SHORT);
if ((cf_offsets = pj_malloc(fcb->cf_offset_size)) == NULL)
	{
	err = Err_no_memory;
	goto BADEND;
	}
if (pj_read(fd, cf_offsets, fcb->cf_offset_size) < fcb->cf_offset_size )
	{
	err = Err_truncated;
	goto BADEND;
	}
fcb->cf_data_size = fcb->cfont.frm_wdt;
fcb->cf_data_size *= fcb->cfont.frm_hgt;
if ((cf_data = pj_malloc(fcb->cf_data_size)) == NULL)
	{
	err = Err_no_memory;
	goto BADEND;
	}
if (pj_read(fd, cf_data, fcb->cf_data_size) < fcb->cf_data_size )
	{
	err = Err_truncated;
	goto BADEND;
	}
fcb->cfont.ch_ofst =  (SHORT *)cf_offsets;
fcb->cfont.fnt_dta = (SHORT *)cf_data;
if (fcb->cfont.flags & 4)	/* swapped... */
	{
	intel_swap(cf_data, fcb->cf_data_size/sizeof(SHORT));
	}
fcb->cfont.hz_ofst = NULL;
if ( ((unsigned)fcb->cfont.id)==((unsigned)0x9000))
	{
	fcb->cfont.id = MPROP;
	if ((fcb->cfont.hz_ofst = pj_malloc(fcb->cf_offset_size)) == NULL)
		{
		err = Err_truncated;
		goto BADEND;
		}
	if (pj_read(fd, fcb->cfont.hz_ofst, fcb->cf_offset_size) 
		< fcb->cf_offset_size )
		{
		err = Err_truncated;
		goto BADEND;
		}
	}
else if (((unsigned)fcb->cfont.id) == ((unsigned)0xB000))
	fcb->cfont.id = MFIXED;
else
	fcb->cfont.id = STPROP;
pj_close(fd);
init_st_vfont(vfont, fcb);
return(Success);
BADEND:
_free_st_font(fcb);
pj_close(fd);
return(err);
}

typedef union {
	  SHORT  theInt;
	  char bytes[2];
} myInt;

static Errcode st_gftext(Raster *rast,
			Vfont *vfont,
			register unsigned char *s,
			int x,int y,
			Pixel color,Text_mode tmode,
			Pixel bcolor)
/*
 * Display a string in ST style font.
 */
{
register Font_hdr *f = vfont->font;
unsigned char *ss;
unsigned char c, lo, tot;
int sx, imageWid;
SHORT *off, wd, ht, *data;
myInt *OWtab, *iPtr;
int missChar;
int font_type;
int maxx;

	maxx = rast->width;

	lo = f->ADE_lo;
	tot = f->ADE_hi-lo;
	off = f->ch_ofst;
	wd = f->frm_wdt;
	ht = f->frm_hgt,
	data = f->fnt_dta;
	OWtab= (myInt *)(f->hz_ofst);
	font_type = f->id;

	while ((c = *s++)!=0)
	{
		if (c == '\t')
			{
			x += vfont->tab_width;
			continue;
			}
		if ((c -= lo) > tot)
			c = tot;
		/* Mac prop font && its a missing char */
		if (font_type == MPROP && (*(OWtab+c)).theInt == -1)
		{
			c=tot;                      /* last char is set */
			missChar=1;
			sx = off[c+1];
			imageWid= f->frm_wdt*8 - sx;  /* sort of a kludge */
		}
		else
		{
			missChar=0;
			sx = off[c];
			imageWid = off[c+1]-sx;
		}
		if(x > maxx)
			break;
		(*blit_for_mode[tmode])(data, wd, sx, 0, rast, x, y, imageWid, ht, color, bcolor);
		switch (font_type)
		{
			case STPROP:
				x += imageWid;
				break;
			case MFIXED:
				x += f->wcel_wdt;
				break;
			case MPROP:
			{
				iPtr=OWtab+c;
				if (!missChar)
					/* -1 means its a missing character */
				{
					x += (int)((*iPtr).bytes[1]);
					ss=s;
					if ((c=*(ss++)) != 0)
						/* look to next char to determine amt to change x */
					{
						if ((c-= lo) > tot)
							c = tot;
						iPtr=OWtab+c;
						/* subtract kern Of Next char */
						/* f->rgt_ofst is neg of Mac maxKern value */
						if ((*iPtr).theInt!=-1)
						   x += (int)((*iPtr).bytes[0])+ f->rgt_ofst;
					}           
			    }
				else /* display the non print char */
					x+=imageWid;
				break;
			}
		}
		x += vfont->spacing;
	}
	return(Success);
}

static int st_fchar_spacing(Vfont *vfont,char *s)
/*
 * Returns width of character cel 
 */
{
register Font_hdr *f = vfont->font;
unsigned char c, lo, tot;
char *offsets;
int width;
int t;

	lo = f->ADE_lo;
	tot = f->ADE_hi - lo;
	c = *s++;
	if ((c -= lo) > tot)
		c = tot;
	switch (f->id)
	{
		case STPROP:
            return(f->ch_ofst[c+1] - f->ch_ofst[c] + vfont->spacing);
		case MPROP:
		{
			offsets = f->hz_ofst+c*2;
			if (offsets[0] == -1 && offsets[1] == -1)	/* missing char */
			{
				t = tot;
                return( f->frm_wdt*8 - f->ch_ofst[t+1] + vfont->spacing);
			}
			else
			{
				width = offsets[1];
				if ((c = *s) != 0)
				{
					if ((c -= lo) > tot)
						c = tot;
					offsets = f->hz_ofst+c*2;
					width += offsets[0] + f->rgt_ofst;
				}
                return(width + vfont->spacing);
			}
		}
	}
}

static int st_widest_char(Vfont *vfont)
/*
 * Returns width of widest char in font.
 */
{
Font_hdr *f = vfont->font;
char buf[2];
int i;
int c;
int widest = 1;
int w;

if (f->id == MFIXED)
	return(f->wcel_wdt);
c = f->ADE_lo;
i = f->ADE_hi - c;
buf[1] = 0;
while (--i >= 0)
	{
	buf[0] = c++;
	w = fchar_spacing(vfont, buf);
	if (w > widest)
		widest = w;
	}
return(widest);
}

static Boolean st_in_font(Vfont *v, int c)
/*
 * Returns TRUE if a letter is in font.
 */
{
Font_hdr *f = v->font;

return(c >= f->ADE_lo && c <= f->ADE_hi);
}

init_st_vfont(Vfont *vfont, Stfcb *stf)
/*
 * Fill in our virtual font structure (which handles many different
 * font types) with functions and data to display an ST style font.
 */
{
int leading;

clear_struct(vfont);
vfont->type = STFONT;
vfont->font = stf;
vfont->gftext = st_gftext;
vfont->close_vfont = free_st_font;
vfont->char_width = st_fchar_spacing;
vfont->in_font = st_in_font;
vfont->image_height = stf->cfont.frm_hgt;
leading = (stf->cfont.frm_hgt+3)>>2;
vfont->line_spacing = vfont->image_height+leading;
vfont->default_leading = vfont->leading = leading;
vfont->widest_image = vfont->widest_char = st_widest_char(vfont);
vfont->tab_width = TABEXP*vfont->widest_char;
if (stf->cfont.id == MFIXED)
	vfont->flags = VFF_MONOSPACE;
scan_init_vfont(vfont);

}


Font_dev st_font_dev = 
/*
 * Font "device" for ST fonts.  Each font type has one of these.
 */
{
NULL,
"Animator 1.0",
"*.FNT",
check_st_font,
load_st_font,
STFONT,
};
