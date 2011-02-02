/* cycle.c - a module called from poco to cycle the color map */
#include "errcodes.h"
#include "ptrmacro.h"
#include "pocorex.h"
#include "rcel.h"
#include "rastlib.h"


extern Rcel *GetPicScreen();


void cycle_rgb3(Rgb3 *ctab, int ccount)
/* Move 2nd color map entry to 1st,  3rd to 2nd, and so on.  Move the
 * 1st color map  entry to the last.  In other words cycle the
 * RGB data once. */
{
Rgb3 save;
int i;

save = ctab[0];				/*  save first  color */
for (i=1; i<ccount; i++)	/*  shift other 255 down one slot */
	{
	ctab[i-1] = ctab[i];
	}
ctab[ccount-1] = save;		/* and store first color in last color */
}

void cycle_screen(Rcel *screen, int cycles)
/* Cycle the colors in screen cycles times.  Wait for vertical blank
 * between doing each cycle. */
{
int i;
Cmap *cmap = screen->cmap;

for (i=0; i<cycles; i++)
	{
	cycle_rgb3(cmap->ctab, cmap->num_colors);	/* shift the color data  */
	WAIT_VSYNC(screen);							/* wait for vertical blank */
	SET_COLORS(screen, 0, 						/* send data to hardware  */
		cmap->num_colors, cmap->ctab);
	}
}

static void do_poco_cycle(int cyc_count)
{
 	cycle_screen(GetPicScreen(), cyc_count); 
}

static void cycle_background(int cyc_count)
{
Rgb3 ori, new;
int i;
Rcel *screen = GetPicScreen();
Cmap *cmap = screen->cmap;

new = ori = cmap->ctab[0];
for (i=0; i<cyc_count; i++)
	{
	WAIT_VSYNC(screen);							/* wait for vertical blank */
	new.r += 4;
	SET_COLORS(screen, 0, 						/* send data to hardware  */
		1, &new);
	}
SET_COLORS(screen, 0, 						/* send data to hardware  */
	1, &ori);
}


static Lib_proto calls[] = {
	{ do_poco_cycle, "void cycle_screen(int cyc_count);" },
	{ cycle_background, "void cycle_background(int cyc_count);" },
};

Hostlib _a_a_pocolib = { NULL, AA_POCOLIB, AA_POCOLIB_VERSION };

Pocorex rexlib_header = {
	{ 
	REX_POCO, 
	POCOREX_VERSION, 
	NOFUNC, NOFUNC, 
	&_a_a_pocolib, 
	POCO_REX_ID_STRING,
	},
	{
	NULL,
	"cycle",
	calls,
	Array_els(calls),
	},
};


