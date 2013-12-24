/* qpoco.c - programming environment.  Little safe subset of C interpreter.
	Has access to most of the Animator libraries.  Loads source modules
	from resource directory.  Internally is a stack based interpreter
	with a lot of code stolen from my pogo langauge, hence the name. */

#include <stdio.h>
#include <math.h>
#include "errcodes.h"
#include "jfile.h"
#include "jimk.h"
#include "textedit.h"
#include "resource.h"
#include "pocoface.h"
#include "commonst.h"
#include "softmenu.h"

/***********  stuff concerned with running poco, not the poco library ***/
extern Poco_lib *get_poco_libs();
extern void po_init_abort_control(int abortable, void *handler);
extern Boolean po_check_abort(void *data);


/* the path from which the currently-running poco program was loaded...
 *	this is used below in get_get_poco_include_pathlistlist() as one of the
 *	paths to search for include and library files.	it is also used in
 *	pocodos.c, in the library function GetProgramDir().
 */

char po_current_program_path[PATH_SIZE] = "";

/* the path of a program to be chained to after the current program
 * exits.  before starting a compile/run operation, we blast this down
 * to a null string.  if the program calls PocoChainTo(path), then the
 * specified path is copied into this string.  if the run then ends
 * without error, we automatically loop through a compile/execute cycle
 * again using the new program.
 */

char po_chainto_program_path[PATH_SIZE];

static void set_current_program_path(char *progpath)
/*****************************************************************************
 * store the path of the current poco program into the global path var.
 ****************************************************************************/
{
if (Success <= get_full_path(progpath, po_current_program_path))
	{
	remove_path_name(po_current_program_path);	/* nuke last name on path str */
	}
else
	po_current_program_path[0] = '\0'; /* oh well */
}

static Names *get_poco_include_pathlist(void)
/*****************************************************************************
 * make list of include directories for poco compile.
 *
 *	 the compiler will search the directories in this list for '#include'
 *	 and '#pragma poco library' files.
 *
 *	 currently, we supply a null path, the path the program is in, and the
 *	 system resource directory as the include paths.
 *
 *	 note that the null path MUST be passed to poco to allow specification
 *	 of device/path names in the poco source (eg, "#include "\xyz\temp.h").
 *	 also note that poco expects each pathname (other than the null path)
 *	 to include the trailing backslash.
 ****************************************************************************/
{
static char  nullpath[] = "";
static char  rbuf[PATH_SIZE] = "";              /* resource dir path buffer */

static Names pathlist[] = { 					/* list of include paths... */
	{&pathlist[1], po_current_program_path},	/* program's dir first      */
	{&pathlist[2], nullpath},					/* current/specified dir 1st*/
	{NULL,		   rbuf},						/* then system resource dir */
	};

if (rbuf[0] == 0)					/* only need to get the resource dir once */
	{
	strcpy(rbuf, resource_dir); 			/* get system resource dir	  */
	}

return(&pathlist);
}

static trunc_to_5_lines(char *source)
/*****************************************************************************
 * Remove <cr>'s and truncate string after 5 lines.
 ****************************************************************************/
{
char *dest;
char c;
int count = 0;

dest = source;
while ((c = *source++) != 0)
	{
	switch (c)
		{
		case '\r':
			break;
		case '\n':
			*dest++ = c;
			if (++count >= 5)
				goto OUT;
			break;
		default:
			*dest++ = c;
			break;
		}
	}
OUT:
*dest++ = 0;
}

void report_err_in_file(char *filename)
/*****************************************************************************
 * Put first five lines of a file into a dialog box.
 ****************************************************************************/
{
char err_buf[512];
Jfile f;
int size;

if ((f = pj_open(filename, JREADONLY)) <= JNONE)
	{
	soft_continu_box("no_err_file");
	return;
	}
size = pj_read(f, err_buf, sizeof(err_buf)-1 );
err_buf[size] = 0;
pj_close(f);
trunc_to_5_lines(err_buf);
continu_box(err_buf);
}

static void poco_report_err(char *phase, Errcode err)
/*****************************************************************************
 *
 ****************************************************************************/
{
if (err != Err_early_exit)
	softerr(err, phase);
}

static Boolean poco_text_changed;

static void qedit_note_changes(long line, int cpos)
/*****************************************************************************
 * Edit poco file and note down that changes have been made.
 ****************************************************************************/
{
if (qedit_poco(line, cpos))
	poco_text_changed = TRUE;
}

static void qpoco_err(char *filename,long err_line,short err_char,Boolean edit_err)
/*****************************************************************************
 *
 ****************************************************************************/
{
report_err_in_file(filename);
if (edit_err)
	qedit_note_changes(err_line, err_char);
}

