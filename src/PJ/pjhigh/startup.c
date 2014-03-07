/* startup.c - Stuff done once only during program start-up. and inverse
 * done at close down */

#define SCRNINIT_CODE
#include <errno.h>
#include <signal.h>
#include <math.h>
#include <ctype.h>
#include "aaconfig.h"
#include "errcodes.h"
#include "pjbasics.h"
#include "resource.h"
#include "argparse.h"
#include "softmenu.h"

Vbcb vb =  /* yep, this is where it is */
{
	-1,  /* video mode at -1  */
};

static char default_config_name[] = "aa.cfg";

#ifdef TESTING
	int debug; /* and this */
#endif /* TESTING */

Errcode builtin_err; /* for poco and other things */

void restore_ivmode()
{
	if(vb.ivmode != -1)
		pj_set_vmode(vb.ivmode);
}
Errcode make_good_dir(char *path)
/* Do a little error handling if current directory looks bad.  Change
   back to start-up directory.	Otherwise just set device */
{
Errcode err;

	if((err = get_dir(path)) < Success)
	{
		change_dir(vb.init_drawer);
		get_full_path(vb.init_drawer,path);
	}
	return(err);
}
int matherr(struct exception *err_info)
/*****************************************************************************
 * this routine catches domain errors, etc, in floating point lib routines.
 * (things like sqrt(-1) and other numerical non sequitors).
 *	 we just set builtin_err so that the poco interpreter will see an error
 *	 upon return from the math function call, then we return non-zero to
 *	 indicate to the built in _matherr routine (our caller) that we don't
 *	 want the default issue-a-message-and-terminate action.
 ****************************************************************************/
{
	_fpreset(); 				/* clear status & re-init math routines */
	builtin_err = Err_float;	/* remember error for poco interpreter */
	return(1);					/* return nonzero, bypasses internal handler */
}

static int fpe_handler(int signum)
/*****************************************************************************
 * this routine catches div-by-zero and overflows in fp math instructions.
 *	 we just set builtin_err so that the poco interpreter will see an error
 *	 upon completion of the current virtual machine instruction, then we
 *	 re-install ourselves since signal handlers are one-shot by definition.
 * IMPORTANT NOTES:
 *	 watcom calls the floating point signal handler from within its
 *	 interupt handler for 80387 exceptions.  upon entry to this routine,
 *	 the hardware stack (ss:esp) is pointing to a 768-byte interupt stack!
 *	 if this routine is ever modified to take more extensive actions (ie,
 *	 calling an error reporting dialog) it will be necessary to switch to
 *	 a bigger stack.
 *	 despite what the watcom docs say, the 'errno' variable is NOT valid
 *	 upon entry to this routine!
 ****************************************************************************/
{
	_fpreset(); 					/* clear status & re-init chip/emulator */
	builtin_err = Err_float;		/* remember error for poco interpreter */
	signal(SIGFPE, fpe_handler);	/* re-install self */
	return(0);						/* don't know who looks at this... */
}
/*****************************************************************/

void cleanup_startup()
{
	cleanup_menu_resource();
	pj_clock_cleanup(); /* un-init clock */
	cleanup_lfiles();
	cleanup_config();
	change_dir(vb.init_drawer);
	restore_ivmode();
	pj_doserr_remove_handler(); 	/* remove dos critical error handler */
#ifdef TESTING
	print_alloclist();
	printf("\nstack used %d\n", pj_get_stack_used());
#endif
}

/*** argument parse functions *****/
static Errcode set_vdriver(Argparse_list *ap, int argc,char **argv,
						   int position)
{
char *cpt;

	cpt = argv[1];
	if(vb.vdriver_name != NULL || argc < 3 || *cpt == '-')
		goto error;
	if(pj_get_path_name(cpt) != cpt) /* name only, no path */
		goto error;
	vb.vdriver_name = cpt;
	vb.vdriver_mode = strtoul(argv[2],&cpt,10);
	if(cpt == NULL || *cpt != 0)
		goto error;
	return(2);
error:
	return(Err_bad_input);
}
static Errcode set_config_name(Argparse_list *ap, int argc,char **argv,
							   int position)
{
	if(vb.config_name != NULL || argc < 2 || *argv[1] == '-')
		return(Err_bad_input);
	vb.config_name = argv[1];
	return(1);
}
static Errcode rethelp(Argparse_list *ap, int argc,char **argv,
					   int position)
{
	return(Err_reported); /* will force help text */
}
static long max_memory_to_grab;
static Errcode set_max_memory(Argparse_list *ap, int argc, char **argv,
						int position)
{
	char *smem = argv[1];

	if(argc < 2 || !isdigit(smem[0]))
		return(Err_bad_input);
	max_memory_to_grab = atol(smem);
	return 1;	/* Number of arguments we eat up (past the -mem) */
}
Errcode init_pj_startup(Argparse_list *more_args,
						Do_aparse do_others,
						int argc, char **argv,
						char *help_key,  /* softmenu help text key */
						char *menufile_name )

