/* csort.c - High level routines for sort by luminance, colormap squeeze,
   color arrange by spectrums or gradients.  Lo level routine for
   the sort. */

#include "jimk.h"
#include "flicmenu.h"
#include "csort.str"

extern UBYTE *cluster_to_cmap();

/* structure to build color histogram */
struct occ_map 
	{
	long count;
	unsigned char rgb[4];
	};

static int pnum;
static long cstart;
static unsigned char *tcmap;
static unsigned char *p, *c, *p2;
static unsigned char *sscmapp;
static int scolors;

/* make sscmapp point to a linear array of rgb triples that we want
   to effect.  (Will take care of cluster complications) */
static
find_sscmapp()
{
if (vs.pal_to == 1)	/* to all colors */
	{
	sscmapp = render_form->cmap;
	scolors = COLORS;
	}
else
	{
	if ((sscmapp = cluster_to_cmap()) == NULL)
		return(0);
	scolors = cluster_count();
	}
return(1);
}

static
free_sscmapp()
{
if (vs.pal_to == 0)
	gentle_freemem(sscmapp);
}

/* Comparison routine so can sort histogram by how frequently used a
   color is. */
static int
occ_cmp(a,b)
struct occ_map *a, *b;
{
return(a->count - b->count);
}

/* Make up a boolean array that reflects whether a particular color is
   present in cluster. */
static
cluster_to_cflags(cflags)
UBYTE *cflags;
{
int i;
UBYTE *p;

zero_structure(cflags, COLORS);
i = cluster_count();
p = vs.buns[vs.use_bun].bundle;
while (--i >= 0)
	{
	cflags[*p++] = 1;
	}
}


/* If in cluster mode simplify remove colors not in cluster from 
   further consideration by making their count in the histogram 0 */
static
delete_not_in_cluster(sbuf)
struct occ_map *sbuf;
{
UBYTE cflags[COLORS];
UBYTE *p;
int i;

if (vs.pal_to == 1)		/* to all */
	return;
cluster_to_cflags(cflags);

/* Mark colors not in bundle with count 0 */
p = cflags;
i = COLORS;
while (--i >= 0)
	{
	if (*p++ == 0)
		sbuf->count = 0;
	sbuf++;
	}
}


/* Sort colors by frequency of use, and remove colors that are unused
   or duplicates of other colors */
static 
uniq_sort(pixels, sbuf, scmap, dcmap, colors)
PLANEPTR pixels, scmap, dcmap;
struct occ_map *sbuf;
int colors;
{
struct occ_map **isb;
struct occ_map *occ;
unsigned i;
int dcount = 0;
PLANEPTR rgb;

/* make color occurence histogram */
zero_structure(sbuf, colors*sizeof(*sbuf) );
for (i=0; i<64000; i++)
	sbuf[*pixels++].count++;
delete_not_in_cluster(sbuf);
if ((isb = begmem(colors*sizeof(struct occ_map *))) == NULL)
	return(0);
for (i=0; i<colors; i++)
	{
	occ = sbuf+i;
	if (occ->count >= BIG_WORD)
		occ->count = BIG_WORD;
	isb[i] = occ;
	copy_bytes(scmap+i*3, sbuf[i].rgb, 3);
	}
sort_array((Name_list *)isb, colors, occ_cmp);
dcount = 0;
for (i=0; i<colors; i++)
	{
	if (isb[i]->count > 0)
		{
		rgb = isb[i]->rgb;
		if (!in_cmap(rgb,dcmap,dcount) )
			{
			copy_bytes(rgb, dcmap+3*dcount, 3);
			dcount++;
			}
		}
	}
freemem(isb);
return(dcount);
}


/* make new cmap conform to order of old cmap as much as possible.
   (Things get pretty scambled up by the histogram sort... */
static
fold_over_cmap(new,old,dest,newsz, oldsz)
PLANEPTR new,old,dest;
int newsz, oldsz;
{
int dcount;
int i;

if (newsz > oldsz)
	newsz = oldsz;
zero_structure(dest, oldsz*3);
dcount = 0;
for (i=0; i<oldsz; i++)
	{
	if (in_cmap(old,new,newsz) )	/* see if this color in new color map */
		{
		if (!in_cmap(old, dest, dcount))	/* see if in dest already */
			{
			copy_bytes(old,dest+3*dcount,3);
			dcount++;
			if (dcount >= newsz)
				break;	/* just for safeties sake */
			}
		}
	old+=3;
	}
}

/* Put result of our color squeezing/packing etc back into
   the current palette in case of 'to cluster'. */
static
fold_in_cluster(ncl, clcount)
PLANEPTR ncl;
int clcount;
{
int i;
int osize;
UBYTE *ocl, *dcl;
int ok;

ok = 0;
osize = cluster_count();
if ((ocl = cluster_to_cmap()) != NULL)
	{
	if ((dcl = begmem(3*osize)) != NULL)
		{
		fold_over_cmap(ncl,ocl,dcl,clcount, osize);
#ifdef OLD
		copy_bytes(dcl, ocl, 3*osize);
		cmap_to_cluster(ocl, osize);
#endif
		cmap_to_cluster(dcl, osize);
		freemem(dcl);
		ok = 1;
		}
	freemem(ocl);
	}
return(ok);
}