static Errcode execute_poco(void **ppev,long *err_line)
/*****************************************************************************
 *
 ****************************************************************************/
{
Errcode err;
void *ocurs;

po_tur_home();
make_render_cashes();
init_poco_tween();
builtin_err = Success;
po_init_abort_control(TRUE, NULL);
ocurs = set_pen_cursor(&plain_ptool_cursor);
err = run_poco(ppev, poco_err_name, po_check_abort, NULL, err_line);
cleanup_toptext();
cleanup_poco_tween();
free_render_cashes();
set_pen_cursor(ocurs); /* restore old cursor */
show_mouse();	/* make cursor visible for sure */
return(err);
}

Errcode run_poco_stripped_environment(char *source_name)
/* Run a poco program that doesn't need much in the way of the
 * poco run time environment (that won't do many ink calls etc. */
{
void	*pev;
char	err_file[PATH_SIZE];
long	err_line;
int 	err_char;
Errcode err;

if ((err = compile_poco(&pev, source_name, poco_err_name
, NULL/*"H:dump"*/, get_poco_libs()
, err_file, &err_line, &err_char, get_poco_include_pathlist())) >= Success)
	{
	err = run_poco(&pev, poco_err_name, po_check_abort, NULL, &err_line);
	free_poco(&pev);
	}
return err;
}


Errcode qrun_poco(char *sourcename, Boolean edit_err)
/*****************************************************************************
 * compile, (and if successfull) run a poco program.
 ****************************************************************************/
{
Errcode err;
char	err_file[PATH_SIZE];
char	chainbuf[PATH_SIZE];
char	*phase;
long	err_line;
int 	err_char;
void	*pev;

CHAIN_ANOTHER_PROGRAM:					// loop point for chaining programs

	po_chainto_program_path[0] = '\0';  // start with no chainto program

	phase = "poco_compile";
	if ((err = compile_poco(&pev, sourcename, poco_err_name,
				NULL/*"H:dump"*/, get_poco_libs(),
				err_file, &err_line, &err_char, 
				get_poco_include_pathlist())) >= Success)
		{
		save_undo();
		err_char = 0;
		phase = "poco_run";
		err = execute_poco(&pev,&err_line);
		}

	if (err < Success)
		{
		po_chainto_program_path[0] = '\0';  // don't allow chaining after error
		if (err == Err_in_err_file)
			qpoco_err(poco_err_name, err_line, err_char, edit_err);
		else
			poco_report_err(phase, err);
		err = Err_reported;
		}

	free_poco(&pev);

	if (po_chainto_program_path[0] != '\0')
		{
		strcpy(chainbuf, po_chainto_program_path);
		sourcename = chainbuf;
		set_current_program_path(chainbuf);
		goto CHAIN_ANOTHER_PROGRAM;
		}

	return err;
}

static void *cl_pev;

Errcode compile_cl_poco(char *name)
/*****************************************************************************
 * invoke poco (compile only) from the command line
 ****************************************************************************/
{
char err_file[PATH_SIZE];
long err_line;
int err_char;

set_current_program_path(name); 	/* used by compiler for #include, etc */

return(compile_poco(&cl_pev, name, poco_err_name,
		NULL, get_poco_libs(),
		err_file, &err_line, &err_char, get_poco_include_pathlist()));
}


Errcode do_cl_poco(char *name)
/*****************************************************************************
 *
 ****************************************************************************/
{
Errcode err = Success;
long	err_line;
char	chainbuf[PATH_SIZE];

CHAIN_ANOTHER_PROGRAM:

	po_chainto_program_path[0] = '\0';

	if (cl_pev != NULL)
		{
		err = execute_poco(&cl_pev,&err_line);
		if (err < Success)
			{
			po_chainto_program_path[0] = '\0';  // blast chain prog on error
			if (err != Err_in_err_file && err != Err_early_exit)
				poco_report_err("poco_run", err);
			}
		free_poco(&cl_pev);
		if (po_chainto_program_path[0] != '\0')
			{
			strcpy(chainbuf, po_chainto_program_path);
			if ((err = compile_cl_poco(chainbuf)) >= Success)
				goto CHAIN_ANOTHER_PROGRAM;
			else
				free_poco(&cl_pev);
			}
		}
	return err;
}

void qrun_pocofile(char *poco_path, Boolean editable)
/*****************************************************************************
 * save current poco program path, run program, restore current path.
 *
 *	this allows a 'use' invokation of a poco program to run with the proper
 *	current-program-path info, but ensures that the current path for the
 *	program in the editor (if any) is preserved across the run.
 *	(hey -- kludge is my middle name.)
 ****************************************************************************/
{
	char save_path[PATH_SIZE];

	strcpy(save_path, po_current_program_path);
	set_current_program_path(poco_path);
	qrun_poco(poco_path,editable);
	strcpy(po_current_program_path,save_path);
}

