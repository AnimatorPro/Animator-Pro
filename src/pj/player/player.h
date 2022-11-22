#ifndef PLAYER_H
#define PLAYER_H

#include <stdio.h>
#include <ctype.h>

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef ERRCODES_H
	#include "errcodes.h"
#endif

#ifndef PJBASICS_H
	#include "pjbasics.h"
#endif

#ifndef RASTCALL_H
	#include "rastcall.h"
#endif

#ifndef FLI_H 
	#include "fli.h"
#endif

#ifndef REQLIB_H
	#include "reqlib.h"
#endif

#ifndef TIMESELS_H
	#include "timesels.h"
#endif

enum abort_keys {
	AKEY_REPLAY = '\b', 
};

enum main_codes {
	PRET_DOMENU = Success,
	PRET_QUIT,
	PRET_RESIZE_SCREEN,
	PRET_DOSCRIPT,
};

/* structure for a ram fli */

typedef struct ramframe {
	void *next;
	LONG doff;	/* disk offset of this frame if negative we have a ram record 
	 			 * if positive then frame data must be read from disk and
				 * the ram frame doesnot include data area beyond doff */

	Fli_frame frame; /* actual fli frame data read in from disk */
	Chunk_id first_chunk; /* for reader/loader to bypass pstamp chunks */
} Ramframe;

typedef struct ramfli {
	void *next;
	char *name;         /* pointer to name */
	Fli_head *fhead;    /* copy of the flis header if disk file un-needed
						 * otherwise frames are missing and file must be 
						 * opened */
	Ramframe *frames;   /* frame data */
	LONG cbuf_size;		/* compression buffer size needed for this fli */
	LONG speed;         /* speed from fli for script scanning */
	LONG minspeed;      /* minimum speed scanned in script */
	USHORT flags;       /* see RF_??? below */
	char nbuf[1];       /* where name lives */
} Ramfli;

#define RF_LOAD_FIRST		0x0001  /* first frame desired in ram */
#define RF_LOAD_RING		0x0002  /* ring frame needed for play */
#define RF_ONE_FRAME		0x0004  /* source fli is one frame */
#define RF_LOAD_ASKED		0x0008  /* load has been asked for */
#define RF_PLAY_ASKED		0x0010  /* a play has been asked for */
#define RF_FREE_ASKED       0x0020  /* we have asked for a free */
#define RF_ON_FLOPPY        0x0040  /* source fli is on a floppy */

/* structure for "Atom" recursion stack */

typedef struct atom {
	void *parent;
	SHORT type;
} Atom;

/* atom types */

enum {
	LOOP_ATOM = 0,
	CHOICE_ATOM = 1,
	GOSUB_ATOM = 2,
};

typedef struct loop_atom {
	Atom hdr;
	LONG top_oset;  	/* offset to loop start position */
	SHORT code_line;    /* line of code of loop top */
	SHORT count;    	/* number of times to loop */
} Loop_atom;

typedef struct choice_atom {
	Atom hdr;
	LONG end_oset;  	/* just after endchoice for choice */
	SHORT end_line;   
} Choice_atom;

typedef struct gosub_atom {
	Atom hdr;
	LONG ret_oset; 
	SHORT ret_line;   
} Gosub_atom;

typedef struct label {
	void *next;
	char *name;
	LONG oset;
	SHORT line;
	char nbuf[1];
} Label;


/* structure allocated for every script called */

typedef struct callnode {
	struct callnode *parent; /* what called this NULL if entry point */
	char scr_path[PATH_SIZE]; /* script file path */
	LONG fpos;  /* file position to re-open file to */
	LONG line;  /* line number for error reporting */
	Label *labels;  /* subroutine labels */
	Atom *atom;   /* bottom of atom stack, NULL if empty */
	char *anext;  /* next available atom on astack */
	char *amax;   /* one byte beyond end of astack */
	USHORT flags;	/* for scanner */
	char astack[24 * sizeof(Loop_atom)];
} Cnode;

#define SCAN_IN_SUB 0x0001



