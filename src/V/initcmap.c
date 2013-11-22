
/* initcmap.c - Has the RGB data for my favorite startup palette - 
   a 32 level grey scale and a bunch of rainbows.  Below this is
   the code to insure the menus are drawn in visible colors.  (find_colors()
   and related routines.).   When menus are on-screen if necessary
   the last five  color registers are co-opted for menu colors.  It'd
   be nice to make this a bit less obtrusive than it is for the artist.
   Most simply squeeze the picture down to < 250 colors. */

#include "jimk.h"

UBYTE sys5;		/* Has system taken over last 5 colors? */

UBYTE init_cmap[COLORS*3] = {
 /* 32 level grey scale */
 0, 0, 0,	 2, 2, 2,	 4, 4, 4,	 6, 6, 6,	
 8, 8, 8,	10,10,10,	12,12,12,	14,14,14,	
16,16,16,	18,18,18,	20,20,20,	22,22,22,	
24,24,24,	26,26,26,	28,28,28,	30,30,30,	
33,33,33,	35,35,35,	37,37,37,	39,39,39,	
41,41,41,	43,43,43,	45,45,45,	47,47,47,	
49,49,49,	51,51,51,	53,53,53,	55,55,55,	
57,57,57,	59,59,59,	61,61,61,	63,63,63,	
/* A hand threaded 6x6x6 rgb cube */
63,51,51,	63,63,51,	51,63,51,	51,63,63,	
51,51,63,	63,51,63,	63,39,39,	63,51,39,	
63,63,39,	51,63,39,	39,63,39,	39,63,51,	
39,63,63,	39,51,63,	39,39,63,	51,39,63,	
63,39,63,	63,39,51,	63,27,27,	63,39,27,	
63,51,27,	63,63,27,	51,63,27,	39,63,27,	
27,63,27,	27,63,39,	27,63,51,	27,63,63,	
27,51,63,	27,39,63,	27,27,63,	39,27,63,	
51,27,63,	63,27,63,	63,27,51,	63,27,39,	
63,15,15,	63,27,15,	63,39,15,	63,51,15,	
63,63,15,	51,63,15,	39,63,15,	27,63,15,	
15,63,15,	15,63,27,	15,63,39,	15,63,51,	
15,63,63,	15,51,63,	15,39,63,	15,27,63,	
15,15,63,	27,15,63,	39,15,63,	51,15,63,	
63,15,63,	63,15,51,	63,15,39,	63,15,27,	
63, 3,15,	63, 3, 3,	63,15, 3,	63,27, 3,	
63,39, 3,	63,51, 3,	63,63, 3,	51,63, 3,	
39,63, 3,	27,63, 3,	15,63, 3,	 3,63, 3,	
 3,63,15,	 3,63,27,	 3,63,39,	 3,63,51,	
 3,63,63,	 3,51,63,	 3,39,63,	 3,27,63,	
 3,15,63,	 3, 3,63,	15, 3,63,	27, 3,63,	
39, 3,63,	51, 3,63,	63, 3,63,	63, 3,51,	
63, 3,39,	63, 3,27,	51, 3,15,	51, 3, 3,	
51,15, 3,	51,27, 3,	51,39, 3,	51,51, 3,	
39,51, 3,	27,51, 3,	15,51, 3,	 3,51, 3,	
 3,51,15,	 3,51,27,	 3,51,39,	 3,51,51,	
 3,39,51,	 3,27,51,	 3,15,51,	 3, 3,51,	
15, 3,51,	27, 3,51,	39, 3,51,	51, 3,51,	
51, 3,39,	51, 3,27,	39, 3,15,	39, 3, 3,	
39,15, 3,	39,27, 3,	39,39, 3,	27,39, 3,	
15,39, 3,	 3,39, 3,	 3,39,15,	 3,39,27,	
 3,39,39,	 3,27,39,	 3,15,39,	 3, 3,39,	
15, 3,39,	27, 3,39,	39, 3,39,	39, 3,27,	
27, 3,15,	27, 3, 3,	27,15, 3,	27,27, 3,	
15,27, 3,	 3,27, 3,	 3,27,15,	 3,27,27,	
 3,15,27,	 3, 3,27,	15, 3,27,	27, 3,27,	
15, 3, 3,	15,15, 3,	 3,15, 3,	 3,15,15,	
 3, 3,15,	15, 3,15,	27,15,15,	27,27,15,	
15,27,15,	15,27,27,	15,15,27,	27,15,27,	
39,15,15,	39,27,15,	39,39,15,	27,39,15,	
15,39,15,	15,39,27,	15,39,39,	15,27,39,	
15,15,39,	27,15,39,	39,15,39,	39,15,27,	
51,15,15,	51,27,15,	51,39,15,	51,51,15,	
39,51,15,	27,51,15,	15,51,15,	15,51,27,	
15,51,39,	15,51,51,	15,39,51,	15,27,51,	
15,15,51,	27,15,51,	39,15,51,	51,15,51,	
51,15,39,	51,15,27,	51,27,27,	51,39,27,	
51,51,27,	39,51,27,	27,51,27,	27,51,39,	
27,51,51,	27,39,51,	27,27,51,	39,27,51,	
51,27,51,	51,27,39,	51,39,39,	51,51,39,	
39,51,39,	39,51,51,	39,39,51,	51,39,51,	
39,27,27,	39,39,27,	27,39,27,	27,39,39,	
27,27,39,	39,27,39,	 3, 3, 3,	15,15,15,	
27,27,27,	39,39,39,	51,51,51,	63,63,63,	
 /* 3 empty slots */
63,22, 3,	39, 7, 5,	36,36,63,	 
 /* Colors the system would like to use for interface */
0, 0, 0,	 22,22,22,	38,38,38,	52,52,52,	63, 0, 0,	
};

