/* pocouser.c - poco library functions to read input state and
   standard dialog boxes. */

#include "errcodes.h"
#include "linklist.h"
#include "jimk.h"
#include "poly.h"
#include "pocoface.h"
#include "pocolib.h"
#include "reqlib.h"
#include "softmenu.h"
#include "commonst.h"
#include "textedit.h"

extern Boolean hide_mouse(void);
extern Boolean show_mouse(void);
extern int qcolor();
extern builtin_err;
extern void disp_line_alot(Short_xy *v);
void cleanup_toptext();
void cleanup_wait_box();
Errcode po_poly_to_arrays(Poly *p, Popot *x, Popot *y);
Errcode po_arrays_to_poly(Poly *p, int ptcount, Popot *px, Popot *py);
extern Boolean po_UdSlider(long vargcount, long vargsize, 
					Popot inum, int min, int max,
				    Popot update,  Popot data, Popot pofmt, ...);

/*****************************************************************************

/* abort handling... */

static struct {
	Boolean abortable;
	void	*abort_handler;
	Popot	abort_data;
	} abort_control;

void po_init_abort_control(int abortable, void *handler)
/*****************************************************************************
 * service routine (called from within qpoco.c)
 ****************************************************************************/
{
	abort_control.abortable = abortable;
	abort_control.abort_handler = handler;
}

Boolean po_get_abortable(void)
/*****************************************************************************
 * Boolean GetAbort(void)
 ****************************************************************************/
{
	return abort_control.abortable;
}

Boolean po_set_abortable(Boolean abort)
/*****************************************************************************
 * Boolean SetAbort(Boolean abort)
 ****************************************************************************/
{
	Boolean was_abortable = abort_control.abortable;
	abort_control.abortable  = abort;
	return was_abortable;
}

void po_set_abort_handler(Popot abort_handler, Popot abort_data)
/*****************************************************************************
 * void SetAbortHandler(Boolean (*handler)(void *data), void *data)
 *
 * specify a poco routine to get called if the user aborts the poco program.
 *	 the specified routine is called immediately after the user selects YES
 *	 in pj's abort dialog box.  the abort handler is provided so that the
 *	 poco program can do any cleanup it thinks is necessary (close files, etc).
 *	 it has carte blanche to do pretty much whatever it wants, but bear in
 *	 mind that the user is expecting a pretty quick shutdown.  while the
 *	 abort routine is executing, further aborts are disabled automatically.
 *	 the routine could re-enable them, but nasty recursion issues must be
 *	 dealt with if it does.  the poco abort handler routine must return a
 *	 boolean value indicating whether pj should complete the abort processing
 *	 (TRUE), or ignore it (FALSE).	the only legitimate use of this is to
 *	 allow a deffered abort (the handler sets a flag telling itself to abort
 *	 on the next iteration of its main loop, or whatever).
 *
 *	 to un-install an abort handler, pass a NULL function pointer in the call.
 *
 *	 note that this routine only handles setting the vector; actual abort
 *	 handling is in po_check_abort, below.
 ****************************************************************************/
{
	void *fuf;

	if (NULL == (fuf = abort_handler.pt))
		{
		abort_control.abort_handler = NULL;
		return;
		}

	if (NULL == (abort_control.abort_handler = po_fuf_code(fuf)))
		{
		builtin_err = Err_function_not_found;
		return;
		}

	abort_control.abort_data = abort_data;
	return;
}

Boolean po_check_abort(void *nobody)
/*****************************************************************************
 * handle abort checking and invokation of (optional) abort handler routine.
 *	see comments under po_set_abort_handler, above.
 ****************************************************************************/
{
	Boolean mouse_was_on;
	Errcode err;
	Pt_num	retval;

	if (abort_control.abortable)
		{
		if (poll_abort()<Success)
			{
			mouse_was_on = show_mouse();
			if (soft_yes_no_box("poco_abort"))
				if (NULL == abort_control.abort_handler)
					{
					return TRUE;
					}
				else
					{
					abort_control.abortable = FALSE; /* prevent bad recursion */
					err = poco_cont_ops(abort_control.abort_handler, &retval,
								sizeof(Popot), abort_control.abort_data);
					if (err < Success)
						return builtin_err = err;
					if (retval.i != FALSE)
						return TRUE;
					abort_control.abortable = TRUE;
					}
			if (!mouse_was_on)
				hide_mouse();
			}
		}
	return(FALSE);
}

