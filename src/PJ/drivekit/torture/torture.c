#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include "torture.h"

SHORT	program_version, program_id;

/*----------------------------------------------------------------------------
 * Miscellanious global and static data...
 *--------------------------------------------------------------------------*/

Tcb 	tcb;						/* structure which holds all global vars*/

static	char	drivername[80]; 	/* full path/filename of driver 		*/

static	char	logfname[] = "TORTURE.LOG";

static	short	torture_version = VRSN_NUM; 	  /* Comes in from makefile */

#define TESTFLIC_LISTFILE		"testflic.dir"

/*----------------------------------------------------------------------------
 * Data related to choosing the tests to be run...
 *--------------------------------------------------------------------------*/

typedef void	(*Tfunc_t)(Raster *r); /* Test function type */

void xxxx(Raster *r){}

static struct {
	Tfunc_t 	test_routine;
	char		*test_name;
	} test_list[] = {
	{test_dots, 	 "put_dot, get_dot, cput_dot, cget_dot"},
	{test_segs, 	 "put_hseg, get_hseg, put_vseg, get_vseg"},
	{test_lines,	 "put_hline, put_vline"},
	{test_rectpix,	 "put_rectpix, get_rectpix"},
	{test_set_rect,  "set_rect"},
	{test_set_rast,  "set_rast"},
	{test_xor_rect,  "xor_rect"},
	{test_mask1blit, "mask1blit"},
	{test_mask2blit, "mask2blit"},
	{NULL,			 "unbrun_rect"},
	{NULL,			 "unlccomp_rect"},
	{NULL,			 "unss2_rect"},
	{test_rastblits, "blit_in_card, blit_to_ram, blit_from_ram"},
	{test_rastswaps, "swap_in_card, swap_to_ram, swap_from_ram"},
	{test_rasttblits,"tblit_in_card, tblit_to_ram, tblit_from_ram"},
	{test_rastxors,  "xor_in_card, xor_to_ram, xor_from_ram"},
	{test_rastzooms, "zoom_in_card, zoom_to_ram, zoom_from_ram"},
	{test_colors,	 "set_colors, wait_vsync"},
	{test_playback,  "flic playback tests"},
	};

#define IMPLEMENTED_TEST_COUNT	((sizeof(test_list)/sizeof(test_list[0]))-1)
#define MAX_TEST_COUNT			26
#define MAX_TEST_CHAR			((char)('a'+IMPLEMENTED_TEST_COUNT))

static char 	tests_to_do[MAX_TEST_COUNT+1] = "\0";
static Boolean	prompt_for_tests = FALSE;

static void init_tcb_fields(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
	tcb.test_via_generics			= TRUE;
	tcb.single_step_mode			= FALSE;
	tcb.got_stop					= FALSE;
	tcb.got_warning 				= FALSE;
	tcb.got_error					= FALSE;
	tcb.exercise_error_handling 	= TRUE;
	tcb.timing_only_run 			= FALSE;
	tcb.mode_has_changed			= FALSE;
	tcb.logfile 					= NULL;
	tcb.datafile					= NULL;
	tcb.safe_clear_screen			= dot_clearscreen;
	tcb.fliclist					= NULL;
	tcb.playback_verify_buffer		= NULL;
	tcb.list_file_name				= TESTFLIC_LISTFILE;
}
Boolean single_step(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
char key;

	if (tcb.single_step_mode)
		{
		putch('\007');
		key = pj_key_in();
		}
	else
		{
		if (pj_key_is())
			key = pj_key_in();
		else
			key = 0;
		}

	switch (key & 0xff)
		{
		case '\^C':
		case 0x1b:
		case 'x':
		case 'X':
			log_progress("\nProgram will be aborted at your request...\n");
			tcb.got_stop = TRUE;
			return FALSE;
		}
	return TRUE;
}

void log_data(char *txt, ...)
/*****************************************************************************
 *
 ****************************************************************************/
{
va_list args;

	va_start(args,txt);
	vfprintf(tcb.datafile, txt, args);
	va_end(args);
	fflush(tcb.datafile);
}

