/* config.c - These routines manipulate the file v.cfg and act on the
   data contained there.  Configuration - state stuff that is semi-permanent
   in nature.  What input device to use, where to put temp files, what
   directory your fonts are in, what comm port to use for the serial
   summa tablet, whether it's safe to speed up the clock interrupt.
   */

#include <ctype.h>
#include "jimk.h"
#include "fli.h"
#include "commonst.h"
#include "config.str"

char tflxname[] = "H:\\AAT\\AATEMP.FLX";	/* main animation temp file */
char new_tflx_name[] = "H:\\AAT\\AATEMP2.FLX";
									/* Animation temp file for to all ops */
char tmacro_name[] = "H:\\AAT\\AATEMP.REC";	/* The current input macro */
char alt_name[] = "H:\\AAT\\AATEMP.PIC";	/* Swap file for alt screen */
char screen_name[] = "H:\\AAT\\AATEMP2.PIC";/* Swap file for current frame */
char cel_name[] = "H:\\AAT\\AATEMP.CEL";	/* Swap file for cel */
char text_name[] = "H:\\AAT\\AATEMP.TXT";	/* Swap file for text buffer */
char cclip_name[] = "H:\\AAT\\AATEMP.CCL";	/* Current color clip */
char mask_name[] = "H:\\AAT\\AATEMP.MSK";	/* Swap file for mask */
char poly_name[] = "H:\\AAT\\AATEMP.PLY";	/* Latest vector shape */
char poly1_name[] = "H:\\AAT\\AATEMP1.PLY";	/* Start shape of tween */
char poly2_name[] = "H:\\AAT\\AATEMP2.PLY";	/* End shape of tween */
char ppoly_name[] = "H:\\AAT\\AATEMPP.PLY";	/* Optics path */
char bscreen_name[] = "H:\\AAT\\AATEMP3.PIC";	
								/* Back frame buffer. Frame c. 4 back */
char another_name[] = "H:\\AAT\\AATEMP.XXX";
								/* swap file for wierd misc uses */
char optics_name[] = "H:\\AAT\\AATEMP.OPT";	/* Stack of optics moves. */
static char temp_path[] = "H:\\AAT";

char default_name[] = "DEFAULT.FLX";	/* User defined startup-state */

char conf_name[] = "aa.cfg";			/* Small stuff unlikely to change*/

struct config vconfg;					/* Ram image of v.cfg */
struct config_ext vconfg_ext;

/* Direct temp files to the right device */
path_temps(c)
char c;
{
temp_path[0] = cclip_name[0] = screen_name[0] = poly_name[0] = 
	poly1_name[0] = poly2_name[0] = 
	ppoly_name[0] = bscreen_name[0] = another_name[0] = 
	alt_name[0] = cel_name[0] = text_name[0] = mask_name[0] = 
	optics_name[0] = tflxname[0] = new_tflx_name[0] = tmacro_name[0] = c;
make_dir(temp_path);
}

static char *copy_overs[] = {
	cclip_name,
	poly_name,
	ppoly_name,
	poly1_name,
	poly2_name,
	text_name,
	optics_name,
	tmacro_name,
	cel_name,
	alt_name,
	mask_name,
};

do_copy_overs(char olddev, char newdev)
{
char nbuf[80];
int i;
char *name;
char **names;

if (newdev == olddev)
	return;
i = Array_els(copy_overs);
names =  copy_overs;
while (--i >= 0)
	{
	name = *names++;
	if (jexists(name))
		{
		strcpy(nbuf, name);
		nbuf[0] = newdev;
		if (!jcopyfile(name, nbuf))
			jdelete(nbuf);
		jdelete(name);
		}
	}
jdelete(bscreen_name);	/* get rid of back frame buffer */
}

/* Copy ram image of configuration to file in startup directory */
rewrite_config()
{
extern char init_drawer[];
int f;
char odrawer[80];

strcpy(odrawer, vs.drawer);
change_dir(init_drawer);
if ((f = jcreate(conf_name))!=0)
	{
	jwrite(f, &vconfg, sizeof(vconfg) );
	jwrite(f, &vconfg_ext, sizeof(vconfg_ext) );
	jclose(f);
	}
change_dir(odrawer);
}


/* Write out current state to default.flx */
static
save_settings()
{
extern char init_drawer[];

flush_tempflx();	/* make sure latest configuration stuff is written */
jclose(tflx);
change_dir(init_drawer);
jcopyfile(tflxname, default_name);
change_dir(vs.drawer);
tflx = jopen(tflxname, 2);
}

static
unasterisk(names, count)
char **names;
int count;
{
char *p;

while (--count >= 0)
	{
	p = *names++;
	p[0] = ' ';
	}
}