typedef struct iparams
	{
	Popot x,y,left,right,key;
	} Iparms;

static Iparms *get_iparms(Iparms *p)
/*****************************************************************************
 * service routine
 ****************************************************************************/
{
if (p->x.pt == NULL || p->y.pt == NULL || p->left.pt == NULL ||
	p->right.pt == NULL || p->key.pt == NULL)
	{
	builtin_err = Err_null_ref;
	return(NULL);
	}
return(p);
}

static void set_iparms(Iparms *p)
/*****************************************************************************
 * service routine
 ****************************************************************************/
{
*((int *)(p->x.pt)) = icb.mx;
*((int *)(p->y.pt)) = icb.my;
*((int *)(p->left.pt)) = ISDOWN(MBPEN);
*((int *)(p->right.pt)) = ISDOWN(MBRIGHT);
if (JSTHIT(KEYHIT))
	*((int *)(p->key.pt)) = icb.inkey;
else
	*((int *)(p->key.pt)) = 0;
}

static void po_wndo_input(Iparms *p, ULONG flags)
/*****************************************************************************
 * Get input from pen-window.
 ****************************************************************************/
{
if ((p = get_iparms(p)) != NULL)
	{
	wait_wndo_input(flags);
	set_iparms(p);
	}
}

static void po_wait_click(Iparms pp)
/*****************************************************************************
 * void WaitClick(int *x, int *y, int *left, int *right, int *key)
 ****************************************************************************/
{
po_wndo_input(&pp, ANY_CLICK);
}

static void po_poll_input(Iparms pp)
/*****************************************************************************
 * void PollInput(int *x, int *y, int *left, int *right, int *key)
 ****************************************************************************/
{
po_wndo_input(&pp, ANY_INPUT);
}

static void po_wait_input(Iparms pp)
/*****************************************************************************
 * void WaitInput(int *x, int *y, int *left, int *right, int *key)
 ****************************************************************************/
{
po_wndo_input(&pp, MMOVE|ANY_CLICK);
}

static void po_physical_input(Iparms *p, ULONG flags)
/*****************************************************************************
 * Get input from pen-window.
 ****************************************************************************/
{
if ((p = get_iparms(p)) != NULL)
	{
	wait_input(flags);
	set_iparms(p);
	}
}


static void po_physical_wait_click(Iparms pp)
/*****************************************************************************
 * void PhysicalWaitClick(int *x, int *y, int *left, int *right, int *key)
 ****************************************************************************/
{
po_physical_input(&pp, ANY_CLICK);
}

static void po_physical_poll_input(Iparms pp)
/*****************************************************************************
 * void PhysicalPollInput(int *x, int *y, int *left, int *right, int *key)
 ****************************************************************************/
{
Iparms *p;

if ((p = get_iparms(&pp)) != NULL)
	{
	check_input(ANY_INPUT);
	set_iparms(p);
	}
}

static void po_physical_wait_input(Iparms pp)
/*****************************************************************************
 * void PhysicalWaitInput(int *x, int *y, int *left, int *right, int *key)
 ****************************************************************************/
{
po_physical_input(&pp, MMOVE|ANY_CLICK);
}

static Boolean po_physical_rub_move_box(Popot x, Popot y, Popot w, Popot h
, Boolean clip_to_screen)
/*****************************************************************************
 * Boolean PhysicalRubMoveBox(int *x, int *y, int *w, int *h
 , Boolean clip_to_screen)
 ****************************************************************************/
{
Wscreen *ws= icb.input_screen;
Marqihdr md;
Rectangle rect;
Rectangle clip_rect;
Rectangle *pclip_rect = NULL;

if (x.pt == NULL || y.pt == NULL || w.pt == NULL || h.pt == NULL)
	{
	builtin_err = Err_null_ref;
	return(FALSE);
	}
rect.x		= *((int *)(x.pt));
rect.y		= *((int *)(y.pt));
rect.width	= *((int *)(w.pt));
rect.height = *((int *)(h.pt));
if (clip_to_screen)
	{
	copy_rectfields(ws->viscel,&clip_rect);
	pclip_rect = &clip_rect;
	}
init_marqihdr(&md,ws->viscel,NULL,ws->SWHITE,ws->SBLACK);
if (marqmove_rect(&md, &rect, pclip_rect) < 0)
	return FALSE;
*((int *)(x.pt)) = rect.x;
*((int *)(y.pt)) = rect.y;
*((int *)(w.pt)) = rect.width;
*((int *)(h.pt)) = rect.height;
return TRUE;
}

