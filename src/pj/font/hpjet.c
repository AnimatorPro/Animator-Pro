#define VFONT_C
#include "errcodes.h"
#include "fontdev.h"
#include "hpjet.h"
#include "memory.h"
#include "rastcall.h"
#include "rastext.h"
#include "util.h"
#include "xfile.h"

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

static Errcode hpjet_get_number(XFILE *f, char end_tag, size_t *result)
/* return ascii represented number starting at current file position and
   ending with end_tag into result */
{
int c;
size_t acc = 0;
Errcode err = Success;

for (;;)
	{
	if ((c = xfgetc(f)) < Success)
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

/* Check that file begins with <esc> ) s    signature */
static Errcode
verify_hpjet_font(XFILE *xf)
{
	Errcode err;
	char buf[3];

	err = xffread(xf, buf, sizeof(buf));
	if (err < Success)
		return Err_truncated;

	if (buf[0] != 0x1b && buf[1] != ')' && buf[2] != 's')
		return Err_bad_magic;

	return Success;
}

static Errcode check_hpjet_font(char *name)
/* Open font file and check signature */
{
	Errcode err;
	XFILE *xf;

	err = xffopen(name, &xf, XREADONLY);
	if (err < Success)
		return err;

	err = verify_hpjet_font(xf);
	xffclose(&xf);
	return err;
}

static Errcode seek_past_first(XFILE *f, char *s, int slen)
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
	if ((c = xfgetc(f)) < Success)
		return(Err_eof);
CHECK_FIRSTC:
	if (c == firstc)
		{
		pt = s;
		i = slen;
		while (--i >= 0)
			{
			if ((c = xfgetc(f)) < Success)
				return(Err_eof);
			if (c != *pt++)
				goto CHECK_FIRSTC;
			}
		return(Success);
		}
	}
}

static Errcode
hpjet_read_chars(XFILE *xf, Hfcb *fcb)
{
Errcode err;
char buf[3];
static char char_sig[] = {0x1b, '*', 'c'};
size_t csig1_num;
size_t csig2_num;
Hpj_letter *let;
int chars_read = 0;

for (;;)
	{
	err = seek_past_first(xf, char_sig, 3);
	if (err < Success)
		{
		if (chars_read < 2) /* figure need at least 2 letters */
			return Err_no_record;
		return Success; /* At end of file. */
		}

	err = hpjet_get_number(xf, 'E', &csig1_num);
	if (err < Success)
		return err;

	if (csig1_num > CMAX)
		return Err_bad_record;

	err = xffread(xf, buf, sizeof(buf));
	if (err < Success)
		return Err_truncated;

	if (buf[0] != 0x1b && buf[1] != '(' && buf[2] != 'w')
		return Err_bad_record;

	err = hpjet_get_number(xf, 'W', &csig2_num);
	if (err < Success)
		return err;

	let = pj_malloc(csig2_num);
	if (let == NULL)
		return Err_no_memory;

	err = xffread(xf, let, csig2_num);
	if (err < Success)
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
return err;
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
XFILE *xf;
Errcode err;
size_t sig1_num;
short head_size;
int hdif;

/* Load HPJET file */
err = xffopen(name, &xf, XREADONLY);
if (err < Success)
	return err;

err = verify_hpjet_font(xf);
if (err < Success)
	goto ERROR;

err = hpjet_get_number(xf, 'W', &sig1_num);
if (err < Success)
	goto ERROR;

err = xffread(xf, &head_size, sizeof(head_size));
if (err < Success)
	{
	err = Err_truncated;
	goto ERROR;
	}
intel_swap(&head_size, 1);

err = xffread(xf, &fcb->head, sizeof(fcb->head));
if (err < Success)
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
	err = xfgetc(xf);
	if (err < Success)
		goto ERROR;
	}
if ((fcb->letters = pj_zalloc(CMAX*sizeof(Hpj_letter *))) == NULL)
	{
	err = Err_no_memory;
	goto ERROR;
	}

err = hpjet_read_chars(xf, fcb);
if (err < Success)
	goto ERROR;

if ((err = fixup_hpjet_font(fcb)) < Success)
	goto ERROR;
if ((err = force_hpjet_space(fcb)) < Success)
	goto ERROR;

err = Success;

ERROR:
	xffclose(&xf);
	return err;
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
			blit_for_mode(tmode,
				(UBYTE *)(let+1), (let->character_width+7)>>3, 0, 0,
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

