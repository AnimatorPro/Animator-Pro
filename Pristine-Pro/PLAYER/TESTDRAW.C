/* Testdraw.c - a program that draws various patterns when the
 * user hits number keys:
 *		0 - clear screen
 *		1 - lines
 *		2 - circles
 *		3 - boxes
 *		4 - blits
 *		<esc> or q or x	- quit
 */
#include "errcodes.h"
#include "vdevcall.h"
#include "vdevinfo.h"
#include "rastlib.h"
#include "rastcall.h"
#include "rcel.h"


/* Some prototypes for gfx.lib functions.  Some day we may document
 * this all.... */
						/* draw a line */
void pj_cline(SHORT x1, SHORT y1, SHORT x2, SHORT y2, 
		   void (*dotout)(SHORT x,SHORT y,void *dotdat),
		   void *dotdat);
						/* draw a circle */
Errcode dcircle(SHORT xcen, SHORT ycen, SHORT diam, 
		void (*dotout)(SHORT x,SHORT y,void *dotdat), void *dotdat,
		Errcode (*hlineout)(SHORT y,SHORT x1,SHORT x2,void *hldat), void *hldat,
		Boolean filled);



Errcode init_screen(Vdevice **vd, Rcel **dcel, 
	char *drv_name, int drv_mode);
Errcode errline(Errcode err, char *txt,...);

SHORT program_version, program_id;

long aa_syslib;

typedef struct td_cb {
/* Test draw control block */
	char *vd_path;  /* video driver path */
	Vdevice *vd;    /* video driver if open */
	SHORT vd_mode;	/* driver mode */
	SHORT ovmode;	/* original display mode */
	Rcel *dcel;     /* display screen cel */
} Td_cb;

Td_cb tcb;


typedef struct cdotdat
	{
	Rcel *cel;
	Pixel color;
	} Cdotdat;

void cdotout(SHORT x, SHORT y, Cdotdat *cdd)
/* unclipped dot out */
{
(*cdd->cel->lib->put_dot)((Raster *)cdd->cel, cdd->color, x, y);
}

void clip_cdotout(SHORT x, SHORT y, Cdotdat *cdd)
/* clipped dot out */
{
(*cdd->cel->lib->cput_dot)((Raster *)cdd->cel, cdd->color, x, y);
}

static void line_test(Rcel *dcel)
{
int width = dcel->width, height = dcel->height;
int steps = height/10;		/* ten pixels between lines */
int xinc = width/steps;		/* x distance between lines */
int yinc = height/steps;	/* y distance between lines */
int x = steps*xinc;
int y = 0;
int i;
Cdotdat cdd;
static int color = 0;

	cdd.cel = dcel;
	cdd.color = ++color;
	for (i=0; i<steps; i++)
	{
		pj_cline(x,0,0,y,cdotout,&cdd);
		x -= xinc;
		y += yinc;
	}
}

int imax(int a, int b)
/* return maximum of a,b */
{
return(a>b ? a : b);
}

typedef struct chlidat
	{
	Rcel *cel;
	Pixel color;
	} Chlidat;

Errcode chliout(SHORT y, SHORT x1, SHORT x2, Chlidat *chd)
/* unclipped horizontal line output */
{
(*chd->cel->lib->set_hline)((Raster *)chd->cel, chd->color, x1, y, x2-x1+1);
return(Success);
}

Errcode clip_chliout(SHORT y, SHORT x1,  SHORT x2, Chlidat *chd)
/* unclipped horizontal line output */
{
pj_set_hline((Raster *)chd->cel, chd->color, x1, y, x2-x1+1);
return(Success);
}

static void circle_test(Rcel *cel)
{
SHORT width = cel->width, height = cel->height;
SHORT xcen = width/2, ycen = height/2;
SHORT maxdiam = imax(width,height)-2;
SHORT step = maxdiam/10;
static SHORT color = 0;
SHORT diam;
Chlidat chd;

chd.cel = cel;
for (diam=maxdiam; diam>=0; diam -= step)
	{
	chd.color = ++color;
	dcircle(xcen,ycen,diam,NULL,NULL,clip_chliout,&chd,TRUE);
	}
}