/* Poco 'marqi' routines to define geometric shapes */

static Boolean po_rub_box(Popot x, Popot y, Popot w, Popot h)
/*****************************************************************************
 * Boolean RubBox(int *x, int *y, int *w, int *h)
 ****************************************************************************/
{
Rectangle rect;

if (x.pt == NULL || y.pt == NULL || w.pt == NULL || h.pt == NULL)
	{
	builtin_err = Err_null_ref;
	return(FALSE);
	}
if(cut_out_rect(&rect) < 0)
	return(FALSE);
else
	{
	*((int *)(x.pt)) = rect.x;
	*((int *)(y.pt)) = rect.y;
	*((int *)(w.pt)) = rect.width;
	*((int *)(h.pt)) = rect.height;
	return(TRUE);
	}
}

static Boolean po_rub_line(int x1, int y1, Popot x2, Popot y2)
/*****************************************************************************
 * Boolean RubLine(int x1, int y1, int *x2, int *y2)
 ****************************************************************************/
{
Short_xy xys[2];
int ret;

if (x2.pt == NULL || y2.pt == NULL)
	{
	builtin_err = Err_null_ref;
	return(FALSE);
	}
xys[0].x = x1;
xys[0].y = y1;
ret = rubba_vertex(&xys[0],&xys[1],&xys[0],disp_line_alot,vs.ccolor);
cleanup_toptext();
if(ret < 0)
	return(FALSE);
else
	{
	*((int *)(x2.pt)) = xys[1].x;
	*((int *)(y2.pt)) = xys[1].y;
	return(TRUE);
	}
}


static Boolean po_rub_circle(Popot x, Popot y, Popot rad)
/*****************************************************************************
 * Boolean RubCircle(int *x, int *y, int *rad)
 ****************************************************************************/
{
Circle_p circp;

if (x.pt == NULL || y.pt == NULL || rad.pt == NULL)
	{
	builtin_err = Err_null_ref;
	return(FALSE);
	}
wait_wndo_input(ANY_CLICK);
if (!ISDOWN(MBPEN))
	return(FALSE);
if(get_rub_circle(&circp.center,&circp.diam,vs.ccolor) < 0)
	return(FALSE);
*((int *)(x.pt)) = circp.center.x;
*((int *)(y.pt)) = circp.center.y;
*((int *)(rad.pt)) = circp.diam>>1;
return(TRUE);
}




static int po_rub_poly(Popot pxlist, Popot pylist)
/*****************************************************************************
 * int RubPoly(int **x, int **y)
 * returns # of points or negative Errcode
 ****************************************************************************/
{
Poly p;
Errcode err;
Popot *px, *py;

if ((px = pxlist.pt) == NULL || (py = pylist.pt) == NULL)
	return(builtin_err = Err_null_ref);
wait_wndo_input(ANY_CLICK);
if (!ISDOWN(MBPEN))
	return(0);
clear_struct(&p);
make_poly(&p, vs.closed_curve);
if ((err = po_poly_to_arrays(&p, px, py)) >= Success)
	err = p.pt_count;
free_polypoints(&p);
return(err);
}

static Boolean po_drag_box(Popot x, Popot y, Popot w, Popot h)
/*****************************************************************************
 * Boolean DragBox(int *x, int *y, int *w, int *h)
 ****************************************************************************/
{
	Rectangle rect;

	if (x.pt == NULL || y.pt == NULL || w.pt == NULL || h.pt == NULL)
		{
		builtin_err = Err_null_ref;
		return(FALSE);
		}

	rect.x		= *((int *)(x.pt));
	rect.y		= *((int *)(y.pt));
	rect.width	= *((int *)(w.pt));
	rect.height = *((int *)(h.pt));

	if(rect_in_place(&rect) >= Success)
		if (clip_move_rect(&rect) >= Success)
			{
			*((int *)(x.pt)) = rect.x;
			*((int *)(y.pt)) = rect.y;
			*((int *)(w.pt)) = rect.width;
			*((int *)(h.pt)) = rect.height;
			return TRUE;
			}

		return FALSE;
}

static int po_ttextf(long vargcount, long vargsize, Popot pofmt, ...)
/*****************************************************************************
 * int printf(char *format, ...)
 ****************************************************************************/
{
va_list args;
int 	rv;

va_start(args, pofmt);

if (Success > po_check_formatf(TTEXTF_MAXCHARS, vargcount, vargsize,
				pofmt.pt, args))
	return builtin_err;

rv = ttextf(pofmt.pt, args, NULL);
va_end(args);
return rv;
}

