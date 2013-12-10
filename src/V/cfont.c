
/* cfont.c  - Stuff to load and free up the 'user' (as opposed to built 
	in system) font */
#include <stdio.h>
#include <string.h>
#include "jimk.h"
#include "cfont.str"
#include "gemfont.h"
#include "jfile.h"
#include "memory.h"

extern unsigned read();

static struct font_hdr cfont;
static FILE *fd;
static long cf_offset_size;
static long cf_data_size;
static WORD *cf_offsets;
static UBYTE *cf_data;
extern struct font_hdr sixhi_font; 
extern struct font_hdr *usr_font;

static void
intel_swap(pt, count)
register char *pt;
int count;
{
register char swap;

while (--count >= 0)
	{
	swap = pt[1];
	pt[1] = pt[0];
	pt[0] = swap;
	pt += 2;
	}
}

/* Release memory associated with user font.  Point user font back to
   compiled-in font */
free_cfont()
{
if (cf_offsets != NULL)
	{
	freemem(cf_offsets);
	cf_offsets = NULL;
	}
if (cf_data != NULL)
	{
	freemem(cf_data);
	cf_data = NULL;
	}
usr_font = &sixhi_font;
}

#ifdef USEFUL
/* make a magnetic copy of compiled-in font */
print_font(title, font)
char *title;
struct font_hdr *font;
{
FILE *f;
long i;
int j, offsets;

if ((f = fopen(title, "w")) == NULL)
	{
	continu_line("Couldn't open sixhi.c!");
	return(0);
	}
fprintf(f, "sixhi_data[] = {\n");
for (i=0; i<cf_data_size; i++)
	{
	fprintf(f, "0x%x,", cf_data[i]);
	if ((i&7)==7)
		fprintf(f, "\n");
	}
fprintf(f, "};\n");
offsets = cf_offset_size/sizeof(WORD);
fprintf(f, "sixhi_ch_ofst[%d] = {\n",offsets);
for (j=0; j<offsets; j++)
	{
	fprintf(f, "%d,", cf_offsets[j]);
	if ((j&7)==7)
		fprintf(f, "\n");
	}
fprintf(f, "};\n");
fprintf(f, "struct font_hdr sixhi_font = {\n");
fprintf(f, "0x%x, %d, \"%s\", %d, %d,\n",
	cfont.id, cfont.size, cfont.facename, cfont.ADE_lo, cfont.ADE_hi);
fprintf(f, "%d, %d, %d, %d, %d,\n",
	cfont.top_dist, cfont.asc_dist, cfont.hlf_dist, cfont.des_dist,
	cfont.bot_dist);
fprintf(f, "%d, %d, %d, %d,\n",
	cfont.wchr_wdt, cfont.wcel_wdt, cfont.lft_ofst, cfont.rgt_ofst);
fprintf(f, "%d, %d, 0x%x, 0x%x,\n",
	cfont.thckning, cfont.undrline, cfont.lghtng_m, cfont.skewng_m);
fprintf(f, "0x%x, NULL, sixhi_ch_ofst, (WORD *)sixhi_data,\n",
	cfont.flags);
fprintf(f, "%d, %d,\n", cfont.frm_wdt, cfont.frm_hgt);
fprintf(f, "NULL,};\n");
fclose(f);
return(1);
}
#endif /* USEFUL */

#ifdef USEFUL
/* make a magnetic copy of a font in memory (Untested lately) */
save_font(title, font)
char *title;
struct font_hdr *font;
{
int file;
long offset_size;
long data_size;

if ((fd = jcreate(title)) == 0)
	{
	cant_create(title);
	return(0);
	}
if (jwrite(fd, font, (long)sizeof(*font)) < sizeof(*font))
	{
	truncated(title);
	goto BADEND;
	}
offset_size = font->ADE_hi - font->ADE_lo + 2;
offset_size*= sizeof(WORD);
if (jwrite(fd, font->ch_ofst, offset_size) < offset_size )
	{
	truncated(title);
	goto BADEND;
	}
data_size = font->frm_wdt;
data_size *= font->frm_hgt;
if (jwrite(fd, font->fnt_dta, data_size) < data_size )
	{
	truncated(title);
	goto BADEND;
	}
jclose(fd);
return(1);
BADEND:
jclose(fd);
return(0);
}
#endif /* USEFUL */

