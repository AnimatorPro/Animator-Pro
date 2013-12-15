/* Convert.c - contains the main routine for the convert program, and
 * various hi-level routines shared by convert's other modules.
 */

#include "errcodes.h"
#include "convert.h"
#include "reqlib.h"
#include <time.h>

USHORT program_id = 1;
USHORT program_version = 0;
Conv_state cs;

char flilores_pdr_name[] = "flilores.pdr";

exit_convert()
/*****************************************************************************
 * Free up all our resources and exit program.
 ****************************************************************************/
{
	pdr_close_ifile(&cs.ifi.ifi);
	conv_free_pdr(&cs.ifi.pdr);
	freez_cel(&cs.ifi.cel);
	cleanup_screen();
	cleanup_startup();
	exit(0);
}
Errcode init_after_screen()
/*****************************************************************************
 * Initialization passed to go_resize_screen().   If this were PJ this
 * would be more complicated....
 ****************************************************************************/
{
	show_mouse();
	return(Success);
}

#ifdef SLUFFED
void status_line(char *fmt,...)
/*****************************************************************************
 * Print a line in a little window on top of the screen (or bottom if
 * the cursor is near the top.
 ****************************************************************************/
{
va_list args;

va_start(args, fmt);
ttextf(fmt,args,NULL);
va_end(args);
}
#endif /* SLUFFED */

void soft_status_line(char *key,...)
/*****************************************************************************
 * Print a line in a little window on top of the screen (or bottom if
 * the cursor is near the top.
 ****************************************************************************/
{
va_list args;

va_start(args, key);
soft_ttextf(key,&args);
va_end(args);
}

Errcode soft_abort(char *soft_key)
/*****************************************************************************
 * Query abuser if they wanna abort using soft_key to fetch the text.
 ****************************************************************************/
{
check_input(ANY_INPUT);
if(JSTHIT(KEYHIT|MBRIGHT) && soft_yes_no_box(soft_key))
	return(Err_abort);
return(Success);
}

char *conv_save_name(char *header, char *suff, char *button)
/*****************************************************************************
 * Bring up file requestor for some sort of save action.  Use the
 * out path.  Check that they're not trying to overwrite the current
 * input file, and query if they want to overwrite an old file with
 * the same name is such a file exists.
 ****************************************************************************/
{
char *name;

if ((name = pj_get_filename(header, suff, button, cs.out.path, cs.out.path,
							TRUE, &cs.out.scroller_top, NULL)) != NULL)
	{
	if (txtcmp(name, cs.in_name) == 0)
		{
		softerr(Err_in_use, "!%s", "conv_over_input", name);
		name = NULL;
		}
	else if (pj_exists(name) && !soft_yes_no_box("!%s", "over_old", name))
		name = NULL;
	}
return(name);
}

static Errcode resize_screen()
/*****************************************************************************
 * Bring up driver requestor to let user set which screen resolution and
 * which driver.
 ****************************************************************************/
{
Errcode err;

	if((err = go_resize_screen(init_after_screen,NULL)) < Success
		&& err != Err_abort)
	{
		return(err); /* screen init failed, fatal */
	}
	if (err >= Success)
	{
		if((err = conv_set_pencel(vb.screen->wndo.width,
								  vb.screen->wndo.height)) < Success)
			return(err);
		conv_see_cel(cs.ifi.cel);
	}
	return(MRET_RESTART);
}

Errcode conv_set_pencel(SHORT width, SHORT height)
/*****************************************************************************
 * Set the size of the destination window.
 ****************************************************************************/
{
Errcode err;

if((err = set_pencel_size(width, height, 0, 0)) >= Success)
	{
	conv_center_cel(cs.ifi.cel);
	conv_see_cel(cs.ifi.cel);
	}
else
	{
	softerr(err, "conv_pencel");
	err = set_pencel_size(vb.screen->wndo.width, vb.screen->wndo.height,0,0);
	}
fliborder_on();
return(err);
}

void freez_cel(Rcel **pcel)
/*****************************************************************************
 * Free a non-null cel pointer and set it to null.
 ****************************************************************************/
{
if (*pcel != NULL)
	{
	pj_rcel_free(*pcel);
	*pcel = NULL;
	}
}

static void blit_rcel(Rcel *cel)
/*****************************************************************************
 * Draw cel onto vb.pencel at current cel x/y offsets.
 ****************************************************************************/
{
pj_blitrect(cel, 0, 0, vb.pencel, cel->x, cel->y, cel->width, cel->height);
}

