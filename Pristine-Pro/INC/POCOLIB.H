#ifndef POCOLIB_H
#define POCOLIB_H

#ifndef LINKLIST_H
	#include "linklist.h"
#endif

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifdef PUBLIC_CODE
	#include "rexlib.h"
#endif

#include "stdarg.h"

/******************************************************************************
 * Basic poco data structures everyone needs to know about...
 *****************************************************************************/

typedef struct popot		/* The poco bounds-checked pointer type */
	{
	void	*pt;
	void	*min;
	void	*max;
	} Popot;

typedef struct string_ref
	{
	Dlnode node;
	int ref_count;
	Popot string;
	} String_ref;
typedef String_ref *PoString;

/* Macro to extract the char * from a poString */
#define PoStringBuf(s) ((*(s))->string.pt)

typedef union pt_num		/* Overlap popular datatypes in the same space */
	{
	int 	i;
	int 	inty;
	short	s;
	UBYTE	*bpt;
	char	c;
	long	l;
	ULONG	ul;
	float	f;
	double	d;
	void	*p;
	int 	doff;		/* data offset */
	int 	(*func)();	/* code pointer */
	Popot	ppt;
	PoString postring;
	} Pt_num;

typedef struct lib_proto	/* Poco library prototype lines */
	{
	void	*func;
	char	*proto;
	} Lib_proto;

typedef struct poco_lib 	/* Poco library main control structure */
	{
	struct
	  poco_lib *next;
	char	   *name;
	Lib_proto  *lib;
	int 	   count;
	Errcode    (*init)(struct poco_lib *lib);
	void	   (*cleanup)(struct poco_lib *lib);
	void	   *local_data;
	Dlheader   resources;
	void	   *rexhead;
	char	   reserved[12];
	} Poco_lib;


#define RNODE_FIELDS \
	Dlnode node; \
	void *resource;

typedef struct rnode		/* Used for resource tracking in builtin libs */
	{
	RNODE_FIELDS
	} Rnode;

/*****************************************************************************
 * handy macros for library and poe routines...
 *
 *	Popot_bufsize	- Evaluates to the number of bytes available between the
 *					  current pointer location and the end of the buffer.
 *	Popot_bufcheck	- Evaluates to Err_null_ref if the pointer is NULL, to
 *					  Err_buf_too_small if there aren't at least 'length'
 *					  bytes available in the buffer, or Success if all is well.
 *					  Sets builtin_err.
 *	Popot_make_null 	- Set a Popot to NULL.
 ****************************************************************************/

#define Popot_bufsize(p)			((char *)((p)->max)-(char *)((p)->pt)+1)
#define Popot_bufcheck(p,length)	(builtin_err = (((p)->pt == NULL) ?\
										Err_null_ref :\
										((Popot_bufsize((p)) < (length)) ? \
										Err_buf_too_small :\
										Success)))
#define Popot_make_null(p)	((p)->pt = (p)->min = (p)->max = NULL)

#ifndef PUBLIC_CODE

/******************************************************************************
 * Private prototypes for PJ's use...
 *****************************************************************************/

extern Errcode	po_init_libs(Poco_lib *lib);
extern void 	po_cleanup_libs(Poco_lib *lib);
extern Errcode	poco_cont_ops(void *code_pt, Pt_num *pret, int argslen, ...);
extern Errcode	po_check_formatf(int maxlen, int vargcount, int vargsize,
						  char *fmt, va_list pargs);
extern void 	po_free(Popot ppt);
extern Popot	po_malloc(int size);
extern Popot	poco_lmalloc(long size);
extern Popot	po_calloc(int size_el, int el_count);
extern void 	poco_freez(Popot *pt);
extern char 	*po_fuf_name(void *fuf);
extern void 	*po_fuf_code(void *fuf);
extern Rnode	*po_in_rlist(Dlheader *sfi, void *f);
extern Errcode	pj_load_pocorex(Poco_lib **lib, char *name,char *id_str);
extern void 	pj_free_pocorexes(Poco_lib **libs);
extern int		po_findpoe(char *libname, Lib_proto **plibreturn);
extern Errcode	po_poe_overtime(void *effect, void *data);
extern Errcode	po_poe_oversegment(void *effect, void *data);
extern Errcode	po_poe_overall(void *effect, void *data);
extern Popot	po_ptr2ppt(void *ptr, int bytes);
extern void 	*po_ppt2ptr(Popot ppt);

extern Poco_lib po_load_save_lib;
extern Poco_lib po_turtle_lib;
extern Poco_lib po_draw_lib;
extern Poco_lib po_blit_lib;
extern Poco_lib po_alt_lib;
extern Poco_lib po_cel_lib;
extern Poco_lib po_user_lib;
extern Poco_lib po_str_lib;
extern Poco_lib po_mem_lib;
extern Poco_lib po_FILE_lib;
extern Poco_lib po_misc_lib;
extern Poco_lib po_mode_lib;
extern Poco_lib po_text_lib;
extern Poco_lib po_time_lib;
extern Poco_lib po_dos_lib;
extern Poco_lib po_math_lib;
extern Poco_lib po_optics_lib;
extern Poco_lib po_globalv_lib;
extern Poco_lib po_title_lib;
extern Poco_lib po_tween_lib;
extern Poco_lib po_flicplay_lib;

extern Errcode	builtin_err;

/* ndef PUBLIC_CODE */ #endif

#ifndef POCO_H	/* the poco subsytem doesn't want to see all the following... */

/*****************************************************************************
 * Stuctures for the Poco builtin library and poe runtime library interfaces.
 *
 * The host treats these structures as arrays of Lib_proto structures.
 * From the host side, these arrays are used internally to provide poco with
 * prototypes and pointers for each of the builtin library routine.
 *
 * The poe interface is interested in the function pointers, but is blind
 * to the prototype strings, which is why the strings are casually named.
 * On the library side, these structures provide the jump tables used to
 * access the poco builtin library functions from within a poe module.
 ****************************************************************************/

/*----------------------------------------------------------------------------
 * Optics libarary
 *--------------------------------------------------------------------------*/

typedef struct polib_optics {
	void	 (*plOptClearState)(void);
	char	 *protostr1;
	void	 (*plOptSetState)(Popot optState);
	char	 *protostr2;
	void	 (*plOptGetState)(Popot optState);
	char	 *protostr3;
	void	 (*plOptFreeState)(Popot optState);
	char	 *protostr4;
	void	 (*plOptClearPos)(void);
	char	 *protostr5;
	void	 (*plOptSetPos)(Popot optPos);
	char	 *protostr6;
	void	 (*plOptGetPos)(Popot optPos);
	char	 *protostr7;
	void	 (*plOptClearPath)(void);
	char	 *protostr8;
	void	 (*plOptSetPath)(int ptcount, Popot x, Popot y);
	char	 *protostr9;
	int 	 (*plOptGetPath)(Popot x, Popot y);
	char	 *protostr10;
	void	 (*plOptFreePath)(Popot x, Popot y);
	char	 *protostr11;
	void	 (*plOptDefaultCenters)(void);
	char	 *protostr12;
	void	 (*plOptContinue)(void);
	char	 *protostr13;
	int 	 (*plOptGetElement)(void);
	char	 *protostr14;
	Errcode  (*plOptSetElement)(int el);
	char	 *protostr15;
	Errcode  (*plOptToFrame)(double time);
	char	 *protostr16;
	Errcode  (*plOptToSegment)(int start, int stop);
	char	 *protostr17;
	Errcode  (*plOptToAll)(void);
	char	 *protostr18;
	} PolibOptics;

/*----------------------------------------------------------------------------
 * SwapScreen library
 *--------------------------------------------------------------------------*/

typedef struct polib_swap {
	Boolean  (*plSwapExists)(void);
	char	 *protostr1;
	void	 (*plSwapClip)(void);
	char	 *protostr2;
	void	 (*plSwapRelease)(void);
	char	 *protostr3;
	Errcode  (*plSwapTrade)(void);
	char	 *protostr4;
	} PolibSwap;

/*----------------------------------------------------------------------------
 * Screen library
 *--------------------------------------------------------------------------*/

