
/* savefli.c - Make a FLIC file from temp file (temp.flx) on scratch
   device.  This thing just takes out the indexing.  Doesn't actually
   have to recompress any images.  (That's done in writefli.c). */

#include "jimk.h"
#include "fli.h"
#include "flicmenu.h"
#include "jfile.h"
#include "memory.h"
#include "savefli.str"

char dirty_file;
char dirty_frame;

extern FILE *tflx;
extern char tflxname[];
extern struct fli_head fhead;
extern long frame1_foff(), flx_file_hi();

stroke_count(strokes)
int strokes;
{
dirty_file = 1;
fhead.strokes+= strokes;
fhead.session+= strokes;
}

dirtyf()
{
stroke_count(1);
}

dirties()
{
dirty_frame = 1;
dirtyf();
}

cleans()
{
dirty_file = dirty_frame = 0;
}


/* scrub_cur_frame() - updates tflx with vf*/
scrub_cur_frame()
{
if (dirty_frame)
	{
	dirty_frame = 0;
	return(sub_cur_frame());
	}
return(TRUE);
}

noroom()
{
continu_line(savefli_100 /* "Not enough room on temporary drive." */);
}

long
write_tflx(cbuf, size)
char *cbuf;
long size;
{
long offset;

if ((offset = find_free_tflx(size)) == 0)
	return(0);
jseek(tflx, offset, 0);	/* seek to free frame */
if (jwrite(tflx, cbuf, size) < size)
	{
	noroom();
	return(0);
	}
return(offset);
}

long
fli_comp1(cbuf,this_screen,this_cmap)
char *cbuf;
char *this_screen, *this_cmap;
{
return(
	fli_comp_frame(cbuf,NULL,NULL,this_screen,this_cmap,FLI_BRUN));
}

f_tempflx()
{
if (tflx)
	{
	jseek(tflx, 0L, 0);
	jwrite(tflx, &fhead, (long)sizeof(fhead) );
	vs.rmyoff = quick_menu.y;
	jwrite(tflx, &vs, (long)sizeof(vs) );
	jwrite(tflx, cur_flx, (long)(fhead.frames_in_table)*sizeof(Flx) );
	}
}


/* This corrupts uf! */
sub_cur_frame()
{
long pos0,size0;
long pos2,size2;
long posz,sizez;
int i;
char *cbuf;
int success = 0;
int pushed = 0;
int unzoomed = 0;

if (!tflx)
	return(0);
if (vs.frame_ix == vs.bframe_ix)	/* mark buffer frame as no good */
	vs.bframe_ix = 0;
/* grovel for memory... freeing up more and more ... */
if ((cbuf = laskmem(CBUF_SIZE)) == NULL)
	{
	unzoomed = 1;
	unzoom();
	if ((cbuf = laskmem(CBUF_SIZE)) == NULL)
		{
		pushed = 1;
		push_pics();
		if ((cbuf = lbegmem(CBUF_SIZE)) == NULL)
			goto OUT;
		}
	}
if (!gb_unfli(&uf,0,0,cbuf))
	goto OUT;
if (vs.frame_ix == 0)
	{
	size0 = fli_comp1(cbuf,render_form->p,render_form->cmap);
	if ((pos0 = write_tflx(cbuf, size0)) == 0)
		{
		unundo();
		goto OUT;
		}
	}
else
	{
	if (!gb_fli_tseek(&uf,0, vs.frame_ix-1, cbuf))
		goto OUT;
	for (i=1;i<vs.frame_ix;i++)
		{
		if (!gb_unfli(&uf,i,0,cbuf))
			goto OUT;
		}
	size0 = fli_comp_frame(cbuf,uf.p,uf.cmap,
		render_form->p,render_form->cmap,FLI_LC);
	if ((pos0 = write_tflx(cbuf, size0)) == 0)
		{
		if (!gb_unfli(&uf,vs.frame_ix,0,cbuf))
			goto OUT;
		unundo();
		goto OUT;
		}
	}
if (!gb_unfli(&uf,vs.frame_ix,0,cbuf))
	goto OUT;
if (!gb_unfli(&uf,vs.frame_ix+1,0,cbuf))
	goto OUT;
cur_flx[vs.frame_ix].foff = pos0;
cur_flx[vs.frame_ix].fsize = size0;
size2 = fli_comp_frame(cbuf,render_form->p,render_form->cmap,
	uf.p,uf.cmap, FLI_LC);
if ((pos2 = write_tflx(cbuf, size2))==0)
	{
	unundo();
	goto OUT;
	}
cur_flx[vs.frame_ix+1].foff = pos2;
cur_flx[vs.frame_ix+1].fsize = size2;

/* possibly have to update last frame + 1 if changing 1st frame */
if (vs.frame_ix == 0)	
	{
	/* advance uf to last frame in file */
	if (!gb_fli_tseek(&uf,vs.frame_ix+1, fhead.frame_count-1,cbuf))
		goto OUT;
	sizez = fli_comp_frame(cbuf,uf.p,uf.cmap,render_form->p,
		render_form->cmap,FLI_LC);
	if ((posz = write_tflx(cbuf, sizez) ) == 0)	/* this error condition not
													handled right arr. */
		goto OUT;
	cur_flx[fhead.frame_count].foff = posz;
	cur_flx[fhead.frame_count].fsize = sizez;
	}
f_tempflx();
success = 1;

OUT:
gentle_freemem(cbuf);
if (pushed)
	pop_pics();
if (unzoomed)
	rezoom();
return(success);
}

sv_fli(name)
char *name;
{
#ifdef NOSAVE
#else /* NOSAVE */
int i;
char *cbuf;
FILE *ofile;
long sz;
long acc;
int success;

success = 0;
ofile = 0;
if (!tflx)
	return(0);
if ((cbuf = lbegmem(CBUF_SIZE)) == NULL)
	return(0);
copy_form(render_form,&uf);
acc = 0;
if ((ofile = jcreate(name) ) == 0)
	{
	cant_create(name);
	goto OUT;
	}
fhead.type = FLIH_MAGIC;
if (jwrite(ofile, &fhead,(long)sizeof(fhead)) < sizeof(fhead))
	{
	truncated(name);
	goto OUT;
	}
acc = sizeof(fhead);
for (i=0;i<=fhead.frame_count;i++)
	{
	jseek(tflx,(cur_flx[i]).foff,0); 
	sz = (cur_flx[i]).fsize;
	if (jread(tflx, cbuf, sz) < sz)
		{
		noroom();
		goto OUT;
		}
	if (jwrite(ofile,cbuf,sz) < sz)
		{
		truncated(name);
		goto OUT;
		}
	acc += sz;
	}
success = 1;
cleans();
OUT:
if (ofile)
	{
	jseek(ofile, 0L, 0);
	fhead.size = acc;
	jwrite(ofile, &fhead, (long)sizeof(fhead) );
	fhead.type = FLIX_MAGIC;
	jclose(ofile);
	copy_form(render_form,&uf);
	zoom_it();
	}
freemem(cbuf);
return(success);
#endif /* NOSAVE */
}


save_fli(name)
char *name;
{
#ifdef NOSAVE
#else /* NOSAVE */
int oix;

flush_tempflx();
if (sv_fli(name))
	{
	oix = vs.frame_ix;
	make_tempflx(name);
	vs.frame_ix = oix;
	return(1);
	}
else
	jdelete(name);
#endif /* NOSAVE */
return(0);
}


