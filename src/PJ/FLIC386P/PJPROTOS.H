/*****************************************************************************
 * PJPROTOS.H - Function prototypes for the client<->FlicLib interfaces.
 *
 *	This file contains prototypes for all the most commonly-used functions.
 *	Additional prototypes for less-commonly used functions are in:
 *
 *		PJCUSTOM.H	- For working with custom rasters.
 *		PJGFX.H 	- For using the graphics library to draw on rasters.
 *
 ****************************************************************************/

#ifndef PJPROTOS_H
#define PJPROTOS_H

#ifndef PJLTYPES_H
	#include "pjltypes.h"
#endif

/*----------------------------------------------------------------------------
 * clock-related functions...
 *--------------------------------------------------------------------------*/

Boolean 		pj_clock_init(void);
void			pj_clock_cleanup(void);
unsigned long	pj_clock_1000(void);
unsigned long	pj_clock_jiffies(void);
unsigned long	pj_clock_jiffies2ms(unsigned long jiffies);
unsigned long	pj_clock_ms2jiffies(unsigned long milliseconds);

/*----------------------------------------------------------------------------
 * general-purpose utility functions...
 *--------------------------------------------------------------------------*/

Boolean 	pj_key_is(void);	/* has key been hit? */
int 		pj_key_in(void);	/* get key input (will wait) */

void		*pj_malloc(size_t amount);
void		*pj_zalloc(size_t amount);
void		pj_free(void *pblock);
void		pj_freez(void *ppblock);

void		pj_doserr_install_handler(void);
void		pj_doserr_remove_handler(void);

char		*pj_error_get_message(Errcode err);
Errcode 	pj_error_internal(Errcode err, char *module_name, int line_number);

/*----------------------------------------------------------------------------
 * video-related data and functions...
 *--------------------------------------------------------------------------*/

extern LocalVdevice *pj_vdev_supervga;
extern LocalVdevice *pj_vdev_vesa;
extern LocalVdevice *pj_vdev_8514;
extern LocalVdevice *pj_vdev_mcga;

void		pj_video_add(LocalVdevice *pldev);
void		pj_video_add_all(void);
Errcode 	pj_video_detect(LocalVdevice **ppldev);
void		pj_video_close(FlicRaster **prast);
Errcode 	pj_video_open(LocalVdevice *pldev, int mode, FlicRaster **pprast);
Errcode 	pj_video_find_open(int width, int height, FlicRaster **pprast);
Errcode 	pj_video_find(LocalVdevice **ppldev, int *pmode,
						int width, int height);
Errcode 	pj_video_mode_info(LocalVdevice *pldev, int mode,
						int *pwidth, int *pheight);
Errcode 	pj_video_get_current(LocalVdevice **ppdev, int *pmode,
						FlicRaster **pprast);

/*----------------------------------------------------------------------------
 * Raster functions...
 *	 (functions related to custom raster types are in pjcustom.h)
 *--------------------------------------------------------------------------*/

Errcode 	pj_raster_make_ram(FlicRaster **pprast, int width, int height);
Errcode 	pj_raster_free_ram(FlicRaster **pprast);

Errcode 	pj_raster_bind_ram(FlicRaster **pprast,
							int width, int height, Pixel *pbuf);
Errcode 	pj_raster_unbind_ram(FlicRaster **pprast);

Errcode 	pj_raster_make_centered(FlicRaster **pprast, FlicRaster *proot,
							Flic *pflic);
Errcode 	pj_raster_free_centered(FlicRaster **pprast);

Errcode 	pj_raster_make_offset(FlicRaster **pprast, FlicRaster *proot,
							Flic *pflic, int x, int y);
Errcode 	pj_raster_free_offset(FlicRaster **pprast);

Errcode 	pj_raster_copy(FlicRaster *psource, FlicRaster *pdest);
Errcode 	pj_raster_clear(FlicRaster *prast);

/*----------------------------------------------------------------------------
 * color map functions...
 *--------------------------------------------------------------------------*/

Errcode 	pj_col_load(char *path, PjCmap *cmap);
Errcode 	pj_col_save(char *path, PjCmap *cmap);

Errcode 	pj_cmap_update(FlicRaster *prast, PjCmap *cmap);

/*----------------------------------------------------------------------------
 * flic read/play functions...
 *--------------------------------------------------------------------------*/

Errcode 	pj_playoptions_init(FlicPlayOptions *poptions);

Errcode 	pj_flic_play(char *filename, FlicPlayOptions *options);
Errcode 	pj_flic_play_once(char *filename, FlicPlayOptions *options);
Errcode 	pj_flic_play_timed(char *filename, FlicPlayOptions *options,
							unsigned long for_milliseconds);
Errcode 	pj_flic_play_until(char *filename, FlicPlayOptions *options,
							UserEventFunc *puserfunc, void *userdata);

Errcode 	pj_flic_play_next(Flic *pflic, FlicRaster *display_raster);
Errcode 	pj_flic_play_frames(Flic *pflic, FlicRaster *display_raster,
							int count);
struct fli_frame;	/* This is declared in pjfli.h */
void 		pj_fli_uncomp_frame(FlicRaster *screen,  struct fli_frame *frame,  
							int colors);	

Errcode 	pj_flic_close(Flic *pflic);
Errcode 	pj_flic_open(char *filename, Flic *pflic);
Errcode 	pj_flic_open_info(char *filename, Flic *pflic, AnimInfo *pinfo);
Errcode 	pj_flic_file_info(char *filename, AnimInfo *pinfo);
Errcode 	pj_flic_info(Flic *pflic, AnimInfo *pinfo);

