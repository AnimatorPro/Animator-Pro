#ifndef RND_H
#define RND_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef PTRMACRO_H
	#include "ptrmacro.h"
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

#ifndef POLYGON_H
	#include "polygon.h"
#endif


#undef PBOX
#define PBOX printf("%s %d\n", __FILE__,__LINE__ );



#define scrcoord USHORT
typedef double real;

#define SCOORD_MAXX 8192
#define SCOORD_MAXY 8192

typedef struct rndhead {
	char type;      /* machine type */
	char level;     /* render slide level */
	SHORT xdots, ydots;     /* total screen x and y size */
	SHORT ncolor;  /* number of rgb color values */
	USHORT maxintens;       /* maximum intensity */
	USHORT nshades; /* number of shades per color */
	USHORT pixwid, pixhgt;  /* pixel height and width */
	SHORT btest;    /* test for byte reversal */
} Rndhead;

typedef struct rndhdr {
	char prs[110]; /* id string */
	Rndhead hdr;
} Rndhdr;

/* first fourty chars of ID string */

#define IDCHARS "AutoShade Rendering Slide\x0D\x0AWritten by: "\
				"Autodesk Animator (256 color) \".PDR.\" module.\x0D\x0A\x1a";

#define IDCKSIZE 38


/* 256 color RND file record types */

enum rtypes {
	RND_LINKUP  = 0,   /* Initialize rendering link up */
	RND_INIT    = 1,   /* Initialise use of rendering device */
	RND_START   = 2,   /* Start new rendering */
	RND_END     = 3,   /* End rendering */
	RND_TERM    = 4,   /* Terminate use of rendering device */
	RND_RESIZE  = 5,   /* Change size of rendering screen */
	RND_CLEAR   = 6,   /* Clear entire display */
	RND_CMAPB   = 7,   /* Begin color map */
	RND_CMAP    = 8,   /* Set color map */
	RND_CMAPE   = 9,   /* End of color map */
	RND_POLY    = 10,  /* Polygon */
	RND_FNAME   = 11,  /* User supplied filename */
	RND_CRANGE  = 12,  /* Continuous color range */
	RND_CPOLY   = 13,  /* Continuous color polygon */
	RND_WSLINE  = 14,  /* Output Scan line to driver */
	RND_RSLINE  = 15,  /* Input Scan line from driver */
	RND_RCMAP   = 16,  /* Input color map rgb from driver */
	RND_ETAIL   = 17,  /* i/o details for hard copy drivers */
	RND_CFGREC  = 59,  /* execution time configuration record */
	RND_NEWCFG  = 60,  /* new configuration record */
	RND_CHGCFG  = 61,  /* change configuration record */
	RND_SHOWCFG = 62,  /* show configuration record */
};


struct pkconfig {                  /* configurator portion of adi driver */
    SHORT pfunc;                   /* function code */
    SHORT preclen;                 /* configuration record length */
    char  pcfgrec[1];
};

struct rhcudetail {
     SHORT new;                       /* new configuration? */
     SHORT type;                      /* Serial, Parallel, or NODEVICE. */
     SHORT baud;                      /* Baud rate */
     SHORT parity;                    /* Parity */
     SHORT data;                      /* Data bits/frame */
     SHORT stop;                      /* Stop bits/frame */
     char handshake;                  /* Hardware, XON/XOFF etc. */
};


struct rd_rgb {         /* continuous color RGB value item */
	real r, g, b;   /* primary color intensities */
};

#define MAXINITNAME 65        /* Maximum characters in an init name */
#define MAXFNAME 100

struct rd_init {              /* Initialisation request (all reply cells)*/
                              /* Also used for resize request */
        USHORT flags;              /* Initialization flags */
        scrcoord xdots, ydots;     /* Total screen size in X and Y */
        USHORT pixwid, pixhgt;     /* Pixel width and pixel height */
        SHORT ncolor;             /* Number of rgb color values,
                                      0x8000 bit = stereo display */
        USHORT maxintens;          /* Maximum intensity */
        USHORT nshades;            /* Number of shades per color */
        /* The last two fields are 4-byte data pointers on all platforms.
           Users will cast them to whatever makes their compiler happy. */
        char name[MAXFNAME + 4];   /* Device name string */
        char rendconf[512];        /* Device configuration block */
};

struct rd_pinit {                /* Initialisation request (all reply cells)*/
                                   /* Also used for resize request */
        USHORT flags;              /* Initialization flags */
        scrcoord xdots, ydots;     /* Total screen size in X and Y */
        USHORT pixwid, pixhgt;     /* Pixel width and pixel height */
        SHORT ncolor;             /* Number of rgb color values,
                                      0x8000 bit = stereo display */
        USHORT maxintens;          /* Maximum intensity */
        USHORT nshades;            /* Number of shades per color */
        char name[MAXINITNAME];    /* Driver name string */
        SHORT adiversion;          /* Protected mode adi level */
};