static Errcode po_ErrBox(long vargcount, long vargsize, Errcode err, Popot pofmt, ...)
/*****************************************************************************
 * ErrCode Qerror(ErrCode err, char *format, ...)
 ****************************************************************************/
{
char etext[ERRTEXT_SIZE];
va_list args;
char *fmt;
Boolean mouse_was_on;

if(!get_errtext(err,etext))
	return(err);

va_start(args, pofmt);

if ((fmt = pofmt.pt) == NULL)
	{
	fmt = "";
	}
else
	{
	if (Success > po_check_formatf(0, vargcount, vargsize, fmt, args))
		return builtin_err;
	}

mouse_was_on = show_mouse();
varg_continu_box(NULL,fmt,args,etext);
if (!mouse_was_on)
	hide_mouse();
va_end(args);
return(Err_reported);
}

static void po_TextBox(long vargcount, long vargsize, Popot pofmt, ...)
/*****************************************************************************
 * void Qtext(char *format, ...)
 ****************************************************************************/
{
va_list args;
char *fmt;
Boolean mouse_was_on;

va_start(args, pofmt);

fmt = pofmt.pt;
if (Success > po_check_formatf(0, vargcount, vargsize, fmt, args))
	return;
mouse_was_on = show_mouse();
varg_continu_box(NULL,fmt,args,NULL);
if (!mouse_was_on)
	hide_mouse();
va_end(args);
}


static Boolean po_YesNo(long vargcount, long vargsize, Popot question, ...)
/*****************************************************************************
 * Boolean Qquestion(char *question, ...)
 ****************************************************************************/
{
va_list args;
char *fmt;
Boolean rv;
Boolean mouse_was_on;

va_start(args, question);
fmt = question.pt;
if (Success > po_check_formatf(0, vargcount, vargsize, fmt, args))
	return builtin_err;
mouse_was_on = show_mouse();
rv = varg_yes_no_box(NULL,fmt,args);
if (!mouse_was_on)
	hide_mouse();
va_end(args);
return rv;
}

static Boolean po_Slider(Popot inum, int min, int max, Popot hailing)
/*****************************************************************************
 * Boolean Qnumber(int *num, int min, int max, char *header)
 ****************************************************************************/
{
short num;
Boolean cancel;
Boolean mouse_was_on;

if (hailing.pt == NULL)
	hailing.pt = "";

if (inum.pt == NULL)
	return(builtin_err = Err_null_ref);
num = *((int *)(inum.pt));

if ((num < SHRT_MIN) ||
	(min < SHRT_MIN) ||
	(max < SHRT_MIN) ||
	(num > SHRT_MAX) ||
	(min > SHRT_MAX) ||
	(max > SHRT_MAX) ||
	(min > max))
	return(builtin_err = Err_parameter_range);

mouse_was_on = show_mouse();
if (FALSE != (cancel = qreq_number(&num,min,max,"%s",hailing.pt)))
	*((int *)(inum.pt)) = num;
if(!mouse_was_on)
	hide_mouse();
return cancel;
}

static Errcode popot_to_pt(Popot *ppp, void **cc, int count)
/*****************************************************************************
 * service routine
 ****************************************************************************/
{
Popot *pp;
int i;

if ((pp = ppp->pt) == NULL)
	return(Err_null_ref);
if (Popot_bufsize(ppp) < count*sizeof(Popot))
	return(Err_index_big);
for (i=0; i<count; ++i, ++cc, ++pp)
	{
	if (NULL == (*cc = pp->pt))
		return(Err_null_ref);
	}
return(Success);
}

static Errcode po_ChoiceBox(long vargcount, long vargsize, Popot pchoices, int ccount, Popot pfmt, ...)
/*****************************************************************************
 * int Qchoice(char **buttons, int bcount, char *header, ...)
 ****************************************************************************/
{
va_list args;
char	*choices[TBOX_MAXCHOICES+1];
char	*fmt;
Errcode rv;
Boolean mouse_was_on;

/* do some error checking on parameters */
if (ccount > TBOX_MAXCHOICES)
	ccount = TBOX_MAXCHOICES;

/* Transfer button strings into NULL-terminated list of C pointers */
if ((builtin_err = popot_to_pt(&pchoices, choices, ccount)) < Success)
	return(builtin_err);
choices[ccount] = NULL;

va_start(args, pfmt);
fmt = pfmt.pt;
if (Success > po_check_formatf(0, vargcount, vargsize, fmt, args))
	return builtin_err;
mouse_was_on = show_mouse();
rv = tboxf_choice(icb.input_screen,NULL,fmt,args,choices,NULL);
if (!mouse_was_on)
	hide_mouse();
va_end(args);
return rv;
}

