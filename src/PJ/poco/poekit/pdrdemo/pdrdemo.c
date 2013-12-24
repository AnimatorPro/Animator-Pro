/*****************************************************************************
 * PDRDEMO.C: A POE module to provide access to PDR picture drivers.
 *
 *	Major POE items/features demonstrated herein:
 *
 *		- Loading other REX modules from within a POE module.
 *		- Using the REX-layer init and cleanup vectors within the
 *		  POE <-> Ani Pro interface to allocate and auto-cleanup
 *		  resources we acquire during processing.
 *		- Interfacing with a PDR module in approximately the same way
 *		  that Ani Pro does.  (Might be of interest to PDR writers.)
 *		- Providing a complex library of interrelated functions to
 *		  Poco programs.
 *
 *	This is a scaled-down version of PDRACCES.POE that was shipped with
 *	Ani Pro.  The full version includes proprietary code for processing
 *	rgb/truecolor images; this demo version does not include that code.
 *	Except for the fact that this demo can't process rgb images, it is
 *	identical to the original PDRACCESS.POE.
 *
 *
 *
 * NOTES:
 *
 *		All functions for this POE exist in this one source code module.
 *		Any references to external modules or RGB processing can be
 *		ignored; they're not used in this demo version.
 *
 * MAINTENANCE:
 *
 *	05/01/91	Ian Lepore
 *				Original version, called PDRACCES.
 *
 *	10/15/91	Ian
 *				Reworked, renamed to PDRDEMO.C, included with POE demo progs.
 ****************************************************************************/

#define RGB_PROCESSING 0	// POE demo doesn't include proprietary rgb code.

/*----------------------------------------------------------------------------
 * include the usual header files
 *	(plus a bunch of UNusual ones, for REX/PDR support)
 *--------------------------------------------------------------------------*/

#include "errcodes.h"
#include "ptrmacro.h"
#include "pocorex.h"
#include "pocolib.h"
#include "syslib.h"
#include "gfx.h"
#include "math.h"
#include "stdio.h"
#include "filepath.h"

#include "picdrive.h"

/*----------------------------------------------------------------------------
 * set up the host libraries we need...
 *	in this case, we need *every* host library, because we have to pass
 *	these libraries along to the REX modules we load.  we don't actually
 *	use functions from all these libraries, but the PDRs we load might.
 *--------------------------------------------------------------------------*/

#define HLIB_TYPE_1 AA_POCOLIB
#define HLIB_TYPE_2 AA_SYSLIB
#define HLIB_TYPE_3 AA_GFXLIB
#define HLIB_TYPE_4 AA_MATHLIB
#define HLIB_TYPE_5 AA_STDIOLIB
#include <hliblist.h>

#define NUM_HOST_LIBS 5
static Libhead *libs_for_rex[NUM_HOST_LIBS+1];

/*----------------------------------------------------------------------------
 * prototypes for routines in rgb2cmap.c...
 *--------------------------------------------------------------------------*/

extern Errcode load_rgbconvert(int loadoption, Image_file *ifile, Anim_info *ainfo, Rcel *screen);

/*----------------------------------------------------------------------------
 * local data and constants...
 *--------------------------------------------------------------------------*/

Boolean report_cache_stats = FALSE;
Boolean isbatch; // global to all modules: used to disable user interaction

#ifndef TRUE
  #define TRUE	1
  #define FALSE 0
#endif

enum {
	RGB_GREY,
	RGB_C6CUBE,
	RGB_C64LEVEL,
	RGB_C256LEVEL,
	RGB_DODITHER,
	};

#define DITHERFLAG 0x00010000

static char pdr_not_loaded[] = "Fatal: No picture driver (PDR) loaded!";

static Popot cancelppt = {" Cancel"};
static Popot clearppt  = {" Cancel/Discard settings"};

static Pdr			*pdr   = NULL;
static Image_file	*ifile = NULL;
static Anim_info	ainfo;
static char 		curpdr_name[14];

static Boolean		rgb_preset_valid = FALSE;
static ULONG		rgb_preset_values = 0;

static Boolean		pdr_preset_valid;
static ULONG		pdr_preset_values;

typedef struct {
	char	*pictype;
	char	*pdrname;
	UBYTE	reads, writes, multiframes;
	} Pictopdr;