static int
load_font_header(FILE *fd, struct font_hdr *fnt)
{
	unsigned int size = 0;
	char dummy[4];

	size += jread(fd, &fnt->id, sizeof(fnt->id));
	size += jread(fd, &fnt->size, sizeof(fnt->size));
	size += jread(fd,  fnt->facename, sizeof(fnt->facename));
	size += jread(fd, &fnt->ADE_lo, sizeof(fnt->ADE_lo));
	size += jread(fd, &fnt->ADE_hi, sizeof(fnt->ADE_hi));
	size += jread(fd, &fnt->top_dist, sizeof(fnt->top_dist));
	size += jread(fd, &fnt->asc_dist, sizeof(fnt->asc_dist));
	size += jread(fd, &fnt->hlf_dist, sizeof(fnt->hlf_dist));
	size += jread(fd, &fnt->des_dist, sizeof(fnt->des_dist));
	size += jread(fd, &fnt->bot_dist, sizeof(fnt->bot_dist));
	size += jread(fd, &fnt->wchr_wdt, sizeof(fnt->wchr_wdt));
	size += jread(fd, &fnt->wcel_wdt, sizeof(fnt->wcel_wdt));
	size += jread(fd, &fnt->lft_ofst, sizeof(fnt->lft_ofst));
	size += jread(fd, &fnt->rgt_ofst, sizeof(fnt->rgt_ofst));
	size += jread(fd, &fnt->thckning, sizeof(fnt->thckning));
	size += jread(fd, &fnt->undrline, sizeof(fnt->undrline));
	size += jread(fd, &fnt->lghtng_m, sizeof(fnt->lghtng_m));
	size += jread(fd, &fnt->skewng_m, sizeof(fnt->skewng_m));
	size += jread(fd, &fnt->flags, sizeof(fnt->flags));
	size += jread(fd,  dummy, 4); /* hz_ofst */
	size += jread(fd,  dummy, 4); /* ch_ofst */
	size += jread(fd,  dummy, 4); /* fnt_dta */
	size += jread(fd, &fnt->frm_wdt, sizeof(fnt->frm_wdt));
	size += jread(fd, &fnt->frm_hgt, sizeof(fnt->frm_hgt));
	size += jread(fd,  dummy, 4); /* nxt_fnt */

	return (size == SIZEOF_FONT_HDR);
}

/* Load up a font chosen by user. */
static int
load_cfont(char *title)
{
char *cf_OWtab;  /*ldg */

free_cfont();
if ((fd = jopen(title, 0)) == 0)
	{
	cant_find(title);
	return(0);
	}
if (!load_font_header(fd, &cfont))
	{
	truncated(title);
	goto BADEND;
	}
cf_offset_size = cfont.ADE_hi - cfont.ADE_lo + 2;
if (cf_offset_size > 300 || cf_offset_size <= 2)
	{
	continu_line(cfont_100 /* "Bad font file format" */);
	goto BADEND;
	}
cf_offset_size*= sizeof(WORD);
if ((cf_offsets = lbegmem(cf_offset_size)) == NULL)
	{
	goto BADEND;
	}
if (jread(fd, cf_offsets, cf_offset_size) < cf_offset_size )
	{
	truncated(title);
	goto BADEND;
	}
cf_data_size = cfont.frm_wdt;
cf_data_size *= cfont.frm_hgt;
if ((cf_data = lbegmem(cf_data_size)) == NULL)
	{
	goto BADEND;
	}
if (jread(fd, cf_data, cf_data_size) < cf_data_size )
	{
	truncated(title);
	goto BADEND;
	}
cfont.ch_ofst =  (WORD *)cf_offsets;
cfont.fnt_dta = (WORD *)cf_data;
if (cfont.flags & 4)	/* swapped... */
	{
	intel_swap(cf_data, cf_data_size/sizeof(WORD));
	}
cfont.hz_ofst = NULL;
if ( ((unsigned)cfont.id)==((unsigned)0x9000))
	{
	cfont.id = MPROP;
	if ((cfont.hz_ofst = begmem(cf_offset_size)) == NULL)
		{
		goto BADEND;
		}
	if (jread(fd, cfont.hz_ofst, cf_offset_size) < cf_offset_size )
		{
		truncated(title);
		freemem(cfont.hz_ofst);
		goto BADEND;
		}
	}
else if (((unsigned)cfont.id) == ((unsigned)0xB000))
	cfont.id = MFIXED;
else
	cfont.id = STPROP;
jclose(fd);
return(1);
BADEND:
free_cfont();
jclose(fd);
return(0);
}

static void
use_sysfont(void)
{
usr_font = &sixhi_font;
strcpy(vs.fonts[0], cfont_101 /* "SYSTEM" */);
}

/* Try to load a font from disk.  If fail point user font back to compiled
   in font. */
load_install_font(fname)
char *fname;
{
int ok;

if (load_cfont(fname) )
	{
	usr_font = &cfont;
	ok = 1;
	}
else
	{
	use_sysfont();
	ok = 0;
	}
return(ok);
}

/* Load up font our state variable says we're using */
grab_usr_font()
{
char fname[14];
char buf[81];
int ok;

sprintf(fname, "%s.FNT", vs.fonts[0]);
make_path_name(vconfg.font_drawer, fname, buf);
if (!jexists(buf))
	use_sysfont();
else
	load_install_font(buf);
}