/* init called on startup only for basic operation environment
 * current directory must be the same as when program was started
 * returns >= Success if ok < Success if error, this will eat the arguments
 * -c and -cfg and parse the others provided in the input */
{
Errcode err;
Boolean force_config;

static Argparse_list mapl[] = {
	ARGP(mapl,0,"-m",set_max_memory),
	ARGP(mapl,APLAST,"-mem",set_max_memory),
};
static Argparse_list apl[] = {
	ARGP(apl,0,"-?",rethelp),
	ARGP(apl,1,"?",rethelp),
	ARGP(apl,2,"/?",rethelp),
	ARGP(apl,3,"-drv",set_vdriver),
	ARGP(apl,4,"-d",set_vdriver),
	ARGP(apl,5,"-cfg",set_config_name),
	ARGP(apl,6,"-c",set_config_name),
	ARGP(apl,7,"-m",set_max_memory),
	ARGP(apl,APLAST,"-mem",set_max_memory),
};

	vb.ivmode = -1; 				/* not saved here; saved by init_screen() */
	pj_init_stack();				/* initialize stack with cookies */
	signal(SIGINT, SIG_IGN);		/* disable control C aborts */
	pj_doserr_install_handler();	/* Disable abort/retry/fail */
	init_stdfiles();				/* make sure lstderr and lstdout work */
	init_scodes();					/* initialize version number, etc */
	get_dir(vb.init_drawer);		/* get startup directory */
	pj_get_devices(NULL);			/* this will initialize get_devices call */
	pj_clock_init();				/* init clock */
	parse_args(mapl,NULL,argc,argv);/* First scan args for memory parameter. */
	init_mem(max_memory_to_grab);	/* initialize memory handler */
	signal(SIGFPE, fpe_handler);	/* error handler for poco floating point */
	if((err = init_resource_path(argv[0])) < Success)
		goto error;
	if((err = init_menu_resource(menufile_name)) < Success)
		goto error;
	apl[Array_els(apl)-1].next = more_args;
	vb.config_name = NULL;

	if(parse_args(apl,do_others==NULL?rethelp:do_others,argc,argv) < Success)
	{
		if(help_key)
			soft_continu_box(help_key);
		else
			continu_box("No help text");
		err = Err_reported;
		goto error;
	}
	/* read config file for user settings or set defaults */

	if(vb.config_name == NULL)
	{
		force_config = TRUE;
		vb.config_name = default_config_name;
	}
	else
		force_config = FALSE;

	if((err = init_config(force_config)) < Success)
		goto error;

#ifdef DOESNT_WORK_YET
	if ((err = open_vdriver(&vb.ram_vd, vd_ram_name)) < 0)
		goto error;
#else
	vb.ram_vd = NULL;
#endif /* DOESNT_WORK_YET */

error:
	return(err);
}
void copy_insure_suffix(char *path, char *suff, char *out, int outsize)

/* used to help with arguments that need to have a suffix if they don't have
 * one. This is safe! for user provided args. wont exceen outsize, truncates.
 */
{
char *psuf;

	--outsize; /* the NUUUUULLL Baybeeee */
	if(out != path)
		sprintf(out,"%.*s",outsize,path);
	if(*(psuf = pj_get_path_suffix(out)) == 0)
	{
		if((outsize -= (psuf - out)) > 0)
		sprintf(psuf,"%.*s", outsize, suff);
	}
}
Errcode open_pj_startup_screen(Errcode (*init_with_screen)(void *iwdat),
							   void *iwdat )

/**** assumes init_pj_startup() has been successfully called *****/
{
Screen_mode smode;
Screen_mode *open_mode;
Screen_mode *alt_mode;

	open_mode = &vconfg.smode;
	alt_mode = NULL;
	if(vb.vdriver_name != NULL)
	{
		clear_struct(&smode);
		copy_insure_suffix(vb.vdriver_name,".drv",smode.drv_name,
						   sizeof(smode.drv_name));
		smode.mode = vb.vdriver_mode;
		open_mode = &smode;
		alt_mode = &smode; /* no alternate, just fail if we can't get it */
	}
	return(init_screen(open_mode, alt_mode, init_with_screen, iwdat));
}
void get_startup_dir(char *buf)
/* little global function */
{
	strcpy(buf, vb.init_drawer);
}
