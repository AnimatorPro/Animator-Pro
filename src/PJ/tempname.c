/* tempname.c  global temporary file names for pj */

#include "tfile.h"

#define TDIR "H:\\PAAT"

char tflxname[] = "=:AATEMP.FLX";	/* main animation temp file */
char tsettings_name[] = "<:AATEMP.SET"; /* settings file for vs, paths 
										 * and default size button */
char ram_tflx_name[] = "=:AATEMP2.FLX";
char disk_tflx_name[] = "<:AATEMP3.FLX";

									/* Animation temp file for to all ops */
char alt_name[] = "<:AATEMP.PIC";	/* Swap file for alt screen */
char screen_name[] = "<:AATEMP2.PIC";/* Swap file for current frame */
char cel_name[] = "=:THECEL.TMP";	/* temp info file for cel */
char cel_fli_name[] = "=:THECEL.FLC";	/* temp image file for thecel */
char text_name[] = "=:AATEMP.TXT";	/* Swap file for text buffer */
char cclip_name[] = "=:AATEMP.CCL";	/* Current color clip */
char mask_name[] = "<:AATEMP.MSK";	/* Swap file for mask */
char poly_name[] = "=:AATEMP.PLY";	/* Latest vector shape */
char ppoly_name[] = "=:AATEMPP.PLY";	/* Optics path */
char bscreen_name[] = "=:AATEMP3.PIC";	
								/* Back frame buffer. Frame c. 4 back */
char another_name[] = "<:AATEMP.XXX";
								/* swap file for wierd misc uses */
char optics_name[] = "=:AATEMP.OPT";	/* Stack of optics moves. */
char rbf_name[] = "=:AATEMP.PTS";		/* Redo's point list */
char flxolayname[] = "<:FLXOVLAY.TMP";	/* temp file for overlay 
											 * processing */
char tween_name[] = "=:AATEMP.TWE";
char poco_source_name[] = "=:AATEMP.POC";
char poco_err_name[] = "=:AATEMP.ERR";
char macro_name[] = "=:AATEMP.REC";

char default_name[] = "DEFAULT.SET";	/* User defined startup-state */

char *work_temp_files[] = /* files that are not needed for state saving */
	{
	ram_tflx_name,
	disk_tflx_name,
	flxolayname,
	NULL
	};

char *state_temp_files[] = /* files that are needed to save state */
	{
	tflxname,
	tsettings_name,
	alt_name,
	screen_name,
	bscreen_name,
	cel_name,
	cel_fli_name,
	text_name,
	cclip_name,
	mask_name,
	poly_name,
	ppoly_name,
	another_name,
	optics_name,
	rbf_name,
	tween_name,
	poco_source_name,
	poco_err_name,
	macro_name,
	NULL,
	};

