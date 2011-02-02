/* blue.c - Stuff to implement much of the trace drop down */

#include "jimk.h"
#include "fli.h"

/* map everything but clearc to destc */
static
set_one_val(pic, clearc, destc)
Vscreen *pic;
UBYTE clearc, destc;
{
UBYTE table[COLORS];

stuff_words((destc<<8)+destc, table, COLORS/sizeof(WORD));
table[clearc] = clearc;
xlat(table, pic->p, 64000);
}

/* blue pic stuff */
static
blue1()
{
set_one_val(render_form, vs.inks[0], vs.inks[1]);
return(1);
}


qblue_pic()
{
uzauto(blue1);
}


/* unblue pic stuff */
static
unblue1()
{
UBYTE table[COLORS];
int i;

for (i=0; i<COLORS; i++)
	table[i] = i;
table[vs.inks[1]] = vs.inks[0];
xlat(table, render_form->p, 64000);
return(1);
}

qunblue_pic()
{
uzauto(unblue1);
}


/*  Stuff for insert tween. */
static
bluesome(s,d,count,ink)
PLANEPTR s,d;
unsigned count;
unsigned char ink;
{
unsigned i;

for (i = 0; i<count; i++)
	{
	if (*s == ink)
		*d = ink;
	s++;
	d++;
	}
}



insert_tween()
{
Vscreen *nf;
int oix;

unzoom();

scrub_cur_frame();

/* Make undo buffer hold current frame, vga screen hold next screen */
oix = vs.frame_ix;
advance_frame_ix();
copy_form(render_form, &uf);
unfli(render_form, vs.frame_ix, 1);

/* set next screen to color 2 (a dark red) */
set_one_val(render_form, vs.inks[0], vs.inks[2]);

/* set this screen to color 1 */
set_one_val(&uf, vs.inks[0], vs.inks[1]);

/* superimpose future on present */
bluesome(render_form->p, uf.p, (unsigned)64000, vs.inks[2]);

/* make combination current screen */
copy_form(&uf, render_form);
see_cmap();

/* and insert frame into FLI */
insert_frames(1, oix);

/* leave frame position one past original */
vs.frame_ix =oix;
advance_frame_ix();

/* Make sure we update FLI when leave this frame */
dirties();

rezoom();
}


/* stuff for remove guides */
static
clean_t1()
{
UBYTE table[COLORS];
int i;

for (i=0; i<COLORS; i++)
	table[i] = i;
table[vs.inks[1]] = vs.inks[0];
table[vs.inks[2]] = vs.inks[0];
xlat(table, render_form->p, 64000);
return(1);
}

clean_tween()
{
uzauto(clean_t1);
}

#define MSZ (Mask_block(XMAX,YMAX))
#define PSZ ((unsigned)64000)

/* routine that helps get changes, repeat changes, and next blue */
static
build_change_mask(p1, p2, mask)
UBYTE *p1, *p2, *mask;
{
UBYTE mod, mword;
unsigned count;

zero_structure(mask, MSZ);
count = PSZ+1;
mod = 0x80;
mword = 0;
while (--count != 0)
	{
	if (*p1++ != *p2++)
		{
		mword |= mod;
		}
	if ((mod >>= 1) == 0)
		{
		mod = 0x80;
		*mask++ = mword;
		mword = 0;
		}
	}
}

/* stuff for repeat changes */
static
update_changes(new, old, dest, mask)
UBYTE *new, *old, *dest, *mask;
{
UBYTE mod, mword;
unsigned count;

count = PSZ+1;
mod = 0;
while (--count != 0)
	{
	if ((mod >>= 1) == 0)
		{
		mword = *mask++;
		mod = 0x80;
		}
	if (mword&mod)
		{
		*dest++ = *new++;
		old++;
		}
	else
		{
		*dest++ = *old++;
		new++;
		}
	}
}

/* make one line of changes */
static
mc1line(s1,s2,d,w,tcolor)
PLANEPTR s1,s2,d;
int w,tcolor;
{
UBYTE c;

while (--w >= 0)
	{
	c = *s2++;
	if (c != *s1++)
		*d++ = c;
	else
		*d++ = tcolor;
	}
}


qget_changes()
{
int w, h;
PLANEPTR s1,s2,d;

if (!push_screen())
	return;
fli_abs_tseek(&uf,vs.frame_ix); /* put the unchanged screen into uf */

/* xor with changed screen and find dimensions of changed result */
xor_form(&uf, render_form);
find_clip(render_form,0);
w = x_1 - x_0 + 1;
h = y_1 - y_0 + 1;

pop_screen();	/* back to original screen */

free_cel(cel);
if ((cel = alloc_cel(w,h,x_0,y_0)) != NULL)
	{
	d = cel->p;
	s1 = uf.p + y_0*BPR + x_0;
	s2 = render_form->p + y_0*BPR + x_0;
	while (--h >= 0)
		{
		mc1line(s1,s2,d,w,vs.inks[0]);
		s1 += BPR;
		s2 += BPR;
		d += cel->bpr;
		}
	copy_cmap(render_form->cmap, cel->cmap);
	show_cel_a_sec();
	}
}


/* Stuff for repeat changes and next blue */

/* selective copy of s to d.  Where d is 'blue' replace it with s.  Used
   by next blue. */
static
restore_blue(s,d,count,ink)
PLANEPTR s, d;
unsigned count;
unsigned char ink;
{
int i;

for (i = 0; i<count; i++)
	{
	if (*d == ink)
		*d = *s;
	s++;
	d++;
	}
}


/* Either do repeat changes or next blue */
static
next_bc(blue)
int blue;	/* blue it or copy changes */
{
PLANEPTR mask;
int ok, oix;

oix = vs.frame_ix;
unzoom();
maybe_push_most();
fli_abs_tseek(&uf,vs.frame_ix);
if (blue)
	restore_blue(uf.p, render_form->p, (unsigned)64000,vs.inks[1]);
if ((mask = begmem(MSZ)) != NULL)
	{
	build_change_mask(uf.p, render_form->p, mask);
	if (push_screen())
		{
		scrub_cur_frame();
		advance_frame_ix();
		copy_form(render_form, &uf);
		ok = unfli(&uf,vs.frame_ix,1);
		pop_screen();
		if (ok)
			{
			if (blue)
				{
				copy_form(&uf, render_form);
				a1blit(XMAX,YMAX,0,0,mask,Mask_line(XMAX), 
					0,0,render_form->p, BPR,vs.inks[1]);
				}
			else
				{
				update_changes(render_form->p, uf.p, render_form->p, mask);
				copy_cmap(uf.cmap, render_form->cmap);
				}
			dirties();
			}
		else
			vs.frame_ix = oix;
		}
	freemem(mask);
	}
maybe_pop_most();
see_cmap();
rezoom();
}


qnext_changes()
{
next_bc(0);
}

qnext_blue()
{
next_bc(1);
}

