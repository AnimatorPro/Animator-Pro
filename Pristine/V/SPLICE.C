/* splice.c - help join two animations together end to end perhaps
   with some transition effect */

#include "jimk.h"
#include "fli.h"
#include "flicmenu.h"
#include "commonst.h"
#include "splice.str"


static char *splice_lines[] = {
	splice_100 /* "to end" */,
	splice_101 /* "to start" */,
	cst_cancel,
	};

static char *join_ends[] = {
	splice_103 /* "Cut" */,
	splice_104 /* "Transition" */,
	cst_cancel,
	};

static char *transition_choices[] = {
	splice_106 /* "Fade Out" */,
	splice_107 /* "Wipe" */,
	splice_108 /* "Venetian" */,
	splice_109 /* "Dissolve" */,
	splice_110 /* "Circle Wipe" */,
	cst_cancel,
	};

#ifdef LATER
show_1_frame(title, fd, lscreen)
char *title;
int fd;
Vscreen *lscreen;
{
int success;
Vcel vcel, *ocel;
WORD ozc;

ozc = vs.zero_clear;
success = 0;
if (!read_next_frame(title,fd,lscreen,0))
	goto OUT;
screen_to_cel( lscreen, &vcel);
vs.zero_clear = 1;
cfit_cel(&vcel, render_form->cmap);
see_a_cel(&vcel);
vs.zero_clear = ozc;
return(1);
OUT:
return(success);
}
#endif LATER


qload_splice()
{
unzoom();
push_pics();
pushed_load_splice();
pop_pics();
rezoom();
}

static
pushed_load_splice()
{
int where;
char *title;
int fd;
struct fli_head flih;
Vscreen *lscreen;
int transition;
int sframe;
char *show;
int goodsplice;
int ttype, tfcount;
int ok;

title = NULL;
where = qchoice(splice_112 /* "Join flic?" */, 
	splice_lines, Array_els(splice_lines));
switch (where)
	{
	case 1:
		sframe = fhead.frame_count-1;
		show = splice_113 /* "Join flic to end?" */;
		break;
	case 2:
		sframe = 0;
		show = splice_114 /* "Join flic to start?" */;
		break;
	default:
		return;
	}
scrub_cur_frame();
if (!fli_tseek(render_form,vs.frame_ix,sframe))
	return;
see_cmap();
vs.frame_ix = sframe;
jdelete(screen_name);
if ((title = get_filename(show, ".FLI")) == NULL)
	return;
if ((fd = read_fli_head(title, &flih)) == 0)
	return;
save_undo();
ok = read_next_frame(title,fd,render_form,0);
jclose(fd);
if (!ok)
	{
	copy_form(&uf, render_form);
	return;
	}
see_cmap();
transition = qchoice(splice_116 /* "How to join the ends?" */, join_ends,
	Array_els(join_ends) );
copy_form(&uf, render_form);
see_cmap();
switch (transition)
	{
	case 0:
		goto OUT;
	case 1:		/* cut */
		ttype = 0;
		tfcount = 0;
		break;
	case 2:		/* transition */
		ttype = qchoice(splice_117 /* "Transition type" */, 
			transition_choices, Array_els(transition_choices) );
		if (ttype == 0)
			goto OUT;
		if (!qreq_number(splice_118 /* "Frames in transition" */,
			&vs.transition_frames, 1, 100))
			goto OUT;
		if ((tfcount = vs.transition_frames) <= 0)
			goto OUT;
		break;
	}
if (!sv_fli(new_tflx_name))
	{
	goto ALMOST;
	}
close_tflx();
switch (where)
	{
	case 1:
		goodsplice = load_2_flis(new_tflx_name,title,
			ttype,tfcount);
		break;
	default:
		goodsplice = load_2_flis(title,new_tflx_name,
			ttype,tfcount);
		break;
	}
if (!goodsplice)
	{
	make_tempflx(new_tflx_name);
	fli_abs_tseek(render_form,0);
	see_cmap();
	}
ALMOST:
jdelete(new_tflx_name);
OUT:
return;
}

extern long write_tflx_start(), frame_to_tflx();

static long 
write_middle_dif(ix,acc,frame,s0,s1)
int ix;
long acc;
struct fli_frame *frame;
Vscreen *s0,*s1;
{
long size;

size = fli_comp_frame(frame,s0->p, s0->cmap, 
	s1->p,s1->cmap,FLI_LC);
if (jwrite(tflx, frame, size) < size)
	{
	noroom();
	return(0);
	}
cur_flx[ix].foff = acc;
cur_flx[ix].fsize = size;
acc += size;
return(acc);
}

static long
write_middle_one(ix, acc, frame)
int ix;
long acc;
struct fli_frame *frame;
{
return(write_middle_dif(ix,acc,frame,render_form,&uf));
}

