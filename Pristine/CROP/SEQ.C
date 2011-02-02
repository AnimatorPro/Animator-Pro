
/* seq.c - handles Atari ST/Cyber Paint .SEQ files.  These are compressed
   frame files much like FLI's on the PC. */

#include "jimk.h"
#include "seq.h"
#include "seq.str"

static UBYTE *stscreen;
static int seq_err = -1;
static seq_fd;
static char *seq_name;

struct seq_header seq_h;
struct neo_head neo_h;

word_uncompress(s, d, length)
WORD *s, *d;
int length;	/* in WORDS! */
{
WORD run_length;
WORD run_data;

for (;;)
	{
	if (length <= 0)  /* check to see if out of data yet */
		break;
	run_length = *s++;
	length -= 1;	/* used up 1 WORD of source */
	if (run_length & 0x8000)  /*  see if it's a compressed run or a literal */
		{
		run_length &= 0x7fff;	/* knock of the hi bit */
		length -= run_length;   /* used up lots more of source */
		while (--run_length >= 0)	/* go through loop run_length times */
			*d++ = *s++;
		}
	else	/* yeah, it's compressed a little */
		{	
		run_data = *s++;
		length -= 1;		/* used another WORD of source */
		while (--run_length >= 0)
			*d++ = run_data;
		}
	}
}

open_verify_seq(name, shead)
char *name;
struct seq_header *shead;
{
int file;

if ((file = jopen(name, 0)) == 0)
	{
	cant_find(name);
	return(0);
	}
if (jread(file, shead, sizeof(*shead)) < sizeof(*shead) )
	{
	truncated(name);
	goto BADEXIT;
	}
intel_swap(&shead->magic);
intel_swap(&shead->version);
intel_swap(&shead->speed);
long_intel_swap(&shead->cel_count);
if (shead->magic != 0xfedc && shead->magic != 0xfedb)
	{
	continu_line(seq_100 /* "Not a Cyber SEQ file" */);
	goto BADEXIT;
	}
return(file);
BADEXIT:
jclose(file);
return(0);
}

open_seq(name)
char *name;
{
if (seq_err != -1)
	{
	continu_line(seq_101 /* "Seq already open" */);
	return(0);
	}
if ((seq_fd = open_verify_seq(name, &seq_h)) == 0)
	return(0);
/* actually the stscreen will be viewed more like an amiga screen 'cause
   the ST word-interleaved blits are such a hassle to cope with.
   Also then can reuse routine to convert Amiga Bit-planes to VGA 
   byte-a-pixel style. */
if ((stscreen = lbegmem(40000L)) == NULL)
	{
	close_seq();
	return(0);
	}
zero_lots(stscreen,40000L);
seq_err = 0;
seq_name = name;
return(1);
}

close_seq()
{
if (seq_fd != 0)
	{
	jclose(seq_fd);
	seq_fd = 0;
	}
gentle_freemem(stscreen);
stscreen = NULL;
seq_err = -1;
}

columns_to_bitplanes(s,d,wpl,height)
WORD *s, *d;
int wpl;	/* words in a line */
int height;	/* # of lines */
{
int i, j, k;
WORD *sp, *dp;
WORD *sl, *dl;
unsigned plane_size;

plane_size = wpl*height;
/* step through planes */
for (i = 0; i< 4; i++)	
	{
	sp = s;
	dp = d;
	/* step through lines */
	for (j=0; j<height; j++)
		{
		sl = sp;
		dl = dp;
		/* step through words */
		for (k=0; k<wpl; k++)
			{
			*dl = *sl;
			dl += 1;
			sl += height;
			}
		sp += 1;
		dp += wpl;
		}
	d += plane_size;
	s += plane_size;
	}
}

de_interleave(s,d,wpl,height)
WORD *s, *d;
int wpl;	/* words in a line */
int height;	/* # of lines */
{
int i, j, k;
WORD *sp, *dp;
WORD *sl, *dl;
unsigned plane_size;

plane_size = wpl*height;
/* step through planes */
for (i = 0; i< 4; i++)	
	{
	sp = s;
	dp = d;
	/* step through lines */
	for (j=0; j<height; j++)
		{
		sl = sp;
		dl = dp;
		/* step through words */
		for (k=0; k<wpl; k++)
			{
			*dl = *sl;
			sl += 4;
			dl += 1;
			}
		sp += 4*wpl;
		dp += wpl;
		}
	s += 1;
	d += plane_size;
	}
}