static Boolean po_FileMenu(Popot suffix, Popot button,
		Popot inpath, Popot outpath, int force_suffix, Popot prompt)
/*****************************************************************************
 * Boolean Qfile(char *suffix, char *button,
 *	  char *inpath, char *outpath, Boolean force_suffix, char *header)
 ****************************************************************************/
{
Boolean rv = TRUE;
Boolean mouse_was_on;
char titbuf[40];

if (prompt.pt == NULL || 0 == strlen(prompt.pt))
	prompt.pt = stack_string("poco_qfile",titbuf);

if (suffix.pt == NULL || 0 == strlen(suffix.pt) || '.' != *(char *)(suffix.pt))
	{
	force_suffix = FALSE;
	suffix.pt = ".*";
	}

if (button.pt == NULL || 0 == strlen(button.pt))
	button.pt = ok_str;

if (inpath.pt == NULL)
	{
	builtin_err = Err_null_ref;
	goto ERR;
	}

if (Popot_bufcheck(&outpath,PATH_SIZE) < Success)
	goto ERR;

mouse_was_on = show_mouse();
if (NULL == (outpath.pt = pj_get_filename(prompt.pt, suffix.pt,
		button.pt, inpath.pt, outpath.pt, force_suffix, NULL, NULL)))
	{
	rv = FALSE;
	}
if (!mouse_was_on)
	hide_mouse();

ERR:
return rv;
}


static Boolean po_qstring(Popot strbuf, int bufsize, Popot hailing)
/*****************************************************************************
 * Boolean Qstring(char *string, int size, char *header)
 ****************************************************************************/
{
Boolean rv;
Boolean mouse_was_on;

if (hailing.pt == NULL)
	hailing.pt = "";
if (bufsize < 2)
	{
	builtin_err = Err_buf_too_small;
	return(FALSE);
	}
mouse_was_on = show_mouse();
if (Popot_bufcheck(&strbuf, bufsize) >= Success)
	rv = qreq_string(strbuf.pt,bufsize-1,"%s", hailing.pt);
else
	rv = FALSE;
if (!mouse_was_on)
	hide_mouse();
return rv;
}


static int po_some_choice(Popot pchoices, int ccount
, USHORT *flags, Popot header)
{
#define CMAX 10
#define CHMAX 60
char *pbuf[CMAX];
int i;
Boolean mouse_was_on;

if (ccount < 0 || ccount > CMAX)
	return(builtin_err = Err_parameter_range);
if (header.pt == NULL)
	header.pt = "";
if ((builtin_err = popot_to_pt(&pchoices, pbuf, ccount)) < Success)
	return(builtin_err);
for (i=0; i<ccount; i++)
	{
	if (strlen(pbuf[i]) > CHMAX)
		return(builtin_err = Err_string);
	}
mouse_was_on = show_mouse();
i = qchoice(flags, header.pt, pbuf, ccount);
if (!mouse_was_on)
	hide_mouse();
if(i < 0)
	return(0);
return(i + 1);
#undef CHMAX
#undef CMAX
}

static int po_qmenu(Popot pchoices, int ccount, Popot header)
/*****************************************************************************
 * int Qmenu(char **choices, int ccount, char *header)
 ****************************************************************************/
{
return po_some_choice(pchoices, ccount, NULL, header);
}

static int po_qmenu_with_flags(Popot pchoices, int ccount
, Popot pflags, Popot header)
/*****************************************************************************
 *int  QmenuWithFlags(char **choices, int ccount, short *flags, char *header)
 ****************************************************************************/
{
int i;
USHORT save;
USHORT *flags;
if ((flags = pflags.pt) != NULL)
	{
	if (Popot_bufcheck(&pflags,ccount * sizeof(USHORT)) < Success)
		return builtin_err;
	/* Rotate flags around to make indexes match the return choice value.
	 * That is put "cancel" at zero. */
	save = flags[0];
	for (i=1; i<ccount; ++i)
		{
		flags[i-1] = flags[i];
		}
	flags[ccount - 1] = save;
	}
return po_some_choice(pchoices, ccount, flags, header);
}

