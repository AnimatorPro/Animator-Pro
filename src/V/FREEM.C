
/* freem.c - 
	This module contains routines to pop you into and out of a state which
    maximizes free memory by swapping everything possible out to disk.
    It includes two routines - 
   		push_pics() - writes buffers to disk
		pop_pics() - restores ram buffers
	These nest.  Only when the nesting count transitions between 0 and 1 is
	anything actually performed */

#include "jimk.h"
#include "fli.h"
#include "inks.h"

extern char *text_buf;

static char pushed_alt;
static char pushed_cel;
static char pushed_screen;
static char pushed_mask;

fake_push()
{
pushed_alt = pushed_cel = pushed_screen = pushed_mask = 1;
}

static
push_alt()
{
if (pushed_alt == 0)
	{
	if (alt_form)
		{
		if (!save_pic(alt_name, alt_form,0))
			return(0);
		free_screen(alt_form);
		alt_form = NULL;
		}
	}
pushed_alt++;
return(1);
}

push_screen()
{
if (pushed_screen == 0)
	{
	if (!save_pic(screen_name, render_form,0))
		return(0);
	}
pushed_screen++;
return(1);
}

static 
save_temp_cel()
{
return(save_pic(cel_name, cel,0));
}

load_temp_cel()
{
return(load_cel(cel_name));
}


push_cel()
{
if (pushed_cel == 0)
	{
	if (cel != NULL && !save_temp_cel())
		return(0);
	free_cel(cel);
	cel = NULL;
	}
pushed_cel++;
return(1);
}

static
push_mask()
{
if (pushed_mask == 0)
	{
	if (mask_plane)
		{
		if (!write_gulp(mask_name, mask_plane, (long)MASK_SIZE))
			return(0);
		freemem(mask_plane);
		mask_plane = NULL;
		}
	}
pushed_mask++;
return(1);
}

#ifdef LATER
push_text()
{
if (pushed_text == 0)
	{
	if (text_buf)
		{
		if (!save_text(text_name) )
			return(0);
		freemem(text_buf);
		text_buf = NULL;
		}
	}
pushed_text++;
return(1);
}
#endif LATER

char loaded_screen;


static
pop_alt()
{
if (--pushed_alt == 0)
	{
	if (!alt_form && jexists(alt_name) )
		{
		if ((alt_form = alloc_screen()) != NULL)
			{
			if (!load_some_pic(alt_name, alt_form))
				{
				free_screen(alt_form);
				alt_form = NULL;
				return(0);
				}
			else
				jdelete(alt_name);
			}
		}
	}
return(1);
}

pop_screen()
{
if (--pushed_screen == 0)
	{
	loaded_screen = 0;
	if (jexists(screen_name))
		{
		if ((loaded_screen = load_some_pic(screen_name, render_form))!=0)
			{
			see_cmap();
			jdelete(screen_name);
			}
		else
			return(0);
		}
	}
return(1);
}

pop_cel()
{
if (--pushed_cel == 0)
	{
	if (!cel && jexists(cel_name))
		{
		if (load_temp_cel())
			{
			jdelete(cel_name);
			}
		else
			return(0);
		}
	}
return(1);
}

static
pop_mask()
{
if (--pushed_mask == 0)
	{
	if (!mask_plane && jexists(mask_name))
		{
		if ((mask_plane = begmem(MASK_SIZE)) != NULL)
			{
			if (read_gulp(mask_name, mask_plane, (long)MASK_SIZE))
				jdelete(mask_name);
			else
				return(0);
			}
		}
	}
return(1);
}

#ifdef LATER
pop_text()
{
if (--pushed_text == 0)
	{
	if (!text_buf && jexists(text_name))
		{
		if (load_text(text_name))
			jdelete(text_name);
		else
			return(0);
		}
	}
return(1);
}
#endif LATER

push_most()
{
return(push_alt() && push_cel() && push_mask());
}

push_pics()
{
return(push_most() && push_screen());
}

pop_most()
{
pop_alt();
pop_cel();
pop_mask();
}

pop_pics()
{
pop_screen();
pop_most();
}

check_loaded_screen()
{
if (!loaded_screen)
	{
	fli_abs_tseek(&uf, vs.frame_ix);
	copy_form(&uf, &vf);
	see_cmap();
	}
else
	{
	copy_form(&vf, &uf);
	}
}

static char pshd,dps;

maybe_push_most()
{
char *pt;
int ok;

if (pshd == 0)
	{
	if ((pt = laskmem(CBUF_SIZE)) == NULL)
		{
		if (!push_most())
			return(0);
		dps = 1;
		}
	else
		{
		dps = 0;
		freemem(pt);
		}
	}
pshd++;
return(1);
}

maybe_pop_most()
{
if (--pshd == 0)
	{
	if (dps)
		pop_most();
	dps = 0;
	}
}

push_inks()
{
if (vs.draw_mode != I_SCRAPE)	/* reveal alt */
	return(push_alt());
return(TRUE);
}

ink_push_cel()
{
if (vs.draw_mode != I_TILE)
	return(push_cel());
return(TRUE);
}

ink_pop_cel()
{
if (vs.draw_mode != I_TILE)
	pop_cel();
}


pop_inks()
{
if (vs.draw_mode != I_SCRAPE)	/* reveal alt */
	pop_alt();
}