#ifdef SLUFFED
make_lines(plane,bpr,height)
UBYTE *plane;
int bpr, height;
{
int i,j;
UBYTE data;

for (i=0; i<height; i++)
	{
	if (i&1)
		data = 0x55;
	else
		data = 0xaa;
	for (j=0; j<bpr; j++)
		*plane++ = data;
	}
}
#endif /* SLUFFED */


next_seq()
{
int i;
WORD *buf1, *buf2;
int ok;
int got_data;
unsigned buf_size;
unsigned plane_size;
unsigned sbpr;
unsigned bsize;
UBYTE *planes;

ok = 0;
if (jread(seq_fd, &neo_h, (long)sizeof(neo_h)) < sizeof(neo_h) )
	{
	truncated(seq_name);
	return(0);
	}
intel_swap(&neo_h.type);
intel_swap(&neo_h.resolution);
intel_swaps(neo_h.colormap, 16);
intel_swap(&neo_h.xoff);
intel_swap(&neo_h.yoff);
intel_swap(&neo_h.width);
intel_swap(&neo_h.height);
long_intel_swap(&neo_h.data_size);
sbpr = seq_Mask_line(neo_h.width);
plane_size = sbpr*neo_h.height;
buf_size = plane_size*4;	/* 4 bitplanes in ST display */
if (neo_h.compress == NEO_UNCOMPRESSED)
	{
	neo_h.data_size = seq_LRaster_block(neo_h.width, (long)neo_h.height);
	}
if (neo_h.type != -1)
	{
	continu_line(seq_102 /* "Bad Cel Magic! File Damaged!" */);
	return(0);
	}
got_data = (neo_h.data_size != 0);
if (got_data)
	{
	buf1 = buf2 = NULL;
	if ((buf1 = begmem(buf_size)) == NULL)
		goto ENDDATA;
	if ((buf2 = begmem(buf_size)) == NULL)
		goto ENDDATA;
	if (jread(seq_fd, buf1, neo_h.data_size) != neo_h.data_size)
		{
		truncated(seq_name);
		goto ENDDATA;
		}
	if (neo_h.compress == NEO_CCOLUMNS)
		{
		bsize = neo_h.data_size>>1;
		intel_swaps(buf1,bsize);	/* swap so counts are right */
		word_uncompress(buf1, buf2, bsize);
		intel_swaps(buf2,plane_size*2);	/* swap back so image right */
		columns_to_bitplanes(buf2,buf1,sbpr>>1,neo_h.height);
		planes = (UBYTE *)buf1;
		}
	else
		{
		de_interleave(buf1,buf2,sbpr>>1,neo_h.height);
		planes = (UBYTE *)buf2;
		}
	if (neo_h.op == NEO_XOR)
		{
		for (i=0; i<4; i++)
			{
			xor_blit_box(neo_h.width, neo_h.height, 0, 0, planes, sbpr, 
				neo_h.xoff, neo_h.yoff, stscreen+8000*i, 40);
			planes += plane_size;
			}
		}
	else
		{
		zero_structure(stscreen, 32000);
		for (i=0; i<4; i++)
			{
			blit_box(neo_h.width, neo_h.height, 0, 0, planes, sbpr, 
				neo_h.xoff, neo_h.yoff, stscreen+8000*i, 40);
			planes += plane_size;
			}
		}
	ok = 1;
ENDDATA:
	gentle_freemem(buf1);
	gentle_freemem(buf2);
	}
else	/* zero data length */
	{
	/* if it's an xor don't have to do anything at all, cool breeze! */
	if (neo_h.op != NEO_XOR)
		zero_structure(stscreen, 32000);
	ok = 1;
	}
put_st_cmap(neo_h.colormap);
conv_screen(stscreen);
return(ok);
}

start_seq()
{
if (seq_err)
	return(0);
/* skip past the offset lists */
jseek(seq_fd, seq_h.cel_count * sizeof(long) + sizeof(seq_h), 0);	
return(next_seq());
}

count_seq()
{
if (seq_err)
	return(0);
return(seq_h.cel_count);
}

speed_seq()
{
if (seq_err)
	return(0);
return(seq_h.speed/100);
}

#ifdef SLUFFED
do_test_blit(mask)
UBYTE *mask;
{
UBYTE *scr;
int i;

if ((scr = lbegmem(40000L)) != NULL)
	{
	for (i=0; i<90; i+=3)
		{
		zero_lots(scr, 40000L);
		blit_box(100,100,0,0,mask,40,i, 0, scr, 40);
		conv_screen(scr);
		}
	freemem(scr);
	}
}
#endif /* SLUFFED */