static Errcode popot_to_names(Popot *ppnames, int pcount, Names **pnames)
/*****************************************************************************
 * convert a poco char *names[]  array to a Names list
 ****************************************************************************/
{
void *s;
Popot *pp;
int i;
Errcode err;
Names *names = NULL;
Names *new;

if ((pp = ppnames->pt) == NULL)
	{
	err = Err_null_ref;
	goto ERR;
	}
if (Popot_bufsize(ppnames) < pcount*sizeof(Popot))
	return(Err_index_big);
for (i=0; i<pcount; i++)
	{
	if ((s = pp->pt) == NULL)
		{
		err = Err_null_ref;
		goto ERR;
		}
	if ((new = pj_malloc(sizeof(*new))) == NULL)
		{
		err = Err_no_memory;
		goto ERR;
		}
	new->name = s;
	new->next = names;
	names = new;
	pp += 1;
	}
*pnames = reverse_slist(names);
return(Success);
ERR:
free_wild_list(&names);
*pnames = NULL; 			/* initialize return list at empty */
return(err);
}


static Boolean po_Qlist(Popot choice_str, Popot choice_ix, Popot items,
		int icount, Popot ipos, Popot header)
/*****************************************************************************
 *
 * Boolean Qlist(char *choicestr, int *choice,
 *	 char **items, int icount, int *ipos, char *header)
 ****************************************************************************/
{
Names *nlist = NULL;
Names *nsel;
char *retbuf = NULL;
Boolean retbuf_allocated = FALSE;
short ipo = 0;
Boolean ret = FALSE;
int maxchars;
Boolean mouse_was_on;

if (choice_ix.pt == NULL)
	{
	builtin_err = Err_null_ref;
	goto OUT;
	}
if (ipos.pt != NULL)
	ipo = *((int *)(ipos.pt));
if (header.pt == NULL)
	header.pt = "";
if ((builtin_err = popot_to_names(&items, icount, &nlist)) < Success)
	goto OUT;
maxchars = longest_name(nlist)+1;

if (choice_str.pt == NULL)
	{
	if ((retbuf = pj_zalloc(maxchars)) == NULL)
		{
		builtin_err = Err_no_memory;
		goto OUT;
		}
	retbuf_allocated = TRUE;
	}
else
	{
	if (Popot_bufcheck(&choice_str, maxchars) < Success)
		goto OUT;
	retbuf = choice_str.pt;
	}

mouse_was_on = show_mouse();
if ((ret = qscroller(retbuf, header.pt, nlist, 10, &ipo)) != 0)
	{
	nsel = name_in_list(retbuf, nlist);
	*((int *)(choice_ix.pt)) = slist_ix(nlist, nsel);
	}
if (!mouse_was_on)
	hide_mouse();
if (ipos.pt != NULL)
	*((int *)(ipos.pt)) = ipo;
OUT:
	free_wild_list(&nlist);
	if (retbuf_allocated)
		pj_gentle_free(retbuf);
	return(ret);
}

static int	 lastbtn;
static Names *lastsel;

static Errcode remember_ok_btn(Names *which, void *data)
{
	lastsel = which;
	lastbtn = 1;
	return Success;
}

static Boolean remember_info_btn(Names *which, void *data)
{
	lastsel = which;
	lastbtn = 2;
	return TRUE;
}

static int po_Qscroll(Popot choice_ix, Popot items,
		int icount, Popot ipos, Popot button_texts, Popot header)