Errcode 	pj_flic_rewind(Flic *pflic);

Errcode 	pj_flic_set_speed(Flic *pflic, int speed);

/*----------------------------------------------------------------------------
 * flic creation functions...
 *--------------------------------------------------------------------------*/

Errcode 	pj_animinfo_init(AnimInfo *pinfo);

Errcode 	pj_flic_complete_filename(char *filename, AnimInfo *pinfo,
							Boolean force_type);

Errcode 	pj_flic_create(char *filename, Flic *pflic, AnimInfo *pinfo);
Errcode 	pj_flic_write_first(Flic *pflic, FlicRaster *firstframe);
Errcode 	pj_flic_write_next(Flic *pflic, FlicRaster *thisframe,
							FlicRaster *priorframe);
Errcode 	pj_flic_write_finish(Flic *pflic, FlicRaster *lastframe);

/*----------------------------------------------------------------------------
 * these pragmas allow -3r clients to use our -3s style functions...
 *	 the FLICLIB3S alias is defined in PJSTYPES.H.
 *--------------------------------------------------------------------------*/

#ifdef __WATCOMC__

	#pragma aux pj_vdev_supervga "*";   /* these are variables, not         */
	#pragma aux pj_vdev_vesa	 "*";   /* functions, but we have to tell   */
	#pragma aux pj_vdev_8514	 "*";   /* watcom not to prepend an underbar*/
	#pragma aux pj_vdev_mcga	 "*";   /* to their names in -3r mode.      */

	#pragma aux (FLICLIB3S) pj_clock_init;
	#pragma aux (FLICLIB3S) pj_clock_cleanup;
	#pragma aux (FLICLIB3S) pj_clock_1000;
	#pragma aux (FLICLIB3S) pj_clock_jiffies;
	#pragma aux (FLICLIB3S) pj_clock_jiffies2ms;
	#pragma aux (FLICLIB3S) pj_clock_ms2jiffies;
	#pragma aux (FLICLIB3S) pj_key_is;
	#pragma aux (FLICLIB3S) pj_key_in;
	#pragma aux (FLICLIB3S) pj_malloc;
	#pragma aux (FLICLIB3S) pj_zalloc;
	#pragma aux (FLICLIB3S) pj_free;
	#pragma aux (FLICLIB3S) pj_freez;
	#pragma aux (FLICLIB3S) pj_doserr_install_handler;
	#pragma aux (FLICLIB3S) pj_doserr_remove_handler;
	#pragma aux (FLICLIB3S) pj_error_get_message;
	#pragma aux (FLICLIB3S) pj_error_internal;
	#pragma aux (FLICLIB3S) pj_video_add;
	#pragma aux (FLICLIB3S) pj_video_add_all;
	#pragma aux (FLICLIB3S) pj_video_detect;
	#pragma aux (FLICLIB3S) pj_video_close;
	#pragma aux (FLICLIB3S) pj_video_open;
	#pragma aux (FLICLIB3S) pj_video_find_open;
	#pragma aux (FLICLIB3S) pj_video_find;
	#pragma aux (FLICLIB3S) pj_video_mode_info;
	#pragma aux (FLICLIB3S) pj_video_get_current;
	#pragma aux (FLICLIB3S) pj_raster_make_ram;
	#pragma aux (FLICLIB3S) pj_raster_free_ram;
	#pragma aux (FLICLIB3S) pj_raster_bind_ram;
	#pragma aux (FLICLIB3S) pj_raster_unbind_ram;
	#pragma aux (FLICLIB3S) pj_raster_make_centered;
	#pragma aux (FLICLIB3S) pj_raster_free_centered;
	#pragma aux (FLICLIB3S) pj_raster_make_offset;
	#pragma aux (FLICLIB3S) pj_raster_free_offset;
	#pragma aux (FLICLIB3S) pj_raster_copy;
	#pragma aux (FLICLIB3S) pj_raster_clear;
	#pragma aux (FLICLIB3S) pj_col_load;
	#pragma aux (FLICLIB3S) pj_col_save;
	#pragma aux (FLICLIB3S) pj_cmap_update;
	#pragma aux (FLICLIB3S) pj_playoptions_init;
	#pragma aux (FLICLIB3S) pj_flic_play;
	#pragma aux (FLICLIB3S) pj_flic_play_once;
	#pragma aux (FLICLIB3S) pj_flic_play_timed;
	#pragma aux (FLICLIB3S) pj_flic_play_until;
	#pragma aux (FLICLIB3S) pj_flic_play_next;
	#pragma aux (FLICLIB3S) pj_flic_play_frames;
	#pragma aux (FLICLIB3S) pj_fli_uncomp_frame;
	#pragma aux (FLICLIB3S) pj_flic_close;
	#pragma aux (FLICLIB3S) pj_flic_open;
	#pragma aux (FLICLIB3S) pj_flic_open_info;
	#pragma aux (FLICLIB3S) pj_flic_file_info;
	#pragma aux (FLICLIB3S) pj_flic_info;
	#pragma aux (FLICLIB3S) pj_flic_rewind;
	#pragma aux (FLICLIB3S) pj_flic_set_speed;
	#pragma aux (FLICLIB3S) pj_animinfo_init;
	#pragma aux (FLICLIB3S) pj_flic_complete_filename;
	#pragma aux (FLICLIB3S) pj_flic_create;
	#pragma aux (FLICLIB3S) pj_flic_write_first;
	#pragma aux (FLICLIB3S) pj_flic_write_next;
	#pragma aux (FLICLIB3S) pj_flic_write_finish;

#endif /* __WATCOMC__ */

#endif /* PJPROTOS_H */
