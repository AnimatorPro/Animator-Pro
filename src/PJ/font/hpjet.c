#define VFONT_C
#include "errcodes.h"
#include "fontdev.h"
#include "hpjet.h"
#include "lstdio.h"
#include "memory.h"
#include "rastcall.h"
#include "rastext.h"
#include "util.h"

#define CMAX 256

static void _free_hpjet_font(Hfcb *fcb)
{
Hpj_letter **s;
int i;

if (fcb != NULL)
	{
	if ((s = fcb->letters) != NULL)
		{
		i = CMAX;
		while (--i >= 0)
			pj_gentle_free(*s++);
		pj_free(fcb->letters);
		}
	pj_free(fcb);
	}
}

static void free_hpjet_font(Vfont *v)
{
_free_hpjet_font(v->font);
v->font = NULL;
}

static Errcode hpjet_get_number(FILE *f, char end_tag, long *result)
/* return ascii represented number starting at current file position and
   ending with end_tag into result */
{
int c;
long acc = 0;
Errcode err = Success;

for (;;)
	{
	if ((c = getc(f)) < Success)
		break;
	if (c == end_tag)
		break;
	c -= '0';
	if (c < 0 || c > '9')
		{
		err = Err_format;
		break;
		}
	acc *= 10;
	acc += c;
	}
*result = acc;
return(err);
}


static Errcode verify_hpjet_font(FILE *f)
/* Check that file begins with <esc> ) s    signature */
{
char buf[3];

if (fread(buf, 1, sizeof(buf), f) < sizeof(buf))
	return(Err_truncated);
if (buf[0] != 0x1b && buf[1] != ')' && buf[2] != 's')
	return(Err_bad_magic);
return(Success);
}

static Errcode check_hpjet_font(char *name)
/* Open font file and check signature */
{
Errcode err;
FILE *f;

if ((f = fopen(name, "rb")) == NULL)
	return(errno);
err = verify_hpjet_font(f);
fclose(f);
return(err);
}


static Errcode seek_past_first(FILE *f, char *s, int slen)
/* Given a pattern s, seek until just past first occurence of pattern in
 * file.  This algorithm won't work perfectly if the first character in
 * the pattern occurs later on.  It's good enough for our uses here though. */
{
int c;
int firstc = *s++;
char *pt;
int i;

slen -= 1;
for (;;)
	{
	if ((c = getc(f)) < Success)
		return(Err_eof);
CHECK_FIRSTC:
	if (c == firstc)
		{
		pt = s;
		i = slen;
		while (--i >= 0)
			{
			if ((c = getc(f)) < Success)
				return(Err_eof);
			if (c != *pt++)
				goto CHECK_FIRSTC;
			}
		return(Success);
		}
	}
}

static Errcode hpjet_read_chars(FILE *f, Hfcb *fcb)
{
char buf[3];
static char char_sig[] = {0x1b, '*', 'c'};
long csig1_num;
long csig2_num;
Errcode err;
Hpj_letter *let;
int chars_read = 0;

for (;;)
	{
	if (seek_past_first(f, char_sig, 3) < Success)
		{
		if (chars_read < 2)		/* figure need at least 2 letters */
			return(Err_no_record);
		return(Success);		/* At end of file. */
		}
	if ((err = hpjet_get_number(f, 'E', &csig1_num)) < Success)
		return(err);
	if (csig1_num > CMAX || csig1_num < 0)
		{
		return(Err_bad_record);
		}
	if (fread(buf, 1, sizeof(buf), f) < sizeof(buf))
		return(Err_truncated);
	if (buf[0] != 0x1b && buf[1] != '(' && buf[2] != 'w')
		return(Err_bad_record);
	if ((err = hpjet_get_number(f, 'W', &csig2_num)) < Success)
		return(err);
	if ((let = pj_malloc(csig2_num)) == NULL)
		return(Err_no_memory);
	if (fread(let, 1, (unsigned )csig2_num, f) < csig2_num)
		{
		err = Err_truncated;
		goto FREE_ERR;
		}
	if (let->format != 4)
		{
		err = Err_version;
		goto FREE_ERR;
		}
	if (let->continuation)	/* for now don't handle continuations */
		{
		err = Err_too_big;
		goto FREE_ERR;
		}
	if (let->descriptor_size != 14)
		{
		err = Err_version;
		goto FREE_ERR;
		}
	intel_swap(&let->left_offset, 5);
	if (fcb->head.spacing == 0)	/* promote monospace font to proportional */
		let->delta_x = (fcb->head.cel_width<<2);
	fcb->letters[csig1_num] = let;
	++chars_read;
	}
FREE_ERR:
pj_free(let);
return(err);
}

