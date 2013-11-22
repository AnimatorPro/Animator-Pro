
/* backwards.c - This is what we got to do to save a flic backwards. */

#include "jimk.h"
#include "fli.h"
#include "backward.str"

static
back_fli(title,cbuf)
char *title;
char *cbuf;
{
int i, last;

copy_form(&vf,&uf);
gb_fli_tseek(&uf,vs.frame_ix,fhead.frame_count-1,cbuf);
if (!fli_first_frame(cbuf, title, vf.p,vf.cmap, uf.p, uf.cmap, fhead.speed))
	return(0);
last = fhead.frame_count;
i = last-1;
while (--i >= 0)
	{
	copy_form(&uf, &vf);
	see_cmap();
	gb_fli_tseek(&uf,last,i,cbuf);
	if (!fli_next_frame(cbuf,vf.p, vf.cmap, uf.p, uf.cmap))
		{
		return(0);
		}
	last = i;
	}
copy_form(&uf,&vf);
gb_fli_tseek(&uf,0,fhead.frame_count-1,cbuf);
if (!fli_last_frame(cbuf,vf.p, vf.cmap, uf.p, uf.cmap))
	return(0);
return(1);
}

/* Save out current FLIC with frames backwards. */
qsave_backwards()
{
char *title;
char *cbuf;

unzoom();
if ((title = get_filename(backward_100 /* "Save flic backwards?" */, ".FLI"))!=NULL)
	{
	if (overwrite_old(title) )
		{
		push_most();
		scrub_cur_frame();
		if ((cbuf = lbegmem(CBUF_SIZE)) != NULL)
			{
			if (!back_fli(title,cbuf))
				{
				truncated(title);
				jdelete(title);
				}
			freemem(cbuf);
			fli_abs_tseek(&uf,vs.frame_ix);
			}
		pop_most();
		copy_form(&uf,&vf);
		see_cmap();
		}
	}
rezoom();
}