static Pictopdr 	pictypes_to_pdrnames[] = {
	{".GIF", "GIF.PDR",      TRUE,  TRUE,  FALSE,},
	{".PCX", "PCX.PDR",      TRUE,  TRUE,  FALSE,},
	{".TIF", "TIFF.PDR",     TRUE,  TRUE,  FALSE,},
	{".TGA", "TARGA.PDR",    TRUE,  TRUE,  FALSE,},
	{".LBM", "LBM.PDR",      TRUE,  FALSE, FALSE,},
	{".RIF", "RIF.PDR",      TRUE,  FALSE, FALSE,},
	{".MAC", "MAC.PDR",      TRUE,  FALSE, FALSE,},
	{".NEO", "NEO.PDR",      TRUE,  FALSE, FALSE,},
	{".PI1", "DEGAS.PDR",    TRUE,  FALSE, FALSE,},
	{".PI2", "DEGAS.PDR",    TRUE,  FALSE, FALSE,},
	{".PI3", "DEGAS.PDR",    TRUE,  FALSE, FALSE,},
	{".PC1", "DEGAS.PDR",    TRUE,  FALSE, FALSE,},
	{".PC2", "DEGAS.PDR",    TRUE,  FALSE, FALSE,},
	{".PC3", "DEGAS.PDR",    TRUE,  FALSE, FALSE,},
	{".SLD", "SLD.PDR",      TRUE,  FALSE, FALSE,},
	{".ANI", "ANIM.PDR",     TRUE,  FALSE, TRUE, },
	{".PCT", "PICT.PDR",     TRUE,  TRUE,  FALSE,},
	{".BMP", "BMP.PDR",      TRUE,  TRUE,  FALSE,},
	{".RLE", "BMP.PDR",      TRUE,  TRUE,  FALSE,},
	{".MOV", "MOV.PDR",      TRUE,  FALSE, TRUE, },
	{".RND", "RND.PDR",      TRUE,  FALSE, TRUE, },
	{NULL,NULL,FALSE,FALSE,FALSE,				 },
	 };

/*----------------------------------------------------------------------------
 * code...
 *--------------------------------------------------------------------------*/

static void strcat(char *s1, char *s2)
/*****************************************************************************
 * yer typical strcat function, since it's missing from aa_syslib.
 ****************************************************************************/
{
	while (*s1)
		++s1;

	while (*s1++ = *s2++)
		;
}

Errcode say_fatal(char *errmsg)
/*****************************************************************************
 * report show-stopping err message to user.
 ****************************************************************************/
{
	if (!isbatch)
		poeQtext(0,0,array2ppt(errmsg));
	return builtin_err = Err_nogood;
}

static void init_pdracces(void)
/*****************************************************************************
 * init the hostlib list array we use when loading other rex modules.
 *	this can't be done by static init, because the pointers to the host libs
 *	(the 'next' pointers) are not valid until after we've been loaded.
 ****************************************************************************/
{
	libs_for_rex[0] = (Libhead *)_a_a_pocolib.next;
	libs_for_rex[1] = (Libhead *)_a_a_syslib.next;
	libs_for_rex[2] = (Libhead *)_a_a_gfxlib.next;
	libs_for_rex[3] = (Libhead *)_a_a_mathlib.next;
	libs_for_rex[4] = (Libhead *)_a_a_stdiolib.next;
	libs_for_rex[5] = NULL;

	isbatch = poeIsBatchRun(); // set global batch run indicator
}

static Rcel *center_virtual_rcel(Rcel *root, Rcel *virt, int width, int height)
/*****************************************************************************
 * given root cel and pointer to an unused Rcel will center a virtual rcel
 * on root for the width and height provided. if they are the same as
 * the root screen size the pointer to the root is returned otherwise
 * pointer to centered virtual rcel.
 ****************************************************************************/
{
	Rectangle rect;

	if(width == root->width && height == root->height)
		return(root);

	rect.x = (root->width - width) / 2;
	rect.y = (root->height - height) / 2;
	rect.width = width;
	rect.height = height;
	pj_rcel_make_virtual(virt, root, &rect);
	return virt;
}