typedef struct polib_screen {
	Popot	 (*plGetPicScreen)(void);
	char	 *protostr1;
	Popot	 (*plGetSwapScreen)(void);
	char	 *protostr2;
	Popot	 (*plGetUndoScreen)(void);
	char	 *protostr3;
	Popot	 (*plGetCelScreen)(void);
	char	 *protostr3a;
	Errcode  (*plAllocScreen)(Popot screen, int width, int height);
	char	 *protostr4;
	void	 (*plFreeScreen)(Popot screen);
	char	 *protostr5;
	void	 (*plGetScreenSize)(Popot s, Popot x, Popot y);
	char	 *protostr6;
	void	 (*plSetPixel)(Popot s, int color, int x, int y);
	char	 *protostr7;
	int 	 (*plGetPixel)(Popot s, int x, int y);
	char	 *protostr8;
	void	 (*plSetBlock)(Popot s, Popot pixbuf, int x, int y, int width, int height);
	char	 *protostr9;
	void	 (*plGetBlock)(Popot s, Popot pixbuf, int x, int y, int width, int height);
	char	 *protostr10;
	void	 (*plIconBlit)(Popot source, int snext, int sx, int sy, int width, int height, Popot dest, int dx, int dy, int color);
	char	 *protostr11;
	void	 (*plBlit)(Popot source, int sx, int sy, int width, int height, Popot dest, int dx, int dy);
	char	 *protostr12;
	void	 (*plKeyBlit)(Popot source, int sx, int sy, int width, int height, Popot dest, int dx, int dy, int key_color);
	char	 *protostr13;
	void	 (*plCopyScreen)(Popot source, Popot dest);
	char	 *protostr14;
	void	 (*plTradeScreen)(Popot a, Popot b);
	char	 *protostr15;
	void	 (*plPicDirtied)(void);
	char	 *protostr16;
/* New with Ani Pro 1.5 */
	Popot	 (*plGetPhysicalScreen)(void);
	char	 *protostr17;
	void	 (*plSetBox)(Popot screen, int color, int x, int y, int width, int height);
	char	 *protostr18;
	void	 (*plMenuText)(Popot screen, int color, int x, int y, Popot text);
	char	 *protostr19;
	int 	(*plMenuTextWidth)(Popot text);
	char	 *protostr20;
	int 	(*plMenuTextHeight)(void);
	char	*protostr21;
	void	(*plGetMenuColors)(Popot black, Popot grey, Popot light, Popot bright, Popot red);
	char	*protostr22;
	} PolibScreen;

/*----------------------------------------------------------------------------
 * Cel library
 *--------------------------------------------------------------------------*/

typedef struct polib_cel {
	Boolean  (*plCelExists)(void);
	char	 *protostr1;
	void	 (*plCelPaste)(void);
	char	 *protostr2;
	void	 (*plCelMove)(int dx, int dy);
	char	 *protostr3;
	void	 (*plCelMoveTo)(int x, int y);
	char	 *protostr4;
	void	 (*plCelTurn)(double angle);
	char	 *protostr5;
	void	 (*plCelTurnTo)(double angle);
	char	 *protostr6;
	Errcode  (*plCelNextFrame)(void);
	char	 *protostr7;
	Errcode  (*plCelBackFrame)(void);
	char	 *protostr8;
	Errcode  (*plCelSetFrame)(int frame);
	char	 *protostr9;
	int 	 (*plCelGetFrame)(void);
	char	 *protostr10;
	int 	 (*plCelFrameCount)(void);
	char	 *protostr11;
	Errcode  (*plCelWhere)(Popot x, Popot y, Popot angle);
	char	 *protostr12;
	Errcode  (*plCelGet)(int x, int y, int width, int height);
	char	 *protostr13;
	Errcode  (*plCelClip)(void);
	char	 *protostr14;
	Errcode  (*plCelRelease)(void);
	char	 *protostr15;
	Errcode  (*plCelClipChanges)(void);
	char     *protostr16;
	} PolibCel;

/*----------------------------------------------------------------------------
 * DOS library
 *--------------------------------------------------------------------------*/

typedef struct polib_dos {
	Errcode  (*plfnsplit)(Popot path, Popot device, Popot dir, Popot file, Popot suf);
	char	 *protostr1;
	Errcode  (*plfnmerge)(Popot path, Popot device, Popot dir, Popot file, Popot suf);
	char	 *protostr2;
	Boolean  (*plDosExists)(Popot filename);
	char	 *protostr3;
	Errcode  (*plDosCopy)(Popot source, Popot dest);
	char	 *protostr4;
	Errcode  (*plDosDelete)(Popot filename);
	char	 *protostr5;
	Errcode  (*plDosRename)(Popot old, Popot new);
	char	 *protostr6;
	Errcode  (*plSetDir)(Popot dir);
	char	 *protostr7;
	Errcode  (*plGetDir)(Popot dir);
	char	 *protostr8;
	int 	 (*plDirList)(Popot list, Popot wild, Boolean get_dirs);
	char	 *protostr9;
	void	 (*plFreeDirList)(Popot list);
	char	 *protostr10;
	void	 (*plGetResourceDir)(Popot dir);
	char	 *protostr11;
	void	 (*plGetProgramDir)(Popot dir);
	char	 *protostr12;
	} PolibDos;

/*----------------------------------------------------------------------------
 * Drawing library
 *--------------------------------------------------------------------------*/

typedef struct polib_draw {
	void	 (*plGetSize)(Popot width, Popot height);
	char	 *protostr1;
	void	 (*plGetAspectRatio)(Popot x, Popot y);
	char	 *protostr2;
	int 	 (*plGetColor)(void);
	char	 *protostr3;
	void	 (*plSetColor)(int color);
	char	 *protostr4;
	void	 (*plClear)(void);
	char	 *protostr5;
	void	 (*plDot)(int x, int y);
	char	 *protostr6;
	int 	 (*plGetDot)(int x, int y);
	char	 *protostr7;
	void	 (*plLine)(int x1, int y1, int x2, int y2);
	char	 *protostr8;
	void	 (*plBox)(int x, int y, int w, int h);
	char	 *protostr9;
	void	 (*plCircle)(int cx, int cy, int radius);
	char	 *protostr10;
	Errcode  (*plPoly)(int ptcount, Popot x, Popot y);
	char	 *protostr11;
	Errcode  (*plSpline)(int ptcount, Popot x, Popot y);
	char	 *protostr12;
	Errcode  (*plOval)(double angle, int xcen, int ycen, int xrad, int yrad);
	char	 *protostr13;
	Errcode  (*plStar)(double angle, int xcen, int ycen, int rad);
	char	 *protostr14;
	Errcode  (*plPetal)(double angle, int xcen, int ycen, int rad);
	char	 *protostr15;
	Errcode  (*plRpoly)(double angle, int xcen, int ycen, int rad);
	char	 *protostr16;
	Errcode  (*plSpiral)(double angle, int xcen, int ycen, int rad, double turns);
	char	 *protostr17;
	Errcode  (*plFill)(int x, int y);
	char	 *protostr18;
	Errcode  (*plFillTo)(int x, int y, int to_color);
	char	 *protostr19;
	Errcode  (*plEdge)(int color);
	char	 *protostr20;
	void	 (*plSetColorMap)(int index, int r, int g, int b);
	char	 *protostr21;
	void	 (*plGetColorMap)(int index, Popot r, Popot g, Popot b);
	char	 *protostr22;
	void	 (*plGetScreenColorMap)(Popot screen, Popot maparray);
	char	 *protostr23;
	void	 (*plSetScreenColorMap)(Popot screen, Popot maparray);
	char	 *protostr24;
	/* New with AniPro 1.5 */
	void	 (*plGetPhysicalSize)(Popot width, Popot height);
	char	 *protostr25;
	int 	 (*plGetBoxBevel)(void);
	char	 *protostr26;
	void	 (*plSetBoxBevel)(int new_bevel);
	char	 *protostr27;
	void	 (*plSetCluster)(int cluster_size, Popot cluster);
	char	 *protostr28;
	Errcode  (*plGetCluster)(Popot cluster_size, Popot cluster);
	char	 *protostr29;
	void	 (*plHLStoRGB)(int h, int l, int s, Popot r, Popot g, Popot b);
	char	 *protostr30;
	void	 (*plRGBtoHLS)(int r, int g, int b, Popot h, Popot l, Popot s);
	char	 *protostr31;
	int 	 (*plClosestColorInScreen)(Popot screen, int r, int g, int b);
	char	 *protostr32;
	Errcode  (*plSqueezeColors)(Popot source_map, int source_count, int *dest_map, int dest_count);
	char	 *protostr33;
	ErrCode  (*plFitScreenToColorMap)(Popot s, Popot new_colors, Boolean keep_key);
	char	 *protostr34;
	} PolibDraw;

/*----------------------------------------------------------------------------
 * AA file library
 *--------------------------------------------------------------------------*/

