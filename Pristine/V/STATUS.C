
/* status.c - display continue alert box with program info */

#include "jimk.h"
#include "fli.h"
#include "status.str"

extern long f_free_tflx();
extern long add_up_frames();




status()
{
char *bufs[10];
char buf0[50];
char buf1[50];
char buf2[50];
char buf3[50];
char buf4[50];
char buf5[50];
char buf6[50];
long tot;
extern long dfree();

bufs[0] = status_100 /* "Autodesk Animator Info:" */;
bufs[1] = status_116 /* cst_ */;
bufs[2] = buf1;
bufs[3] = buf2;
bufs[4] = buf3;
bufs[5] = buf4;
bufs[6] = buf5;
bufs[7] = NULL;

sprintf(buf1, status_102 /* "%ld bytes free %ld largest" */, 
	mem_free*16L, largest_frag()*16L);
sprintf(buf2, status_103 /* "this frame update %ld" */, 
	cur_flx[vs.frame_ix].fsize);
sprintf(buf3, status_104 /* "next frame update %ld" */,  
	cur_flx[vs.frame_ix+1].fsize );
tot = add_up_frames();
sprintf(buf4, status_105 /* "total updates %ld average %ld" */, 
	tot, tot/(fhead.frame_count+1));
sprintf(buf5, status_106 /* "%ld free on drive %c" */, 
	dfree(vconfg.scratch_drive+1),
	vconfg.scratch_drive+'A');
#ifdef LATER
sprintf(buf6, status_107 /* "%ld strokes %ld this session" */, 
	fhead.strokes, fhead.session);
#endif LATER
continu_box(bufs);
}

static char *about_lines[] =
	{
	status_108 /* "    Autodesk Animator" */,
	status_109 /* "  v. 1.02      s/n $$$$$$$$$$$$" */,
	status_110 /* "Copyright 1989,1990,1991 by Jim Kent." */,
	status_111 /* "          3/30/90" */,
	status_112 /* "  Produced exclusively for" */,
	status_113 /* "        Autodesk Inc." */,
	status_114 /* "             By" */,
	status_115 /* "       Yost Group Inc." */,
	status_116 /* cst_ */,
	NULL,
	};

#ifdef LATER
char *credits_lines[] =
	{
	"Programming             Jim Kent",
	"Production             Gary Yost",
	"                     Bob Bennett",
	"Alpha Testing        Jack Powell",
	"                     Doug Thomas",
	"                      Ann Phelan",
	"Moral Support    Heidi Brumbaugh",  
	"                      Eric Lyons",
	"Torture Testing       Chris Kent",
	"                     Terry Fritz",
	"                  Cindy Peringer",
	NULL,
	};
#endif LATER

about()
{
continu_box(about_lines);
#ifdef LATER
continu_box(credits_lines);
#endif LATER
}