static Errcode do_qchoice(char *optstr, UBYTE curvalue, Boolean force_dialog)
/*****************************************************************************
 * display a pdr's output options to user via Qmenu, return selected option.
 * the pdr's options are stored in one long string with the items delimited
 * by newlines.  we split the items up into separate strings because that's
 * how the pocolib Qmenu routine wants to see them.
 ****************************************************************************/
{
	int   thechoice;
	Popot prompt;
	Popot choices[10];
	int   numchoices;
	char  choicedata[1024];
	char  *pdata = choicedata;
	Popot *pchoice = choices;

	if (isbatch)	// if batch run, do not interact with user, just
		return 0;	// return 0, ie, default option value.

	// first is the prompt string, isolate it...

	prompt.pt = prompt.min = prompt.max = pdata;
	while (*optstr && *optstr != '\n')
		*pdata++ = *optstr++;
	++optstr;
	*pdata++ = '\0';

	// next are the option strings...

	for (numchoices = 0; numchoices < 10 && *optstr; ++numchoices, ++pchoice) {
		pchoice->pt = pchoice->min = pchoice->max = pdata;
		*pdata++ = (curvalue == numchoices) ? '*' : ' '; // set star on cur item
		while (*optstr && *optstr != '\n')
			*pdata++ = *optstr++;
		if (*optstr)
			++optstr;
		*pdata++ = '\0';
	}

	if (numchoices == 0)
		return 0;		/* about the best we can do */

	choices[numchoices-1] = (force_dialog) ? clearppt : cancelppt;

	thechoice = poeQmenu(ptr2ppt(choices,10*sizeof(Popot)), numchoices, prompt);
	if (builtin_err)
		return builtin_err;

	if (thechoice <= 0)
		return Err_abort;

	return thechoice-1;
}

static char *skip_smukey(char *text)
/*****************************************************************************
 * some PDRs (smart ones) have softmenu keys followed by embedded text.
 * this routine will skip the smu key (if any), returning a pointer to the
 * embedded (English-language) text.  we have to do this because we don't
 * have any access to Animator's softmenu system.
 ****************************************************************************/
{
	if (RL_ISKEYTEXT(text))
		while (*text++)
			/* do nothing */ ;

	return text;
}

static Errcode opt_pdr_get(Pdroptions *popt, Boolean force_dialog)
/*****************************************************************************
 * the PDR has given us a non-NULL Pdroptions pointer, conduct Qchoice dialogs
 * so that the PDR's option values are set before pd->create_ifile() is called.
 ****************************************************************************/
{
	Errcode err;
	int 	counter;
	char	**lst;
	char	*text;
	UBYTE	*option;

	if (popt == NULL)
		return Success;

	*((ULONG *)(&popt->option1)) = pdr_preset_values;
	popt->options_valid = TRUE;

	if (pdr_preset_valid && !force_dialog)
		return Success;

	if (isbatch) {							// on a batch run, don't interact
		*((ULONG *)(&popt->option1)) = 0;	// with the user, just go with the
		return Success; 					// pdr's default (0-value) options.
	}

	lst = &popt->choicelst1;
	option = &popt->option1;

	for (counter = 0; counter < 4; ++counter, ++lst, ++option) {
		if (*lst != NULL) {
			text = skip_smukey(*lst);
			if (*text == '\0')
				continue;
			err = do_qchoice(text, *option, force_dialog);
			if(err < Success) {
				popt->options_valid = FALSE;
				return err;
			} else {
				*option = err;
			}
		}
	}

	return Success;
}

static int opt_rgb_get(Boolean force_dialog)
/*****************************************************************************
 *
 ****************************************************************************/
{

#if !RGB_PROCESSING 		// demo version doesn't support rgb, this code
		return RGB_C64LEVEL;// prevents ever showing the user a list of rgb
#else						// options, which might be misleading to the user.

	int 		 curchoice;
	int 		 thechoice;
	int 		 do_dither;
	static Popot prompt    = {" Color handling options for RGB picture: "};
	static Popot choices[] = {{" Load as greyscale"},
							  {" Load color approximation (fast)"},
							  {" Load in 64-level color (medium)"},
							  {" Load in 256-level color (slow)"},
							  {" Dither"},
							  {NULL},		// filled in at runtime
							  };
	static Popot pchoices = {choices, choices, Array_els(choices)+choices};

	if (rgb_preset_valid && !force_dialog)
		return rgb_preset_values;

	choices[5] = (force_dialog) ? clearppt : cancelppt;

	if (isbatch)			// if batch run, do not interact with user,
		return RGB_C64LEVEL;// use default of 64-level rgb.

	do_dither = rgb_preset_values & DITHERFLAG;
	curchoice = rgb_preset_values & ~DITHERFLAG;
	for (thechoice = 0; thechoice < 4; ++thechoice)
		*((char *)(choices[thechoice].pt)) = (thechoice == curchoice) ? '*' : ' ';


	for (;;) {

		*((char *)(choices[RGB_DODITHER].pt)) = (do_dither) ? '*' : ' ';

		thechoice = poeQmenu(pchoices, Array_els(choices), prompt);
		if (builtin_err)
			return builtin_err;

		switch (thechoice) {
		  default:
			return Err_abort;
		  case 1:
			return RGB_GREY 	| do_dither;
		  case 2:
			return RGB_C6CUBE	| do_dither;
		  case 3:
			return RGB_C64LEVEL | do_dither;
		  case 4:
			return RGB_C256LEVEL| do_dither;
		  case 5:
			do_dither ^= DITHERFLAG;
			break;
		}
	}
#endif /* RGB_PROCESSING */
}