typedef struct polib_aafile {
	Errcode  (*plLoadFlic)(Popot name);
	char	 *protostr1;
	Errcode  (*plSaveFlic)(Popot name);
	char	 *protostr2;
	Errcode  (*plLoadPic)(Popot name);
	char	 *protostr3;
	Errcode  (*plSavePic)(Popot name);
	char	 *protostr4;
	Errcode  (*plLoadCel)(Popot name);
	char	 *protostr5;
	Errcode  (*plSaveCel)(Popot name);
	char	 *protostr6;
	Errcode  (*plLoadPath)(Popot name);
	char	 *protostr7;
	Errcode  (*plSavePath)(Popot name);
	char	 *protostr8;
	Errcode  (*plLoadPoly)(Popot name);
	char	 *protostr9;
	Errcode  (*plSavePoly)(Popot name);
	char	 *protostr10;
	Errcode  (*plLoadColors)(Popot name);
	char	 *protostr11;
	Errcode  (*plSaveColors)(Popot name);
	char	 *protostr12;
	Errcode  (*plLoadTitles)(Popot name);
	char	 *protostr13;
	Errcode  (*plSaveTitles)(Popot name);
	char	 *protostr14;
	Errcode  (*plLoadMask)(Popot name);
	char	 *protostr15;
	Errcode  (*plSaveMask)(Popot name);
	char	 *protostr16;
	Errcode  (*plSaveScreenPic)(Popot screen, Popot name);
	char	 *protostr17;
	Errcode  (*plLoadScreenPic)(Popot screen, Popot name);
	char	 *protostr18;
	} PolibAAFile;

/*----------------------------------------------------------------------------
 * Misc library
 *--------------------------------------------------------------------------*/

typedef struct polib_misc {
	void	 (*plexit)(Errcode err);
	char	 *protostr1;
	void	 (*plNewFlic)(void);
	char	 *protostr2;
	Errcode  (*plReset)(void);
	char	 *protostr3;
	Errcode  (*plResizeReset)(int width, int height);
	char	 *protostr4;
	int 	 (*plGetChangeCount)(void);
	char	 *protostr4a;
	int 	 (*plPocoVersion)(void);
	char	 *protostr5;
	void	 (*plRedo)(void);
	char	 *protostr6;
	void	 (*plRestore)(void);
	char	 *protostr7;
	int 	 (*plrnd)(int max);
	char	 *protostr8;
	int 	 (*plrand)(void);
	char	 *protostr9;
	void	 (*plsrand)(int seed);
	char	 *protostr10;
	long	 (*plclock)(void);
	char	 *protostr11;
	Boolean  (*plIsBatchRun)(void);
	char	 *protostr12;
	Boolean  (*plPocoChainTo)(Popot program_path);
	char	 *protostr13;
	Errcode  (*plsystem)(Popot command_line);	/* new with v1.5 */
	char	 *protostr14;
	} PolibMisc;

/*----------------------------------------------------------------------------
 * Mode library
 *--------------------------------------------------------------------------*/

typedef struct polib_mode {
	Errcode  (*plSetInk)(Popot name);
	char	 *protostr1;
	void	 (*plGetInk)(Popot buf);
	char	 *protostr2;
	void	 (*plSetInkStrength)(int percent);
	char	 *protostr3;
	int 	 (*plGetInkStrength)(void);
	char	 *protostr4;
	void	 (*plSetInkDither)(Boolean dither);
	char	 *protostr5;
	Boolean  (*plGetInkDither)(void);
	char	 *protostr6;
	void	 (*plSetFilled)(Boolean fill);
	char	 *protostr7;
	Boolean  (*plGetFilled)(void);
	char	 *protostr8;
	void	 (*plSetBrushSize)(int size);
	char	 *protostr9;
	int 	 (*plGetBrushSize)(void);
	char	 *protostr10;
	void	 (*plSetKeyMode)(Boolean clear);
	char	 *protostr11;
	Boolean  (*plGetKeyMode)(void);
	char	 *protostr12;
	void	 (*plSetKeyColor)(int color);
	char	 *protostr13;
	int 	 (*plGetKeyColor)(void);
	char	 *protostr14;
	void	 (*plSetMaskUse)(Boolean use_it);
	char	 *protostr15;
	Boolean  (*plGetMaskUse)(void);
	char	 *protostr16;
	void	 (*plSetMaskCreate)(Boolean make_it);
	char	 *protostr17;
	Boolean  (*plGetMaskCreate)(void);
	char	 *protostr18;
	void	 (*plSetStarPoints)(int points);
	char	 *protostr19;
	int 	 (*plGetStarPoints)(void);
	char	 *protostr20;
	void	 (*plSetStarRatio)(int ratio);
	char	 *protostr21;
	int 	 (*plGetStarRatio)(void);
	char	 *protostr22;
	void	 (*plSetSplineTCB)(int t, int c, int b);
	char	 *protostr23;
	void	 (*plGetSplineTCB)(Popot t, Popot c, Popot b);
	char	 *protostr24;
	void	 (*plSetTwoColorOn)(Boolean setit);
	char	 *protostr25;
	Boolean  (*plGetTwoColorOn)(void);
	char	 *protostr26;
	void	 (*plSetTwoColor)(int color);
	char	 *protostr27;
	int 	 (*plGetTwoColor)(void);
	char	 *protostr28;
	void	 (*plSetClosed)(Boolean closed);
	char	 *protostr29;
	Boolean  (*plGetClosed)(void);
	char	 *protostr30;
	void	 (*plSetCycleDraw)(Boolean cycle);
	char	 *protostr31;
	Boolean  (*plGetCycleDraw)(void);
	char	 *protostr32;
	Boolean  (*plGetMultiFrame)(void);
	char	 *protostr33;
	void	 (*plSetMultiFrame)(Boolean multi);
	char	 *protostr34;
	} PolibMode;

/*----------------------------------------------------------------------------
 * Text library
 *--------------------------------------------------------------------------*/

typedef struct polib_text {
	void	 (*plText)(int x, int y, Popot string);
	char	 *protostr1;
	void	 (*plWordWrap)(int x, int y, int width, int height, Popot text);
	char	 *protostr2;
	void	 (*plSetJustify)(int just);
	char	 *protostr3;
	int 	 (*plGetJustify)(void);
	char	 *protostr4;
	int 	 (*plStringWidth)(Popot string);
	char	 *protostr5;
	int 	 (*plFontHeight)(void);
	char	 *protostr6;
	int 	 (*plTallestChar)(void);
	char	 *protostr7;
	void	 (*plGetFontName)(Popot name);
	char	 *protostr8;
	Errcode  (*plLoadFont)(Popot name);
	char	 *protostr9;
	void	 (*plGetFontDir)(Popot dir);
	char	 *protostr10;
	void	 (*plQfont)(void);
	char	 *protostr11;
/* From here on down new with Ani 1.5 */
	Boolean  (*plCanScaleFont)(void);
	char	 *protostr12;
	Errcode  (*plScaleFont)(int height);
	char	 *protostr13;
	void	 (*plSetFontSpacing)(int spacing);
	char	 *protostr14;
	int 	 (*plGetFontSpacing)(void);
	char	 *protostr15;
	void	 (*plSetFontLeading)(int leadint);
	char	 *protostr16;
	int 	 (*plGetFontLeading)(void);
	char	 *protostr17;
	int 	 (*plWordWrapCountLines)(int width, Popot text);
	char	 *protostr18;
	} PolibText;

/*----------------------------------------------------------------------------
 * Time library
 *--------------------------------------------------------------------------*/