static void box_test(Rcel *cel)
{
SHORT width = cel->width, height = cel->height;
SHORT xcen = width/2, ycen = height/2;
SHORT xstep = width/20;
SHORT ystep = height/20;
SHORT wstep = 2*xstep;
SHORT hstep = 2*ystep;
SHORT x,y;
static SHORT color = 0;

for (x=0,y=0;  x<xcen && y<ycen;  x+=xstep,y+=ystep)
	{
	pj_set_rect(cel, ++color, x, y, width,  height);
	width -= wstep;
	height -= hstep;
	}
}

static Errcode test_draw(Rcel *dcel)
/* Draw various shapes in response to abuser input */
{
int scancode;			/* keyboard scancode */
char asckey;			/* ascii representation of keyboard scan code */

	for (;;)
	{
		asckey = scancode = pj_key_in();	/* Hey dos, what did they hit? */
#define ESCKEY 0x1b
		switch (asckey)						/* Quit program on any 
											 * escape looking key */
		{
			case ESCKEY:
			case 'q':
			case 'Q':
			case 'x':
			case 'X':
				goto done_drawing;
			case '0':
				pj_set_rast(dcel,0);
				break;
			case '1':
				line_test(dcel);
				break;
			case '2':
				circle_test(dcel);
				break;
			case '3':
				box_test(dcel);
				break;
#ifdef SOON
			case '4':
				blit_test(dcel);
				break;
#endif /* SOON */
		}
	}
done_drawing:
	return(Success);
}

static Errcode getargs(int argc,char **argv)
/*  Process command line into  tcb control block */
{
int argcount;
int switchval;
int switch_arg;
int parm_err;
int pos_parm = 0;
static char help_text[] = 
{ 
"\n"
"Usage: testdraw [options]\n\n"
"options: -v video_driver mode (default to mcga mode 0 320 by 200)\n"
};

    parm_err   = FALSE;
    switch_arg = FALSE;

	tcb.vd_path = pj_mcga_name;

    for (argcount = 1; argcount < argc; ++argcount )
    {
        if ((!switch_arg) && *argv[argcount] == '-')
        {
            switchval = tolower(*(1 + argv[argcount]));

            switch(switchval)
            {
            case 'v':
                switch_arg = switchval;
                break;
            default:
                parm_err = TRUE;
            }
        }
        else if (switch_arg) /* if a -x switch needs a parm */
        {
            switch (switch_arg)
            {
                case 'v':
					if(tcb.vd_path != &pj_mcga_name[0])
						goto sa_parm_error;
					tcb.vd_path = argv[argcount];
					if(++argcount >= argc)
					{
						--argcount;
						goto missing_arg;
					}
					if(sscanf(argv[argcount],"%hu%c", &tcb.vd_mode, 
													  &tcb.vd_mode) != 1)
					{
						goto sa_parm_error;
					}
                    break;
                default:
				sa_parm_error:
                    parm_err = TRUE;
            }
            if (!parm_err)
                switch_arg = FALSE;
        }
        else
        {
            ++pos_parm;

            switch(pos_parm)
            {
				default:
                    parm_err = TRUE;
            }
        }

        if (parm_err)
        {
            printf ( "Error: Input argument %d \"%s\" invalid.\n",
                      argcount,
                      argv[argcount] );
			goto error;
        }
    }

    if(switch_arg)
    {
missing_arg:
        printf ( "Error: Missing argument for -%c.\n", switch_arg );
		goto error;
    }

	return(Success);
error:
	printf("%s", help_text);
	return(Err_reported);
}

main(int argc, char **argv)
{
Errcode err;

	tcb.ovmode = pj_get_vmode();

	if((err = getargs(argc,argv)) < Success)
		goto error;

	if((err = init_screen(&tcb.vd, &tcb.dcel, tcb.vd_path, tcb.vd_mode)) 
		< Success)
		goto error;

	err = test_draw(tcb.dcel);

	goto done;

error:
	errline(err,"failure in main");
done:
	pj_rcel_free(tcb.dcel);
	pj_close_vdriver(&tcb.vd);
	pj_set_vmode(tcb.ovmode);
	exit((err < Success)?-1:0);
}