void log_error(char *txt, ...)
/*****************************************************************************
 *
 ****************************************************************************/
{
va_list args;

	fprintf(tcb.logfile, "Error:   ");
	va_start(args,txt);
	vfprintf(tcb.logfile, txt, args);
	va_end(args);
	fflush(tcb.logfile);
	tcb.got_error = TRUE;
	tcb.got_stop  = TRUE;
}

void log_verror(int x, int y, int found, int expected)
/*****************************************************************************
 *
 ****************************************************************************/
{
va_list args;

	fprintf(tcb.logfile, "Data verification error:  at x=%d y=%d "
						 "found value %d expected %d.\n",
						 x, y, found, expected);
	fflush(tcb.logfile);
	tcb.got_error = TRUE;
	tcb.got_stop  = TRUE;
}

void log_warning(char *txt, ...)
/*****************************************************************************
 *
 ****************************************************************************/
{
va_list args;

	fprintf(tcb.logfile, "Warning: ");
	va_start(args,txt);
	vfprintf(tcb.logfile, txt, args);
	va_end(args);
	fflush(tcb.logfile);
	tcb.got_warning = TRUE;
}


void log_progress(char *txt, ...)
/*****************************************************************************
 *
 ****************************************************************************/
{
va_list args;

	va_start(args,txt);
	vfprintf(tcb.logfile, txt, args);
	va_end(args);
	fflush(tcb.logfile);
}

void log_start(char *txt, ...)
/*****************************************************************************
 *
 ****************************************************************************/
{
va_list args;

	va_start(args,txt);
	vfprintf(tcb.logfile, txt, args);
	va_end(args);
	fflush(tcb.logfile);

	tcb.elapsed_time = 0;
	time_start();
}

void log_end(char *txt, ...)
/*****************************************************************************
 *
 ****************************************************************************/
{
va_list args;

	if (tcb.elapsed_time == 0)	/* if piecemeal timing has not been done, */
		time_end(); 			/* we calc the elapsed time here.		  */

	va_start(args,txt);
	fprintf(tcb.logfile, ">>> Elapsed time = %8.4f seconds.\n",
			(double)(tcb.elapsed_time) / CLK_TCK);
	vfprintf(tcb.logfile, txt, args);
	va_end(args);
	fflush(tcb.logfile);
}

void log_bypass(char *txt)
/*****************************************************************************
 *
 ****************************************************************************/
{

	fprintf(tcb.logfile, "Testing bypassed for %s, not provided by driver.\n\n", txt);
	fflush(tcb.logfile);
}

void clear_screen(void)
/*****************************************************************************
 * clear the screen using the highest driver function known to work.
 * at any given point, we don't know if the driver's high-level routines
 * work, so clearing the screen can be tricky.	every time we successfully
 * test a function that would be usefull in clearing the screen, we reset
 * the safe_clear_screen pointer, so that a call to this routine will always
 * give the fastest clear screen that is known to work. <yech>
 ****************************************************************************/
{
	tcb.safe_clear_screen();
}

static Boolean validate_test_selections(char *instr, char *severity)
/*****************************************************************************
 *
 ****************************************************************************/
{
char	c;
char	*outstr = tests_to_do;
short	counter = 0;

	while (c = *instr++)
		{
		if (!isalpha(c))	/* this helps with menu-based input, accepts	*/
			continue;		/* comma-delimited list & trailing \n as valid	*/
		c |= 0x20;			/* quick force-to-lowercase */
		if (c >= 'a' && c <= MAX_TEST_CHAR)
			{
			if (counter++ < MAX_TEST_COUNT)
				*outstr++ = c;
			else
				{
				printf("%s: too many tests specified (max allowed is %d)\n",
						severity, MAX_TEST_COUNT);
				}
			}
		else
			{
			printf("%s: requested test '%c' is invalid "
				   "(valid range is 'a' thru '%c').\n",
					severity, c, MAX_TEST_CHAR);
			return FALSE;
			}
		}

	*outstr = '\0';
	return TRUE;
}

static void set_default_test_selections(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
short counter;
char  *outstr = tests_to_do;

	for (counter = 0; counter <= IMPLEMENTED_TEST_COUNT; ++counter)
		*outstr++ = 'a' + counter;
	*outstr = '\0';
}