static Errcode fixup_hpjet_font(Hfcb *fcb)
/* Scan through every letter in font to figure out the dimension of it
 * in our terms. */
{
int widest = 0;
int miny = 0, maxy = 0;
int minleft = 10000;
int temp;
int i;
Hpj_letter **lets;
Hpj_letter *letter;
Errcode err = Success;

	/* Go through and rearrange some of the letter constants to make
	 * less calculation during font drawing time.  Also store the
	 * minimum and maximum y values of any font imagery. */
i = CMAX;
lets = fcb->letters;
while (--i >= 0)
	{
	if ((letter = *lets++) != NULL)
		{
		if ((letter->delta_x = ((letter->delta_x+3)>>2)) > widest)
			widest = letter->delta_x;
		if ((temp = letter->top_offset =  
			 fcb->head.baseline_distance - 1 - letter->top_offset) < miny)
			 miny = temp;
		if ((temp += letter->character_height) > maxy)
			maxy = temp;
		if ((temp = letter->left_offset) < minleft)
			minleft = temp;
		}
	}
i = CMAX;
lets = fcb->letters;
while (--i >= 0)
	{
	if ((letter = *lets++) != NULL)
		{
		letter->top_offset -= miny;
		letter->left_offset -= minleft; 
		}
	}
fcb->tallest = maxy - miny;
fcb->widest = widest;
return(err);
}

static Errcode force_hpjet_space(Hfcb *fcb)
/* Make sure that space is one of the letters in the font.   */
{
Hpj_letter *l;
int w,h;
int cbytes;

if (fcb->letters[' '] == NULL)
	{
	if ((l = fcb->letters['t']) != NULL)	/* copy width from letter
											 * t if it exists */
		w = l->delta_x;
	else
		w = (fcb->head.cel_width+2)/3;	/* Make space width from cel_width */
										/* (The /3 is pretty arbitrary.) */
	h = fcb->head.cel_height;
	cbytes = sizeof(*l) + h*((w+7)>>3);
	if ((l = pj_zalloc(cbytes)) == NULL)
		return(Err_no_memory);
	l->delta_x = l->character_width = w;
	l->character_height = h;
	fcb->letters[' '] = l;
	}
return(Success);
}

static Errcode ld_hpjet_font(char *name, Hfcb *fcb)
{
FILE *f;
Errcode err;
long sig1_num;
short head_size;
int hdif;

/* Load HPJET file */
if ((f = fopen(name, "rb")) ==  NULL)
	return(errno);
if ((err = verify_hpjet_font(f)) < Success)
	goto ERROR;
if ((err = hpjet_get_number(f, 'W', &sig1_num)) < Success)
	goto ERROR;
if (fread(&head_size, 1, sizeof(head_size), f) 
	< sizeof(head_size) )
	{
	err = Err_truncated;
	goto ERROR;
	}
intel_swap(&head_size, 1);
if (fread(&fcb->head, 1, sizeof(fcb->head), f) < sizeof(fcb->head) )
	{
	err = Err_truncated;
	goto ERROR;
	}
if (fcb->head.format != 0)
	{
	err = Err_version;
	goto ERROR;
	}
intel_swap(&fcb->head.baseline_distance, 3);
intel_swap(&fcb->head.symbol_set, 4);
intel_swap(&fcb->head.text_height, 2);
/* skip past font name */
hdif = head_size - sizeof(fcb->head) - 2;	/* size of font name */
while (--hdif >= 0)
	{
	if ((err = getc(f)) < Success)
		goto ERROR;
	}
if ((fcb->letters = pj_zalloc(CMAX*sizeof(Hpj_letter *))) == NULL)
	{
	err = Err_no_memory;
	goto ERROR;
	}
if ((err = hpjet_read_chars(f, fcb)) < Success)
	goto ERROR;
if ((err = fixup_hpjet_font(fcb)) < Success)
	goto ERROR;
if ((err = force_hpjet_space(fcb)) < Success)
	goto ERROR;
fclose(f);
return(Success);
ERROR:
fclose(f);     
return(err);
}