typedef struct polib_time {
	long	 (*plClock1000)(void);
	char	 *protostr1;
	void	 (*plsleep)(double seconds);
	char	 *protostr2;
	void	 (*plNextFrame)(void);
	char	 *protostr3;
	void	 (*plBackFrame)(void);
	char	 *protostr4;
	void	 (*plSetFrame)(int frame);
	char	 *protostr5;
	int 	 (*plGetFrame)(void);
	char	 *protostr6;
	Errcode  (*plSetFrameCount)(int count);
	char	 *protostr7;
	int 	 (*plGetFrameCount)(void);
	char	 *protostr8;
	void	 (*plPlayFlic)(long frames);
	char	 *protostr9;
	void	 (*plSetSpeed)(int speed);
	char	 *protostr10;
	int 	 (*plGetSpeed)(void);
	char	 *protostr11;
	Errcode  (*plInsertFrames)(int count);
	char	 *protostr12;
	Errcode  (*plDeleteFrames)(int count);
	char	 *protostr13;
	void	  *plOverTime;		/* not accessible to poe modules */
	char	 *protostr14;
	void	  *plOverAll;		/* not accessible to poe modules */
	char	 *protostr15;
	void	  *plOverSegment;	/* not accessible to poe modules */
	char	 *protostr16;
	void	 (*plSetTimeSelect)(Boolean is_multi);
	char	 *protstr17;
	Boolean  (*plGetTimeSelect)(void);
	char	 *protstr18;
	void	 (*plSetFSA)(int fsa);
	char	 *protstr19;
	int 	 (*plGetFSA)(void);
	char	 *protstr20;
	void	 (*plSetSegStart)(int frame);
	char	 *protstr21;
	int 	 (*plGetSegStart)(void);
	char	 *protstr22;
	void	 (*plSetSegEnd)(int frame);
	char	 *protstr23;
	int 	 (*plGetSegEnd)(void);
	char	 *protstr24;
	void	 (*plSetStill)(Boolean still);
	char	 *protstr25;
	Boolean  (*plGetStill)(void);
	char	 *protstr26;
	void	 (*plSetInSlow)(Boolean InSlow);
	char	 *protstr27;
	Boolean  (*plGetInSlow)(void);
	char	 *protstr28;
	void	 (*plSetOutSlow)(Boolean OutSlow);
	char	 *protstr29;
	Boolean  (*plGetOutSlow)(void);
	char	 *protstr30;
	void	 (*plSetPingPong)(Boolean PingPong);
	char	 *protstr31;
	Boolean  (*plGetPingPong)(void);
	char	 *protstr32;
	void	 (*plSetReverse)(Boolean reverse);
	char	 *protstr33;
	Boolean  (*plGetReverse)(void);
	char	 *protstr34;
	void	 (*plSetComplete)(Boolean complete);
	char	 *protstr35;
	Boolean  (*plGetComplete)(void);
	char	 *protstr36;
	} PolibTime;

/*----------------------------------------------------------------------------
 * Turtle library
 *--------------------------------------------------------------------------*/

typedef struct polib_turtle {
	void	 (*plMove)(double amount);
	char	 *protostr1;
	void	 (*plBack)(double amount);
	char	 *protostr2;
	void	 (*plLeft)(double angle);
	char	 *protostr3;
	void	 (*plRight)(double angle);
	char	 *protostr4;
	void	 (*plPenUp)(void);
	char	 *protostr5;
	void	 (*plPenDown)(void);
	char	 *protostr6;
	Boolean  (*plIsDown)(void);
	char	 *protostr7;
	void	 (*plMoveTo)(double x, double y, double angle);
	char	 *protostr8;
	void	 (*plWhere)(Popot x, Popot y, Popot angle);
	char	 *protostr9;
	void	 (*plHome)(void);
	char	 *protostr10;
	} PolibTurtle;

/*----------------------------------------------------------------------------
 * User interface library
 *--------------------------------------------------------------------------*/

typedef struct polib_user {
	int 	 (*plprintf)(long vcount, long vsize, Popot format, ...);
	char	 *protostr1;
	void	 (*plunprintf)(void);
	char	 *protostr2;
	void	 (*plQtext)(long vcount, long vsize, Popot format, ...);
	char	 *protostr3;
	int 	 (*plQchoice)(long vcount, long vsize, Popot buttons, int bcount, Popot header, ...);
	char	 *protostr4;
	int 	 (*plQmenu)(Popot choices, int ccount, Popot header);
	char	 *protostr5;
	Boolean  (*plQquestion)(long vcount, long vsize, Popot question, ...);
	char	 *protostr6;
	Boolean  (*plQnumber)(Popot num, int min, int max, Popot header);
	char	 *protostr7;
	Boolean  (*plQstring)(Popot string, int size, Popot header);
	char	 *protostr8;
	Boolean  (*plQfile)(Popot suffix, Popot button, Popot inpath, Popot outpath, Boolean force_suffix, Popot header);
	char	 *protostr9;
	Boolean  (*plQlist)(Popot choicestr, Popot choice, Popot items, int icount, Popot ipos, Popot header);
	char	 *protostr10;
	int 	 (*plQcolor)(void);
	char	 *protostr11;
	Errcode  (*plQerror)(long vcount, long vsize, Errcode err, Popot format, ...);
	char	 *protostr12;
	Boolean  (*plRubBox)(Popot x, Popot y, Popot w, Popot h);
	char	 *protostr13;
	Boolean  (*plRubCircle)(Popot x, Popot y, Popot rad);
	char	 *protostr14;
	Boolean  (*plRubLine)(int x1, int y1, Popot x2, Popot y2);
	char	 *protostr15;
	int 	 (*plRubPoly)(Popot x, Popot y);
	char	 *protostr16;
	Boolean  (*plDragBox)(Popot x, Popot y, Popot w, Popot h);
	char	 *protostr16a;
	void	 (*plWaitClick)(Popot x, Popot y, Popot left, Popot right, Popot key);
	char	 *protostr17;
	void	 (*plPollInput)(Popot x, Popot y, Popot left, Popot right, Popot key);
	char	 *protostr18;
	void	 (*plWaitInput)(Popot x, Popot y, Popot left, Popot right, Popot key);
	char	 *protostr19;
	Boolean  (*plGetAbort)(void);
	char	 *protostr20;
	Boolean  (*plSetAbort)(Boolean abort);
	char	 *protostr21;
	void	  *plSetAbortHandler;	/* Not accessible to poe modules */
	char	 *protostr22;
	Boolean  (*plHideCursor)(void);
	char	 *protostr23;
	Boolean  (*plShowCursor)(void);
	char	 *protostr24;
	Boolean  (*plQscroll)(Popot choice, Popot items, int icount, Popot ipos, Popot button_texts, Popot hdr);
	char	 *protostr25;
	Boolean  (*plUdQnumber)(long vcount, long vsize, Popot inum, int min, int max, Popot update, Popot data, Popot pofmt, ...);
	char	 *protostr26;
	int 	(*plQedit)(Popot ptext, int max_size, Popot cursor_position, Popot top_line);
	char	 *protostr27;
	int 	(*plQeditFile)(Popot pop_file_name, Popot cursor_position, Popot top_line);
	char	 *protostr28;
	void	 (*plPhysicalWaitClick)(Popot x, Popot y, Popot left, Popot right, Popot key);
	char	 *protostr29;
	void	 (*plPhysicalPollInput)(Popot x, Popot y, Popot left, Popot right, Popot key);
	char	 *protostr30;
	void	 (*plPhysicalWaitInput)(Popot x, Popot y, Popot left, Popot right, Popot key);
	char	 *protostr31;
	Boolean  (*plPhysicalRubMoveBox)(Popot x, Popot y, Popot width, Popot height, Boolean clip_to_screen);
	char	 *protostr32;
	int (*plQmenuWithFlags)(Popot pchoices, int ccount, Popot pflags, Popot header);
	char	 *protostr33;
	} PolibUser;


/*----------------------------------------------------------------------------
 * Global Variable Library
 *--------------------------------------------------------------------------*/

typedef struct polib_globalv {
	Errcode  (*plGlobalVarGet)(Popot name, Popot value);
	char	 *protostr1;
	Errcode  (*plGlobalVarSet)(Popot name, Popot value);
	char	 *protostr2;
	Errcode  (*plGlobalVarDelete)(Popot name);
	char	 *protostr3;
	Errcode  (*plGlobalVarFlush)(void);
	char	 *protostr4;
	Errcode  (*plGlobalVarFirst)(Popot nameptr, Popot valueptr);
	char	 *protostr5;
	Errcode  (*plGlobalVarNext)(Popot nameptr, Popot valueptr);
	char	 *protostr6;
	} PolibGlobalv;

/*----------------------------------------------------------------------------
 * Titling library
 *--------------------------------------------------------------------------*/
typedef struct polib_title {
	void	(*plTitleSetMovement)(int movement);
	char	*protostr3;
	int 	(*plTitleGetMovement)(void);
	char	*protostr4;
	void	(*plTitleSetScrolling)(int scrolling);
	char	*protostr5;
	int 	(*plTitleGetScrolling)(void);
	char	*protostr6;
	Errcode (*plTitleSetText)(char *text);
	char	*protostr7;
	Errcode (*plTitleSetTextFromFile)(char *file_name);
	char	*protostr8;
	char	(*plTitleGetText)(void);
	char	*protostr9;
	Boolean (*plTitleHasText)(void);
	char	*protostr10;
	void	(*plTitleSetPosition)(int x, int y, int w, int h);
	char	*protostr11;
	void	(*plTitleGetPosition)(int *x, int *y, int *w, int *h);
	char	*protostr12;
	void	(*plTitleEdit)(void);
	char	*protostr13;
	Errcode (*plTitleRender)(void);
	char	*protostr14;
	} PolibTitle;

/*----------------------------------------------------------------------------
 * Tween library
 *--------------------------------------------------------------------------*/