/*****************************************************************************
 * Boolean Qscroll(int *choice, char **items, int icount, int *ipos, char *hdr)
 ****************************************************************************/
{
	Names	*nlist = NULL;
	Names	*cursel;
	char	*btexts[3];
	char	**usebtexts;
	Popot	*pptbtexts;
	short	ipo = -1;
	int 	ret;
	int 	maxchars;
	Boolean mouse_was_on;
	void	*use_info_btn;	// lazy, lazy

	if (choice_ix.pt == NULL)
		{
		builtin_err = Err_null_ref;
		goto OUT;
		}

	if (header.pt == NULL)
		header.pt = "";

	if ((builtin_err = popot_to_names(&items, icount, &nlist)) < Success)
		goto OUT;
	maxchars = longest_name(nlist)+1;

	if (ipos.pt != NULL)
		ipo = *((int *)(ipos.pt));
	if (ipo < 0)
		cursel = NULL;
	else
		cursel = slist_el(nlist, ipo);

	if (button_texts.pt == NULL)
		usebtexts = NULL;
	else
		{
		if (Popot_bufsize(&button_texts) < 3*sizeof(Popot))
			{
			builtin_err = Err_index_big;
			goto OUT;
			}
		usebtexts = btexts;
		pptbtexts = button_texts.pt;
		if ( (NULL == (btexts[0] = pptbtexts[0].pt)) ||
			 (NULL == (btexts[2] = pptbtexts[2].pt)) )
			{
			builtin_err = Err_null_ref;
			goto OUT;
			}
		if (NULL == (btexts[1] = pptbtexts[1].pt))
			btexts[1] = "";
		}

	if (*btexts[1] == '\0')
		use_info_btn = NULL;
	else
		use_info_btn = remember_info_btn;

	lastbtn = 0;
	lastsel = NULL;
	mouse_was_on = show_mouse();
	ret = go_driver_scroller(header.pt, nlist, cursel,
							  remember_ok_btn,
							  use_info_btn,
							  NULL, usebtexts);
	if (!mouse_was_on)
		hide_mouse();

	*((int *)(choice_ix.pt)) = ipo = slist_ix(nlist, lastsel);

	if (ret >= Success)
		ret = lastbtn;

	if (ipos.pt != NULL)
		*((int *)(ipos.pt)) = ipo;
OUT:
	free_wild_list(&nlist);
	return(ret);
}

static int position_cursor_and_edit(Text_file *gf, Popot *pop_cursor_position
,	Popot *pop_top_line)
/*****************************************************************************
 * Check that the cursor-position and top-line-in-text-window pointers are
 * good,  and call the text editor.  Returns error or size of text.
 ****************************************************************************/
{
	static int stop_line, scursor_position;
	int *pcursor_position;
	int *ptop_line;

	/* Set cursor & top line in window position to be the what they pass in
	 * or if they pass in NULL just whatever it last was. */
	if ((pcursor_position = pop_cursor_position->pt) == NULL)
		pcursor_position = &scursor_position;
	if ((ptop_line = pop_top_line->pt) == NULL)
		ptop_line = &stop_line;

	gf->text_yoff = *ptop_line;
	gf->tcursor_p = *pcursor_position;
	full_screen_edit(gf);
	*pcursor_position = gf->tcursor_p;
	*ptop_line = gf->text_yoff;
	return gf->text_size;
}

static int po_edit(Popot ptext, int max_size, Popot pop_cursor_position
, Popot pop_top_line)
/*****************************************************************************
 * Edit a text buffer in memory.  Input should be a zero-terminated string
 * inside a buffer of at least max_size.  (The max size should be two
 * characters longer than the string you actually want.  For instance
 * if they need to edit an 8 character file name make max_size 10.)
 * It's ok for cursor_position and top_line to be NULL.
 ****************************************************************************/
{
	int size;
	Text_file tf;

	if (Popot_bufcheck(&ptext,max_size) < Success)
		return builtin_err;
	clear_struct(&tf);
	tf.text_alloc = max_size;
	tf.text_buf = ptext.pt;
	tf.text_size = strlen(tf.text_buf);
	return position_cursor_and_edit(&tf, &pop_cursor_position, &pop_top_line);
}

static int po_edit_file(Popot pop_file_name
, Popot pop_cursor_position, Popot pop_top_line)
/*****************************************************************************
 * int QeditFile(char *file_name, int *cursor_position, int *top_line);
 *		Read in a file, edit it, and write it back out.
 ****************************************************************************/
{
	char *file_name;
	Text_file tf;
	int size;
	Errcode err;

/* Make sure that all the parameters are good. */
	if ((file_name = pop_file_name.pt) == NULL)
		return (builtin_err = Err_null_ref);

	clear_struct(&tf);
	load_text_file(&tf, file_name);
	size = position_cursor_and_edit(&tf, &pop_cursor_position, &pop_top_line);
	if ((err = save_text_file(&tf)) < Success)
		size = err;
	free_text_file(&tf);
	return size;
}