UBYTE pure_white[3] = {63,63,63};
UBYTE pure_black[3] = {0,0,0};
#ifdef SLUFFED
UBYTE pure_red[] = {63, 0, 0};
UBYTE pure_green[] = {0, 63, 0};
UBYTE pure_blue[] = {0, 0, 63};
#endif SLUFFED

#ifdef SLUFFED
static UBYTE pure_grey[] = {24, 24, 24};
static UBYTE pure_bright[3] = {58, 58, 58};
static UBYTE bright_grey[] = {42, 42, 42};
#endif SLUFFED


WORD mc_colors[5];

/* min difference color threshold */
#define SEE_T (6*6)

/* 1st system grabbable color */
#define SYS_C (251)


static f_colors()
{
int i;

for (i=0; i<5; i++)
	mc_colors[i] = closestc(vs.mcideals[i], render_form->cmap,COLORS);
}

/* Are colors distinct enough from each other? */
visible_cmap()
{
int i, j, t;
UBYTE *a, *b;
UBYTE mycolors[6][3];

for (i=0; i<5; i++)
	{
	copy_bytes(render_form->cmap + 3*mc_colors[i], mycolors[i], 3);
	}
for (i=0; i<4; i++)	/* let menu red be invisible if it wants... */
	{
	a = mycolors[i];
	for (j=0; j<4; j++)
		{
		if (i != j)
			{
			b = mycolors[j];
			if (color_dif(a,b) < SEE_T)
				{
				return(0);
				}
			}
		}
	}
return(1);
}

static
check_cmap()
{
int i;

sys5 = !visible_cmap();
if (sys5)
	{
	jset_colors(SYS_C, 5, vs.mcideals);
	for (i=0; i<5; i++)
		mc_colors[i] = i + SYS_C;
	}
}

uncheck_cmap()
{
if (sys5)
	jset_colors(COLORS-5, 5, render_form->cmap+3*(COLORS-5));
}



find_colors()
{
f_colors();
check_cmap();
}