static void close_ifile(void)
/*****************************************************************************
 * call the pdr close_image_file function.
 ****************************************************************************/
{
	if (ifile == NULL || pdr == NULL)
		return;

	if (pdr->close_image_file == NULL) {
		builtin_err = Err_driver_protocol;
		return;
	}

	pdr->close_image_file(&ifile);
	ifile = NULL;

}

static Errcode open_ifile(char *path)
/*****************************************************************************
 * call the pdr open_image_file function.
 ****************************************************************************/
{
	Errcode err;

	if (pdr->max_read_frames < 1)
		return Err_unimpl;
	if (pdr->open_image_file == NULL)
		return builtin_err = Err_driver_protocol;

	memset(&ainfo, 0, sizeof(ainfo));
	err = pdr->open_image_file(pdr, path, &ifile, &ainfo);
	if (err < Success)
		return err;

	ifile->pd = pdr;
	ifile->write_mode = FALSE;

	return Success;
}

static Errcode create_ifile(char *path, Rcel *screen)
/*****************************************************************************
 * call the pdr create_image_file function.
 ****************************************************************************/
{
	Errcode err;

	if (pdr->max_write_frames < 1)
		return Err_unimpl;
	if (pdr->create_image_file == NULL)
		return builtin_err = Err_driver_protocol;

	if (pdr->poptions != NULL)
		if (Success < (err = opt_pdr_get(pdr->poptions, FALSE)))
			return err;

	ainfo.width  = screen->width;
	ainfo.height = screen->height;
	ainfo.depth  = 8;
	ainfo.x = 0;
	ainfo.y = 0;
	ainfo.num_frames = 1;
	ainfo.millisec_per_frame = DEFAULT_AINFO_SPEED;

	if (pdr->spec_best_fit != NULL) /* we call this because the pdr docs say */
		pdr->spec_best_fit(&ainfo); /* we will, but we ignore the results */

	err = pdr->create_image_file(pdr, path, &ifile, &ainfo);
	if (err < Success)
		return err;

	ifile->pd = pdr;
	ifile->write_mode = TRUE;

	return Success;
}

static Errcode dummy_seek_frame(int ix, void *data)
/*****************************************************************************
 * we have to provide a seek_frame function to the pdr; this is it.
 ****************************************************************************/
{
	return Success;
}

static Errcode save_frame(Rcel *screen)
/*****************************************************************************
 *
 ****************************************************************************/
{
	Errcode err;
	Rcel	*workcel = NULL;

	if (pdr->save_frames == NULL)
		return builtin_err = Err_driver_protocol;

	if (ifile->needs_work_cel)
		return Err_unimpl;	/* for now */

	err = pdr->save_frames(ifile, screen, 1, dummy_seek_frame, NULL, workcel);

	return err;
}

static Errcode load_mappedframe(Rcel *screen)
/*****************************************************************************
 * load a normal (non-rgb) picture.
 ****************************************************************************/
{
	Errcode err;
	Rcel	vcel;
	Rcel	*loadcel;

	if (pdr->read_first_frame == NULL)
		return builtin_err = Err_driver_protocol;

	loadcel = center_virtual_rcel(screen, &vcel, ainfo.width, ainfo.height);

	err = pdr->read_first_frame(ifile, loadcel);

	return err;
}

static Errcode load_rgbframe(Rcel *screen)
/*****************************************************************************
 * set up to load rgb picture, call color or greyscale loader based on option.
 ****************************************************************************/
{
#if !RGB_PROCESSING

	return Err_rgb_convert;

#else

	Errcode err;
	Rcel	vcel;
	Rcel	*loadcel;
	ULONG	loadoption;

	if (pdr->rgb_seekstart == NULL || pdr->rgb_readline == NULL)
		return builtin_err = Err_driver_protocol;

	loadcel = center_virtual_rcel(screen, &vcel, ainfo.width, ainfo.height);

	if (Success > (err = opt_rgb_get(FALSE)))
		return err;
	else
		loadoption = err;

	err = load_rgbconvert(loadoption, ifile, &ainfo, loadcel);

	return err;

#endif /* RGB_PROCESSING */
}