static Boolean do_menu_dialog(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
short	index;
char	instring[2*MAX_TEST_COUNT];

REPROMPT:

	fprintf(stdout,"\nSelect one or more tests to perform:\n");

	for (index = 0; index <= IMPLEMENTED_TEST_COUNT; ++index)
		{
		if (test_list[index].test_routine != NULL)
			fprintf(stdout, "\t%c - %s\n",
					'a'+index, test_list[index].test_name);
		}

	fprintf(stdout, "\t* - All tests\n"
					"\t0 - Exit Program\n"
					"Enter selection(s): ");

	fflush(stdout);

	fgets(instring, sizeof(instring)-1, stdin);

	if (instring[0] == '*')
		{
		set_default_test_selections();
		return TRUE;
		}

	if (!validate_test_selections(instring, "error"))
		goto REPROMPT;

	if (strlen(tests_to_do) == 0)
		return FALSE;
}

static void usage(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
	printf(
		"Usage:\n"
		"  torture [options] driver_name [mode] [options]\n"
		"\n"
		"Where:\n"
		"  driver_name    = Filename of the driver (include drive and path if needed).\n"
		"  [mode]         = The numeric index of the driver mode to test.\n"
		"  [options]      = One or more options from the list below.\n"
		"\n"
		"Options: (Note: either '/' or '-' may be used to preface an option switch)\n"
		"    /c[+|-]       = Enable/disable use of set_hline for fast clear screen.\n"
		"    /f[testlist]  = Specify one or more functions to be tested...\n"
		"                      If /f is not specified, all functions are tested.\n"
		"                      Specifying /f with no list will present a test menu.\n"
		"    /g[+|-]       = Enable/disable testing of calls via the generic library.\n"
		"    /lfilename    = Send logging output to the named file.\n"
		"    /pfilename    = Name of file containing list of flic files to use during\n"
		"                    testing.  Can also be the name of a single flic to use.\n"
		"    /s[+|-]       = Enable/disable single-step (wait-for-key) testing.\n"
		"    /t[+|-]       = Enable/disable timing-only run.\n"
		"\n"
		"Defaults are: mode 0 /c- /g+ /ltorture.log /ptestflic.dir /s- /t-\n"
		);
}

static void getargs(int argc, char **argv)
/*****************************************************************************
 *
 ****************************************************************************/
{
char	*argp;
char	optchar;
Boolean polarity;

	drivername[0] = '\0';
	tcb.test_vmode = -1;

	if (argc < 2)
		{
		usage();
		exit(0);
		}

	while (--argc)
		{
		argp = *++argv;
		if (*argp == '-' || *argp == '/')
			{
			optchar = *++argp;
			polarity = (*++argp != '-');
			switch (optchar)
				{
				case 'c':                   /* use set_hline for clearscreen */
					if (polarity == FALSE)
						tcb.safe_clear_screen = dot_clearscreen;
					else
						tcb.safe_clear_screen = hline_clearscreen;
					break;
				case 'e':                   /* exercise error logic flag */
					tcb.exercise_error_handling = polarity;
					break;
				case 'f':                   /* list of tests to do */
					if (*argp == '\0')
						prompt_for_tests = TRUE;
					else
						{
						prompt_for_tests = FALSE;
						if (!validate_test_selections(argp, "fatal"))
							exit(-1);
						}
					break;
				case 'g':
					tcb.test_via_generics = polarity;
					break;
				case 'h':
				case '?':                   /* help (usage) display */
					usage();
					exit(0);
					break;
				case 'l':                   /* alternate logfile name */
					if (*argp == '\0')
						{
						printf("fatal: a filename must be specified with the '-l' option.\n");
						usage();
						exit(-1);
						}
					strcpy(logfname, argp);
					break;
				case 'p':                   /* alternate test script/flic name */
					if (*argp == '\0')
						{
						printf("fatal: a filename must be specified with the '-p' option.\n");
						usage();
						exit(-1);
						}
					tcb.list_file_name = argp;
					break;
				case 's':                   /* single-step flag */
					tcb.single_step_mode = polarity;
					break;
				case 't':                   /* timing-only flag */
					tcb.timing_only_run = polarity;
					break;
				case 'v':
					printf("Test program version is %hd\n", torture_version);
					break;
				default:					/* unknown option */
					printf("error: unknown parm -%s ignored.\n", --argp);
					break;
				}
			}
		else
			{
			if (drivername[0] == '\0')
				strcpy(drivername, argp);
			else
				if (isdigit(*argp) && tcb.test_vmode == -1)
					tcb.test_vmode = atoi(argp);
				else
					printf("error: unknown positional parm '%s' ignored.\n", argp);
			}
	} /* END while (--argc) */

	if (drivername[0] == '\0')
		{
		printf("fatal: no driver name specified on command line.\n");
		usage();
		exit(-1);
		}

	if (NULL == strchr(drivername, '.'))
		strcat(drivername, ".drv");

	if (tcb.test_vmode == -1)
		tcb.test_vmode = 0;

	if (tests_to_do[0] == '\0' && prompt_for_tests == FALSE)
		set_default_test_selections();
}

