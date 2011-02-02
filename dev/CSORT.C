/* csort.c - High level routines for sort by luminance, colormap squeeze,
   color arrange by spectrums or gradients.  Lo level routine for
   the sort. */

#include "errcodes.h"
#include "jimk.h"
#include "menus.h"

extern UBYTE *cluster_to_ctable();

/* structure to build color histogram */

typedef struct occ_map {
	LONG count;
	Rgb3 rgb;
} Occ_map;

/* structure to pass to auto functions */

typedef struct csort_dat {
	SHORT pnum;
	UBYTE *ssctable;
	int scolors;
	int inertia;
	Cmap *tcmap;
	Rgb3 *p;
	Occ_map *c; 
	Rgb3 *p2;
} Csort_dat;



static int find_ssctable(Csort_dat *cpd)
/* make ssctable point to a linear array of rgb triples that we want
   to effect.  (Will take care of cluster complications) */
{
	if (vs.pal_to == 1)	/* to all colors */
	{
		cpd->ssctable = (UBYTE *)(vb.pencel->cmap->ctab);
		cpd->scolors = COLORS;
	}
	else
	{
		if ((cpd->ssctable = cluster_to_ctable()) == NULL)
			return(Err_no_memory);
		cpd->scolors = cluster_count();
	}
	return(Success);
}

static void free_ssctable(Csort_dat *cpd)
{
	if (vs.pal_to == 0)
		pj_freez(&cpd->ssctable);
}

static LONG occ_cmp(Occ_map *a, Occ_map *b)

/* Comparison routine so can sort histogram by how frequently used a
   color is. */
{
return(a->count - b->count);
}

static void cluster_to_cflags(UBYTE *cflags)
/* Make up a boolean array that reflects whether a particular color is
   present in cluster. */
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


static void delete_not_in_cluster(Occ_map *sbuf)

/* If in cluster mode simplify remove colors not in cluster from 
   further consideration by making their count in the histogram 0 */
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



static int uniq_sort(Rcel *r, 
	         		 Occ_map *sbuf, 
			  		 Rgb3 *sctab, 
			  		 Rgb3 *dctab, 
			  		 SHORT colors)

/* Sort color table by frequency of use, and remove colors that are unused
   or duplicates of other colors returns number of colors found */
{
Occ_map **isb;
USHORT i;
ULONG li;
int dcount = 0;
Rgb3 *rgb;
UBYTE *pixels;
UBYTE *lbuf;
SHORT numpix;
SHORT y;

	/* make color occurence histogram */
	clear_mem(sbuf, colors*sizeof(*sbuf) );

	if(r->type == RT_BYTEMAP)
	{
		pixels = ((Bytemap *)r)->bm.bp[0];
		li = ((Bytemap *)r)->bm.psize;

		while(li--)
			sbuf[*pixels++].count++;
	}
	else
	{
		if((lbuf = pj_malloc(r->width)) == NULL)
			return(Err_no_memory);

		for(y = 0;y < r->height;++y)
		{
			numpix = r->width;
			pj__get_hseg(r,lbuf,0,y,numpix);
			pixels = lbuf;
			while(numpix--)
				sbuf[*pixels++].count++;
		}
		pj_free(lbuf);
	}
	delete_not_in_cluster(sbuf);
	if ((isb = pj_malloc(colors*sizeof(Occ_map *))) == NULL)
		return(Err_no_memory);

	for (i=0; i<colors; i++)
	{
		isb[i] = sbuf+i;
		sbuf[i].rgb = sctab[i];
	}

	sort_indarray(isb, colors, (FUNC)occ_cmp);
	dcount = 0;
	for (i=0; i<colors; i++)
	{
		if (isb[i]->count > 0)
		{
			rgb = &(isb[i]->rgb);
			if (!in_ctable(rgb,dctab,dcount) )
			{
				dctab[dcount] = *rgb;
				dcount++;
			}
		}
	}
	pj_free(isb);
	return(dcount);
}


static void fold_over_ctable(Rgb3 *new,
						     Rgb3 *old,
						     Rgb3 *dest,
						     int newsz, int oldsz)

/* make new ctable conform to order of old ctable as much as possible.
   (Things get pretty scambled up by the histogram sort... */
{
int dcount;
int i;

	if (newsz > oldsz)
		newsz = oldsz;
	clear_mem(dest, oldsz*sizeof(Rgb3));
	dcount = 0;
	for (i=0; i<oldsz; i++)
	{
		if (in_ctable(old,new,newsz)) /* see if this color in new color map */
		{
			if (!in_ctable(old, dest, dcount))	/* see if in dest already */
			{
				dest[dcount] = *old;
				dcount++;
				if (dcount >= newsz)
					break;	/* just for safeties sake */
			}
		}
		++old;
	}
}

static int fold_in_cluster(Rgb3 *ncl, int clcount)

/* Put result of our color squeezing/packing etc back into
   the current palette in case of 'to cluster'. */
{
UBYTE cflags[COLORS];
int osize;
UBYTE *ocl, *dcl;
Errcode err;

	err = Err_no_memory;
	osize = cluster_count();
	if ((ocl = cluster_to_ctable()) != NULL)
	{
		if ((dcl = begmem(3*osize)) != NULL)
		{
		   fold_over_ctable(ncl,(Rgb3 *)ocl,(Rgb3 *)dcl,clcount, osize);
#ifdef OLD
			pj_copy_bytes(dcl, ocl, 3*osize);
			ctable_to_cluster(ocl, osize);
#endif
			ctable_to_cluster(dcl, osize);
			pj_free(dcl);
			err = 0;
		}
		pj_free(ocl);
	}
	return(err);
}