void pdr_unload(void)
/*****************************************************************************
 * if a PDR is currently loaded, unload it.
 ****************************************************************************/
{
	if (ifile != NULL)
		close_ifile();

	if (pdr != NULL)
		pj_rexlib_free((Rexlib **)&pdr);
	pdr = NULL;
	curpdr_name[0] = '\0';
}


Errcode pdr_load(Popot pdrname)
/*****************************************************************************
 * load a PDR, return status.
 ****************************************************************************/
{
	static int lib_init_done = FALSE;
	Errcode err;
	char	pdrpath[PATH_SIZE];

	if (pdrname.pt == NULL)
		return builtin_err = Err_null_ref;

	if (pdr != NULL) {			/* for now, we can only have one pdr loaded */
		if (0 == strcmp(pdrname.pt, curpdr_name))
			return Success;
		pdr_unload();
	}

	pdr_preset_valid = FALSE; /* clear any preset options */

	poeGetResourceDir(array2ppt(pdrpath));
	if (builtin_err)
		return builtin_err;
	strcat(pdrpath,pdrname.pt);

	err = pj_rexlib_load(pdrpath, REX_PICDRIVER, (Rexlib **)&pdr, libs_for_rex, NULL);

	if (err < Success)
		return err;

	if (Success > (err = pj_rexlib_init((Rexlib *)pdr, NULL))) {
		pdr_unload();
		return err;
	}

	strcpy(curpdr_name, pdrname.pt);
	return Success;
}

Errcode pdr_find_load(Popot picpath)
/*****************************************************************************
 * find the right pdr (based on table lookup of file type) and load it.
 ****************************************************************************/
{
	char		*ftype;
	Pictopdr	*p2p = pictypes_to_pdrnames;
	Popot		pptpdrname;

	if (picpath.pt == NULL)
		return builtin_err = Err_null_ref;

	ftype = pj_get_path_suffix(picpath.pt);
	if (ftype == NULL || *ftype == '\0')
		return Err_pic_unknown;

	for (p2p = pictypes_to_pdrnames; p2p->pictype != NULL; ++p2p) {
		if (0 == stricmp(p2p->pictype, ftype)) {
			pptpdrname.pt = pptpdrname.min = pptpdrname.max = p2p->pdrname;
			return pdr_load(pptpdrname);
		}
	}

	return Err_pic_unknown;
}

Boolean opt_exist(void)
/*****************************************************************************
 * does the currently-loaded pdr have output options?
 ****************************************************************************/
{
	if (pdr == NULL)
		return say_fatal(pdr_not_loaded);

	return (pdr->poptions == NULL) ? FALSE : TRUE;
}

void opt_pdrclear(void)
/*****************************************************************************
 * clear any preset pdr options.
 ****************************************************************************/
{
	pdr_preset_valid = FALSE;
}

void opt_rgbclear(void)
/*****************************************************************************
 * clear any preset rgb options.
 ****************************************************************************/
{
	rgb_preset_valid = FALSE;
}

long opt_rgbpreset(Boolean do_dialog, ULONG options)
/*****************************************************************************
 * conduct rgb-load options dialog and remember the responses for later.
 ****************************************************************************/
{
	Errcode err = Success;

	rgb_preset_valid  = TRUE;
	rgb_preset_values = options;

	if (!do_dialog)
		return Success;

	if (Success > (err = opt_rgb_get(TRUE))) {
		rgb_preset_valid = FALSE;
		return err;
	} else {
		return rgb_preset_values = err;
	}
}

long opt_pdrpreset(Boolean do_dialog, ULONG options)
/*****************************************************************************
 * conduct pdr save options dialog and remember the responses for later.
 ****************************************************************************/
{
	Errcode    err;
	Pdroptions topt;   /* temp structure for getting option presets */

	if (pdr == NULL)
		return say_fatal(pdr_not_loaded);

	if (pdr->poptions == NULL)
		return Err_not_found;

	pdr_preset_valid  = TRUE;
	pdr_preset_values = options;

	if (!do_dialog)
		return Success;

	topt = *(pdr->poptions);	/* load temp from pdr's options structure */

	if (Success > (err = opt_pdr_get(&topt, TRUE))) {
		pdr_preset_valid = FALSE;
		return err;
	} else {
		return pdr_preset_values = *((ULONG *)&(topt.option1));
	}
}