typedef struct polib_tween {
	Errcode (*plTweenLoad)(Popot pop_file_name);
	char	*protostr1;
	Errcode (*plTweenSave)(Popot pop_file_name);
	char	*protostr2;
	Boolean (*plTweenExists)(void);
	char	*protostr3;
	void	(*plTweenClear)(void);
	char	*protostr4;
	Errcode (*plTweenSetStart)(int ptcount, Popot pop_x, Popot pop_y);
	char	*protostr5;
	Errcode (*plTweenGetStart)(Popot pop_ptcount, Popot pop_x, Popot pop_y);
	char	*protostr6;
	Errcode (*plTweenSetEnd)(int ptcount, Popot pop_x, Popot pop_y);
	char	*protostr7;
	Errcode (*plTweenGetEnd)(Popot pop_ptcount, Popot pop_x, Popot pop_y);
	char	*protostr8;
	void	(*plTweenSwapEnds)(void);
	char	*protostr9;
	Errcode (*plTweenEndToStart)(void);
	char	*protostr10;
	Errcode (*plTweenOneLink)(int start_point, int end_point);
	char	*protostr11;
	Errcode (*plTweenSetLinks)(int link_count, Popot pop_starts, Popot pop_ends);
	char	*protostr12;
	Errcode (*plTweenGetLinks)(Popot pop_link_count, Popot pop_starts, Popot pop_ends);
	char	*protostr13;
	void	(*plTweenClearLinks)(void);
	char	*protostr14;
	void	(*plTweenSetSplined)(Boolean is_splined);
	char	*protostr15;
	Boolean (*plTweenGetSplined)(void);
	char	*protostr16;
	Errcode (*plTweenTrails)(int steps);
	char	*protostr17;
	Errcode (*plTweenMakePoly)(double time, Popot pop_ptcount, Popot pop_x, Popot pop_y);
	char	*protostr18;
	Errcode (*plTweenRender)(void);
	char	*protostr19;
	} PolibTween;

/*----------------------------------------------------------------------------
 * FlicPlay library
 *--------------------------------------------------------------------------*/

typedef struct polib_flicplay {
	void *	reserved1; /* typedef appears here */
	char	*protostr00;
	Errcode (*plFlicInfo)(Popot path, Popot width, Popot height, Popot speed, Popot frames);
	char	*protostr01;
	Popot	(*plFlicOpenInfo)(Popot path, Popot width, Popot height, Popot speed, Popot frames);
	char	*protostr02;
	Popot	(*plFlicOpen)(Popot path);
	char	*protostr03;
	void	(*plFlicClose)(Popot theflic);
	char	*protostr04;
	void	(*plFlicRewind)(Popot theflic);
	char	*protostr05;
	void	(*plFlicSeekFrame)(Popot theflic, int theframe);
	char	*protostr06;
	void	(*plFlicOptions)(Popot theflic, int speed, int keyit_stops_playback, Popot playback_screen, int xoffset, int yoffset);
	char	*protostr07;
	void	(*plFlicPlay)(Popot theflic);
	char	*protostr08;
	void	(*plFlicPlayOnce)(Popot theflic);
	char	*protostr09;
	void	(*plFlicPlayTimed)(Popot theflic, int milliseconds);
	char	*protostr10;
	void	(*plFlicPlayCount)(Popot theflic, int frame_count);
	char	*protostr11;
	void	*plFlicPlayUntil; /* not accessible to poe modules */
	char	*protostr12;
	} PolibFlicPlay;

/*----------------------------------------------------------------------------
 * Macros defining the number of entries in each of the builtin libs...
 *--------------------------------------------------------------------------*/

#define SIZEOF_POLIB_ENTRY		(sizeof(char *)+sizeof(void (*)()))

#define POLIB_OPTICS_SIZE		(sizeof(PolibOptics  )/SIZEOF_POLIB_ENTRY)
#define POLIB_SWAP_SIZE 		(sizeof(PolibSwap	 )/SIZEOF_POLIB_ENTRY)
#define POLIB_SCREEN_SIZE		(sizeof(PolibScreen  )/SIZEOF_POLIB_ENTRY)
#define POLIB_CEL_SIZE			(sizeof(PolibCel	 )/SIZEOF_POLIB_ENTRY)
#define POLIB_DOS_SIZE			(sizeof(PolibDos	 )/SIZEOF_POLIB_ENTRY)
#define POLIB_DRAW_SIZE 		(sizeof(PolibDraw	 )/SIZEOF_POLIB_ENTRY)
#define POLIB_AAFILE_SIZE		(sizeof(PolibAAFile  )/SIZEOF_POLIB_ENTRY)
#define POLIB_MISC_SIZE 		(sizeof(PolibMisc	 )/SIZEOF_POLIB_ENTRY)
#define POLIB_MODE_SIZE 		(sizeof(PolibMode	 )/SIZEOF_POLIB_ENTRY)
#define POLIB_TEXT_SIZE 		(sizeof(PolibText	 )/SIZEOF_POLIB_ENTRY)
#define POLIB_TIME_SIZE 		(sizeof(PolibTime	 )/SIZEOF_POLIB_ENTRY)
#define POLIB_TURTLE_SIZE		(sizeof(PolibTurtle  )/SIZEOF_POLIB_ENTRY)
#define POLIB_USER_SIZE 		(sizeof(PolibUser	 )/SIZEOF_POLIB_ENTRY)
#define POLIB_GLOBALV_SIZE		(sizeof(PolibGlobalv )/SIZEOF_POLIB_ENTRY)
#define POLIB_TITLE_SIZE		(sizeof(PolibTitle	 )/SIZEOF_POLIB_ENTRY)
#define POLIB_TWEEN_SIZE		(sizeof(PolibTween	 )/SIZEOF_POLIB_ENTRY)
#define POLIB_FLICPLAY_SIZE 	(sizeof(PolibFlicPlay)/SIZEOF_POLIB_ENTRY)

#ifdef REXLIB_INTERNALS
/*****************************************************************************
 * The Porexlib structure (the host<->poe interface for AA_POCOLIB usage.)
 ****************************************************************************/

struct rgb3;	/* just enough to allow its use in prototypes */

typedef Errcode OTFunc(void *data, int ix, int total, int scale);

typedef struct porexlib {
	Libhead 	   hdr;
	Errcode 	   *pl_builtin_err;
	void		   *(*pl_getpicscreen)(void);
	void		   *(*pl_ppt2ptr)(Popot ppt);
	Popot		   (*pl_ptr2ppt)(void *ptr, int bytes);
	int 		   (*pl_getmucolors)(Pixel **indicies,
					   struct rgb3 **lastrgbs, struct rgb3 **idealrgbs);
	int 		   (*pl_findpoe)(char *poename, Lib_proto **plibreturn);
	Errcode 	   (*pl_overtime)(OTFunc *effect, void *data);
	Boolean 	   (*pl_checkabort)(void *data);
	Errcode 	   (*pl_oversegment)(OTFunc *effect, void *data);
	Errcode 	   (*pl_overall)(OTFunc *effect, void *data);
	char		   *vb;   /* for internal bugfix/patch usage only! */
	char		   *vs;   /* for internal bugfix/patch usage only! */
	long		   reserved1[4];
	PolibUser	   *pluser;
	PolibOptics    *ploptics;
	PolibSwap	   *plswap;
	PolibScreen    *plscreen;
	PolibCel	   *plcel;
	PolibDos	   *pldos;
	PolibDraw	   *pldraw;
	PolibAAFile    *plaafile;
	PolibMisc	   *plmisc;
	PolibMode	   *plmode;
	PolibText	   *pltext;
	PolibTime	   *pltime;
	PolibTurtle    *plturtle;
	PolibGlobalv   *plglobalv;
	PolibTitle	   *pltitle;
	PolibTween	   *pltween;
	PolibFlicPlay  *plflicplay;
	long		   reserved2[1];
} Porexlib;

/* REXLIB_INTERNALS */ #endif

#ifdef PUBLIC_CODE
/*****************************************************************************
 * For the poe side only:
 *	 Prototypes for functions that live in rexlib\pocolib.lib...
 *	 Macros to provide a poe module indirect access to builtin poco libs...
 ****************************************************************************/

#ifndef SCALE_ONE
  #define SCALE_ONE (1<<14) 	/* used by overtime effects routines */
#endif

extern	Hostlib _a_a_pocolib;	/* this helps multi-source-module POE code */

#define _plptr				   ((Porexlib *)_a_a_pocolib.next)

