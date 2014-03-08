#include "jimk.h"
#include "composit.h"
#include "errcodes.h"
#include "flicel.h"
#include "flipath.h"
#include "flx.h"
#include "imath.h"
#include "memory.h"
#include "rastrans.h"

Compocb ccb;

typedef struct process_frame_data {
	void *cbuf; 	/* note for now you cant use the cbuf and the lbufs
					 * together they are same memory */
	Pixel *lbuf;
	Pixel *aline;
	Pixel *dline;
	Rcel *dest;
	Rcel *history;
	Boolean invert; /* use mask inverted 0 = b 1 = a */
	int docfit;   /* 0 = none, 1 = a, 2 = b, 3 = both */
	Pixel *acfit;
	Pixel *bcfit;
	int frame;
	int frame_count;
	Boolean reverse_wipe;
	Bitmap *boxil_mask;
	ULONG boxils_done;
} Pframedat;

typedef Errcode (*Initpframe)(Pframedat *pfd);
typedef Errcode (*Getmask)(int tscale1,int tscale0,Pframedat *pfd);
typedef void (*Closepframe)(Pframedat *pfd);

/***********************

	on scales for a Getmask:

	tscale1 is such that the first frame of the transition
	is (1/(transition_frames+1))*SCALE_ONE and
	the last is transition_frames/(transition_frames+1) * SCALE_ONE

	tscale0 is such that
	the first frame is 0/transition_frames-1 and or 0
	the last is transition_frames-1/transition_frames-1 or SCALE_ONE

***********************/



/**** cel blitter ****/

typedef struct pldat {
	Rcel *dest;
	Pixel *xlat;
} Pldat;

static Errcode putxlat_line(Pldat *pld, Pixel *line,
							Coor x, Coor y, Ucoor width)
{
	if(pld->xlat != NULL)
		pj_xlate(pld->xlat,line,width);
	pj_put_hseg(pld->dest,line,x,y,width);
	return(poll_abort());
}

static Errcode copy_fcel(Flicel *cel, Rcel *screen, Celcfit *cfit, int clearc)

/* if cel raster and screen are  the same will do nothing clearc of -2 will
 * make it use the old cfit if cfit is present */
{
Pldat pld;
Rcel *rc;
Errcode err = Success;

	start_abort_atom();

	if((rc = cel->rc) == screen)
		goto done;

	pld.xlat = NULL;
	if(cfit == NULL)
	{
		if(!cmaps_same(rc->cmap,screen->cmap))
		{
			pj_cmap_copy(rc->cmap,screen->cmap);
			if(screen == vb.pencel)
				see_cmap();
		}
	}
	else if( clearc == -2
			 || make_simple_cfit(rc->cmap, screen->cmap, cfit, clearc))
	{
		pld.xlat = cfit->ctable;
	}

	if(cel->flags & FCEL_XFORMED)
	{
		pld.dest = screen;
		err = raster_transform(rc,screen,&cel->xf,putxlat_line, &pld, 0 );
	}
	else
	{
		if(pld.xlat)
		{
			xlatblit(rc,0,0,screen,rc->x,rc->y,
					 rc->width,rc->height,pld.xlat);
		}
		else
		{
			pj_blitrect(rc,0,0,screen,rc->x,rc->y,
					 rc->width,rc->height);
		}
	}
done:
	return(errend_abort_atom(err));
}

/***** write un-composited frames to tempflx ******/


static Errcode do_gotlast_ringit(void *cbuf)
{
Errcode err;

	/* last frame in pencel put first in undo */
	if((err = unfli(undof, 0, 0)) < Success)
		return(err);
	err = write_ring_flxframe(NULL,&flix,cbuf,vb.pencel,undof);
}
static Errcode write_celframes(Flicel *src, SHORT start, SHORT num_frames,
							   Boolean keep_cel, Boolean ringit,
							   Celcfit *cfit)