typedef struct playcb {
	char *vd_path;  /* video driver path */
	SHORT vd_mode;	/* driver mode */

	SHORT script_mode; /* we are playing a script */ 
	SHORT lock_key; /* key input locked out during script play (actual value
	 				 * of key used to lock input */
	SHORT scr_scroll_top;     /* top name for file selector */
	char scr_root[PATH_SIZE]; /* path for entry point script */
	char scr_wild[WILD_SIZE]; /* wildcard for file selector */

	Cnode *cn;   /* pnode script stack list for call stack */
	FILE *scr_file; /* script file pointer */
	int toklen; 	/* length of token in token */
	char token[128]; /* actual token from file */
	int reuse_tok; 	/* flag to reuse token set to toklen of token if to be
					 * reused by get_token() */

	Ramfli *ramflis;	/* list of loaded ram flis */
	Fli_frame *cbuf; 	/* compression buffer for fli */

/* display cel data, the cel is maintained the same size as the current image 
 * file and is a virtual cel to the screen */

	Rectangle osize;    /* saved size for comparison */
	Rcel virtual;   	/* virtual for making dcel */
	Rcel *dcel;     	/* display cel for current fli */ 

/**** current fli play description ****/

	char loadpdr[FILE_NAME_SIZE]; /* pdr to load image file with from script */
	char fliname[PATH_SIZE];  /* current fli path from script */
	LONG fliline;	/* line number fli name found on */
	LONG flioset;	/* file offset fli name found on */

	Flifile flif;  	/* current fli file and header */
	Ramfli *rfli;	/* synchronous ram fli if a ram fli is open */
	Ramframe *rframe; /* current ram frame for the ram fli */
	LONG next_frame_oset; /* for fli frame seeker */
	SHORT frame_ix; /* current frame index */
	SHORT abort_key; /* the key that produced the last Err_abort */

/* fli play control items from the script control the play loop */

	LONG speed;	   	/* frame speed in 1000ths of a sec */
	int sspeed;	   	/* scripted speed in 1000ths of a sec use if not -1 */
	int loops;		/* scripted number of loops to play fli 0 == forever */
	int pause;      /* scripted pause in 1000ths of a sec -1 == default
					 * 0 == pause forever */
	int fadein;     /* fade in time in 1/1000 sec */
	int fadeout;    /* fade out time in 1/1000 sec */
	Rgb3 in_from;   /* color to fade in from */
	Rgb3 out_to;    /* color to fade out to */
	LONG cktime;	/* time for speed check in play loop */

/* stuff for choice key monitoring during choice play */

	char *choices;      /* string of key choices */
	LONG *choice_osets;  /* offsets to choices in script */
	LONG *choice_lines;  /* line numbers for offsets */
	int choice;  /* the actual key hit for the choice */

/* script scanning stuff */

	Names *scan_calls;
	LONG max_cbuf_size; /* gathered from script scan */

} Playcb;


extern Playcb pcb;
extern Menuhdr player_menu;

typedef struct scan_data {
	int sub_level;
} Scan_data;

typedef struct wordfunc {
	void *next;
	char *word;
	Errcode (*func)(Scan_data *sd);
	Errcode (*scanfunc)(Scan_data *sd);
} Wordfunc;

#define INIT_WF(ix,func,scanfunc,word)  \
	{ ix>0?&(WTAB[ix-1]):NULL, word, func, scanfunc }

void close_curfli();
Errcode open_curfli(Boolean load_colors,Boolean force_ram);
Errcode open_curpic(Boolean load_colors);
Errcode play_fli();
Errcode play_picture();
Boolean player_do_keys();
Errcode load_picture(char *pdr_name,char *picname,Boolean load_colors);

Ramfli *find_ramfli(char *local_path);
Errcode add_ramfli(char *name, Ramfli **prf);

extern Minitime_data playfli_data;

Errcode script_error(Errcode err,char *key,...);

/* tokenizer things */

int get_token();
void reuse_token();
Errcode scan_number(char type, void *pnum);
Errcode get_number(char type, void *pnum);
Errcode getuint(int *pint);

#endif /* PLAYER_H Leave at end of file */