#define poeprintf			   _plptr->pluser->plprintf
#define poeunprintf 		   _plptr->pluser->plunprintf
#define poeQtext			   _plptr->pluser->plQtext
#define poeQchoice			   _plptr->pluser->plQchoice
#define poeQmenu			   _plptr->pluser->plQmenu
#define poeQquestion		   _plptr->pluser->plQquestion
#define poeQnumber			   _plptr->pluser->plQnumber
#define poeQstring			   _plptr->pluser->plQstring
#define poeQfile			   _plptr->pluser->plQfile
#define poeQlist			   _plptr->pluser->plQlist
#define poeQcolor			   _plptr->pluser->plQcolor
#define poeQerror			   _plptr->pluser->plQerror
#define poeRubBox			   _plptr->pluser->plRubBox
#define poeRubCircle		   _plptr->pluser->plRubCircle
#define poeRubLine			   _plptr->pluser->plRubLine
#define poeRubPoly			   _plptr->pluser->plRubPoly
#define poeDragBox			   _plptr->pluser->plDragBox
#define poeWaitClick		   _plptr->pluser->plWaitClick
#define poePollInput		   _plptr->pluser->plPollInput
#define poeWaitInput		   _plptr->pluser->plWaitInput
#define poeGetAbort 		   _plptr->pluser->plGetAbort
#define poeSetAbort 		   _plptr->pluser->plSetAbort
#define poeHideCursor		   _plptr->pluser->plHideCursor
#define poeShowCursor		   _plptr->pluser->plShowCursor
#define poeQscroll			   _plptr->pluser->plQscroll
	/* Next ones are new with Ani Pro 1.5 */
#define poeQedit			   _plptr->pluser->plQedit
#define poeQeditFile		   _plptr->pluser->plQeditFile
#define poePhysicalWaitClick   _plptr->pluser->plPhysicalWaitClick
#define poePhysicalPollInput   _plptr->pluser->plPhysicalPollInput
#define poePhysicalWaitInput   _plptr->pluser->plPhysicalWaitInput
#define poePhysicalRubMoveBox  _plptr->pluser->plPhysicalRubMoveBox
#define poeQmenuWithFlags      _plptr->pluser->plQmenuWithFlags

#define poeOptClearState	   _plptr->ploptics->plOptClearState
#define poeOptSetState		   _plptr->ploptics->plOptSetState
#define poeOptGetState		   _plptr->ploptics->plOptGetState
#define poeOptFreeState 	   _plptr->ploptics->plOptFreeState
#define poeOptClearPos		   _plptr->ploptics->plOptClearPos
#define poeOptSetPos		   _plptr->ploptics->plOptSetPos
#define poeOptGetPos		   _plptr->ploptics->plOptGetPos
#define poeOptClearPath 	   _plptr->ploptics->plOptClearPath
#define poeOptSetPath		   _plptr->ploptics->plOptSetPath
#define poeOptGetPath		   _plptr->ploptics->plOptGetPath
#define poeOptFreePath		   _plptr->ploptics->plOptFreePath
#define poeOptDefaultCenters   _plptr->ploptics->plOptDefaultCenters
#define poeOptContinue		   _plptr->ploptics->plOptContinue
#define poeOptGetElement	   _plptr->ploptics->plOptGetElement
#define poeOptSetElement	   _plptr->ploptics->plOptSetElement
#define poeOptToFrame		   _plptr->ploptics->plOptToFrame
#define poeOptToSegment 	   _plptr->ploptics->plOptToSegment
#define poeOptToAll 		   _plptr->ploptics->plOptToAll

#define poeSwapExists		   _plptr->plswap->plSwapExists
#define poeSwapClip 		   _plptr->plswap->plSwapClip
#define poeSwapRelease		   _plptr->plswap->plSwapRelease
#define poeSwapTrade		   _plptr->plswap->plSwapTrade

#define poeGetPicScreen 	   _plptr->plscreen->plGetPicScreen
#define poeGetSwapScreen	   _plptr->plscreen->plGetSwapScreen
#define poeGetUndoScreen	   _plptr->plscreen->plGetUndoScreen
#define poeGetCelScreen 	   _plptr->plscreen->plGetCelScreen
#define poeAllocScreen		   _plptr->plscreen->plAllocScreen
#define poeFreeScreen		   _plptr->plscreen->plFreeScreen
#define poeGetScreenSize	   _plptr->plscreen->plGetScreenSize
#define poeSetPixel 		   _plptr->plscreen->plSetPixel
#define poeGetPixel 		   _plptr->plscreen->plGetPixel
#define poeSetBlock 		   _plptr->plscreen->plSetBlock
#define poeGetBlock 		   _plptr->plscreen->plGetBlock
#define poeIconBlit 		   _plptr->plscreen->plIconBlit
#define poeBlit 			   _plptr->plscreen->plBlit
#define poeKeyBlit			   _plptr->plscreen->plKeyBlit
#define poeCopyScreen		   _plptr->plscreen->plCopyScreen
#define poeTradeScreen		   _plptr->plscreen->plTradeScreen
#define poePicDirtied		   _plptr->plscreen->plPicDirtied
/* New with Ani Pro 1.5 */
#define poeGetPhysicalScreen   _plptr->plscreen->plGetPhysicalScreen
#define poeSetBox			   _plptr->plscreen->plSetBox
#define poeMenuText 		   _plptr->plscreen->plMenuText
#define poeMenuTextWidth	   _plptr->plscreen->plMenuTextWidth
#define poeMenuTextHeight	   _plptr->plscreen->plMenuTextHeight
#define poeGetMenuColors	   _plptr->plscreen->plGetMenuColors

#define poeCelExists		   _plptr->plcel->plCelExists
#define poeCelPaste 		   _plptr->plcel->plCelPaste
#define poeCelMove			   _plptr->plcel->plCelMove
#define poeCelMoveTo		   _plptr->plcel->plCelMoveTo
#define poeCelTurn			   _plptr->plcel->plCelTurn
#define poeCelTurnTo		   _plptr->plcel->plCelTurnTo
#define poeCelNextFrame 	   _plptr->plcel->plCelNextFrame
#define poeCelBackFrame 	   _plptr->plcel->plCelBackFrame
#define poeCelSetFrame		   _plptr->plcel->plCelSetFrame
#define poeCelGetFrame		   _plptr->plcel->plCelGetFrame
#define poeCelFrameCount	   _plptr->plcel->plCelFrameCount
#define poeCelWhere 		   _plptr->plcel->plCelWhere
#define poeCelGet			   _plptr->plcel->plCelGet
#ifndef PATCH10A_H /* if PATCH10A_H is defined, this has been set already */
  #define poeCelRelease()		 _plptr->plcel->plCelRelease(); /* new: v178 */
#endif
#define poeCelClipChanges      _plptr->plcel->plCelClipChanges

#define poefnsplit			   _plptr->pldos->plfnsplit
#define poefnmerge			   _plptr->pldos->plfnmerge
#define poeDosExists		   _plptr->pldos->plDosExists
#define poeDosCopy			   _plptr->pldos->plDosCopy
#define poeDosDelete		   _plptr->pldos->plDosDelete
#define poeDosRename		   _plptr->pldos->plDosRename
#define poeSetDir			   _plptr->pldos->plSetDir
#define poeGetDir			   _plptr->pldos->plGetDir
#define poeDirList			   _plptr->pldos->plDirList
#define poeFreeDirList		   _plptr->pldos->plFreeDirList
#define poeGetResourceDir	   _plptr->pldos->plGetResourceDir
#define poeGetProgramDir	   _plptr->pldos->plGetProgramDir

#define poeGetSize			   _plptr->pldraw->plGetSize
#define poeGetAspectRatio	   _plptr->pldraw->plGetAspectRatio
#define poeGetColor 		   _plptr->pldraw->plGetColor
#define poeSetColor 		   _plptr->pldraw->plSetColor
#define poeClear			   _plptr->pldraw->plClear
#define poeDot				   _plptr->pldraw->plDot
#define poeGetDot			   _plptr->pldraw->plGetDot
#define poeLine 			   _plptr->pldraw->plLine
#define poeBox				   _plptr->pldraw->plBox
#define poeCircle			   _plptr->pldraw->plCircle
#define poePoly 			   _plptr->pldraw->plPoly
#define poeSpline			   _plptr->pldraw->plSpline
#define poeOval 			   _plptr->pldraw->plOval
#define poeStar 			   _plptr->pldraw->plStar
#define poePetal			   _plptr->pldraw->plPetal
#define poeRpoly			   _plptr->pldraw->plRpoly
#define poeSpiral			   _plptr->pldraw->plSpiral
#define poeFill 			   _plptr->pldraw->plFill
#define poeFillTo			   _plptr->pldraw->plFillTo
#define poeEdge 			   _plptr->pldraw->plEdge
#define poeSetColorMap		   _plptr->pldraw->plSetColorMap
#define poeGetColorMap		   _plptr->pldraw->plGetColorMap
#define poeGetScreenColorMap   _plptr->pldraw->plGetScreenColorMap
#define poeGetScreenColorMap   _plptr->pldraw->plGetScreenColorMap
#define poeGetPhysicalSize	   _plptr->pldraw->plGetPhysicalSize