static void tile_cel(Rcel *c)
/*****************************************************************************
 * Fill up screen with copies of cel.
 ****************************************************************************/
{
int ox,oy,x,y;
int w, h;
int ix,iy;
int xmax = vb.pencel->width;
int ymax = vb.pencel->height;

if (c == NULL )
	return;
if ((w = c->width) < 1)
	return;
if ((h = c->height) < 1)
	return;
ox = x = c->x;
oy = y = c->y;
while (x > 0)
	x -= w;
while (y > 0)
	y -= h;
for (ix = x; ix < xmax; ix+=w)
	for (iy = y; iy < ymax; iy += h)
		{
		c->x = ix;
		c->y = iy;
		blit_rcel(c);
		}
c->x = ox;
c->y = oy;
}

static void background_cel(Rcel *cel)
/*****************************************************************************
 * Fill up screen with cel.  Draw color 0 around bits of screen not covered
 * by cel.
 ****************************************************************************/
{
#define BCOLOR bcolor
int offend,end;
static int bcolor = 32;

if (cel->x > 0)
	pj_set_rect(vb.pencel, BCOLOR, 0, 0, cel->x, vb.pencel->height);
end = cel->x + cel->width;
if ((offend = vb.pencel->width - end) > 0)
	pj_set_rect(vb.pencel, BCOLOR, end, 0, offend, vb.pencel->height);
if (cel->y > 0)
	pj_set_rect(vb.pencel, BCOLOR, cel->x, 0, cel->width, cel->y);
end = cel->y + cel->height;
if ((offend = vb.pencel->height - end) > 0)
	pj_set_rect(vb.pencel, BCOLOR, cel->x, end, cel->width, offend);
blit_rcel(cel);
#undef BCOLOR
}

void conv_update_cmap(Cmap *cmap)
/*****************************************************************************
 * Update screen (& pencel) color map.
 ****************************************************************************/
{
if (!cmaps_same(cmap, vb.pencel->cmap))
	{
	pj_cmap_copy(cmap, vb.pencel->cmap);
	pj_cmap_load(vb.screen,cmap);
	find_mucolors(vb.screen);
	}
}
void conv_see_cel(Rcel *cel)
/*****************************************************************************
 * Fill up screen with cel.  Draw color 0 around bits of screen not covered
 * by cel.
 ****************************************************************************/
{

if (cel == NULL)
	{
	pj_clear_rast(vb.pencel);
	return;
	}
conv_update_cmap(cel->cmap);
if (cs.no_tile)
	background_cel(cel);
else
	tile_cel(cel);
}

void conv_center_cel(Rcel *cel)
/*****************************************************************************
 * Adjust x/y position of cel so that it's on center of pencel
 ****************************************************************************/
{
if (cel != NULL)
	{
	cel->x = (vb.pencel->width - cel->width)/2;
	cel->y = (vb.pencel->height - cel->height)/2;
	}
}

void hide_mp(void)
{
	stack_hide_cgroup(vb.screen);
}

void show_mp(void)
{
	stack_show_cgroup(vb.screen);
}

void grey_cmap(Cmap *cmap)
/*****************************************************************************
 * Make up an all shades of grey color map.
 ****************************************************************************/
{
int i;
UBYTE *c = (UBYTE *)(cmap->ctab);

for (i=0; i<RGB_MAX; i++)
	{
	*c++ = i;
	*c++ = i;
	*c++ = i;
	}
}


void main(int argc, char **argv)
{
Errcode err;
UBYTE oldconfig;

	if((err = init_pj_startup(NULL,NULL,argc,argv,
						      "conv_help","aa.mu")) < Success)
	{
		goto error;
	}
	oldconfig = err;

	if((err = open_pj_startup_screen(init_after_screen)) < Success)
		goto error;

	if((err = conv_set_pencel(vb.screen->wndo.width, vb.screen->wndo.height))
							   < Success)
		goto error;

	if(!oldconfig)
		soft_continu_box("newconfig");


	{
	extern Local_pdr fli_conv_pdr;

		add_local_pdr(&fli_conv_pdr);
	}

	err = MRET_RESTART;
	for(;;)
	{
		switch(err)
		{
			case MRET_QUIT:
				exit_convert();
			case MRET_RESTART:
				err = go_converter();
				break;
			case MRET_RESIZE_SCREEN:
				err = resize_screen();
				break;
			default:
				break;
		}
	}

error:
	softerr(err, "conv_err");
	exit_convert();
}