/* Writes frames into tempflx from source cel starting starting with tflx
 * and writing frames of source from start to end. */
{
Errcode err;
Fli_frame *cbuf;
Rcel *rc;
LONG size;
LONG cbufsize;
Boolean must_compress;
SHORT max;
Boolean force_read;

	max = start + num_frames;

	rc = src->rc;
	cbufsize = pj_fli_cel_cbuf_size(vb.pencel);

	if(!num_frames) /* special case no frames added but ringing requested */
	{
		if(!ringit)
			return(Success);  /* we assume next call will make first frame */
		if(!flix.hdr.frame_count)
			return(Err_bad_input);
		if(NULL == (cbuf = pj_malloc(cbufsize)))
			goto nomem_error;
		err = do_gotlast_ringit(cbuf);
		goto done;
	}

	if(flix.hdr.frame_count != 0) /* put history in undo */
		save_undo();

	must_compress = (src->flif.hdr.width != vb.pencel->width
					|| src->flif.hdr.height != vb.pencel->height
					|| cfit != NULL);
	if(!must_compress && !keep_cel)
	{
		/* this is a bit weird: if we don't need to keep the cel raster and
		 * we don't need to cfit or resize the cel, we can conserve ram.
		 * We free the cels raster and use the pencel instead.
		 * This will keep the pencel with the current (and active for the user)
		 * frame of the cel.  First we copy cel contents free, and replace
		 * pointer.  We null out the pointer on exit if not the original
		 * cel raster.
		 *	  free_fcel_raster() will set a flag if it has already freed and
		 * won't attempt to free the pencel. In this case it can be done
		 * before allocing the cbuf. copy_fcel() also will do nothing if
		 * the cel and the screen are one in the same */
		pj_rcel_copy(rc,vb.pencel);
		see_cmap();
		free_fcel_raster(src);
		src->rc = vb.pencel; /* will now use pencel as cel raster */
	}
	else  if( !(src->flags & FCEL_XFORMED) && (rc->x > 0 || rc->y > 0))
	{
		/* if source cel is smaller than screen clear borders around it */
		if(cfit)
			pj_set_rast(vb.pencel,cfit->ctable[0]);
		else
			pj_clear_rast(vb.pencel);
	}

	force_read = (fcel_needs_seekbuf(src) || num_frames > 1);

	if(force_read && (size = fcel_cbuf_size(src)) > cbufsize)
		cbufsize = size;
	if(src->flags & FCEL_RAMONLY)
	{
		must_compress = TRUE;
	}
	else if(!must_compress)
	{
		force_read = TRUE;
	}
	if(NULL == (cbuf = pj_malloc(cbufsize)))
		goto nomem_error;

	if((err = reopen_fcelio(src, JREADONLY)) < Success)
		goto error;

	if((err = gb_seek_fcel_frame(src, start, cbuf, force_read)) < Success)
		goto error;

	if(flix.hdr.frame_count == 0) /* writing first frame */
	{
		if(must_compress || start != 0)
		{
			if((err = copy_fcel(src,vb.pencel,cfit,ccb.clearc)) < Success)
				goto error;
			err = write_first_flxframe(NULL,&flix,cbuf,vb.pencel);
		}
		else
			err = write_first_flxchunk(NULL, &flix, cbuf);
	}
	else /* undof has last frame saved to tflx */
	{
		if((err = copy_fcel(src,vb.pencel,cfit,ccb.clearc)) < Success)
			goto error;
		err = write_next_flxframe(NULL,&flix,cbuf,undof,vb.pencel);
	}
	if(err < Success)
		goto error;

	++start; /* did one */

	if(must_compress)
	{
		while(start < max)
		{
			if((err = poll_abort()) < Success)
				goto error;

			if((err = gb_seek_fcel_frame(src, start, cbuf, TRUE)) < Success)
				goto error;
			if(pj_i_is_empty_rec(cbuf)) /* just write an empty frame !! */
			{
				if((err = write_next_flxempty(NULL,&flix,1)) < Success)
					goto error;
			}
			else
			{
				save_undo(); /* move previous frame to undo */
				if((err = copy_fcel(src,vb.pencel,cfit,ccb.clearc)) < Success)
					goto error;
				if((err = write_next_flxframe(NULL,&flix,cbuf,
											  undof,vb.pencel)) < Success)
				{
					goto error;
				}
			}
			++start;
		}
		if(!keep_cel)
			free_fcel_raster(src);
	}
	else /* same size no cfitting - no recompression needed */
	{
		if(!keep_cel)
		{
			free_fcel_raster(src);
			if(ringit) /* don't need to keep last image */
				src->rc = NULL;
		}

		while(start < max)
		{
			if((err = poll_abort()) < Success)
				goto error;
			if((err = gb_seek_fcel_frame(src, start, cbuf, TRUE)) < Success)
				goto error;
			if((err = write_next_flxchunk(NULL, &flix, cbuf)) < Success)
				goto error;
			++start;
		}

		if(!keep_cel && ringit)
		{
			if((err = ring_tflx(cbuf)) < Success)
				goto error;
			goto done;
		}
		/* make sure last frame is in vb.pencel */

		if((err = copy_fcel(src,vb.pencel,cfit,ccb.clearc)) < Success)
			goto error;
	}

	if(ringit)
		err = do_gotlast_ringit(cbuf);

	goto done;

nomem_error:
	err = Err_no_memory;
done:
error:
	if(src->rc != rc)
		src->rc = NULL;
	pj_gentle_free(cbuf);
	close_fcelio(src);
	return(err);
}
/***** masked cel moving ******/
static void nocfit_line(Pixel *aline,Pixel *bline,Pixel *dline,Pixel *maxdline)
{
	while(dline < maxdline)
	{
		if(*dline)
		{
			*dline++ = *bline++;
			++aline;
		}
		else
		{
			*dline++ = *aline++;
			++bline;
		}
	}
}
static void acfit_line(Pixel *aline,Pixel *bline,Pixel *dline,Pixel *maxdline,
					   Pixel *acfit)
{
	while(dline < maxdline)
	{
		if(*dline)
		{
			*dline++ = *bline++;
			++aline;
		}
		else
		{
			*dline++ = acfit[*aline++];
			++bline;
		}
	}
}
static void bcfit_line(Pixel *aline,Pixel *bline,Pixel *dline,Pixel *maxdline,
					   Pixel *bcfit)
{
	while(dline < maxdline)
	{
		if(*dline)
		{
			*dline++ = bcfit[*bline++];
			++aline;
		}
		else
		{
			*dline++ = *aline++;
			++bline;
		}
	}
}
static void abcfit_line(Pixel *aline,Pixel *bline,Pixel *dline,Pixel *maxdline,
						Pixel *acfit, Pixel *bcfit)
{
	while(dline < maxdline)
	{
		if(*dline)
		{
			*dline++ = bcfit[*bline++];
			++aline;
		}
		else
		{
			*dline++ = acfit[*aline++];
			++bline;
		}
	}
}
static Errcode put_celsline(Pframedat *pfd, Pixel *bline,
						 Coor x, Coor y, Ucoor width)