static int hpjet_char_width(Vfont *v, UBYTE *s)
{
Hfcb *fcb = v->font;
Hpj_letter *let;

if ((let = fcb->letters[oem_to_ansi[s[0]]]) == NULL)
	return(v->widest_char+v->spacing);
else
	{
	if (s[1] == 0)	/* last character - use image size */
		return(let->left_offset + let->character_width + v->spacing);
	else			/* otherwise use distance to next character */
		return(let->delta_x+v->spacing);
	}
}

static Boolean hpjet_in_font(Vfont *v, int c)
{
Hfcb *fcb = v->font;

c = oem_to_ansi[c];
return(c >= 0 && c < CMAX && fcb->letters[c] != NULL);
}

static Errcode hpjet_gftext(Raster *rast,
			Vfont *vfont,
			register unsigned char *s,
			int x,int y,
			Pixel color, Text_mode tmode,
			Pixel bcolor)
{
Hfcb *fcb = vfont->font;
unsigned char c;
Hpj_letter *let;
int maxx;

	maxx = rast->width;

	while ((c = *s++) != 0)
	{
		c = oem_to_ansi[c];
		if (c == '\t')
		{
			x += vfont->tab_width;
		}
		else if ((let = fcb->letters[c]) != NULL)
		{
			if(x > maxx)
				break;
			(*blit_for_mode[tmode])((UBYTE *)(let+1), (let->character_width+7)>>3, 0, 0,
				rast, x + let->left_offset, y + let->top_offset,
				let->character_width, let->character_height, color, bcolor);
			x += let->delta_x+vfont->spacing;
		}
		else
			x += vfont->widest_char+vfont->spacing;
	}
	return(Success);
}


static void attatch_hpjet_font(Vfont *vfont, Hfcb *fcb)
{
clear_struct(vfont);
vfont->type = HPJET;
vfont->font = fcb;
vfont->close_vfont = free_hpjet_font;
vfont->gftext = hpjet_gftext;
vfont->char_width = hpjet_char_width;
vfont->in_font = hpjet_in_font;
vfont->widest_image = vfont->widest_char = fcb->widest;
vfont->image_height = fcb->tallest;
vfont->line_spacing = fcb->head.cel_height;
if (vfont->line_spacing < vfont->image_height)
	vfont->line_spacing = vfont->image_height;
vfont->tab_width = vfont->widest_char*TABEXP;
scan_init_vfont(vfont);
}

static Errcode
load_hpjet_font(char *title, Vfont *vfont, SHORT height, SHORT unzag_flag)
{
Errcode err;
Hfcb *hfcb = NULL;
(void)height;
(void)unzag_flag;

if ((hfcb = pj_zalloc(sizeof(*hfcb))) == NULL)
	{
	err = Err_no_memory;
	goto ERROR;
	}
if ((err = ld_hpjet_font(title, hfcb)) < Success)
	goto ERROR;
attatch_hpjet_font(vfont, hfcb);
return(Success);
ERROR:
_free_hpjet_font(hfcb);
return(err);
}

Font_dev hpjet_font_dev = {
NULL,
"HP Downloadable",
"*.M*",
check_hpjet_font,
load_hpjet_font,
HPJET,
0
};