static void insure_changes(char *poco_file, char *poco_path)
/*****************************************************************************
 * Make sure user has a chance to save his changes to the program before
 * he loads in a new program or starts a fresh one.
 ****************************************************************************/
{
if (!poco_text_changed)
	return;
if (poco_file[0] == 0)
	{
	if (soft_yes_no_box("save_changes_first"))
		{
		qsave_poco(poco_path);
		}
	}
else
	{
	if (soft_yes_no_box("!%s", "save_changes_to_first", poco_file))
		{
		pj_copyfile(poco_source_name, poco_path);
		}
	}
}

void go_pgmn()
/*****************************************************************************
 *  The main loop for the Poco programming menu.
 ****************************************************************************/
{
int choice;
char pbuf[PATH_SIZE];
char *poco_file;
USHORT mdis[9];


for (;;)
	{
	/* set up asterisks and disables */
	clear_mem(mdis, sizeof(mdis));

	if (!pj_exists(poco_source_name))
		mdis[1] = mdis[3] = mdis[4] = QCF_DISABLED;

	vset_get_path(POCO_PATH,pbuf);

	poco_file = pj_get_path_name(pbuf);

	choice = soft_qchoice(mdis, "!%.18s", "poco_program", poco_file );

	switch (choice)
		{
		case 0:
			qedit_note_changes(-1L,-1);
			break;
		case 1:
			qrun_poco(poco_source_name,TRUE);
			break;
		case 2:
			insure_changes(poco_file, pbuf);
			qload_poco(pbuf);
			break;
		case 3: 	/* save */
			if (poco_file[0] == 0)
				goto SAVE_AS;
			pj_copyfile(poco_source_name, pbuf);
			poco_text_changed = FALSE;
			break;
		case 4: 	/* save as */
SAVE_AS:
			qsave_poco(pbuf);
			break;
		case 5: 	/* new	*/
			insure_changes(poco_file, pbuf);
			pj_delete(poco_source_name);
			poco_text_changed = FALSE;
			poco_file[0] = 0;
			vset_set_path(POCO_PATH,pbuf);
			break;
		case 6:
			print_pocolib("pocolib.txt",get_poco_libs());
			break;
		default:
		case Err_abort:
			goto OUT;
		}
	}
OUT:
return;
}

#define QLS_LOAD 0
#define QLS_SAVE 1
#define QLS_USE 2

static Errcode qls_poco(char *pbuf,char *prompt, char *button, int qls_mode,
	int path_type)
/*****************************************************************************
 * Put up a file requestor to load, save, or use a poco program (depending
 * on qls_mode variable).	Then take appropriate load/save/use action.
 ****************************************************************************/
{
Boolean got_it = FALSE;
Errcode err = Success;
char poco_path[PATH_SIZE];

if(vset_get_filename(prompt,".POC;.H",button,path_type, poco_path,1) != NULL)
	{
	switch (qls_mode)
		{
		case QLS_LOAD:
			{
			if (!pj_exists(poco_path))
				{
				cant_find(poco_path);
				err = Err_no_file;
				}
			else
				{
				vs.ped_cursor_p = vs.ped_yoff = 0;
				set_current_program_path(poco_path);
				if ((err = pj_copyfile(poco_path,poco_source_name)) >= Success)
					poco_text_changed = FALSE;
				}
			}
			break;
		case QLS_USE:
			qrun_pocofile(poco_path,FALSE);
			break;
		case QLS_SAVE:
			{
			if (overwrite_old(poco_path))
				{
				if ((err = pj_copyfile(poco_source_name,poco_path)) >= Success)
					poco_text_changed = FALSE;
				}
			else
				err = Err_extant;
			}
			break;
		}
	}
else
	err  = Err_abort;

if(err >= Success)
	{
	strcpy(pbuf,poco_path);
	}
else
	{
	pj_get_path_name(pbuf)[0] = 0;
	}
return(err);
}

static Errcode qload_poco(char *pbuf)
/*****************************************************************************
 *
 ****************************************************************************/
{
char sbuf[50];

return(qls_poco(pbuf,stack_string("load_poco",sbuf),
	load_str, QLS_LOAD, POCO_PATH));
}

static Errcode qsave_poco(char *pbuf)
/*****************************************************************************
 *
 ****************************************************************************/
{
char sbuf[50];

return(qls_poco(pbuf,stack_string("save_poco",sbuf),
	save_str, QLS_SAVE, POCO_PATH));
}

Errcode quse_poco()
/*****************************************************************************
 *
 ****************************************************************************/
{
char pbuf[PATH_SIZE];
char sbuf[50];
char ubuf[16];


return(qls_poco(pbuf,stack_string("use_poco",sbuf),
	stack_string("use_str",ubuf), QLS_USE, POCO_USE_PATH));
}

#undef QLS_LOAD
#undef QLS_SAVE
#undef QLS_USE


