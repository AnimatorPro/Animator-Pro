#ifndef CONVERT_H
#define CONVERT_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef AACONFIG_H
	#include "aaconfig.h"
#endif

#ifndef PJBASICS_H
	#include "pjbasics.h"
#endif

#ifndef ANIMINFO_H
	#include "animinfo.h"
#endif

#ifndef PICDRIVE_H
	#include "picdrive.h"
#endif

#ifndef ERRCODES_H
	#include "errcodes.h"
#endif

#ifndef MEMORY_H
	#include "memory.h"
#endif

enum main_codes {
	MRET_QUIT = 0,
	MRET_RESTART,
	MRET_RESIZE_SCREEN,
};


enum rgb_load_options {
	RGB_GREY = 0,
	RGB_COLOR,
	RGB_SCALED,
	RGB_DITHER,
	};

void freez_cel(Rcel **pcel);

typedef struct
	{
	Pdr *pdr;
	Rcel *cel;
	Anim_info ai;
	Image_file *ifi;
	SHORT frame_ix;
	} Ifi_state;

typedef struct conv_path {
	SHORT scroller_top;       /* top name of scroller */
	char path[PATH_SIZE];     /* actual path name */
	char wildcard[WILD_SIZE]; /* wild card for scroller menu */
} Conv_path;


typedef struct conv_state
	{
	Ifi_state ifi;
	char pdr_name[PATH_SIZE];
	char in_name[PATH_SIZE];
	Conv_path in, out;
	SHORT scalew, scaleh;
	int   rgb_loadoption;
	UBYTE is_rgbinput;
	UBYTE do_dither;
	UBYTE no_tile;
	UBYTE colors_256;
	UBYTE recalc_colors;
	UBYTE slide_complete;
	SHORT slidex,slidey;
	SHORT slide_frames;
	UBYTE pad[32];
	} Conv_state;
extern Conv_state cs;

void status_line(char *fmt,...);
Errcode soft_abort(char *soft_key);
char *conv_save_name(char *header, char *suff, char *button);
Errcode conv_set_pencel(SHORT width, SHORT height);
Errcode softerr(Errcode err,char *key,...);
void conv_see_cel(Rcel *cel);
void conv_center_cel(Rcel *cel);
void conv_update_cmap(Cmap *cmap);
void grey_cmap(Cmap *cmap);
Errcode load_other();
Errcode get_a_flic(char *pdr_name, char *name, char *suff);
Errcode save_a_pic(char *pdr_name);
Errcode conv_seek(int ix, void *data);
Errcode ifi_cel_seek(int ix, void *data);
Errcode save_a_flic(char *pdr_name, char *name, int frames,
					Errcode (*seek)(int ix, void *data));
Errcode get_new_pdr(Pdr **ppdr, char *pdr_name);
void qscale_menu();
void conv_move();
void qconv_slide();
void view_flic();
void view_pic();

Errcode conv_pdropt_qchoice(Pdroptions *popt);

Errcode convrgb_read_image(Image_file *ifile, Rcel *screen, Anim_info *ai, int rgb_choice);
Errcode convrgb_qoptions(void);

extern Pdr targa_pdr;
extern char targa_pdr_name[];
extern Pdr fli_pdr;
extern char fli_pdr_name[];
extern char flilores_pdr_name[];
extern Pdr tiff_pdr;
extern char tiff_pdr_name[];

#endif /* CONVERT_H Leave at end of file */