static char *serial_lines[] =
	{
	config_119 /* " COM 1" */,
	config_120 /* " COM 2" */,
	config_121 /* " COM 3" */,
	config_122 /* " COM 4" */,
	cst__cancel,
	};

static
get_serial_port()
{
int choice;

unasterisk(serial_lines, Array_els(serial_lines) );
(serial_lines[vconfg.comm_port])[0] = '*';
choice = qchoice(config_124 /* "Select com port for tablet" */, serial_lines,
	Array_els(serial_lines) );
if (choice)
	{
	vconfg.comm_port = choice - 1;
	}
}

static char *pucky4_lines[] =
	{
	config_125 /* " Stylus" */,
	config_130 /* " Puck" */,
	cst__cancel,
	};

#ifdef WACOM
static char *wpucky4_lines[] =
	{
	config_128 /* " Pressure sensitive stylus" */,
	config_129 /* " Side button stylus" */,
	config_130 /* " Puck" */,
	cst__cancel,
	};
#endif WACOM

static
get_pucky4(lines, lcount)
char *lines[];
int lcount;
{
int choice;

unasterisk(lines,  lcount);
lines[vconfg.pucky4][0] = '*';
choice = qchoice(config_132 /* "Select Pointer" */, lines, lcount);
if (choice > 0)
	{
	vconfg.pucky4 = choice-1;
	return(1);
	}
else
	return(0);
}

#ifdef UNUSED
static qcustom_driver()
{
char odrawer[80];
char vip_path[80];
char *name;

strcpy(odrawer, vs.drawer);
if ((name = get_filename("Select a Virtual Input driver", ".VIP"))!= NULL)
	{
	if (check_exe(name) )
		strcpy(vconfg_ext.vip_driver, name);
	rewrite_config();
	}
change_dir(odrawer);
}
#endif /* UNUSED */

static char *input_lines[] =
	{
	config_133 /* " Microsoft Compatible Mouse" */,
	config_134 /* " Summagraphics MM 1201 12x12 Tablet" */,
#ifdef WACOM
	config_135 /* " Wacom II pressure sensitive Tablet" */,
#endif WACOM
	config_136 /* " Set serial port for tablet" */,
	cst__cancel,
	};


static
config_input()
{
int choice;

unasterisk(input_lines, Array_els(input_lines) );
input_lines[vconfg.dev_type][0] = '*';
choice = qchoice(config_138 /* "Select graphics input device" */, input_lines,
	Array_els(input_lines) );
switch (choice)
	{
	case 0:
		return;
	case 1:
		vconfg.dev_type = INP_MMOUSE;
		break;
	case 2:
		if (!get_pucky4(pucky4_lines, Array_els(pucky4_lines)))
			return;
		vconfg.dev_type = INP_SUMMA2;
		break;
#ifdef WACOM
	case 3:
		if (!get_pucky4(wpucky4_lines, Array_els(wpucky4_lines)))
			return;
		vconfg.dev_type = INP_WACOM2;
		break;
	case 4:
#else WACOM
	case 3:
#endif WACOM
		get_serial_port();
		break;
	}
rewrite_config();
cleanup_input();
init_input();
}

static char *clock_lines[] =
	{
	config_139 /* " Autodesk Animator normal clock." */,
	config_140 /* " Slower clock that works with more mice." */,
	config_141 /* " Leave things as they are." */,
	};
/* Decide if it's safe to install 70Hz clock interrupt */
static
config_clock()
{
char c;

unasterisk(clock_lines, Array_els(clock_lines) );
clock_lines[vconfg.noint][0] = '*';
c = qchoice(config_142 /* "Adjust Clock Configuration" */, 
	clock_lines, Array_els(clock_lines));
unconfig_ints();
switch (c)
	{
	case 1:
		vconfg.noint = 0;
		rewrite_config();
		break;
	case 2:
		vconfg.noint = 1;
		rewrite_config();
		break;
	default:
		break;
	}
config_ints();
}


static char *conf_lines[] = 
	{
	config_143 /* " Drive for temporary files." */,
	config_144 /* " Save default.flx." */,
	config_145 /* " Clock driver" */,
	config_146 /* " Input Device (Mouse/Tablet)" */,
	config_147 /* " Display coordinates" */,
	cst__cancel,
	};

/* configure numbered item menu */
new_config()
{
int c;

conf_lines[4][0] = (vs.dcoor ? '*' : ' ');
c = qchoice(config_149 /* "Adjust Animator Configuration" */, 
	conf_lines, Array_els(conf_lines));
switch (c)
	{
	case 1:
		config_scratch();
		break;
	case 2:
		save_settings();
		break;
	case 3:
		config_clock();
		break;
	case 4:
		config_input();
		break;
	case 5:
		vs.dcoor = !vs.dcoor;
		break;
	}
}