void main(int argc, char **argv)
/*****************************************************************************
 *
 ****************************************************************************/
{
short	counter;
short	test_index;
Boolean driver_is_loaded;
Errcode err;

	pj_clock_init();

	old_vmode = pj_get_vmode();
	init_tcb_fields();
	getargs(argc, argv);

	if (NULL == (tcb.logfile = fopen(logfname, "wt")))
		{
		printf("fatal: error attempting to open logging file %s.\n", logfname);
		tcb.got_error = TRUE;
		goto done;
		}

	if (NULL == (tcb.datafile = fopen("torture.dat", "wt")))
		{
		printf("fatal: error attempting to open logging file TORTURE.DAT.\n");
		tcb.got_error = TRUE;
		goto done;
		}

	log_start("Loading device driver...\n");
	pj_set_gs();
	if((err = pj_open_ddriver(&tcb.vd, drivername)) < Success)
		{
		log_error("driver load failed, Errcode = %d.\n", err);
		printf("fatal: error attempting to load driver, Errcode = %d.\n", err);
		tcb.got_error = TRUE;
		goto done;
		}
	else
		{
		driver_is_loaded = TRUE;
		log_progress("...driver loaded successfully.\n\n");
		}

	if (prompt_for_tests)
		if (!do_menu_dialog())
			goto done;

	check_device_sanity();
	if (tcb.got_stop)
		goto done;

	test_device_modeinfo();
	if (tcb.got_stop)
		goto done;

	test_device_open();
	if (tcb.got_stop)
		goto done;

	test_device_cels();
	if (tcb.got_stop)
		goto close;

	set_default_colors(&tcb.display_raster);

	for (counter = 0; tests_to_do[counter]; ++counter)
		{
		test_index = tests_to_do[counter] - 'a';
		if (test_list[test_index].test_routine != NULL)
			test_list[test_index].test_routine(&tcb.display_raster);
		if (tcb.got_stop)
			break;
		}

close:

	if (tcb.offscrn_raster.type >= RT_FIRST_VDRIVER)
		test_close_raster(&tcb.offscrn_raster);

done:

	log_start("Unloading driver...\n"
			  "  close_device() will be called, followed by the rex-layer\n"
			  "  cleanup routine (if any).\n");
	if (driver_is_loaded)
		{
		pj_close_vdriver(&tcb.vd);
		if (tcb.mode_has_changed)
			old_video();
		}
	log_progress("...driver successfully unloaded.\n\n");

	if (tcb.datafile != NULL)
		fclose(tcb.datafile);

	if (tcb.logfile != NULL)
		fclose(tcb.logfile);

	free_flic_names();

	printf("Testing completed ");

	if (tcb.got_error)
		printf("with errors.\n");
	else if (tcb.got_warning)
		printf("with warnings.\n");
	else
		printf("successfully.\n");

	printf("  %s contains the progress tracking messages.\n", logfname);
	printf("  TORTURE.DAT contains the display of the internal data structures.\n");

	pj_clock_cleanup();
	exit(tcb.got_error ? -1 : 0);
}