/* line function for rendering masked cel lines */
{
Pixel *dline;
Pixel *maxdline;
Pixel *aline;

	aline = pfd->aline;
	dline = pfd->dline;
	maxdline = dline + width;

	pj__get_hseg(pfd->dest,dline,x,y,width);
	pj__get_hseg(ccb.fcela->rc,aline,x,y,width);

	switch(pfd->docfit)
	{
		case 0:
			nocfit_line(aline,bline,dline,maxdline);
			break;
		case 1:
			acfit_line(aline,bline,dline,maxdline,pfd->acfit);
			break;
		case 2:
			bcfit_line(aline,bline,dline,maxdline,pfd->bcfit);
			break;
		case 3:
			abcfit_line(aline,bline,dline,maxdline,pfd->acfit,pfd->bcfit);
			break;
	}
	pj__put_hseg(pfd->dest,dline,x,y,width);
	return(poll_abort());
}
static Errcode mask_move_cels(Pframedat *pfd)

/* ccb.cel a goes to mask 0 ccb.celb goes to mask non 0 */
{
int y;
Pixel *bline, *bgetbuf;
Rcel *bcel;
Coor width;
Coor bx, bwidth;
Errcode err = Success;

	width = pfd->dest->width;
	pfd->aline = pfd->lbuf;
	pfd->dline = pfd->aline + width;

	if(ccb.fcelb->flags & FCEL_XFORMED)
	{
		err = raster_transform(ccb.fcelb->rc,pfd->dest,&ccb.fcelb->xf,
								put_celsline, pfd, 0 );
		goto OUT;
	}

	bline = pfd->dline + width;
	bcel = ccb.fcelb->rc;
	bx = bcel->x;
	bgetbuf = bline;
	bwidth = width;


	if(bx)
	{
		clear_mem(bline,sizeof(Pixel)*width);
		if(bx > 0) /* smaller than dest */
		{
			bgetbuf += bx;
			bx = 0;
			bwidth = ccb.fcelb->flif.hdr.width;
		}
		else /* bigger than dest */
			bx = -bx;
	}

	for(y = 0; y < pfd->dest->height;++y)
	{
		if(y < bcel->y || y >= (bcel->y + bcel->height))
			clear_mem(bline,sizeof(Pixel)*width);
		else
			pj_get_hseg(bcel,bgetbuf,bx,y - bcel->y,bwidth);
		if((err = put_celsline(pfd,bline,0,y,width)) < Success)
			break;
	}
OUT:
	return(errend_abort_atom(err));
}

static Errcode make_trans_colors(int time_scale,Cmap *cmapa,Cmap *cmapb,
								  Cmap *dcmap)
{
	switch(ccb.cfit_type)
	{
		case FIT_TOA:
			pj_cmap_copy(cmapa,dcmap);
			break;
		case FIT_TOB:
			break;
		case FIT_BLEND:
			get_cmap_blend(time_scale, cmapa, cmapb, dcmap);
			break;
	}
	return(Success);
}

static Errcode write_transition_frames(Initpframe init_pframe,
									   Getmask get_mask_frame,
									   Closepframe cleanup_pframe)