/* color pack one frame */
static
cpack1()
{
int ccount;
int max;

copy_cmap(render_form->cmap, tcmap);
max = uniq_sort(vf.p, c, vf.cmap, p2, COLORS);
ccount = pnum;
if (ccount > max)
	ccount = max;
pack_cmap(p2,(long)max,p,ccount);
if (vs.pal_to == 1)	/* to all colors */
	fold_over_cmap(p,tcmap,vf.cmap,ccount, COLORS);
else	/* to cluster */
	if (!fold_in_cluster(p, ccount))
		return(0);
refit_screen(&vf, vf.cmap, tcmap, ccount);
return(1);
}

/* Presuming we got all the buffers we need, query user as to how
   many colors he wants in destination and then pack that baby */
static
do_cpack()
{
if ((cstart = uniq_sort(vf.p, c, vf.cmap, p2, COLORS)) > 0)
	{
	pnum = cstart;
	if (qreq_number(csort_100 /* "Squeeze to how many colors?" */, 
		&pnum, 1, cstart))
		{
		if (pnum > 1)
			{
			if (pnum >COLORS)
				pnum = COLORS;
			doauto(cpack1);
			}
		}
	}
}

/* "Squeeze" down # of colors in palette eventually winding down
   into the threshold streaming algorithm in cpack.c   */
cpack()
{
hide_mp();
push_most();
if ((tcmap = begmem(COLORS*3)) != NULL)
	{
	if ((p = begmem(COLORS*3)) != NULL)
		{
		if ((p2 = begmem(COLORS*3)) != NULL)
			{
			if ((c = begmem(COLORS*sizeof(struct occ_map))) != NULL)
				{
				do_cpack();
				freemem(c);
				}
			freemem(p2);
			}
		freemem(p);
		}
	freemem(tcmap);
	}
pop_most();
draw_mp();
}

static PLANEPTR cm;

/* Comparison routine for luminance sort */
static 
rccmp(a,b)
unsigned char *a, *b;
{
PLANEPTR p1, p2;
int i;
int acc;

p1 = cm + 3* (a[0]);
p2 = cm + 3* (b[0]);
i = 3;
acc = 0;
while (--i >= 0)
	{
	acc += *p1++;
	acc -= *p2++;
	}
return(acc);
}


/* luminance sort one frame */
static
csort1()
{
UBYTE *p, **pp;
PLANEPTR ncmap, old_cmap;
int i;
int success;
unsigned char c;

success = 0;
if (!find_sscmapp())
	return(0);
cm = ncmap = sscmapp;
if ((p = begmem(scolors)) != NULL)
	{
	if ((pp = begmem(scolors*sizeof(*pp) )) != NULL)
		{
		if ((old_cmap = begmem(scolors*3)) != NULL)
			{
			copy_bytes(ncmap, old_cmap, 3*scolors);
			for (i=0; i<scolors; i++)
				{
				pp[i] = p+i;
				p[i] = i;
				}
			sort_array(pp, scolors, rccmp);
			for (i=0; i<scolors;i++)
				{
				c = *(pp[i]);
				copy_bytes( c * 3 + old_cmap, i*3 + ncmap, 3);
				}
			if (vs.pal_to == 0)	/* to cluster */
				{
				cmap_to_cluster(sscmapp, scolors);
				}
			refit_vf();
			freemem(old_cmap);
			success = 1;
			}
		freemem(pp);
		}
	freemem(p);
	}
free_sscmapp();
return(success);
}

/* Go sort by luminance */
csort()
{
hmpauto(csort1);
}



/* say is this a thread (aka gradient) or a spectrum? */
static int inertia;

/* Thread out one frame */
static
cthread1()
{
int ok;
UBYTE gotit[COLORS];
UBYTE *tcmap;


if (!find_sscmapp())
	return(0);
ok = 0;
zero_structure(gotit, COLORS);
if ((tcmap = begmem(scolors*3)) != NULL)
	{
	copy_bytes(sscmapp, tcmap, scolors*3);
	rthread_cmap(gotit, tcmap, sscmapp, inertia, scolors);
	freemem(tcmap);
	if (vs.pal_to == 0)	/* to cluster */
		{
		cmap_to_cluster(sscmapp, scolors);
		}
	refit_vf();
	ok = 1;
	}
free_sscmapp();
return(ok);
}

/* Do a 'gradients' color rearrangement */
cthread()
{
inertia = 0;
hmpauto(cthread1);
}

/* Do a 'spectrums' color rearrangement */
cspec()
{
inertia = 1;
hmpauto(cthread1);
}

