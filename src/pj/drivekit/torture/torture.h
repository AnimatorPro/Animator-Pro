
#define VDEV_INTERNALS		/* need full contents of vdev*.h files */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errcodes.h"
#include "vdevice.h"
#include "vdevinfo.h"
#include "rastlib.h"
#include "rastcall.h"
#include "raster.h"

#define CLK_TCK (1000)
#define time_it(exp)	{\
						tcb.start_time = pj_clock_1000();\
						exp;\
						tcb.elapsed_time += pj_clock_1000() - tcb.start_time;\
						}
#define time_start()	tcb.start_time = pj_clock_1000()
#define time_end()		tcb.elapsed_time += pj_clock_1000() - tcb.start_time

#define is_generic(lib, func) ((lib)->func == tcb.vd->grclib->func)

#define WPAT 60
#define HPAT 40
#define XPAT 65
#define YPAT 10

typedef struct flic_list {
	struct flic_list
			*next;				// these filled in by get_flic_list...
	char	name[128];
	int 	width;
	int 	height;
	int 	xoffset;
	int 	yoffset;
	void	*in_memory;    // these filled in by load_flic_file...
	void	*frame1_pointer;
	void	*frame2_pointer;
	int 	num_frames;
	int 	type;
	} FlicList;

typedef struct torture_cb {
	short		test_vmode;
	FILE		*datafile;
	FILE		*logfile;
	long		start_time;
	long		elapsed_time;
	void		(*safe_clear_screen)(void);
	Vdevice 	*vd;
	Raster		bytemap_raster;
	Raster		verification_raster;
	Raster		display_raster;
	Raster		offscrn_raster;
	Boolean 	test_via_generics;
	Boolean 	single_step_mode;
	Boolean 	exercise_error_handling;
	Boolean 	timing_only_run;
	Boolean 	got_error;
	Boolean 	got_warning;
	Boolean 	got_stop;
	Boolean 	mode_has_changed;
	Coor		error_x;
	Coor		error_y;
	Pixel		error_expected;
	Pixel		error_found;
	char		*list_file_name;
	FlicList	*fliclist;
	void		*playback_verify_buffer;
	} Tcb;

extern Tcb		tcb;

typedef void	(*Pdotfunc_t)	(Raster *r, Pixel c, Coor x, Coor y);
typedef Pixel	(*Gdotfunc_t)	(Raster *r, Coor x, Coor y);
typedef void	(*Linefunc_t)	(Raster *r, Pixel c, Coor x, Coor y, Ucoor wh);
typedef void	(*Segfunc_t)	(Raster *r, void *pixbuf, Ucoor x, Ucoor y, Ucoor wh);
typedef void	(*Rpixfunc_t)	(Raster *r, void *pixbuf, Coor x, Coor y, Ucoor w, Ucoor h);
typedef void	(*Sxrectfunc_t) (Raster *r, Pixel c, Coor x, Coor y, Ucoor w, Ucoor h);
typedef void	(*Srastfunc_t)	(Raster *r, Pixel c);
typedef void	(*M1blitfunc_t) (UBYTE *mbytes, Coor mbpr, Coor mx, Coor my,
								 Raster *r, Coor rx, Coor ry, Ucoor w, Ucoor h,
								 Pixel oncolor);
typedef void	(*M2blitfunc_t) (UBYTE *mbytes, Coor mbpr, Coor mx, Coor my,
								 Raster *r, Coor rx, Coor ry, Ucoor w, Ucoor h,
								 Pixel oncolor, Pixel offcolor);
typedef void	(*Bltswpfunc_t) (Raster *ra, Coor ax, Coor ay,
								 Raster *rb, Coor bx, Coor by,
								 Coor w, Coor h);
typedef void	(*Tblitfunc_t)	(Raster *ra, Coor ax, Coor ay,
								 Raster *rb, Coor bx, Coor by,
								 Coor w, Coor h, Pixel tcolor);