/* rd_start is passed to the driver everytime we start a new
   rendering. */

struct rd_start {                  /* Start rendering request */
        USHORT flags;
};


typedef struct rd_map {     /* color map entry for CMAP record type */
	SHORT ix;     	/* index */
	USHORT r,g,b;   /* red, green, and blue intensity these range between 
					 * 0 and 0xFF only the upper 8 bits are unused */
} Rd_cmap;



struct rd_crange {              /*color range for continuous tones */
	real minshade;  /* minimum shade multiplier */
	real maxshade;  /* maximum shade multiplier */
	short rstretch; /* contrast stretch requested? */
	struct rd_rgb bcolor;  /* background color request */
	real ambient;   /* ambient contribution */
};





/* The POLY record has the following format: */

typedef struct rd_poly {             /* polygon */
	USHORT flags;   		/* flags */
	SHORT color;   		/* fill color */
	SHORT ecolor;  		/* edge color */
	SHORT nvert;    		/* number of vertices total */
	scrcoord vx[10];			/* 10 vertices */
	scrcoord vy[10];			/* 10 vertices */
} Rd_poly;

/* The CPOLY polygon record is shown next: */

struct rd_cpoly {               /* polygon in continuous color */
	USHORT flags;   /* flags */
	struct rd_rgb color;   /* fill color */
	struct rd_rgb ecolor;  /* edge color */
	real shadef;    /* shading factor */
	SHORT nvert;    /* number of vertices total */
	scrcoord vx [10];       /* next 10 vertices */
	scrcoord vy [10];
	real sf [10];   /* shade factor at each vertex */
};

/* Values in flags of rd_poly/rd_cpoly packets */
#define RF_MORE    0x1             /* this packet not complete */
#define RF_CONT    0x2             /* this packet is continuation of prev */
#define RF_LEFT    0x4             /* image intended for left eye only */
#define RF_RIGHT   0x8             /* image intended for right eye only */
#define RF_SF      0x10            /* shade factors included (rd_cpoly only) */
/* top 10 bits reserved for invisible edges */


/* Currently, the "flags" field is unused. The "nvert" field specifies the
 * number of X/Y vertex fields that follow, up to 10. */

/* AutoShade uses the structure RD_SLINE to pass scanline data between
 * AutoShade and the rendering driver. Its record is as follows: */

struct rd_sline {       /* scan line output */
	/* USHORT flags; flags This field is NOT in the file */
	scrcoord x;     	/* start x */
	scrcoord y;         /* start y */
	USHORT xrpt;    	/* number of times to repeat data */
	USHORT sdlen;   	/* number of bytes of data */
	/* followed by UBYTE sdata[1536]; scan line data */
};

/* Member XRPT indicates the number of times the data is repeated, but
 * doesn't figure into the amount of data or number of pixels being
 * represented. This member is one unless the data is repeated
 * horizontally (as in outputting a constant color or a horizontally
 * repeating pattern).
 * 
 * Member SDLN is the total number of bytes of scanline data pointed to by
 * SDATA. */


/* The image file object produced by the PDR */

#define  Rfile struct rnd_image_file  /* NOT a typedef */

struct rnd_image_file {
	Image_file ifhd;
	FILE *file;		/* buffered file handle */
	Rcel *screen;  	/* drawing screen */
	Errcode lasterr;  /* last error returned to host if aborted */
	Anim_info ainfo; /* info created with or opened with */
	Rndhead hdr;     /* header data from opened file */
	void *pktbuf;    /* packet buffer */
	void *pixbuf;    /* output pixel buffer */
	Poly pg;         /* current polygon being assembled */
	Pointlist pts;   /* free polygon points */
	Pixel pgcolor;   /* current polygon fill color */
};


#define PKT_BUFSIZE  2048


/* utility functions */
int strncmp(char *as,char *bs,int sz);
void freez(void *pmem);
void stuff_bytes(UBYTE data, void *buf, unsigned count);
void copy_bytes(void *src,void *dst,int count);
int pj_uscale_by(USHORT x, USHORT p, USHORT q);

/* file io functions */
Errcode rf_write(Rfile *rf, void *buf, int size);
Errcode rf_write_oset(Rfile *rf, void *buf, int size, int orfset);
Errcode rf_read(Rfile *rf,void *buf,LONG size);
Errcode rf_seek(Rfile *rf, LONG count,int whence);
Errcode rf_read_oset(Rfile *rf,void *buf,LONG size,LONG offset);


#endif /* RND_H Leave at end of file */
