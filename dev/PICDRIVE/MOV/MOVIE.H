#ifndef MOVIE_H
#define MOVIE_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef PTRMACRO_H
	#include "ptrmacro.h"
#endif

#ifndef ERRCODES_H
	#include "errcodes.h"
#endif

#ifndef STDIO_H
	#include <stdio.h> 
#endif

#ifndef SYSLIB_H
	#include "syslib.h"
#endif

#ifndef PICDRIVE_H
	#include "picdrive.h"
#endif


#undef PBOX
#define PBOX printf("%s %d\n", __FILE__,__LINE__ );


#define FHDR1 "AutoFlix Movie File 1.0\r\n"  /* File title */
#define FILEVEL  1                 /* File format level */

#define CALLDEPTH   100            /* Depth of call stack */

#define XSIZE    ((unsigned) 640)
#define YSIZE    ((unsigned) 350)

#define XBYTES   (XSIZE/8)
#define SCRBYTES (XBYTES * YSIZE)

/*  File header structure  */

typedef struct mheader {
        short hfilevel;            /* File level */
        short hframes;             /* Frames in movie */
        short hfoptions;           /* Option bits */
#define HFOFWD   1                    /* Forward */
#define HFOPAL   2                    /* Palindromic */
#define HFOMAXH  4                    /* System commands: max headroom */
        short hnpasses;            /* Number of passes.  0 = infinite */
        short hspeedf;             /* Default speed factor */
        long hscorea;              /* Musical score start address */
        long hscorel;              /* Musical score length */
} Mheader;

typedef struct mhead {
    char hmfhdr[25];           /* Fixed file header */
    char hmfdev[70];           /* Device movie made for */
	Mheader h;
} Mhead;



/*  Frame table item  */

#define FTCHARS  20                /* Frame name length */
typedef struct framei {
        char ftname[FTCHARS];      /* Frame name or other data */
        short ftype;               /* Frame type bits */
#define FTBPAUSE    0x1               /* Pause */
#define FTBLABEL    0x2               /* Label */
#define FTBSPEED    0x4               /* Speed change */
#define FTBSLIDE    0x8               /* Image is a slide */
#define FTBDELAY    0x10              /* Delay */
#define FTBSAMEAS   0x20              /* Same as another frame */
#define FTBSCORE    0x40              /* Musical score item */
#define FTBOVERL    0x80              /* Overlay start command */
#define FTBOVERE    0x100             /* Overlay end command */
#define FTBBINC     0x200             /* Image includes buttons */
#define FTBBACT     0x400             /* Button action statement */
#define FTBBACTC    0x401             /* Button action call */
#define FTBDISS     0x402             /* Dissolve specification */
#define FTBPHOLD    0x403             /* Place holder for spliced frames */
#define FTBNULL     0x404             /* Null: ignore this item */
#define FTBREPEAT   0x405             /* Repeat */
#define FTBLOOP     0x406             /* End loop */
#define FTBSYSTEM   0x407             /* External program call */
#define FTBGOTO     0x800             /* Go to label */
#define FTBCALL     0x801             /* Call to label */
#define FTBRETURN   0x802             /* Return to stack top */
#define FTBTEXT     0x1000            /* Image is text */
#define FTBQUIT     0x2000            /* Quit */
#define FTBMOVIE    0x4000            /* Image is another movie */

                                      /* Test if item is not an image */
#define FTBNOTIMG (FTBPAUSE|FTBLABEL|FTBSPEED|FTBDELAY|FTBSCORE|FTBOVERL\
	|FTBOVERE|FTBBACT|FTBGOTO|FTBQUIT)
} Framei;

/* Button descriptor */

struct bdesc {
        short bnum;                /* Button number */
        short bxmin, bymin;        /* Lower left corner */
        short bxmax, bymax;        /* Upper right corner */
};

#define MAXBUTTONS 156             /* Maximum buttons in frame */
extern struct bdesc buttons[];     /* Button descriptor array */
extern short nbuttons;             /* Number of buttons defined */

#define BUTTONS  1                 /* Enable buttons support code */
#define BCOLBASE 100               /* Button colour base */

#define real     double
#define Boolean  short
#define scrcoord short

#define BLACK    0
#define LSOLID   0xFFFF

#define EOS '\0'

#define  Mfile struct movie_file  /* NOT a typedef */

struct movie_file {
	Image_file ifl;
	FILE *file;		/* buffered file handle */
	Rcel *screen;  	/* drawing screen */
	Errcode lasterr;  /* last error returned to host if aborted */
	SHORT flags;
	Anim_info ainfo;

	SHORT cur_frame;		   /* current frame 0 is first */
	SHORT next_y;			   /* next line */

	Mheader hdr;			   /* file header without magic */

	/* stuff from John Walker file */
	Framei *hframe;	   		   /* Frame info table */
	LONG *hframea;		   	   /* File pointers to frames array */
	SHORT *hfllim;		   	   /* Leftmost bit in frame array */
	LONG *hflen;		   	   /* Frame data total length array */

};

/* utility functions */

int strncmp(char *as,char *bs,int sz);
void freez(void *pmem);
void stuff_bytes(UBYTE data, void *buf, unsigned count);
void copy_bytes(void *src,void *dst,int count);

/* file io functions */
Errcode mf_write(Mfile *mf, void *buf, int size);
Errcode mf_write_oset(Mfile *mf, void *buf, int size, int offset);
Errcode mf_read(Mfile *mf,void *buf,LONG size);
Errcode mf_seek(Mfile *mf, LONG count,int whence);
Errcode mf_read_oset(Mfile *mf,void *buf,LONG size,LONG offset);

#endif /* MOVIE_H Leave at end of file */