typedef void	(*Xorastfunc_t) (Raster *s, Raster *d);
typedef void	(*Zoomfunc_t)	(Raster *ra, Coor ax, Coor ay,
								 Raster *rb, Coor bx, Coor by,
								 Ucoor w, Ucoor h, LONG zoom_x, LONG zoom_y);
typedef void	(*Colorfunc_t)	(Raster *r, LONG start, LONG count, void *cbuf);
typedef void	(*Vsyncfunc_t)	(Raster *r);

#define RASTER_PRIMARY	0
#define RASTER0 		0
#define RASTER1 		1
#define RASTER2 		2
#define RASTER3 		3

/* in libraries */

extern void 	pj_set_vmode(int mode);
extern int		pj_get_vmode(void);

extern Errcode	pj_open_ddriver(Vdevice **vd, char *dvrname);
extern void 	pj_close_vdriver(Vdevice **vd);

extern Boolean	pj_clock_init(void);
extern void 	pj_clock_cleanup(void);
extern ULONG	pj_clock_1000(void);

extern void 	pj_set_gs(void);

extern int		pj_key_in(void);
extern int		pj_key_is(void);

extern short	old_vmode;				/* functions in pjstubs.c */
extern void 	old_video(void);

/* in torture.c */

void log_error(char *txt, ...);
void log_verror(int x, int y, int found, int expected);
void log_data(char *txt, ...);
void log_warning(char *txt, ...);
void log_progress(char *txt, ...);
void log_start(char *txt, ...);
void log_end(char *txt, ...);
void log_bypass(char *txt);
void clear_screen(void);
Boolean single_step(void);

/* in chkdev.c */

void	check_device_sanity(void);
void	test_device_modeinfo(void);
void	test_device_open(void);
void	test_device_close(void);
void	test_device_cels(void);

/* in utlrast.c */

void	init_raster(Raster *r);
void	init_bytemap_raster(Raster *r, Vdevice *vd, Vmode_info *vm, PLANEPTR pptr);
void	make_ripple(Raster *r,
					Ucoor width, Ucoor height,
					Coor xstart, Coor ystart,
					Pixel color_start,
					short color_incr);
Boolean verify_ripple(Raster *r,
					Ucoor width, Ucoor height,
					Coor xstart, Coor ystart,
					Pixel color_start,
					short color_incr);
void	draw_blitpattern(Raster *r, short fgcolor, short bgcolor);
Boolean verify_raster(Raster *sr, Raster *vr, Boolean do_logging);

/* in chkrast1.c */

void	check_raster_sanity(Raster *r, USHORT rastnum);
void	test_close_raster(Raster *r);

/* in chkr*.c */

void	dot_clearscreen(void);		/* in chkrdots.c */
void	test_dots(Raster *r);		/* in chkrdots.c */

void	test_segs(Raster *r);		/* in chkrsegs.c */

void	hline_clearscreen(void);	/* in chkrline.c */
void	test_lines(Raster *r);		/* in chkrline.c */

void	test_rectpix(Raster *r);	/* in chkrrect.c */
void	test_set_rect(Raster *r);	/* in chkrrect.c */
void	test_set_rast(Raster *r);	/* in chkrrect.c */
void	test_xor_rect(Raster *r);	/* in chkrrect.c */

void	test_mask1blit(Raster *r);	/* in chkrmask.c */
void	test_mask2blit(Raster *r);	/* in chkrmask.c */

void	test_rastblits(Raster *r);

void	test_rastswaps(Raster *r);

void	test_rasttblits(Raster *r);

void	test_rastxors(Raster *r);

void	test_rastzooms(Raster *r);

void	test_colors(Raster *r); 			/* in chkrcolr.c */
void	set_default_colors(Raster *r);		/* in chkrcolr.c */

void	test_playback(Raster *r);			/* in chkrplay.c */

struct anim_info;

Errcode get_flic_info(char *name, struct anim_info *pinf);	/* in flicinfo.c */

Errcode get_flic_names(void);				/* in getflist.c */
void	free_flic_names(void);