/* note the scaling is done so that it goes from 0 to one less than frame after
 * wipe the frame before is all cel a and the frame after is all cel b and all
 * inbetween are part a and part a from 1/(count+1) count/(count+1) proportion
 * of mask generator 0 being all a and count being all b */
{
Errcode err;
SHORT frame_a, frame_b;
Cmap *cmapa, *cmapb, *dcmap;
int mask_scale0, mask_scale1, cmap_scale;
Pframedat pfd;
LONG cbufsize, size;
Celcfit blend_cfit;

	clear_struct(&pfd);
	if(ccb.preview_mode)
	{
		pfd.dest = undof;
		pfd.history = vb.pencel;
		save_undo();
	}
	else
	{
		pfd.dest = vb.pencel;
		pfd.history = undof;
	}

	init_celcfit(&blend_cfit);
	if((err = reopen_fcelio(ccb.start_cel, JREADONLY)) < Success)
		goto error;
	if((err = reopen_fcelio(ccb.end_cel, JREADONLY)) < Success)
		goto error;

	cbufsize = pj_fli_cel_cbuf_size(pfd.dest);
	if( fcel_needs_seekbuf(ccb.fcelb)
		&& (size = fcel_cbuf_size(ccb.fcelb)) > cbufsize)
	{
		cbufsize = size;
	}
	if((vs.co_type == COMP_MASK)
		&& fcel_needs_seekbuf(ccb.mask_cel)
		&& ((size = fcel_cbuf_size(ccb.mask_cel)) > cbufsize))
	{
			cbufsize = size;
	}

	if(NULL == (pfd.cbuf = pj_malloc(cbufsize)))
		goto nomem_error;

	/* the mask mover uses cel a and cel b not start_cel and end_cel
	 * uses invert flag */

	pfd.reverse_wipe = vs.co_reverse;
	pfd.acfit = ccb.fcela->cfit->ctable;
	pfd.bcfit = ccb.fcelb->cfit->ctable;
	pfd.frame_count = ccb.transition_frames;

	/** calculate and preset frame counts etc */

	frame_a = ccb.cela_transtart;
	frame_b = ccb.celb_start;
	cmapa = ccb.start_cel->rc->cmap;
	cmapb = ccb.end_cel->rc->cmap;
	dcmap = pfd.dest->cmap;

	if(init_pframe && (err = (*init_pframe)(&pfd)) < Success)
		goto error;

	for(pfd.frame = 0;pfd.frame < pfd.frame_count;++pfd.frame)
	{
		if(!ccb.preview_mode)
			save_undo();

		if((err = poll_abort()) < Success)
			goto error;

		if((err = gb_seek_fcel_frame(ccb.start_cel, frame_a,
									 pfd.cbuf, FALSE)) < Success)
		{
			goto error;
		}
		if((err = gb_seek_fcel_frame(ccb.end_cel, frame_b,
									 pfd.cbuf, FALSE)) < Success)
		{
			goto error;
		}

		if(pfd.frame_count > 1)
		{
			mask_scale0 = rscale_by(SCALE_ONE, pfd.frame, pfd.frame_count-1);
			mask_scale1 = cmap_scale =
				rscale_by(SCALE_ONE, pfd.frame + 1,pfd.frame_count+1);

			if(pfd.reverse_wipe)
			{
				mask_scale0 = SCALE_ONE - mask_scale0;
				mask_scale1 = SCALE_ONE - mask_scale1;
			}
		}
		else
		{
			mask_scale0 = mask_scale1 = cmap_scale = SCALE_ONE/2;
		}


		if((err = (*get_mask_frame)(mask_scale0,mask_scale1,&pfd)) < Success)
			goto error;

		if((err = make_trans_colors(cmap_scale,cmapa,cmapb,dcmap)) < Success)
			goto error;

		if(!ccb.preview_mode && !cmaps_same(dcmap,undof->cmap))
			see_cmap();

		/* note cfit is done in absolute order so docfit is set to absolute
		 * order (not start and end) because the mask_move_cels() does
		 * move in this order and doesn't care about start and end */

		pfd.docfit = 0;
		if(make_simple_cfit(ccb.fcela->rc->cmap,dcmap,
							ccb.fcela->cfit,ccb.clearc))
		{
			pfd.docfit = 1;
		}
		if(make_simple_cfit(ccb.fcelb->rc->cmap,dcmap,
							ccb.fcelb->cfit,ccb.clearc))
		{
			pfd.docfit |= 2;
		}

		if((err = mask_move_cels(&pfd)) < Success)
			goto error;

		if(!ccb.preview_mode)
		{
			if(flix.hdr.frame_count == 0)
				err = write_first_flxframe(NULL,&flix,pfd.cbuf,pfd.dest);
			else
			{
				err = write_next_flxframe(NULL,&flix,pfd.cbuf,
										  pfd.history,pfd.dest);
			}
			if(err < Success)
				goto error;
			dirties();
		}
		else
		{
			++ccb.preview_frame; /* for poll abort verify in preview */
			zoom_unundo();
		}

		if(!vs.co_still)
		{
			++frame_a;
			++frame_b;
		}
	}
	goto done;

nomem_error:
	err = Err_no_memory;
error:
done:
	if(cleanup_pframe)
		(*cleanup_pframe)(&pfd);
	pj_gentle_free(pfd.cbuf);
	close_fcelio(ccb.fcelb);
	close_fcelio(ccb.fcela);
	return(err);
}
static Errcode do_transition(Initpframe init_pframe,
							 Getmask get_mask_frame,
							 Closepframe close_pframe)
{
Errcode err;
Celcfit *cfit;

	cfit = NULL;
	switch(ccb.cfit_type)
	{
		case FIT_TOB:
			if((err = seek_fcel_frame(ccb.start_cel,ccb.cela_transtart)) < 0)
				goto error;
			if((err = seek_fcel_frame(ccb.end_cel, ccb.celb_start)) < Success)
				goto error;
			pj_cmap_copy(ccb.end_cel->rc->cmap,vb.pencel->cmap);
			cfit = ccb.start_cel->cfit;
			if(!make_simple_cfit(ccb.start_cel->rc->cmap,vb.pencel->cmap,
								 cfit,ccb.clearc))
			{
				cfit = NULL;
			}
			see_cmap();
			break;
	}

	if(!ccb.preview_mode)
	{
		if((err = write_celframes(ccb.start_cel, ccb.cela_start,
								  ccb.cela_frames, TRUE,FALSE,cfit)) < Success)
		{
			goto error;
		}
	}

	ccb.preview_frame += ccb.cela_frames;
	if((err = write_transition_frames(init_pframe,
									  get_mask_frame,close_pframe)) < Success)
	{
		goto error;
	}

	if(!ccb.preview_mode)
		free_fcel(&ccb.mask_cel); /* done with mask if used */

	cfit = NULL;
	switch(ccb.cfit_type)
	{
		case FIT_TOA:
			if((err = seek_fcel_frame(ccb.end_cel,ccb.celb_tailstart)) < 0)
				goto error;
			pj_cmap_copy(ccb.start_cel->rc->cmap,vb.pencel->cmap);
			cfit = ccb.end_cel->cfit;
			if(!make_simple_cfit(ccb.end_cel->rc->cmap,vb.pencel->cmap, cfit,
								 ccb.clearc))
			{
				cfit = NULL;
			}
			see_cmap();
			break;
	}

	if(!ccb.preview_mode)
	{
		free_fcel_raster(ccb.start_cel); /* done with this */

		err = write_celframes(ccb.end_cel, ccb.celb_tailstart,
							  ccb.celb_frames, FALSE,TRUE,cfit);
	}
	else
		ccb.preview_frame += ccb.celb_frames;

error:
	close_fcelio(ccb.start_cel);
	close_fcelio(ccb.end_cel);
	return(err);
}

/********* mask maker functions *********/
static Errcode init_masked_pframe(Pframedat *pfd)