static Errcode cpack1(Csort_dat *cpd)
/* color pack one frame */
{
SHORT ccount;
SHORT max;
Errcode err;

	pj_cmap_copy(vb.pencel->cmap, cpd->tcmap);

	max = uniq_sort(vb.pencel, cpd->c, 
					vb.pencel->cmap->ctab, cpd->p2, COLORS);

	ccount = cpd->pnum;
	if (ccount > max)
		ccount = max;
	pack_ctable(cpd->p2,(long)max,cpd->p,ccount);
	if (vs.pal_to == 1)	/* to all colors */
	{
		fold_over_ctable(cpd->p,cpd->tcmap->ctab,
						  vb.pencel->cmap->ctab, ccount, COLORS);
	}
	else	/* to cluster */
	{
		if((err = fold_in_cluster(cpd->p, ccount)) < 0)
			return(err);
		ccount = COLORS;
	}

	refit_rcel(vb.pencel, vb.pencel->cmap, cpd->tcmap);
	fold_in_mucolors(vb.pencel->cmap,ccount,vb.screen);
	return(Success);
}

static void do_cpack(Csort_dat *cpd)
/* Presuming we got all the buffers we need, query user as to how
   many colors he wants in destination and then pack that baby */
{
Errcode err;

	cpd->pnum = uniq_sort(vb.pencel, cpd->c, 
						  vb.pencel->cmap->ctab, cpd->p2, COLORS);

	if(cpd->pnum < 0)
	{
		err = cpd->pnum;
		goto error;
	}

	if (soft_qreq_number(&cpd->pnum,1,Max(cpd->pnum,1),"squeeze_to"))
	{
		if( cpd->pnum > 0
			|| (cpd->pnum == 0 && vs.pal_to != 1))
		{
			if (cpd->pnum > COLORS)
				cpd->pnum = COLORS;
			err = do_autodraw(cpack1,cpd);
		}
		else
			return;
	}
error:
	softerr(err,"cant_cpack");
}

void cpack(void)
/* "Squeeze" down # of colors in palette eventually winding down
   into the threshold streaming algorithm in cpack.c   */
{
Csort_dat cpd;

	hide_mp();
	push_most();

	if (pj_cmap_alloc(&cpd.tcmap,COLORS) >= Success)
	{
		if ((cpd.p = begmem(COLORS*3)) != NULL)
		{
			if ((cpd.p2 = begmem(COLORS*3)) != NULL)
			{
				if ((cpd.c = begmem(COLORS*sizeof(Occ_map))) != NULL)
				{
					do_cpack(&cpd);
					pj_free(cpd.c);
				}
				pj_free(cpd.p2);
			}
			pj_free(cpd.p);
		}
		pj_cmap_free(cpd.tcmap);
	}
	pop_most();
	show_mp();
}

static LONG rccmp(UBYTE *a, UBYTE *b, PLANEPTR cm)
/* Comparison routine for luminance sort */
{
PLANEPTR p1, p2;
int i;
long acc;

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


static int csort1(Csort_dat *cpd)
/* luminance sort one frame */
{
UBYTE *p, **pp;
PLANEPTR ncmap, old_ctab;
int i;
Errcode err;
unsigned char c;

	if((err = find_ssctable(cpd)) < 0)
		return(err);

	err = Err_no_memory;

	ncmap = cpd->ssctable;
	if ((p = begmem(cpd->scolors)) != NULL)
	{
		if ((pp = begmem(cpd->scolors*sizeof(*pp) )) != NULL)
		{
			if ((old_ctab = begmem(cpd->scolors*3)) != NULL)
			{
				pj_copy_bytes(ncmap, old_ctab, 3*cpd->scolors);
				for (i=0; i<cpd->scolors; i++)
				{
					pp[i] = p+i;
					p[i] = i;
				}
				sort_indarray(pp, cpd->scolors, (FUNC)rccmp, ncmap);
				for (i=0; i<cpd->scolors;i++)
				{
					c = *(pp[i]);
					pj_copy_bytes( old_ctab + (c * 3), ncmap + (i*3), 3);
				}
				if (vs.pal_to == 0)	/* to cluster */
				{
					ctable_to_cluster(cpd->ssctable, cpd->scolors);
				}
				refit_vf();
				pj_free(old_ctab);
				err = 0;
			}
			pj_free(pp);
		}
		pj_free(p);
	}
	free_ssctable(cpd);
	return(err);
}

void csort(void)
/* Go sort by luminance */
{
Csort_dat cpd;

	hmpauto(csort1,&cpd);
}

/* say is this a thread (aka gradient) or a spectrum? */

static int cthread1(Csort_dat *cpd)
/* Thread out one frame */
{
Errcode err;
UBYTE gotit[COLORS];
UBYTE *tctable;


	if((err = find_ssctable(cpd)) < 0)
		return(err);
	err = Err_no_memory;

	clear_mem(gotit, COLORS);
	if ((tctable = begmem(cpd->scolors*3)) != NULL)
	{
		pj_copy_bytes(cpd->ssctable, tctable, cpd->scolors*3);
		rthread_cmap(gotit, tctable, cpd->ssctable, cpd->inertia, cpd->scolors);
		pj_free(tctable);
		if (vs.pal_to == 0)	/* to cluster */
		{
			ctable_to_cluster(cpd->ssctable, cpd->scolors);
		}
		refit_vf();
		err = 0;
	}
	free_ssctable(cpd);
	return(err);
}

void cthread(void)
/* Do a 'gradients' color rearrangement */
{
Csort_dat cpd;

	cpd.inertia = 0;
	hmpauto(cthread1,&cpd);
}

void cspec(void)
/* Do a 'spectrums' color rearrangement */
{
Csort_dat cpd;

	cpd.inertia = 1;
	hmpauto(cthread1,&cpd);
}

