/* Stepper.c - a program that lets you step through an animation a
 * frame at a time. */

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include "errcodes.h"
#include "vdevcall.h"
#include "vdevinfo.h"
#include "ffile.h"
#include "filepath.h"
#include "fli.h"
#include "rastcall.h"



SHORT program_version, program_id;

long aa_syslib;

typedef struct stepcb {
	char *vd_path;  /* video driver path */
	Vdevice *vd;    /* video driver if open */
	SHORT vd_mode;	/* driver mode */
	SHORT ovmode;	/* original display mode */
	Rcel *dcel;     /* display screen cel */
	char *fli_path; /* fli file name */
} Stepcb;

Stepcb scb;

void old_video()
/* memory debug routine wants this */
{
}

int get_errtext(Errcode err,char *buf)
/* Generate text to match error code */
{
	if(err != Success && err != Err_reported && err != Err_abort)
		return(sprintf(buf, "code %d", err ));
	return(0);
}
Errcode errline(Errcode err, char *txt,...)
/* Display (printf formatted) error message */
{
char buf[256];
va_list args;

	if(!get_errtext(err, buf))
		return(err);
	printf("%s\n", buf);
	va_start(args,txt);
	vprintf(txt,args);
	va_end(args);
	printf("\nPress any key to continue:\n");
	dos_wait_key();
	return(Err_reported);
}
Errcode step_init_screen(Vdevice **vd, Rcel **dcel, 
					char *drv_name, int drv_mode)
/* Open up display driver and open a screen in the requested mode.
 * Display error message if there's a problem. */
{
Errcode err;
Vmode_info mode_info;

	if((err = pj_open_vdriver(vd,drv_name)) < Success)
		goto error;

	if((err = pj_vd_get_mode(*vd,drv_mode,&mode_info)) < Success)
		goto error;

	if((err = alloc_display_rcel(*vd, dcel, 
							 mode_info.width.actual,
							 mode_info.height.actual, drv_mode)) < Success)
	{
		goto error;
	}

error:
	return(errline(err,"Can't open screen driver \"%s\"\nmode %d", 
				   drv_name, drv_mode));
}
static Errcode step_fli(char *fliname)
/* Load up a fli and step through it frame by frame each time
 * user hits a key.  If he hits <ESC> or  some other quit looking
 * key we'll quit instead of advancing frame. */
{
Errcode err;			/* oop, aak, eek, if negative there's trouble */
Flifile flif;  			/* current fli file and header */
Rcel cel;				/* (clipped) cel to play fli */
int frame=0;			/* current frame index */
int scancode;			/* keyboard scancode */
char asckey;			/* ascii representation of keyboard scan code */


							/* Open up fli file and verify header */
	if((err = pj_fli_open(fliname,&flif,JREADONLY)) < Success)
		goto error;

	/* The following block of code transform our display cel (scb.dcel)
	 * into a cel that is the size of the fli,  but which will
	 * clip drawing outside of the display cel (cel).  The purpose is to
	 * let us play fli's larger than the display screen without
	 * going kaboom as memory is overwritten.
	 * This won't consume more than a few K of memory.  It only
	 * creates a Cel that _seems_ larger. */
	{
	Rectangle celrect;

		celrect.x = (scb.dcel->width - flif.hdr.width)/2;
		celrect.y = (scb.dcel->height - flif.hdr.height)/2;
		celrect.width = flif.hdr.width;
		celrect.height = flif.hdr.height;
		pj_rcel_make_virtual(&cel, scb.dcel, &celrect);
	}


											/* Position file pointer 
											 * to first frame of fli */
	if((err = pj_fli_seek_first(&flif)) < Success)
		goto error;

	for (;;)
	{
											/* Get next frame of fli
											 * onto display cel, and don't
											 * forget to update the color
											 * map. */
		if((err = pj_fli_read_next(fliname,&flif,&cel,TRUE)) < Success)
		{
			goto error;
		}
											/* See if it's time to wrap
											 * around the fli back to
											 * the second frame.
											 * (The very last frame of
											 * a fli brings you back to the
											 * first frame) */
		if(++frame > flif.hdr.frame_count)	
		{
			frame = 1;
			if((err = pj_fli_seek_second(&flif)) < Success)
				goto error;
		}

											/* See if tired (ab)user
											 * is ready to go on to
											 * better things. */

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
				goto done_stepping;
		}
	}
done_stepping:
error:
	pj_fli_close(&flif);
	return(errline(err,"Unable to play flic \"%s\"", fliname ));
}

static Errcode getargs(int argc,char **argv)
/*  Process command line into  scb control block */
{
int argcount;
int switchval;
int switch_arg;
int parm_err;
int pos_parm = 0;
static char help_text[] = 
{ 
"\n"
"Usage: stepper file.fli [options]\n\n"
"options: -v video_driver mode (default to mcga mode 0 320 by 200)\n"
};

    parm_err   = FALSE;
    switch_arg = FALSE;

	if(argc < 2) /* no input */
	{
		printf(help_text);
		exit(0);
	}

	scb.vd_path = pj_mcga_name;

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
					if(scb.vd_path != &pj_mcga_name[0])
						goto sa_parm_error;
					scb.vd_path = argv[argcount];
					if(++argcount >= argc)
					{
						--argcount;
						goto missing_arg;
					}
					if(sscanf(argv[argcount],"%hu%c", &scb.vd_mode, 
													  &scb.vd_mode) != 1)
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
                case 1:
                    scb.fli_path = argv[argcount];
                    break;
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

    if(scb.fli_path == NULL )
    {
        printf ( "Error: Missing fli file name.\n" );
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

	scb.ovmode = pj_get_vmode();

	if((err = getargs(argc,argv)) < Success)
		goto error;

	if((err = step_init_screen(&scb.vd, &scb.dcel, scb.vd_path, scb.vd_mode)) 
		< Success)
		goto error;

	err = step_fli(scb.fli_path);

	goto done;

error:
	errline(err,"failure in main");
done:
	pj_rcel_free(scb.dcel);
	pj_close_vdriver(&scb.vd);
	pj_set_vmode(scb.ovmode);
	exit((err < Success)?-1:0);
}