/* always returns success */
{
	if(pfd->reverse_wipe)
		pfd->invert = !vs.co_b_first;
	else
		pfd->invert = vs.co_b_first;
	pfd->lbuf = pfd->cbuf;
	return(Success);
}
static Errcode make_circle_mask(int tscale0,int tscale1,Pframedat *pfd)
{
Rcel *dest = pfd->dest;
int radius;
int cenx;
int ceny;

	cenx = dest->width/2;
	ceny = dest->height/2;
	radius = (tscale1 * (5+calc_distance(0,0,cenx,ceny)))/SCALE_ONE;
	pj_set_rast(dest,pfd->invert);
	circle(dest,!pfd->invert, cenx, ceny, radius<<1, TRUE);
	return(Success);
}
static Errcode make_box_mask(int tscale0,int tscale1,Pframedat *pfd)
{
Rcel *dest = pfd->dest;
int cenx;
int ceny;
int width, height;

	cenx = dest->width/2;
	ceny = dest->height/2;
	width = ((tscale1 * cenx)+1)/SCALE_ONE;
	height = ((tscale1 * ceny)+1)/SCALE_ONE;

	pj_set_rast(dest,pfd->invert);
	pj_set_rect(dest,!pfd->invert, cenx-width, ceny-height,
			 width<<1,height<<1);
	return(Success);
}
static Errcode make_diamond_mask(int tscale0,int tscale1,Pframedat *pfd)
{
Rcel *dest = pfd->dest;
int width, height;
Short_xy dpg[4];

	dpg[0].x = dpg[2].x = dest->width/2;
	dpg[1].y = dpg[3].y = dest->height/2;

	width = ((tscale1 * dest->width))/SCALE_ONE;
	height = ((tscale1 * dest->height))/SCALE_ONE;

	dpg[0].y = dpg[1].y - height;
	dpg[2].y = dpg[1].y + height;

	dpg[3].x = dpg[0].x - width;
	dpg[1].x = dpg[0].x + width;

	pj_set_rast(dest,pfd->invert);
	return(polygon(dest,!pfd->invert,dpg,4,TRUE));
}
static Errcode make_dissolve_mask(int tscale0,int tscale1,Pframedat *pfd)
{
Errcode err;
Coor x, y, dy;
Rasthdr spec;
ULONG pcount;
ULONG cp;
Rcel *dest = pfd->dest;
Bytemap *dotmask;
UBYTE *plane;
UBYTE dotcol;

	clear_struct(&spec);
	spec.width = 320;
	spec.height = 200;
	spec.pdepth = 8;

	if((err = pj_alloc_bytemap(&spec,&dotmask)) < Success)
		return(err);

	pj_set_rast(dotmask,pfd->invert);
	plane = (UBYTE *)(dotmask->bm.bp[0]);
	dotcol = !pfd->invert;

	pcount = (tscale1 * (320*200) + SCALE_ONE/2) / SCALE_ONE;

	cp = 0;
	while(pcount-- > 0)
	{
		plane[cp] = dotcol;
		cp += (27*27);
		cp %= (320*200);
	}

	y = 0;
	for(;;)
	{
		dy = y;
		for(x = 0;x < dest->width;x += 320)
		{
			pj_blitrect(dotmask,0,0,dest,x,dy,320,200);
			--dy;
		}
		if((dy + 200) >= dest->height)
			break;
		y += 200;
	}
	pj_rast_free(dotmask);
	return(Success);
}
static Errcode make_corbox_mask(int tscale0,int tscale1, Pframedat *pfd)
{
Rcel *dest = pfd->dest;
int x,y,width,height;

	width = ((tscale1 * dest->width)+1)/SCALE_ONE;
	height = ((tscale1 * dest->height)+1)/SCALE_ONE;

	x = y = 0;
	switch(vs.co_type)
	{
		case COMP_BOXRTOP:
			x = dest->width - width;
			break;
		case COMP_BOXRBOT:
			x = dest->width - width;
		case COMP_BOXLBOT:
			y = dest->height - height;
			break;
	}

	pj_set_rast(dest,pfd->invert);
	pj_set_rect(dest,!pfd->invert, x, y, width, height);
	return(Success);
}
static Errcode make_vmask(int tscale,Pframedat *pfd, Boolean centered)
{
Rcel *dest = pfd->dest;
int width, x;

	width = ((tscale * dest->width)+1)/SCALE_ONE;
	pj_set_rast(dest,pfd->invert);
	x = 0;
	if(centered)
		x = (dest->width - width)>>1;
	pj_set_rect(dest,!pfd->invert,x,0,width, dest->height);
	return(Success);
}
static Errcode make_vert_mask(int tscale0,int tscale1,Pframedat *pfd)
{
	return(make_vmask(tscale1,pfd,0));
}
static Errcode make_vwedge_mask(int tscale0,int tscale1,Pframedat *pfd)
{
	return(make_vmask(tscale1,pfd,1));
}
static Errcode make_hmask(int tscale,Pframedat *pfd, Boolean centered)
{
Rcel *dest = pfd->dest;
Coor y,height;

	height = ((tscale * dest->height)+1)/SCALE_ONE;
	pj_set_rast(dest,pfd->invert);
	y = 0;
	if(centered)
		y = (dest->height - height)>>1;
	pj_set_rect(dest,!pfd->invert,0,y,dest->width,height);
	return(Success);
}
static Errcode make_horiz_mask(int tscale0,int tscale1,Pframedat *pfd)
{
	return(make_hmask(tscale1,pfd,0));
}
static Errcode make_hwedge_mask(int tscale0,int tscale1,Pframedat *pfd)
{
	return(make_hmask(tscale1,pfd,1));
}
static Errcode make_tdiag_mask(int tscale0,int tscale1, Pframedat *pfd)
{
Rcel *dest = pfd->dest;
Short_xy pg[3];
SHORT height, rightx;
Pixel invert;

	height = ((2*tscale1 * dest->height)+1)/SCALE_ONE;
	rightx = dest->width - 1;

	pg[0].x = rightx;
	pg[0].y = height - dest->height;
	pg[1].x = 0;
	pg[1].y = height;

	if(tscale1 < SCALE_ONE/2)
	{
		invert = pfd->invert;
		pg[2].x = pg[2].y = 0;
	}
	else
	{
		invert = !pfd->invert;
		pg[2].x = rightx;
		pg[2].y = dest->height-1;
	}
	pj_set_rast(dest,invert);
	return(polygon(dest,!invert,pg,3,TRUE));
}
static Errcode make_bdiag_mask(int tscale0,int tscale1, Pframedat *pfd)
{
Rcel *dest = pfd->dest;
Short_xy pg[3];
SHORT height, rightx;
Pixel invert;

	height = ((2*tscale1 * dest->height)+1)/SCALE_ONE;
	rightx = dest->width - 1;

	pg[0].x = 0;
	pg[0].y = dest->height - height;
	pg[1].x = rightx;
	pg[1].y = (dest->height<<1) - height;

	if(tscale1 < SCALE_ONE/2)
	{
		invert = pfd->invert;
		pg[2].x = 0;
		pg[2].y = dest->height-1;
	}
	else
	{
		invert = !pfd->invert;
		pg[2].x = rightx;
		pg[2].y = 0;
	}
	pj_set_rast(dest,invert);
	return(polygon(dest,!invert,pg,3,TRUE));
}
static Errcode make_blind_mask(int tscale, Pframedat *pfd,
							   Boolean vertical, Pixel offcol, Pixel oncol,
							   USHORT slatspace )
{
Rcel *dest = pfd->dest;
SHORT totsize;
SHORT slatstart;
USHORT slatsize;
USHORT *height, *width, *sheight, *swidth;
SHORT *xval, *yval;
static SHORT zeroval = 0;

	if(vertical)
	{
		totsize = dest->width;
		sheight = height = &dest->height;
		width = &slatsize;
		swidth = &slatspace;
		xval = &slatstart;
		yval = &zeroval;
	}
	else
	{
		totsize = dest->height;
		swidth = width = &dest->width;
		height = &slatsize;
		sheight = &slatspace;
		yval = &slatstart;
		xval = &zeroval;
	}

	slatsize = ((tscale*slatspace)+(SCALE_ONE/2))/SCALE_ONE;
	if(slatsize < 1)
		slatsize = 1;
	slatstart = ((totsize%slatspace)/-2) - slatsize/2;
	slatspace -= slatsize;

	while(slatstart < totsize)
	{
		pj_set_rect(dest,oncol,*xval,*yval,*width,*height);
		slatstart += slatsize;
		pj_set_rect(dest,offcol,*xval,*yval,*swidth,*sheight);
		slatstart += slatspace;
	}
	return(Success);
}
static Errcode make_venetian_mask(int tscale0, int tscale1, Pframedat *pfd)
{
	return(make_blind_mask(tscale1, pfd, 0, pfd->invert, !pfd->invert,
						   vs.co_venetian_height));
}
static Errcode make_louver_mask(int tscale0, int tscale1, Pframedat *pfd)
{
	return(make_blind_mask(tscale1, pfd, 1, pfd->invert, !pfd->invert,
						   vs.co_louver_width));
}
Errcode draw_slatmask(Boolean *pvertical,USHORT size)
{
Pframedat pfd;

	clear_struct(&pfd);
	pfd.dest = vb.pencel;
	make_blind_mask(SCALE_ONE/2, &pfd, *pvertical,sgrey,swhite,size);
	return(Success);
}
/***** boxilate transition *****/

