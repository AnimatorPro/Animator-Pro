/* convflic.c - stuff that defines which file is currently loaded into
 * convert,  and routines for playing back and saving the file in
 * various formats.
 */

#include "errcodes.h"
#include "convert.h"
#include "commonst.h"
#include "resource.h"
#include "reqlib.h"
#include "softmenu.h"

Errcode conv_load_pdr(char *pdr_name, Pdr **ppdr)
/*****************************************************************************
 * Get a built in PDR or one loaded from disk.
 ****************************************************************************/
{
	char	pathbuf[PATH_SIZE];
	Errcode err;

	make_resource_name(pdr_name, pathbuf);
	err = load_pdr(pathbuf, ppdr);
	return err;
}

void conv_free_pdr(Pdr **ppdr)
/*****************************************************************************
 * Free up a PDR unless it's a built in one.
 ****************************************************************************/
{
	free_pdr(ppdr);
}

static Errcode get_new_flic(Pdr *pdr, char *pdr_name,
	Pdr **pnew_pdr, char *flic_name)
/*****************************************************************************
 * Update convert-state with new PDR, new flic_cel, and new anim_info.
 ****************************************************************************/
{
Pdr *new_pdr;
Ifi_state ifi;
Errcode err;
USHORT	newwidth;
USHORT	newheight;

		/* first open the file with the PDR.  If have an error at this
		 * point, wont have to lose the old data. */

clear_struct(&ifi);
copy_rectfields(vb.pencel, &ifi.ai);

newwidth  = ifi.ai.width;  /* save current cel w/h in case we have rgb input */
newheight = ifi.ai.height; /* and we're asked to scale to current cel size.  */

if ((err = pdr_open_ifile(pdr, flic_name, &ifi.ifi, &ifi.ai)) < Success)
	return(err);

if (ifi.ai.depth > 8)	/* pdepth greater than 8, it must be an RGB file... */
	{
	if ((cs.rgb_loadoption = err = convrgb_qoptions()) < Success)
		return err;
	if (cs.rgb_loadoption != RGB_SCALED) /* if user wants scaling, leave the */
		{								 /* current cel w/h unchanged, we'll */
		newwidth  = ifi.ai.width;		 /* scale to it.  otherwise set the  */
		newheight = ifi.ai.height;		 /* new cel w/h to the same size as  */
		}								 /* the image to be loaded.		  */
	cs.is_rgbinput = TRUE;
	ifi.ai.depth = 8;		   /* will be 8 when we get done with it */
	}
else
	{
	newwidth  = ifi.ai.width;  /* for non-rgb images, we don't do scaling on */
	newheight = ifi.ai.height; /* a load, set cel w/h to incoming image size.*/
	cs.is_rgbinput = FALSE;
	}

		/* From here on we're committed.  Errors will result in a
		 * blank screen. */
pdr_close_ifile(&cs.ifi.ifi);
if ((new_pdr = *pnew_pdr) != NULL)	/* if have a new PDR, transfer it to
									 * the cs.pdr. */
	{
	conv_free_pdr(&cs.ifi.pdr);
	cs.ifi.pdr = new_pdr;
	strcpy(cs.pdr_name, pdr_name);
	*pnew_pdr = NULL;
	}

freez_cel(&cs.ifi.cel); 			/* free old fli cel */
ifi.pdr = cs.ifi.pdr;				/* transfer pdr */
cs.ifi = ifi;
strcpy(cs.in_name, flic_name);

err = valloc_anycel(&cs.ifi.cel, newwidth, newheight);
pj_get_default_cmap(cs.ifi.cel->cmap);

return(err);
}


Errcode get_new_pdr(Pdr **ppdr, char *pdr_name)
/*****************************************************************************
 * Load new PDR, and set *ppdr toit.  Report error if any.
 ****************************************************************************/
{
Errcode err;

if((err = conv_load_pdr(pdr_name, ppdr)) < Success)
	err = cant_query_driver(err,pdr_name);
return(err);
}


