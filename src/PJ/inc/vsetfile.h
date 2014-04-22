#ifndef VSETFILE_H
#define VSETFILE_H

#ifndef PTRMACRO_H
	#include "ptrmacro.h"
#endif

#ifndef GFX_H
	#include "gfx.h"
#endif

#ifndef VMAGICS_H
	#include "vmagics.h"
#endif

#ifndef JFILE_H
	#include "jfile.h"
#endif

#ifndef CMAP_H
#include "cmap.h"
#endif

struct vsettings;
enum vset_chunks {
	VSET_FLIDEF_ID = 1,     /* chunk id for fli defaults chunk */
	VSET_PATHARRAY_ID = 2,  /* chunk id for path array chunk */
	VSET_VS_ID = 3,  		/* chunk id vsettings chunk */
	VSET_CMAP_ID = 4,	   /* color map chunk only in default.set file */
	VSET_SLOWVS_ID = 5,    /* slow vsettings chunk */
	VSET_SIZER,
};
#define VSET_MAX_ID (VSET_SIZER-1) 

/* flags used in byte array for read settings chunk */
#define VSET_IGNORE_CHUNK 0x01
#define VSET_CHUNK_READ 0x02


#define VSET_VS_VERS  10

/* Default info for fli reset and resize menu */

#define VSET_FLIDEF_VERS 0

typedef struct vset_flidef {  
	Fat_chunk id;
	SHORT version;
	Rectangle rect;
	LONG speed;
	SHORT frame_count;
} Vset_flidef;

/* paths for use by program in defaults and working settings file */

enum vset_path_ids {
	FLI_PATH = 0,
	CEL_PATH,
	PIC_PATH,
	POCO_PATH,
	FONT_PATH,
	TEXT_PATH,
	MASK_PATH,
	JOIN_PATH,
	JOIN_MASK_PATH,
	PALETTE_PATH,
	TWEEN_PATH,
	POLY_PATH,
	OPTICS_PATH,
	OPTPATH_PATH,
	SETTINGS_PATH,
	MACRO_PATH,
	POCO_USE_PATH,
	SNAPSHOT_PATH,
	VSET_NUM_PATHS,  /* leave at end to get total number of path records */
};

#define VSET_PATHARRAY_VERS 0

typedef struct vset_path {
	SHORT scroller_top;       /* top name of scroller */
	char path[PATH_SIZE];     /* actual path name */
	char wildcard[WILD_SIZE]; /* wild card for scroller menu */
} Vset_path;

#define VSET_PATHS_SIZE ((VSET_NUM_PATHS*sizeof(Vset_path))+sizeof(Chunk_id))

typedef struct vset_paths {
	Fat_chunk id;
	Vset_path path_recs[VSET_NUM_PATHS];
} Vset_paths;


/* items that don't need to be in ram */

#define VSET_SLOWVS_VERS  0

#define MAX_INKS 100

typedef struct slow_vsettings {
	Fat_chunk id;
	Rgb3 mc_ideals[6];      /* menu colors */
	UBYTE inkstrengths[MAX_INKS];
} Slow_vsettings;

#define VSETCHUNK_VERSION 0 /* version for whole chunk of stuff */ 

typedef struct vsetfile {
	Fat_chunk id;
	Fat_chunk paths_id;
	XFILE *xf;
} Vsetfile;

/* vpsubs.c */
extern void reres_settings(void);

/* vsetfnam.c */
extern char *
vset_get_filename(char *prompt, char *suffi, char *button,
		int path_type, char *outpath, Boolean force_suffix);

/* vsetting.c */
extern void rethink_settings(void);
extern Errcode load_default_settings(Vset_flidef *fdef);
extern Errcode reload_tsettings(struct vsettings *pvs, Vset_flidef *fdef);
extern Errcode flush_tsettings(Boolean full_flush);
extern Errcode vset_get_pathinfo(int ptype, Vset_path *cpath);
extern Errcode vset_get_path(int ptype, char *path);
extern Errcode vset_set_pathinfo(int ptype, Vset_path *cpath);
extern Errcode vset_set_path(int ptype, char *path);
extern void save_default_settings(void);
extern Errcode write_fli_settings(XFILE *xf, SHORT chunk_id);
extern Errcode load_default_flidef(Vset_flidef *fdef);
extern void qsave_vsettings(void);
extern void qload_vsettings(void);

#endif
