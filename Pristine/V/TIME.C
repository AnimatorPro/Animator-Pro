
/* time.c - stuff to insert and delete frames in a FLIC.  Used mostly
   by timemenu.c. */

#include "jimk.h"
#include "fli.h"
#include "time.str"

extern long fli_comp1(), write_tflx();
extern long frame1_foff(), flx_file_hi();

qinsert_frames()
{
int x;

x = 1;
if (qreq_number(
	time_100 /* "How many frames to insert at current frame?" */, 
	&x, 1, 100))
	{
	if (x > 0)
		{
		scrub_cur_frame();
		insert_frames(x, vs.frame_ix);
		}
	}
}

delete_some(x)
int x;
{
scrub_cur_frame();
if (vs.frame_ix == 0)
	delete_first_frame(x);
else
	delete_middle_frames(vs.frame_ix, x);
if (vs.frame_ix >= fhead.frame_count)
	vs.frame_ix = fhead.frame_count-1;
vs.bframe_ix = 0;	/* invalidate back screen cashe */
fli_abs_tseek(render_form, vs.frame_ix);
see_cmap();
dirtyf();
save_undo();
}


qdelete_frames()
{
int x;

x = 1;
if (qreq_number(
	time_101 /* "How many frames to delete?" */,
	&x, 1, 100))
	{
	if (x > 0)
		{
		delete_some(x);
		}
	}
}

static
delete_middle_frames(start,frames)
int start;
int frames;
{
char *cbuf;
long size0;

/* don't delete past the end mate! */
if (start + frames >= fhead.frame_count)
	frames = fhead.frame_count - start;

/* frame before start of deleted segment goes into uf */
fli_abs_tseek(&uf, start-1);
/* frame after end goes into vf */
fli_tseek(&vf, vs.frame_ix, start+frames);	

if ((cbuf = lbegmem(CBUF_SIZE)) == NULL)
	goto OUT;

/* copy our offset/size table down a notch */
copy_structure(cur_flx+start+frames, cur_flx+start, 
	(fhead.frame_count+1 - (start+frames))*sizeof(Flx));

fhead.frame_count -= frames;

/* deallocate an old delta */
cur_flx[start].foff = 0;
cur_flx[start].fsize = 0;

/* compress and write middle frame */
size0 = fli_comp_frame(cbuf,
	uf.p, uf.cmap,
	render_form->p,render_form->cmap,
	FLI_LC);
if ((cur_flx[start].foff = write_tflx(cbuf, size0)) == 0)
	goto OUT;
cur_flx[start].fsize = size0;

OUT:
gentle_freemem(cbuf);
}

static
delete_first_frame(frames)
int frames;
{
char *cbuf;
long size0;

/* check for reasonable frames value */
if (frames >= fhead.frame_count)
	frames = fhead.frame_count-1;
/* at entry render_form is our frame with no menus up.  Will use
   uf (undo form) for temporary buffer.  */

/* uf goes to end frame */
fli_abs_tseek(&uf, fhead.frame_count-1);

/* render_form goes to this frame */
fli_abs_tseek(render_form, frames);

/* copy our offset/size table down a notch */
copy_structure(cur_flx+frames, cur_flx, 
	(fhead.frame_count+1 - frames)*sizeof(Flx));

/* show 1st frame as deallocated */
cur_flx[0].foff = 0;
cur_flx[0].fsize = 0;

fhead.frame_count -= frames;

/* show ring frame as deallocated */
cur_flx[fhead.frame_count].foff = 0;
cur_flx[fhead.frame_count].fsize = 0;


/* get a compression buffer */
if ((cbuf = lbegmem(CBUF_SIZE)) == NULL)
	goto OUT;

/* compress and write new 1st frame */
size0 = fli_comp1(cbuf,render_form->p,render_form->cmap);
if ((cur_flx[0].foff = write_tflx(cbuf, size0))==0)
	goto OUT;
cur_flx[0].fsize = size0;

/* compress and write ring frame */
size0 = fli_comp_frame(cbuf,
	uf.p, uf.cmap,
	render_form->p,render_form->cmap,
	FLI_LC);
if ((cur_flx[fhead.frame_count].foff = write_tflx(cbuf, size0)) == 0)
	goto OUT;
cur_flx[fhead.frame_count].fsize = size0;

/* update uf again */
OUT:
gentle_freemem(cbuf);
}