Errcode get_a_flic(char *pdr_name, char *name, char *suff)
/*****************************************************************************
 * Load in PDR and ask user for a flic to load.
 * If user chooses a flic, then make the new PDR the
 * current PDR, and go try to load first frame of the flic into
 * a new pencel.
 ****************************************************************************/
{
char sufbuf[PDR_SUFFI_SIZE];
Pdr *pdr;
Errcode err;
char owild[WILD_SIZE];
char titbuf[40];

if ((err = get_new_pdr(&pdr, pdr_name)) < Success)
	{
	goto OUT;
	}
if (suff == NULL)	/* If they don't supply a suffix then use PDR default */
	{
	pdr_get_suffi(pdr, sufbuf);
	suff = sufbuf;
	}
if (name == NULL)
	{
	strcpy(owild,cs.in.wildcard);		/* Save wildcard */
	if (strcmp(pdr_name, cs.pdr_name) != 0) 
								/* If have new PDR trash old wildcard */
		cs.in.wildcard[0] = 0;
	name = pj_get_filename(stack_string("conv_loadf",titbuf), suff, load_str,
						   cs.in.path, cs.in.path,
						   FALSE, &cs.in.scroller_top,
						   cs.in.wildcard);
	if (name == NULL)	/* They canceled out of file requestor */
		strcpy(cs.in.wildcard,owild);
	}
if (name != NULL)
	{
	if ((err = get_new_flic(pdr, pdr_name, &pdr, name)) >= Success)
		{
		soft_put_wait_box("!%s", "conv_waitld", name);
		if (cs.is_rgbinput)
			{
			if ((err = convrgb_read_image(cs.ifi.ifi, cs.ifi.cel, &cs.ifi.ai,
							cs.rgb_loadoption)) < Success)
				goto OUT;
			}
		else
			{
			if ((err = pdr_read_first(cs.ifi.ifi, cs.ifi.cel)) < Success)
				goto OUT;
			}
		cs.scalew = cs.ifi.ai.width;
		cs.scaleh = cs.ifi.ai.height;
		conv_center_cel(cs.ifi.cel);
		conv_see_cel(cs.ifi.cel);
		}
	}
OUT:
/* pdr_close_ifile(&cs.ifi.ifi); */
conv_free_pdr(&pdr);
return(softerr(err,NULL));
}

void view_pic()
/*****************************************************************************
 * Hide mouse and menus until user clicks.
 ****************************************************************************/
{
hide_mp();
hide_mouse();
wait_click();
show_mouse();
show_mp();
}

void view_flic()
/*****************************************************************************
 * Play flic until there's an error or user hits a key or right clicks.
 ****************************************************************************/
{
Errcode err = Success;

if (cs.ifi.ai.num_frames < 2)		/* save some work if only 1 frame */
	{
	view_pic();
	return;
	}
hide_mouse();
for (;;)
	{
	check_input(ANY_INPUT);
	if(JSTHIT(KEYHIT|MBRIGHT))
		break;
	if (++cs.ifi.frame_ix == cs.ifi.ai.num_frames)
		{
		cs.ifi.frame_ix = 0;
		err = pdr_read_first(cs.ifi.ifi, cs.ifi.cel);
		}
	else
		{
		err = pdr_read_next(cs.ifi.ifi, cs.ifi.cel);
		}
	if (err < Success)
		goto ERROR;
	conv_see_cel(cs.ifi.cel);
	}
ERROR:
	show_mouse();
	softerr(err, "!%s%d", "conv_view", cs.in_name,
		cs.ifi.frame_ix+1);
}

static Errcode return_success()
/*****************************************************************************
 * Food for things that want a function but don't really need on in this case.
 ****************************************************************************/
{
return(Success);
}

Errcode save_a_pic(char *pdr_name)
/*****************************************************************************
 * Query user for filename and save it in a single frame format defined
 * by pdr_name.
 ****************************************************************************/
{
	char sufbuf[PDR_SUFFI_SIZE];
	Pdr *pdr;
	Errcode err;
	char *pt;
	Anim_info ai;
	Image_file *ifile = NULL;
	char sbuf[40];

	if ((err = get_new_pdr(&pdr, pdr_name)) < Success)
		goto OUT;

	if (pdr->poptions != NULL)
		{
		if ((err = conv_pdropt_qchoice(pdr->poptions)) < Success)
			goto OUT;
		}

	pdr_get_suffi(pdr, sufbuf);
	if ((pt = conv_save_name(stack_string("conv_svpic", sbuf), 
							 sufbuf, save_str)) == NULL)
	{
		goto OUT;
	}
	ai = cs.ifi.ai;
	ai.width = vb.pencel->width;
	ai.height = vb.pencel->height;
	ai.num_frames = 1;
	soft_put_wait_box("!%s", "conv_waitsv", pt);
	if ((err = pdr_create_ifile(pdr, pt, &ifile, &ai)) < Success)
		goto OUT;
	if ((err = pdr_save_frames(ifile, vb.pencel,1,return_success,NULL,NULL))
		< Success)
		{
		pdr_close_ifile(&ifile);
		pj_delete(pt);		/* on error delete bogus file */
		err = softerr(err,"!%s", "cant_save", pt);
		goto OUT;
		}
OUT:
	pdr_close_ifile(&ifile);
	conv_free_pdr(&pdr);
	return(softerr(err,NULL));
}