static Errcode init_boxil_mask(Pframedat *pfd)
{
Errcode err;
Rasthdr spec;

	srandom(1);
	pfd->reverse_wipe = FALSE; /* this doesn't make sense here it's random */
	init_masked_pframe(pfd);

	/* verify vs data */

	if(vs.co_boxil_width <= 0)
		vs.co_boxil_width = 1;
	if(vs.co_boxil_height <= 0)
		vs.co_boxil_height = 1;

	copy_rasthdr(vb.pencel,&spec);
	spec.width = (vb.pencel->width + vs.co_boxil_width - 1)
						/ vs.co_boxil_width;
	spec.height = (vb.pencel->height + vs.co_boxil_height - 1)
						/ vs.co_boxil_height;
	spec.pdepth = 1;

	if((err = pj_alloc_bitmap(&spec,&(pfd->boxil_mask))) >= Success)
		pj_set_rast(pfd->boxil_mask,pfd->invert?1:0);

	pfd->boxils_done = 0;
	return(err);
}
static void cleanup_boxil_mask(Pframedat *pfd)
{
	pj_rast_free(pfd->boxil_mask);
}
void zoom_boxil_mask(Raster *boxil_mask,Raster *dest,
							Coor hsize, Coor vsize)
{
Clipbox cb;
Coor x,y,width,height;

	if(hsize <= 0)
		hsize = 1;
	if(vsize <= 0)
		hsize = 1;

	x = 0;
	width = dest->width;
	if(width % hsize)
	{
		width = ((width/hsize) + 1) * hsize;
		x = width - dest->width;
	}

	y = 0;
	height = dest->height;
	if(height % vsize)
	{
		height = ((height/vsize) + 1) * vsize;
		y = height - dest->height;
	}

	if(x || y)
	{
		pj_clipbox_make(&cb,dest,x/-2,y/-2,width,height);
		dest = (Raster *)&cb;
	}
	if(vsize != 1 || hsize != 1)
	{
		pj_zoomblit(boxil_mask,0,0,dest,0,0,width,height,hsize,vsize);
	}
	else
	{
		pj_blitrect(boxil_mask,0,0,dest,0,0,width,height);
	}
}
static Errcode get_boxil_mask(int tscale0,int tscale1, Pframedat *pfd)
{
ULONG boxils_needed;
Coor x, y;
Ucoor width, height;
extern int random();

	width = (pfd->boxil_mask->width);
	height = (pfd->boxil_mask->height);

	boxils_needed = (ULONG)( ((FLOAT)(width*height)) *
							((FLOAT)tscale1)/((FLOAT)SCALE_ONE));

	while(pfd->boxils_done < boxils_needed)
	{
		x = ((ULONG)random())%width;
		y = ((ULONG)random())%height;

		if(pfd->invert)
		{
			if(!pj__get_dot(pfd->boxil_mask,x,y))
				continue;
			pj__put_dot(pfd->boxil_mask,0,x,y);
		}
		else
		{
			if(pj__get_dot(pfd->boxil_mask,x,y))
				continue;
			pj__put_dot(pfd->boxil_mask,1,x,y);
		}
		++pfd->boxils_done;
	}

	zoom_boxil_mask((Raster *)(pfd->boxil_mask),(Raster *)(pfd->dest),
					vs.co_boxil_width, vs.co_boxil_height );

	return(Success);
}

