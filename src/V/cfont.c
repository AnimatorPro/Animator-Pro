
/* cfont.c  - Stuff to load and free up the 'user' (as opposed to built 
	in system) font */
#include <stdio.h>
#include "jimk.h"
#include "gemfont.h"
#include "cfont.str"

extern unsigned read();

static struct font_hdr cfont;
static fd;
static long cf_offset_size;
static long cf_data_size;
static WORD *cf_offsets;
static UBYTE *cf_data;
extern struct font_hdr sixhi_font; 
extern struct font_hdr *usr_font;

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
#endif USEFUL

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
#endif USEFUL

/* Load up a font chosen by user. */
static
load_cfont(title)
char *title;
{
char *cf_OWtab;  /*ldg */

free_cfont();
if ((fd = jopen(title, 0)) == 0)
	{
	cant_find(title);
	return(0);
	}
if (jread(fd, &cfont, (long)sizeof(cfont)) < sizeof(cfont))
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

static 
use_sysfont()
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