Errcode ifi_cel_seek(int ix, void *data)
/*****************************************************************************
 * Routine provided to pic-driver for saving animations.  This will
 * seek to a given frame of the loaded in fli.
 ****************************************************************************/
{
Errcode err;

if (ix == cs.ifi.frame_ix)
	return(Success);
if (ix < cs.ifi.frame_ix)
	{
	if ((err = pdr_read_first(cs.ifi.ifi, cs.ifi.cel)) < Success)
		return(err);
	cs.ifi.frame_ix = 0;
	}
while (ix > cs.ifi.frame_ix)
	{
	if ((err = pdr_read_next(cs.ifi.ifi, cs.ifi.cel)) < Success)
		return(err);
	++cs.ifi.frame_ix;
	}
return(Success);
}

Errcode conv_seek(int ix, void *data)
/*****************************************************************************
 * Routine provided to pic-driver for saving animations.  This will
 * seek to a given frame of the loaded in fli.
 ****************************************************************************/
{
Errcode err;

if ((err = soft_abort("conv_abort")) < Success)
	return(err);
err = ifi_cel_seek(ix,data);
conv_see_cel(cs.ifi.cel);
return(err);
}

Errcode save_a_flic(char *pdr_name, char *name, int frames,
					Errcode (*iseek)(int ix, void *data))
/*****************************************************************************
 * Query user for filename and save it in a multiple frame format defined
 * by pdr_name.
 ****************************************************************************/
{
Errcode err;
Pdr *pdr;
Image_file *ifile = NULL;
Anim_info ai;
char sufbuf[PDR_SUFFI_SIZE];
Rcel *work_cel = NULL;
char sbuf[40];

if ((err = get_new_pdr(&pdr, pdr_name)) < Success)
	goto OUT;
ai = cs.ifi.ai;
ai.width = vb.pencel->width;
ai.height = vb.pencel->height;
ai.num_frames = frames;
ai.depth = 8;
if (!pdr->create_image_file)	/* make sure it's a writable device */
	{
	err = softerr(Err_no_message,"!%s", "readonly_pdr", pdr_name );
	goto OUT;
	}
if (!pdr_best_fit(pdr, &ai))	/* and that it can handle this resolution */
	{
	if (ai.num_frames != frames)
		err = softerr(Err_unimpl, "!%s%d", "conv_num_frames",
					  pdr_name, ai.num_frames);
	else
		err = softerr(Err_wrong_res, "!%d%d", "conv_wrong_res",
					  ai.width, ai.height);
	goto OUT;
	}
pdr_get_suffi(pdr, sufbuf);
if (name == NULL)
	{
	if ((name = conv_save_name(stack_string("conv_svflic", sbuf),
										sufbuf, save_str)) == NULL)
		{
		err = Err_abort;
		goto OUT;
		}
	}
if ((err = valloc_anycel(&work_cel, ai.width, ai.height))
	< Success)
	goto OUT;
if ((err = (*iseek)(0,NULL)) < Success)	/* seek back to first frame */
	goto OUT;
if ((err = pdr_create_ifile(pdr, name, &ifile, &ai)) < Success)
	goto OUT;
if ((err = pdr_save_frames(ifile, vb.pencel,frames,iseek,
						   NULL,work_cel)) < Success)
	{
	pdr_close_ifile(&ifile);
	pj_delete(name);		/* on error delete bogus file */
	goto OUT;
	}
OUT:
pdr_close_ifile(&ifile);
freez_cel(&work_cel);
conv_free_pdr(&pdr);
if (name == NULL)	/* so don't crash reporting error below */
	name = "";
return(softerr(err,"!%s", "cant_save", name));
}