void opt_enable_stats(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
	report_cache_stats = TRUE;
}

Errcode pic_getsize(Popot path, Popot width, Popot height, Popot depth)
/*****************************************************************************
 * return the width, height, and pixel depth of a picture file.
 ****************************************************************************/
{
	Errcode err;

	if (path.pt == NULL || width.pt == NULL ||
		height.pt == NULL || depth.pt == NULL)
		return builtin_err = Err_null_ref;

	if (Success > (err = pdr_find_load(path)))
		return err;

	if (Success > (err = open_ifile(path.pt))) /* open fills in ainfo */
		return err;

	*((int *)width.pt)	= ainfo.width;
	*((int *)height.pt) = ainfo.height;
	*((int *)depth.pt)	= ainfo.depth;

	close_ifile();

	return err;
}

Errcode pic_load(Popot path, Popot screen)
/*****************************************************************************
 * load a picture.
 ****************************************************************************/
{
	Errcode err;
	Boolean allow_retry = TRUE;

	/*------------------------------------------------------------------------
	 * if a pdr is already loaded, we will try to use it to load the picture,
	 * else we try to figure out which pdr to use and load it.
	 * if the pdr was pre-loaded and it fails to process the picture, we
	 * unload it and come back to here to figure out which pdr to use.	once
	 * we've been through the figure-it-out routine, we disable further
	 * retries; at that point we just can't find a pdr that can cope.
	 * the main point of this complicated retry stuff is to allow a poco
	 * program to preload a pdr we don't know about (and would thus never
	 * find for ourselves).
	 *----------------------------------------------------------------------*/

RETRY:

	if (pdr == NULL) {
		if (Success > (err = pdr_find_load(path)))
			return err;
		allow_retry = FALSE;
	}

	if (path.pt == NULL)
		return builtin_err = Err_null_ref;

	if (screen.pt == NULL)
		screen.pt = GetPicScreen();

	if (Success > (err = open_ifile(path.pt))) {
		if (allow_retry) {
			pdr_unload();
			goto RETRY;
		} else
			return err;
	}

	pj_set_rast((Rcel *)screen.pt,0);

	if (ainfo.depth > 8)
		err = load_rgbframe(screen.pt);
	else
		err = load_mappedframe(screen.pt);

	close_ifile();

	poePicDirtied();	/* signal to pj that we've changed the pic screen */

	return err;
}

Errcode pic_save(Popot path, Popot screen)
/*****************************************************************************
 * save a picture.
 ****************************************************************************/
{
	Errcode err;

	if (pdr == NULL) {
		if (Success > (err = pdr_find_load(path)))
			return err;
	}

	if (path.pt == NULL)
		return builtin_err = Err_null_ref;

	if (screen.pt == NULL)
		screen.pt = GetPicScreen();

	if (Success > (err = create_ifile(path.pt, screen.pt)))
		return err;

	err = save_frame(screen.pt);

	close_ifile();
	return err;
}

/*----------------------------------------------------------------------------
 * Setup rexlib/pocorex interface structures...
 *--------------------------------------------------------------------------*/

static Lib_proto calls[] = {
	{pdr_unload,	   "void    PicDriverUnload(void);"},
	{pdr_load,		   "Errcode PicDriverLoad(char *pdrname);"},
	{pdr_find_load,    "Errcode PicDriverFindLoad(char *picpath);"},
//	{pdr_list,		   "Errcode PicDriverList(char **list, Boolean reads, "
//							   "Boolean writes, Boolean multiframes);"},

	{opt_exist, 	   "Boolean PicPdrHasOptions(void);"},
	{opt_pdrpreset,    "long    PicPdrOptionsPreset(Boolean do_dialog, long options);"},
	{opt_rgbpreset,    "long    PicRgbOptionsPreset(Boolean do_dialog, long options);"},
	{opt_pdrclear,	   "void    PicPdrOptionsClear(void);"},
	{opt_rgbclear,	   "void    PicRgbOptionsClear(void);"},

//	{opt_enable_stats, "void    PicEnableCacheStats(void);"}, // for debugging

	{pic_load,		   "Errcode PicLoad(char *path, Screen *screen);"},
	{pic_save,		   "Errcode PicSave(char *path, Screen *screen);"},
	{pic_getsize,	   "Errcode PicGetSize(char *path,"
					   " int *width, int *height, int *depth);"},

};

Setup_Pocorex(init_pdracces, pdr_unload, "PDR Access Library v0.8", calls);