#ifndef PATCH10A_H /* if PATCH10A_H is defined, these have been set already */
  #define poeGetBoxBevel	   _plptr->pldraw->plGetBoxBevel /* new: v179 */
  #define poeSetBoxBevel	   _plptr->pldraw->plSetBoxBevel /* new: v179 */
#endif
	/* New to Ani Pro 1.5 */
#define poeSetCluster		   _plptr->pldraw->plSetCluster
#define poeGetCluster		   _plptr->pldraw->plGetCluster
#define poeHLStoRGB 			_plptr->pldraw->plHLStoRGB
#define poeRGBtoHLS 			_plptr->pldraw->plRGBtoHLS
#define poeClosestColorInScreen _plptr->pldraw->plClosestColorInScreen
#define poeSqueezeColors		_plptr->pldraw->plSqueezeColors
#define poeFitScreenToColorMap	_plptr->pldraw->plFitScreenToColorMap

#define poeLoadFlic 		   _plptr->plaafile->plLoadFlic
#define poeSaveFlic 		   _plptr->plaafile->plSaveFlic
#define poeLoadPic			   _plptr->plaafile->plLoadPic
#define poeSavePic			   _plptr->plaafile->plSavePic
#define poeLoadCel			   _plptr->plaafile->plLoadCel
#define poeSaveCel			   _plptr->plaafile->plSaveCel
#define poeLoadPath 		   _plptr->plaafile->plLoadPath
#define poeSavePath 		   _plptr->plaafile->plSavePath
#define poeLoadPoly 		   _plptr->plaafile->plLoadPoly
#define poeSavePoly 		   _plptr->plaafile->plSavePoly
#define poeLoadColors		   _plptr->plaafile->plLoadColors
#define poeSaveColors		   _plptr->plaafile->plSaveColors
#define poeLoadTitles		   _plptr->plaafile->plLoadTitles
#define poeSaveTitles		   _plptr->plaafile->plSaveTitles
#define poeLoadMask 		   _plptr->plaafile->plLoadMask
#define poeSaveMask 		   _plptr->plaafile->plSaveMask
#define poeSaveScreenPic	   _plptr->plaafile->plSaveScreenPic
#define poeLoadScreenPic	   _plptr->plaafile->plLoadScreenPic

#define poeexit 			   _plptr->plmisc->plexit
#define poeNewFlic			   _plptr->plmisc->plNewFlic
#define poeReset			   _plptr->plmisc->plReset
#define poeResizeReset		   _plptr->plmisc->plResizeReset
#define poeGetChangeCount	   _plptr->plmisc->plGetChangeCount
#define poePocoVersion		   _plptr->plmisc->plPocoVersion
#define poeRedo 			   _plptr->plmisc->plRedo
#define poeRestore			   _plptr->plmisc->plRestore
#define poernd				   _plptr->plmisc->plrnd
#define poerand 			   _plptr->plmisc->plrand
#define poesrand			   _plptr->plmisc->plsrand
#define poeclock			   _plptr->plmisc->plclock
#define poeIsBatchRun		   _plptr->plmisc->plIsBatchRun
#define poePocoChainTo		   _plptr->plmisc->plPocoChainTo
#define poesystem			   _plptr->plmisc->plsystem 	/* new: v1.5 */

#define poeSetInk			   _plptr->plmode->plSetInk
#define poeGetInk			   _plptr->plmode->plGetInk
#define poeSetInkStrength	   _plptr->plmode->plSetInkStrength
#define poeGetInkStrength	   _plptr->plmode->plGetInkStrength
#define poeSetInkDither 	   _plptr->plmode->plSetInkDither
#define poeGetInkDither 	   _plptr->plmode->plGetInkDither
#define poeSetFilled		   _plptr->plmode->plSetFilled
#define poeGetFilled		   _plptr->plmode->plGetFilled
#define poeSetBrushSize 	   _plptr->plmode->plSetBrushSize
#define poeGetBrushSize 	   _plptr->plmode->plGetBrushSize
#define poeSetKeyMode		   _plptr->plmode->plSetKeyMode
#define poeGetKeyMode		   _plptr->plmode->plGetKeyMode
#define poeSetKeyColor		   _plptr->plmode->plSetKeyColor
#define poeGetKeyColor		   _plptr->plmode->plGetKeyColor
#define poeSetMaskUse		   _plptr->plmode->plSetMaskUse
#define poeGetMaskUse		   _plptr->plmode->plGetMaskUse
#define poeSetMaskCreate	   _plptr->plmode->plSetMaskCreate
#define poeGetMaskCreate	   _plptr->plmode->plGetMaskCreate
#define poeSetStarPoints	   _plptr->plmode->plSetStarPoints
#define poeGetStarPoints	   _plptr->plmode->plGetStarPoints
#define poeSetStarRatio 	   _plptr->plmode->plSetStarRatio
#define poeGetStarRatio 	   _plptr->plmode->plGetStarRatio
#define poeSetSplineTCB 	   _plptr->plmode->plSetSplineTCB
#define poeGetSplineTCB 	   _plptr->plmode->plGetSplineTCB
#define poeSetTwoColorOn	   _plptr->plmode->plSetTwoColorOn
#define poeGetTwoColorOn	   _plptr->plmode->plGetTwoColorOn
#define poeSetTwoColor		   _plptr->plmode->plSetTwoColor
#define poeGetTwoColor		   _plptr->plmode->plGetTwoColor
#define poeSetClosed		   _plptr->plmode->plSetClosed
#define poeGetClosed		   _plptr->plmode->plGetClosed
#define poeSetCycleDraw 	   _plptr->plmode->plSetCycleDraw
#define poeGetCycleDraw 	   _plptr->plmode->plGetCycleDraw
#define poeGetMultiFrame	   _plptr->plmode->plGetMultiFrame
#define poeSetMultiFrame	   _plptr->plmode->plSetMultiFrame

#define poeText 			   _plptr->pltext->plText
#define poeWordWrap 		   _plptr->pltext->plWordWrap
#define poeSetJustify		   _plptr->pltext->plSetJustify
#define poeGetJustify		   _plptr->pltext->plGetJustify
#define poeStringWidth		   _plptr->pltext->plStringWidth
#define poeFontHeight		   _plptr->pltext->plFontHeight
#define poeTallestChar		   _plptr->pltext->plTallestChar
#define poeGetFontName		   _plptr->pltext->plGetFontName
#define poeLoadFont 		   _plptr->pltext->plLoadFont
#define poeGetFontDir		   _plptr->pltext->plGetFontDir
#define poeQfont			   _plptr->pltext->plQfont
	/* Next are new with Ani Pro 1.5 */
#define poeCanScaleFont 	   _plptr->pltext->plCanScaleFont
#define poeScaleFont		   _plptr->pltext->plScaleFont
#define poeSetFontSpacing	   _plptr->pltext->plSetFontSpacing
#define poeGetFontSpacing	   _plptr->pltext->plGetFontSpacing
#define poeSetFontLeading	   _plptr->pltext->plSetFontLeading
#define poeGetFontLeading	   _plptr->pltext->plGetFontLeading
#define poeWordWrapCountLines  _plptr->pltext->plWordWrapCountLines

