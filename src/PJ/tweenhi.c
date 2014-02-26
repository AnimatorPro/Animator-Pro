/* tweenhi.c  - hi level tween functions callable outside the
   tween menu.  Load/save/wire-frame tween. */

#include "jimk.h"
#include "commonst.h"
#include "errcodes.h"
#include "imath.h"
#include "input.h"
#include "marqi.h"
#include "memory.h"
#include "tween.h"
#include "vmagics.h"
#include "xfile.h"

extern int ld_poly(XFILE *f, Poly *poly);

void a_wireframe_tween(Tween_state *tween,
	int frames, int speed, 
	Pixel dit_color, Pixel dash_color, Boolean closed,
	int play_mode)
/* Go do a wire-frame simulation of what tween move will look like
   so user can get a sense of what the timing will be before
   he goes to the pixel perfect (and slow) preview or even
   (gasp) to render it. */
{
int i;
long clock;
int scale; 
Marqihdr mh;
Errcode err = Success;
Tw_tlist tlist;
Short_xyz *points;
int point_count;

if (frames < 0)
	return;

if ((err = ts_to_tw_list(tween, closed, &tlist)) < Success)
	goto OUTBUF;
cinit_marqihdr(&mh,dit_color,dash_color,TRUE);

hide_mouse();
clock = pj_clock_1000();
do
	{
	for (i=0; i<frames; i++)
		{
		scale = calc_time_scale(i, frames);
		calc_tween_points(&tlist, closed, scale, &points, &point_count);
		msome_vector((void *)points,point_count,mh.pdot,&mh,
			!closed, sizeof(Short_xyz));
		err = poll_abort();
		clock += speed;
		do wait_sync(); while (clock > pj_clock_1000());
		if (clock < pj_clock_1000())
			clock = pj_clock_1000();
		msome_vector((void *)points, point_count,undo_marqidot,&mh,
			!closed, sizeof(Short_xyz));
		if(err < Success) /* abort */
			goto OUTLOOP;
		}
	} while (play_mode == TWEEN_LOOP);
OUTLOOP:
show_mouse();
OUTBUF:
trash_tw_list(&tlist);
softerr(err, NULL);
}

Errcode save_tween(char *name, Tween_state *ts)
{
XFILE *f;
Errcode err, cerr;
Tween_file_header tfh;
Tween_link *link, *next;

if ((f = xfopen(name, wb_str)) == NULL)
	return(xerrno());

clear_struct(&tfh);
tfh.magic = TWEEN_MAGIC;
tfh.tcount = 2;
tfh.link_count = listlen(&ts->links);
if (xfwrite(&tfh, 1, sizeof(tfh), f) < sizeof(tfh))
	goto IOERR;
for (link = (Tween_link *)(ts->links.head);
	NULL  != (next = (Tween_link *)(link->node.next));
	link = next)
	{
	if (xfwrite(&link->start, 1, 2*sizeof(link->start), f) <
		2*sizeof(link->start))
		goto IOERR;
	}
if ((err = s_poly(f, &ts->p0)) < Success)
	goto CLOSEOUT;
if ((err = s_poly(f, &ts->p1)) <  Success)
	goto CLOSEOUT;

goto CLOSEOUT;
IOERR:
	err = xerrno();
CLOSEOUT:
	cerr = xfclose(f);
	if (cerr < Success)		/* return primary error, not close error */
		{
		if (err >= Success)	/* but if close is 1st error return it... */
			err = cerr;
		}
	if (err < Success)
		pj_delete(name);
	return(err);
}

static Errcode ld_tween(char *name, Tween_state *ts)
{
XFILE *f;
Errcode err;
Tween_file_header tfh;
Tween_link *newl;
long i;

if ((f = xfopen(name, rb_str)) == NULL)
	return(xerrno());
if ((xfread(&tfh, 1, sizeof(tfh), f)) < sizeof(tfh))
	goto IOERR;
if (tfh.magic != TWEEN_MAGIC)
	{
	err = Err_bad_magic;
	goto CLOSEOUT;
	}
for (i=0; i<tfh.link_count; i++)
	{
	if ((err = news(newl)) < Success)
		goto CLOSEOUT;
	if (xfread(&newl->start, 1, 2*sizeof(newl->start), f)
		< 2*sizeof(newl->start))
		goto IOERR;
	add_tail(&ts->links,&newl->node);
	}
if ((err = ld_poly(f, &ts->p0)) <  Success)
	goto CLOSEOUT;
if ((err = ld_poly(f, &ts->p1)) <  Success)
	goto CLOSEOUT;
goto CLOSEOUT;
IOERR:
err = xerrno();

CLOSEOUT:
xfclose(f);
return(err);
}

Errcode load_tween(char *name, Tween_state *ts)
{
Errcode err;

init_tween_state(ts);
if ((err = ld_tween(name, ts)) < Success)
	{
	trash_tween_state(ts);
	}
return(err);
}

Errcode test_load_tween(char *name)
{
Tween_state ts;
Errcode err;

err = load_tween(name,&ts);
trash_tween_state(&ts);
return(err);
}



extern char curveflag;

Errcode tween1(Tween1_data *twd, int ix, int  intween, int  scale)
{
Poly dpoly;
Short_xyz *v;
int vcount;
LLpoint *d;
Errcode err;
int ocurve;
Boolean closed;
Tw_tlist tlist;
int i;

closed = vs.fillp || vs.closed_curve;
ocurve = curveflag;
curveflag = twd->is_spline;
if ((err = ts_to_tw_list(twd->ts, closed, &tlist)) < Success)
	goto OUT;
calc_tween_points(&tlist, closed, scale, &v, &vcount);
dpoly.pt_count = vcount;
if ((dpoly.clipped_list = d = 
	 pj_malloc(dpoly.pt_count*sizeof(LLpoint))) == NULL)
	{
	err = Err_no_memory;
	goto OUT;
	}
linkup_poly(&dpoly);
i = vcount;
while (--i>=0)
	{
	d->x = v->x;
	d->y = v->y;
	d->z = v->z;
	d = d->next;
	v += 1;
	}
err = render_poly(&dpoly, vs.fillp,vs.closed_curve);
pj_free(dpoly.clipped_list);
if (vs.cycle_draw) 
	cycle_ccolor();
OUT:
trash_tw_list(&tlist);
curveflag = ocurve;
return(err);
}

void render_a_tween(Tween_state *ts)
{
Tween1_data twda;
SHORT omulti;

twda.is_spline = vs.tween_spline;
twda.ts = ts;
omulti = vs.multi;
vs.multi = TRUE;
do_autodraw(tween1,&twda);
vs.multi = omulti;
}

Errcode tween_trail_frame(Tween_state *ts, int steps)
{
Tween1_data twda;
int i;
int scale;
Errcode err;

twda.is_spline = vs.tween_spline;
twda.ts = ts;
for (i=0; i<steps; i++)
	{
	scale =  calc_time_scale(i, steps);
	if ((err = tween1(&twda, i, steps, scale)) < Success)
		break;
	if (vs.cycle_draw) 
		cycle_ccolor();
	}
return(err);
}