/***** main composit function *****/

/**************** key mask transition **************/

static Errcode init_flicel_mask(Pframedat *pfd)
{
	init_masked_pframe(pfd);
	return(reopen_fcelio(ccb.mask_cel,JREADONLY));
}
static void cleanup_flicel_mask(Pframedat *pfd)
{
	close_fcelio(ccb.mask_cel);
}
static Errcode get_flicel_mask(int tscale0,int tscale1, Pframedat *pfd)
{
Errcode err;
int frame;
Celcfit *cfit;

	/* although math is ok to 300 or so insure 1 to 1 case */

	frame = ccb.mask_cel->flif.hdr.frame_count;
	if(pfd->frame_count == frame)
	{
		if(pfd->reverse_wipe)
			frame = pfd->frame-(1+frame);
		else
			frame = pfd->frame;
	}
	else
		frame = ((tscale0*(frame-1))+SCALE_ONE/2)/SCALE_ONE;

	if((err = gb_seek_fcel_frame(ccb.mask_cel, frame, pfd->cbuf, FALSE)) < 0)
		goto error;
	cfit = ccb.mask_cel->cfit;
	pj_stuff_bytes(!pfd->invert,cfit->ctable,sizeof(cfit->ctable));
	cfit->ctable[0] = pfd->invert;
	err = copy_fcel(ccb.mask_cel, pfd->dest, cfit, -2);
error:
	return(err);
}

/******* simple cut function ******/

static Errcode do_cut()
/* no cfitting, no transition frames. */
{
Errcode err;

	if((err = write_celframes(ccb.start_cel, ccb.cela_start, ccb.cela_frames,
							  FALSE,FALSE,NULL)) < Success)
	{
		goto error;
	}
	err = write_celframes(ccb.end_cel, ccb.celb_start, ccb.celb_frames,
						  TRUE,TRUE,NULL);
error:
	return(err);
}
/***** main composit function *****/

