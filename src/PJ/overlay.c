/* overlay.c - handle requests off the "composite..." menu.  Stuff to
   mix two FLICS onto the same screen and eventually same file.  */

#include "jimk.h"
#include "commonst.h"
#include "errcodes.h"
#include "flx.h"
#include "inks.h"
#include "menus.h"
#include "render.h"
#include "softmenu.h"

static void load_overlay(int how);
static int load_fli_overlay(char *title, int how);

void qload_overlay(void)
{
int how;

	how = soft_qchoice(NULL, "!%s", "composit_mu", vl.ink->ot.name );
	
	if (how >= 0)
	{
		unzoom();
		push_inks();
		ink_push_cel();
		load_overlay(how);
		ink_pop_cel();
		pop_inks();
		rezoom();
	}
}

static char *overl_msgs[3] = {
	"load_comp_over",
	"load_comp_under",
	"load_comp_cross",
	};

static void load_overlay(int how)
{
char *title;
SHORT ounder;
char sbuf[50];
Vset_path cpath;

	vset_get_pathinfo(FLI_PATH,&cpath);
	if((title = pj_get_filename(stack_string(overl_msgs[how],sbuf),
								".FLC;.CEL;.FLI", ok_str,
								cpath.path,NULL,FALSE, 
								&cpath.scroller_top,cpath.wildcard))!=NULL)
	{
		ounder = vs.render_under;
		if(how == 1)	/* underlay */
		{
			how = 0;
			vs.render_under = 1;
		}
		else
			vs.render_under = 0;
		draw_flibord();
		load_fli_overlay(title, how);
		vs.render_under = ounder;
	}
}
static int may_cfit_blit_cel(Rcel *cel, int fit)
{
Tcolxldat txd;
Pixel fitab[256];

	txd.tcolor = vs.inks[0];

	if (fit == 0 || fit == 4 || fit == 3)
		txd.xlat = NULL;
	else
	{
		fitting_ctable(cel->cmap->ctab, vb.pencel->cmap->ctab, fitab);
		txd.xlat = fitab;
	}
	return(rblit_cel(cel,&txd));
}

typedef struct abtdat {
	int *frame;
	USHORT *totframes;
} Vabortdat;

static Boolean olay_abort_verify(void *vabortdat)
{
	Vabortdat *vd = vabortdat;
	return(soft_yes_no_box("!%d%d", "olay_abort", *vd->frame+1, 
						   *vd->totframes));
}
static int load_fli_overlay(char *title, int how)

/* returns ecode if can't do. This reports errors.*/
{
Errcode err;
Rcel *loadcel = NULL;
Cmap *cmap = NULL;
int usr_fit = 0; /* start off not knowing how to fit colors */
Flifile flif;
int i = 0;
int fit_option;
int fcount;
Vabortdat vd;
Boolean overlay_fit;

	clear_struct(&flif);

	vd.totframes = &flif.hdr.frame_count;
	vd.frame = &i;
	set_abort_verify(olay_abort_verify,&vd);

	if((err = save_pic(screen_name, vb.pencel,0,TRUE)) < 0)
		goto error;

	if((err = pj_fli_open(title,&flif,JREADONLY)) < 0)
		goto error;

	/* allocate fli size cel to hold fli frame(s) */
	if((err = valloc_ramcel(&loadcel,flif.hdr.width,flif.hdr.height)) < 0)
		goto error;
	if((err = pj_cmap_alloc(&cmap,COLORS)) < Success)
		goto error;
	if((err = pj_fli_seek_first(&flif)) < 0)
		goto error;

	for (i=0; i<flif.hdr.frame_count; i++)
	{
		if (i != 0 && vs.frame_ix == 0)
		{
			if (soft_qchoice(NULL, "comp_past") != 0)
				goto aborted;
		}
		if ((err = pj_fli_read_next(title,&flif,loadcel,0)) < 0)
			goto error;
		if (!cmaps_same(loadcel->cmap, vb.pencel->cmap))
		{
			if (!usr_fit)
			{
				if((usr_fit = 1 + soft_qchoice(NULL, "comp_cmap")) <= 0)
					goto aborted;
			}
			fit_option = usr_fit;
		}
		else
			fit_option = 0;

		/* switch to get the color map into cmap */
		switch (fit_option)
		{
			case 0:	/* both are same.  yea! */
			case 2:	/* keep current */
			case 4:	/* No Fit */
				pj_cmap_copy(vb.pencel->cmap,cmap);
				break;
			case 3: /*use overlay cmap */
				pj_cmap_copy(loadcel->cmap, cmap);
				goto do_screen_cfit;
			case 1: /* compromise cmap */
				compromise_cmap(vb.pencel->cmap, loadcel->cmap, cmap);
			do_screen_cfit:
				cfit_rcel(vb.pencel,cmap);
				pj_cmap_copy(cmap,vb.pencel->cmap);
				see_cmap();
				break;
		}

		if (i == 0)  /* let user position first frame */
		{
			save_undo();
			switch (fit_option)	/* figure out whether to fit cel to display
								 * for user to position. */
				{
				case 0:	/* both are same.  yea! */
				case 3: /*use overlay cmap */
				case 4:	/* No Fit */
					overlay_fit = FALSE;
					break;
				case 1: /* compromise cmap */
				case 2:	/* keep current */
					overlay_fit = TRUE;
					break;
				}
			move_rcel(loadcel,overlay_fit,FALSE);

			if(!soft_yes_no_box("olay_start"))
			{
				load_pic(screen_name, vb.pencel, 0, TRUE);
				goto aborted;
			}
			/* we're committed now */
			pj_delete(screen_name);
			dirties();
		}
		switch (how)
		{
			case 0:	 /* over/under */
				if(i != 0)
					save_undo();
				if((err = may_cfit_blit_cel(loadcel, fit_option)) < 0)
					goto error;
				break;
			case  2: /* crossfade */
				if ((fcount = flif.hdr.frame_count-1) == 0)
					fcount = 1;
				pj_rcel_copy(vb.pencel, undof);
				if((err = transpblit(loadcel, 0, FALSE, 
					(i*(long)100+flif.hdr.frame_count/2)/fcount)) < Success)
				{
					goto error;
				}
				break;
		}
		if((err = sub_cur_frame()) < Success)
			goto error;
		if(poll_abort() < Success)
			goto aborted;
		vs.frame_ix++;
		check_loop();
		if((err = unfli(vb.pencel,vs.frame_ix,1)) < 0)
			goto error;
	}
	err = 0;
	goto OUT;

aborted:
	err = Err_abort;
error:
	err = softerr(err,"!%s", "comp_load", title);
OUT:
	pj_cmap_free(cmap);
	pj_rcel_free(loadcel);
	pj_fli_close(&flif);
	set_abort_verify(NULL, NULL);
	return(err);
}


