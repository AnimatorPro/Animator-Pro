#ifndef PICFILE_H
#define PICFILE_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef JFILE_H
	#include "jfile.h"
#endif

#ifndef RCEL_H
	#include "rcel.h"
#endif

#ifndef VMAGICS_H
	#include "vmagics.h"
#endif

struct anim_info;
struct cmap;

/* Old style pic header, Animator 1.0 compatible */
typedef struct GCC_PACKED opic_header {
	USHORT type;  	/* type == OPIC_MAGIC */
	RECT_FIELDS; 	/* width, height, x, y */
	char d;  		/* pixel depth */
	char compress; 	/* unused */
	LONG csize;    	/* w * h */
	char reserved[16];
} Opic_header;
STATIC_ASSERT(picfile, sizeof(Opic_header) == 32);

typedef struct GCC_PACKED pic_header {
	Chunk_id id;      	/* type is PIC_MAGIC */
	RECT_FIELDS;   	  	/* width; height; x; y; size and positioning info */
	LONG user_id;      	/* arbitrary id number for user */
	UBYTE depth;		/* pixel depth needed to represent pic */
	UBYTE reserved[45]; /* for a total of 64 bytes */
} Pic_header;
STATIC_ASSERT(picfile, sizeof(Pic_header) == 64);

/* chunk types after header */

enum pic_chunks {
	PIC_CMAP = 0,		/* palette chunk 0-255 Fat_chunk + rgb bytes */
	PIC_BYTEPIXELS = 1,	/* uncompressed pixels in Bytemap format */
	PIC_BITPIXELS = 2,	/* uncompressed pixels in Bitmap format */
};

/* "pic" file io calls */

Errcode pic_anim_info(char *ifname, struct anim_info *ainfo);
Errcode pj_read_pichead(Jfile f,Pic_header *pic);

extern Errcode
pj_read_picbody(Jfile f, Pic_header *pic, Raster *cel, struct cmap *cmap);

Errcode load_pic(char *name,Rcel *rcel,LONG check_id, Boolean load_colors);
Errcode save_pic(char *name,Rcel *screen,LONG id, Boolean save_colors);

/* picfiles.c */
extern Boolean is_fli_pdr_name(char *path);
extern char *get_flisave_pdr(char *pdr_path);
extern Errcode get_picsave_info(char *sufbuf, char *titlebuf, int titlesize);
extern Errcode get_flisave_info(char *sufbuf, char *titlebuf, int titlesize);
extern char *get_pictype_suffi(void);
extern char *get_fliload_suffi(char *sufbuf);
extern char *get_celload_suffi(char *sufbuf);
extern void go_pic_pdr_menu(void);
extern void go_flic_pdr_menu(void);

extern Errcode
find_pdr_loader(char *ifname, Boolean multi_frame,
		struct anim_info *ainfo, char *pdr_name, Rcel *screen);

extern Errcode load_any_picture(char *name, Rcel *screen);
extern Errcode save_current_pictype(char *name, Rcel *screen);
extern Errcode save_gif(char *name, Rcel *screen);

/* vpaint.c */
extern void qload_pic(void);
extern void qsave_pic(void);

#endif