#define poeClock1000		   _plptr->pltime->plClock1000
#define poesleep			   _plptr->pltime->plsleep
#define poeNextFrame		   _plptr->pltime->plNextFrame
#define poeBackFrame		   _plptr->pltime->plBackFrame
#define poeSetFrame 		   _plptr->pltime->plSetFrame
#define poeGetFrame 		   _plptr->pltime->plGetFrame
#define poeSetFrameCount	   _plptr->pltime->plSetFrameCount
#define poeGetFrameCount	   _plptr->pltime->plGetFrameCount
#define poePlayFlic 		   _plptr->pltime->plPlayFlic
#define poeSetSpeed 		   _plptr->pltime->plSetSpeed
#define poeGetSpeed 		   _plptr->pltime->plGetSpeed
#define poeInsertFrames 	   _plptr->pltime->plInsertFrames
#define poeDeleteFrames 	   _plptr->pltime->plDeleteFrames
#define poeSetTimeSelect	   _plptr->pltime->plSetTimeSelect
#define poeGetTimeSelect	   _plptr->pltime->plGetTimeSelect
#define poeSetFSA			   _plptr->pltime->plSetFSA
#define poeGetFSA			   _plptr->pltime->plGetFSA
#define poeSetSegStart		   _plptr->pltime->plSetSegStart
#define poeGetSegStart		   _plptr->pltime->plGetSegStart
#define poeSetSegEnd		   _plptr->pltime->plSetSegEnd
#define poeGetSegEnd		   _plptr->pltime->plGetSegEnd
#define poeSetStill 		   _plptr->pltime->plSetStill
#define poeGetStill 		   _plptr->pltime->plGetStill
#define poeSetInSlow		   _plptr->pltime->plSetInSlow
#define poeGetInSlow		   _plptr->pltime->plGetInSlow
#define poeSetOutSlow		   _plptr->pltime->plSetOutSlow
#define poeGetOutSlow		   _plptr->pltime->plGetOutSlow
#define poeSetPingPong		   _plptr->pltime->plSetPingPong
#define poeGetPingPong		   _plptr->pltime->plGetPingPong
#define poeSetReverse		   _plptr->pltime->plSetReverse
#define poeGetReverse		   _plptr->pltime->plGetReverse
#define poeSetComplete		   _plptr->pltime->plSetComplete
#define poeGetComplete		   _plptr->pltime->plGetComplete

#define poeMove 			   _plptr->plturtle->plMove
#define poeBack 			   _plptr->plturtle->plBack
#define poeLeft 			   _plptr->plturtle->plLeft
#define poeRight			   _plptr->plturtle->plRight
#define poePenUp			   _plptr->plturtle->plPenUp
#define poePenDown			   _plptr->plturtle->plPenDown
#define poeIsDown			   _plptr->plturtle->plIsDown
#define poeMoveTo			   _plptr->plturtle->plMoveTo
#define poeWhere			   _plptr->plturtle->plWhere
#define poeHome 			   _plptr->plturtle->plHome

#define poeGlobalVarGet 	   _plptr->plglobalv->plGlobalVarGet
#define poeGlobalVarSet 	   _plptr->plglobalv->plGlobalVarSet
#define poeGlobalVarDelete	   _plptr->plglobalv->plGlobalVarDelete
#define poeGlobalVarFlush	   _plptr->plglobalv->plGlobalVarFlush
#define poeGlobalVarFirst	   _plptr->plglobalv->plGlobalVarFirst
#define poeGlobalVarNext	   _plptr->plglobalv->plGlobalVarNext

	/* This whole library new with Ani Pro 1.5 */
#define poeTitleSetMovement 	_plptr->pltitle->plTitleSetMovement
#define poeTitleGetMovement 	_plptr->pltitle->plTitleGetMovement
#define poeTitleSetScrolling	_plptr->pltitle->plTitleSetScrolling
#define poeTitleGetScrolling	_plptr->pltitle->plTitleGetScrolling
#define poeTitleSetText 		_plptr->pltitle->plTitleSetText
#define poeTitleSetTextFromFile _plptr->pltitle->plTitleSetTextFromFile
#define poeTitleGetText 		_plptr->pltitle->plTitleGetText
#define poeTitleHasText 		_plptr->pltitle->plTitleHasText
#define poeTitleSetPosition 	_plptr->pltitle->plTitleSetPosition
#define poeTitleGetPosition 	_plptr->pltitle->plTitleGetPosition
#define poeTitleEdit			_plptr->pltitle->plTitleEdit
#define poeTitleRender			_plptr->pltitle->plTitleRender

	/* Likewise this whole library new with Ani Pro 1.5 */
#define poeTweenLoad			_plptr->pltween->plTweenLoad
#define poeTweenSave			_plptr->pltween->plTweenSave
#define poeTweenExists			_plptr->pltween->plTweenExists
#define poeTweenClear			_plptr->pltween->plTweenClear
#define poeTweenSetStart		_plptr->pltween->plTweenSetStart
#define poeTweenGetStart		_plptr->pltween->plTweenGetStart
#define poeTweenSetEnd			_plptr->pltween->plTweenSetEnd
#define poeTweenGetEnd			_plptr->pltween->plTweenGetEnd
#define poeTweenSwapEnds		_plptr->pltween->plTweenSwapEnds
#define poeTweenEndToStart		_plptr->pltween->plTweenEndToStart
#define poeTweenOneLink 		_plptr->pltween->plTweenOneLink
#define poeTweenSetLinks		_plptr->pltween->plTweenSetLinks
#define poeTweenGetLinks		_plptr->pltween->plTweenGetLinks
#define poeTweenClearLinks		_plptr->pltween->plTweenClearLinks
#define poeTweenSetSplined		_plptr->pltween->plTweenSetSplined
#define poeTweenGetSplined		_plptr->pltween->plTweenGetSplined
#define poeTweenTrails			_plptr->pltween->plTweenTrails
#define poeTweenMakePoly		_plptr->pltween->plTweenMakePoly
#define poeTweenRender			_plptr->pltween->plTweenRender

	/* This whole library also new with Ani Pro 1.5 */
#define poeFlicInfo 			_plptr->plflicplay->plFlicInfo
#define poeFlicOpenInfo 		_plptr->plflicplay->plFlicOpenInfo
#define poeFlicOpen 			_plptr->plflicplay->plFlicOpen
#define poeFlicClose			_plptr->plflicplay->plFlicClose
#define poeFlicRewind			_plptr->plflicplay->plFlicRewind
#define poeFlicSeekFrame		_plptr->plflicplay->plFlicSeekFrame
#define poeFlicOptions			_plptr->plflicplay->plFlicOptions
#define poeFlicPlay 			_plptr->plflicplay->plFlicPlay
#define poeFlicPlayOnce 		_plptr->plflicplay->plFlicPlayOnce
#define poeFlicPlayTimed		_plptr->plflicplay->plFlicPlayTimed
#define poeFlicPlayCount		_plptr->plflicplay->plFlicPlayCount

/*----------------------------------------------------------------------------
 * POE-specific functions...
 *	 these are available to POE modules, but not to Poco programs.
 *--------------------------------------------------------------------------*/

#define builtin_err 		   (*(_plptr->pl_builtin_err))
#define ptr2ppt 			   _plptr->pl_ptr2ppt
#define ppt2ptr 			   _plptr->pl_ppt2ptr
#define GetPicScreen		   _plptr->pl_getpicscreen
#define GetMenuColors		   _plptr->pl_getmucolors
#define FindPoe 			   _plptr->pl_findpoe
#define OverTime			   _plptr->pl_overtime
#define OverSegment 		   _plptr->pl_oversegment
#define OverAll 			   _plptr->pl_overall
#define CheckAbort			   _plptr->pl_checkabort

/*----------------------------------------------------------------------------
 * new functions...
 *	to use these functions, you must be running under a v1.1 or higher
 *	Ani Pro host, or you must call apply_poe_patches_1_0a() from your POE's
 *	rex-layer init function.
 *--------------------------------------------------------------------------*/

#define GetPhysicalScreen()    (*(void**)(_plptr->vb+0x62))

/*----------------------------------------------------------------------------
 * these macros provide a shorthand call for common uses of ptr2ppt...
 *--------------------------------------------------------------------------*/

#define array2ppt(ary)				ptr2ppt(ary, sizeof(ary))
#define str2ppt(str)				ptr2ppt(str, sizeof(str))
#define var2ppt(var)				ptr2ppt(&var, sizeof(var))

#else /* ie, not PUBLIC_CODE */

/*****************************************************************************
 * For the host side only:
 *	Declarations of the library proto/ptr structures
 *	found in poco*.c modules in the host root directory...
 ****************************************************************************/

extern PolibOptics	  po_liboptics;
extern PolibSwap	  po_libswap;
extern PolibScreen	  po_libscreen;
extern PolibCel 	  po_libcel;
extern PolibDos 	  po_libdos;
extern PolibDraw	  po_libdraw;
extern PolibAAFile	  po_libaafile;
extern PolibMisc	  po_libmisc;
extern PolibMode	  po_libmode;
extern PolibText	  po_libtext;
extern PolibTime	  po_libtime;
extern PolibTurtle	  po_libturtle;
extern PolibUser	  po_libuser;
extern PolibGlobalv   po_libglobalv;
extern PolibTitle	  po_libtitle;
extern PolibTween	  po_libtween;
extern PolibFlicPlay  po_libflicplay;

/* not PUBLIC_CODE */ #endif
/* not POCO_H */  #endif

#endif /* POCOLIB_H */