qmake_frames()
{
int x;

x = fhead.frame_count;
if ( qreq_number(
	time_102 /* "How many frames do you want in this flic?" */,  &x, 1,
	100))
	if (x > 0)
		make_some(x);
}

static
make_some(x)
int x;
{
if (x > 0)
	{
	if (x >= fhead.frame_count ||
		yes_no_line(
			time_103 /* "This will chop off frames at the end ok?" */) )
		{
		scrub_cur_frame();
		make_frames(x);
		}
	}
}

/* change  the number of frames in this fli.  Will chop off ones at end if
   less, add empty frames (duplicates of last one) if at beginning */
static
make_frames(frames)
int frames;
{
if (frames == fhead.frame_count)	/* whew! That was easy */
	return;
if (frames > fhead.frame_count)
	insert_frames(frames-fhead.frame_count, fhead.frame_count-1);
else
	{
	delete_middle_frames(frames,fhead.frame_count - (frames));
	if (vs.frame_ix >= fhead.frame_count)
		vs.frame_ix = fhead.frame_count-1;
	fli_abs_tseek(render_form, vs.frame_ix);
	see_cmap();
	save_undo();
	}
dirtyf();
}


static expand_cur_flx(new)
register int new;
{
register Flx *new_flx;
long newbytes;
long off1;
int i;


if (1+new <= fhead.frames_in_table)
	return(1);
#ifdef DEBUG
continu_line("Expanding frames_in_table");
#endif DEBUG
new += 100;	/* don't want to have to do this all the time! */
if (new > MAXFRAMES)
	new = MAXFRAMES;
if ((new_flx = begmemc(new*sizeof(Flx) )) == NULL)
	return(0);
copy_structure(cur_flx, new_flx, (fhead.frame_count+1)*sizeof(Flx));
freemem(cur_flx);
cur_flx = new_flx;
newbytes = (new - fhead.frames_in_table)*sizeof(Flx);
off1 = frame1_foff();
copy_in_file(tflx, flx_file_hi()-off1, off1, off1+newbytes);
i = fhead.frame_count+1;
while (--i >= 0)
	{
	new_flx->foff += newbytes;
	new_flx++;
	}
fhead.frames_in_table = new;
return(1);
}

check_max_frames(long new)
{
if (new > MAXFRAMES)
	{
	char buf[40];

	sprintf(buf, time_104 /* "No more than %d frames!" */, MAXFRAMES);
	continu_line(buf);
	return(0);
	}
}

insert_frames(count, where)
int count, where;
{
Flx *open_part;
struct fli_frame frame;
long offset;
struct fli_frame *buf;
int i;
int new;

/* make sure we don't go over the limit. */
new = count+fhead.frame_count;
if (!check_max_frames((long)new))
	return(0);
/* expand frames_in_table if necessary */
if (!expand_cur_flx(new))
	return(0);
/* make up structure for a blank frame */
zero_structure(&frame, sizeof(frame) );
frame.size = sizeof(frame);
frame.type = FLIF_MAGIC;
where++;
open_part = cur_flx + where;
back_copy_bytes(open_part, open_part+count, 
	(fhead.frame_count+1-where) * sizeof(Flx) );
zero_structure(open_part, count*sizeof(Flx) );
fhead.frame_count+=count;
if ((buf = lbegmem(count * (long)sizeof(frame))) == NULL)
	return(0);
for (i=0; i<count; i++)
	copy_structure(&frame, buf+i, sizeof(frame) );
offset = write_tflx(buf, (long)count * sizeof(frame) );
freemem(buf);
if (offset == 0)
	return(0);
while (--count >= 0)
	{
	open_part->foff = offset;
	open_part->fsize = sizeof(frame);
	open_part++;
	offset += sizeof(frame);
	}
vs.bframe_ix = 0;	/* invalidate back screen cashe */
dirtyf();
return(1);
}