/*----------------------------------------------------------------------------
 * library protos...
 *
 * Maintenance notes:
 *	To add a function to this library, add the pointer and prototype string
 *	TO THE END of the following list.  Then go to POCOLIB.H and add a
 *	corresponding prototype to the library structure typedef therein which
 *	matches the name of the structure below.  When creating the prototype
 *	in POCOLIB.H, remember that all arguments prototyped below as pointers
 *	must be defined as Popot types in the prototype in pocolib.h; any number
 *	of stars in the Poco proto still equate to a Popot with no stars in the
 *	pocolib.h prototype.
 *
 *	DO NOT ADD NEW PROTOTYPES OTHER THAN AT THE END OF AN EXISTING STRUCTURE!
 *	DO NOT DELETE A PROTOTYPE EVER! (The best you could do would be to point
 *	the function to a no-op service routine).  Breaking these rules will
 *	require the recompilation of every POE module in the world.  These
 *	rules apply only to library functions which are visible to POE modules
 *	(ie, most of them).  If a specific typedef name exists in pocolib.h, the
 *	rules apply.  If the protos are coded as a generic array of Lib_proto
 *	structs without an explicit typedef in pocolib.h, the rules do not apply.
 *--------------------------------------------------------------------------*/

PolibUser po_libuser = {
po_ttextf,
	"int     printf(char *format, ...);",
cleanup_toptext,
	"void    unprintf(void);",
po_TextBox,
	"void    Qtext(char *format, ...);",
po_ChoiceBox,
	"int     Qchoice(char **buttons, int bcount, char *header, ...);",
po_qmenu,
	"int     Qmenu(char **choices, int ccount, char *header);",
po_YesNo,
	"Boolean Qquestion(char *question, ...);",
po_Slider,
	"Boolean Qnumber(int *num, int min, int max, char *header);",
po_qstring,
	"Boolean Qstring(char *string, int size, char *header);",
po_FileMenu,
	"Boolean Qfile(char *suffix, char *button,"
	" char *inpath, char *outpath, Boolean force_suffix, char *header);",
po_Qlist,
	"Boolean Qlist(char *choicestr, int *choice, "
	"char **items, int icount, int *ipos, char *header);",
qcolor,
	"int     Qcolor(void);",
po_ErrBox,
	"ErrCode Qerror(ErrCode err, char *format, ...);",
po_rub_box,
	"Boolean RubBox(int *x, int *y, int *w, int *h);",
po_rub_circle,
	"Boolean RubCircle(int *x, int *y, int *rad);",
po_rub_line,
	"Boolean RubLine(int x1, int y1, int *x2, int *y2);",
po_rub_poly,
	"int     RubPoly(int **x, int **y);",
po_drag_box,
	"Boolean DragBox(int *x, int *y, int *w, int *h);",
po_wait_click,
	"void    WaitClick(int *x, int *y, int *left, int *right, int *key);",
po_poll_input,
	"void    PollInput(int *x, int *y, int *left, int *right, int *key);",
po_wait_input,
	"void    WaitInput(int *x, int *y, int *left, int *right, int *key);",
po_get_abortable,
	"Boolean GetAbort(void);",
po_set_abortable,
	"Boolean SetAbort(Boolean abort);",
po_set_abort_handler,
	"void    SetAbortHandler(Boolean (*handler)(void *data), void *data);",
hide_mouse,
	"Boolean HideCursor(void);",
show_mouse,
	"Boolean ShowCursor(void);",
po_Qscroll,
	"Boolean Qscroll(int *choice, char **items, int icount,"
	" int *ipos, char **button_texts, char *header);",
po_UdSlider,
 	"Boolean UdQnumber(int *num, int min, int max," 
      "Errcode (*update)(void *data, int num), void *data, char *fmt,...);",
po_edit,
	"int Qedit(char *text_buffer, int max_size,  int *cursor_position,"
	" int *top_line);",
po_edit_file,
	"int QeditFile(char *file_name, int *cursor_position, int *top_line);",
po_physical_wait_click,
	"void PhysicalWaitClick(int *x, int *y, int *left, int *right, int *key);",
po_physical_poll_input,
	"void PhysicalPollInput(int *x, int *y, int *left, int *right, int *key);",
po_physical_wait_input,
	"void PhysicalWaitInput(int *x, int *y, int *left, int *right, int *key);",
po_physical_rub_move_box,
	"Boolean PhysicalRubMoveBox(int *x, int *y, int *width, int *height, Boolean clip_to_screen);",
po_qmenu_with_flags,
	"int     QmenuWithFlags(char **choices, int ccount"
	", short *flags, char *header);",
};

Poco_lib po_user_lib =
	{
	NULL, "User Interface",
	(Lib_proto *)&po_libuser, POLIB_USER_SIZE,
	};