static long
write_transition_frames(ttype, tfcount, cur_ix, acc, cbuf)
int ttype, tfcount;	/* transition type and count */
int cur_ix;		/* frame index */
long acc;	/* file offset */
struct fli_frame *cbuf;		/* CBUF_SIZE compression buffer/frame head */
{
int i;
Vscreen *tframe;
PLANEPTR icmap;
int half, vhalf;

half = tfcount/2;
vhalf = tfcount - half;
if ((tframe = clone_screen(render_form)) == NULL)
	{
	return(0);
	}
if ((icmap = begmem(COLORS*3)) == NULL)
	{
	free_screen(tframe);
	return(0);
	}
copy_cmap(render_form->cmap,icmap);
for (i=1; i<=tfcount; i++)
	{
	copy_form(render_form,tframe);
	/* first do colors */
	switch (ttype)
		{
		case 1:	/* fade */
			if (i <= half)
				{
				true_fades(icmap, pure_black, i, half, render_form->cmap,
					COLORS);
				}
			else
				{
				true_fades(uf.cmap, pure_black, tfcount-i, 
					vhalf, render_form->cmap,
					COLORS);
				}
			break;
		case 2:
		case 3:
		case 4:
		case 5:
			true_blends(icmap,uf.cmap,
				i, tfcount+1, render_form->cmap,COLORS);
			break;
		}
	see_cmap();
	/* now do pixels */
	switch(ttype)
		{
		case 1:  /* fade in/out */
			if (i == half)
				copy_structure(uf.p,render_form->p,64000);
			break;
		case 2:  /* whipe */
			blit8(XMAX,(int)((long)YMAX*i/(tfcount+1)), 0, 0, uf.p, uf.bpr,
				0,0,render_form->p,render_form->bpr);
			break;
		case 3:	/* venetian */
			venetian_tween(&uf, render_form, i, tfcount+1);
			break;
		case 4:	/* dissolve */
			dissolve_tween(&uf, render_form, i, tfcount+1);
			break;
		case 5: /* disk */
			diskd_tween(&uf, render_form, i, tfcount+1);
			break;
		default:
			break;
		}
	if ((acc = write_middle_dif(cur_ix,acc,cbuf,tframe,render_form))
		== 0)
		{
		freemem(icmap);
		free_screen(tframe);
		return(0);
		}
	cur_ix++;
	}
freemem(icmap);
free_screen(tframe);
return(write_middle_one(cur_ix, acc, cbuf));
}

static
load_2_flis(f1name,f2name,ttype,tfcount)
char *f1name, *f2name;
int ttype,tfcount;	/* transition type and frame count */
{
int f1,f2;
struct fli_head fh1,fh2;
long acc;
long frames;
int i;
int end;
struct fli_frame *frame;


if ((f1 = read_fli_head(f1name, &fh1)) == 0)
	return(0);
if ((f2 = read_fli_head(f2name, &fh2)) == 0)
	{
	jclose(f1);
	return(0);
	}
if ((frame = lbegmem(CBUF_SIZE)) == NULL)
	goto BADEND;
frames = fh1.frame_count + fh2.frame_count + tfcount + 100L;
fhead.frame_count = end = frames - 100;
if (frames >= MAXFRAMES)
	{
	continu_line(splice_119 /* "Too many frames, sorry." */);
	goto BADEND;
	}
if ((acc = write_tflx_start(frames)) == 0)
	goto BADEND;
/* go copy the first file to temp.flx */
for (i=0;i < fh1.frame_count; i++)
	{
	if ((acc = frame_to_tflx(f1, f1name, frame, acc, i)) <= 0)
		goto BADEND;
	uncompfli(render_form, frame, 1);
	}

/* deal with the transition from last frame of 1st file to start frame
   of 2nd file */
if (!gb_read_next_frame(f2name,f2,&uf,frame,0))
	goto BADEND;
switch (ttype)
	{
	case 0:
		if ((acc = write_middle_one(fh1.frame_count, acc, frame)) == 0)
			goto BADEND;
		break;
	default:
		if ((acc = write_transition_frames(
			ttype, tfcount,
			fh1.frame_count,acc,frame))
			== 0)
			goto BADEND;
		break;
	}
copy_form(&uf, render_form);	/* make it visible to user */
see_cmap();

/* go copy the second file to temp.flx */
for (i = fh1.frame_count+1+tfcount; i<end; i++)
	{
	if ((acc = frame_to_tflx(f2, f2name, frame, acc, i)) <= 0)
		goto BADEND;
	uncompfli(render_form, frame, 1);
	}

/* reread first frame of first file and make loop delta with last
   frame of second file */
jclose(f1);
if ((f1 = read_fli_head(f1name, &fh1)) == 0)
	goto BADEND;
if (!gb_read_next_frame(f1name,f1,&uf,frame,0))
	goto BADEND;
if ((acc = write_middle_one(end, acc, frame)) == 0)
	goto BADEND;
vs.frame_ix = end-1;
if (!finish_tflx())
	goto BADEND;
dirtyf();

GOODEND:
gentle_freemem(frame);
gentle_close(f1);
gentle_close(f2);
return(1);
BADEND:
gentle_freemem(frame);
gentle_close(f1);
gentle_close(f2);
close_tflx();
jdelete(tflxname);
return(0);
}


