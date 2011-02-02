#ifndef ANIM_H
#define ANIM_H

#ifdef ALLOCATE_GLOBALS
#define EXTERN 
#else
#define EXTERN extern
#endif


struct iff_frame {
	struct iff_frame *parent;
	union bytes4 id;
	LONG chunk_size;
	LONG chunk_left;
	void *data;
	};






struct animcb {
	char *anim_name;  	/* name of currently open anim for error reporting */
	Bfile bfile;		/* currently open file for anim opened for anim name */
	struct iff_frame *iframe;  /* current iff_frame NULL if no frame yet */
	PLANEPTR screen0;   /* primary amiga screen */
	PLANEPTR screen1;   /* work screen */
	};


EXTERN struct animcb *animcb 
#ifdef ALLOCATE_GLOBALS
	= NULL
#endif
;

typedef struct {
   UBYTE operation; /* =5 for RIFF style */
   UBYTE mask;
   UWORD w,h;
   WORD  x,y;
   ULONG abstime; /* for timing w.r.t. start of anim file in jiffies (notused)*/
   ULONG reltime; /* for timing w.r.t. last frame in jiffies */
   UBYTE interleave; /* =0 for XORing to two frames back, =1 for last frame
                             (not used yet)  */
   UBYTE pad0;
   ULONG bits;
   UBYTE pad[16];
   } AnimationHeader;

#endif