static Boolean comp_abort_verify()
{
	if(ccb.preview_mode)
	{
		if(vs.co_type == COMP_CUT)
			ccb.preview_frame = flix.hdr.frame_count+1;

		return(soft_yes_no_box("!%d%d", "join_pabort",
						  ccb.preview_frame,
						  ccb.total_frames));
	}
	return(soft_yes_no_box("!%d%d", "join_abort",
							flix.hdr.frame_count+1,
							ccb.total_frames ));
}
Errcode render_composite(Boolean do_preview)
/*****************************************************************************
 * Main entry point to this module.   Fill in the initial fields of ccb
 * (see comment on struct compo_cb in composit.h) before calling this.
 ****************************************************************************/
{
Errcode err;
SHORT flx_ix, exit_flx_ix;
Initpframe init_pframe = NULL;
Getmask get_mask = NULL;
Closepframe close_pframe = NULL;

	ccb.preview_mode = do_preview;
	ccb.preview_frame = 0;

	if(vs.co_type == COMP_MASK)
	{
		if(reload_mask_cel() < Success)
			return(Err_abort);
		scale_fcel_to_screen(ccb.mask_cel,vb.pencel);
	}
	else
		free_fcel(&ccb.mask_cel);

	set_abort_verify(comp_abort_verify);

	exit_flx_ix = flx_ix = vs.frame_ix;

	if((err = sv_fli(ram_tflx_name)) < Success)
		goto save_error;
	close_tflx();

	free_fcel(&ccb.fcela);
	if((err = load_fli_fcel(ram_tflx_name,NULL,NULL,&ccb.fcela)) < Success)
		goto error;

/* scale cels to screen and center them */

	clear_fcel_xform(ccb.fcela);
	clear_fcel_xform(ccb.fcelb);
	if(vs.co_matchsize)
		scale_fcel_to_screen(ccb.fcelb,vb.pencel);

	center_fcel_in_screen(ccb.fcela,vb.pencel);
	center_fcel_in_screen(ccb.fcelb,vb.pencel);

/*** set start and end and pre-load frame count and index values */

	ccb.clearc = -1; /* for now ignore color 0 */

	if(vs.co_b_first)
	{
		ccb.start_cel = ccb.fcelb;
		ccb.end_cel = ccb.fcela;
		ccb.cfit_type = INVERT_CFITMODE(vs.co_cfit);
	}
	else
	{
		ccb.start_cel = ccb.fcela;
		ccb.end_cel = ccb.fcelb;
		ccb.cfit_type = vs.co_cfit;
	}

	ccb.cela_start = 0;
	ccb.cela_frames = ccb.start_cel->flif.hdr.frame_count;
	ccb.cela_transtart = ccb.cela_start + ccb.cela_frames;

	ccb.celb_tailstart = ccb.celb_start = 0;
	ccb.celb_frames = ccb.end_cel->flif.hdr.frame_count;

	if(vs.co_still)
	{
		ccb.cela_transtart -= 1;
	}
	else
	{
		ccb.cela_frames -= ccb.transition_frames;
		ccb.cela_transtart -= ccb.transition_frames;
		ccb.celb_frames -= ccb.transition_frames;
		ccb.celb_tailstart += ccb.transition_frames;
	}

	ccb.total_frames =
		ccb.cela_frames + ccb.celb_frames + ccb.transition_frames;

	/* open new tempflx for new total frames */

	if((err = empty_tempflx(ccb.total_frames)) < Success)
		goto error;

	flix.hdr.speed = ccb.fcela->flif.hdr.speed; /* preserve speed */
	flix.hdr.frame_count = 0;  /* this one is REALLY empty */

	/* set finish up index to be same position on initial fli */

	if(vs.co_b_first)
	{
		flx_ix += ccb.cela_frames;
		if(vs.co_still)
			flx_ix += ccb.transition_frames;
		else
			flx_ix -= ccb.transition_frames;
	}

	err = Err_unimpl; /* anything not hit by switch */

	init_pframe = init_masked_pframe;
	switch(vs.co_type)
	{
		case COMP_CUT:
			err = do_cut();
			break;
		case COMP_CIRCLE:
			get_mask = make_circle_mask;
			goto call_wipe;
		case COMP_HWEDGE:
			get_mask = make_hwedge_mask;
			goto call_wipe;
		case COMP_VERTW:
			get_mask = make_vert_mask;
			goto call_wipe;
		case COMP_BOX:
			get_mask = make_box_mask;
			goto call_wipe;
		case COMP_VWEDGE:
			get_mask = make_vwedge_mask;
			goto call_wipe;
		case COMP_HORIZW:
			get_mask = make_horiz_mask;
			goto call_wipe;
		case COMP_BOXLTOP:
		case COMP_BOXRTOP:
		case COMP_BOXRBOT:
		case COMP_BOXLBOT:
			get_mask = make_corbox_mask;
			goto call_wipe;
		case COMP_VENETIAN:
			get_mask = make_venetian_mask;
			goto call_wipe;
		case COMP_LOUVER:
			get_mask = make_louver_mask;
			goto call_wipe;
		case COMP_DIAGTOP:
			get_mask = make_tdiag_mask;
			goto call_wipe;
		case COMP_DIAGBOT:
			get_mask = make_bdiag_mask;
			goto call_wipe;
		case COMP_MASK:
			init_pframe = init_flicel_mask;
			get_mask = get_flicel_mask;
			close_pframe = cleanup_flicel_mask;
			goto call_wipe;
		case COMP_DISSOLVE:
			get_mask = make_dissolve_mask;
			goto call_wipe;
		case COMP_BOXIL:
			init_pframe = init_boxil_mask;
			get_mask = get_boxil_mask;
			close_pframe = cleanup_boxil_mask;
			goto call_wipe;
		case COMP_DIAMOND:
			get_mask = make_diamond_mask;
			goto call_wipe;
call_wipe:
			err = do_transition(init_pframe, get_mask, close_pframe);
			break;
		default:
			err = Err_bad_input;
			break;
	}

	if(err < Success)
		goto error;
	if(ccb.preview_mode)
	{
		err = Err_abort;
		goto error;
	}

	exit_flx_ix = flx_ix;
	free_fcel(&ccb.fcelb); /* done with this now */
	dirties();

error:

	free_fcel(&ccb.fcela); /* cel a was only temporary */

	if( (err >= Success)
		 || (make_tempflx(ram_tflx_name,0) < Success))
	{
		clear_flipath(ccb.tflxpath);
	}

	/* if all done and new tempflx is ok or
	 * we re-loaded tempflx from saved temp file seek to previous
	 * position */

	exit_flx_ix = wrap_frame(exit_flx_ix);
	vs.bframe_ix = 0; /* no no no */
	fli_abs_tseek(undof,exit_flx_ix);
	if(err < Success)
		pj_delete(bscreen_name);
	vs.frame_ix = exit_flx_ix;

	update_flx_path(&flix,&ccb.tflxpath->fid,ccb.tflxpath->path);

	if(ccb.fcelb && ccb.fcelb->rc == NULL) /* will be freed if success */
	{
		if(alloc_fcel_raster(ccb.fcelb) < Success)
			free_fcel(&ccb.fcelb);
	}

save_error:
	err = softerr(err,"join_flis");
	set_abort_verify(NULL);
	pj_delete(ram_tflx_name);
	zoom_unundo();
	return(err);
}
