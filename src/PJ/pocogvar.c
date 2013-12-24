/*****************************************************************************
 * POCOGVAR.C - Builtin lib routines to support Poco global string vars.
 *
 *	 Notes:
 *		Global string variables store data longer than the duration of the
 *		poco program that sets them (indeed, they live longer than a single
 *		host execution).  They are useful for communicating between related
 *		sets of poco programs, for storing user preferences, and for building
 *		auto-restart checkpointing kind of systems into a program.	Since they
 *		exist primarily to provide convenience, the routines in this module
 *		take a pretty mellow attitude towards failure.	If a given request
 *		cannot be serviced due to file I/O errors, we don't set builtin_err
 *		to shut down the whole show, we just report Err_not_found on a
 *		retrieval, or flat-out ignore the error on a store.
 *
 *		The global string vars come in two flavors, permenant and temporary.
 *		The permenant vars are stored in a file in the resource directory;
 *		nothing makes them go away except an explicit delete request.  The
 *		temporary ones are stored in a PJ tempfile (device '=:'), and will
 *		disappear when a 'Reset' is done, or when a Quit/Abandon happens.
 *
 *		The logic for maintaining these variables is cheesy to the max.  The
 *		first time a poco program makes a call to a routine in this library,
 *		all the global vars from both files are loaded into a linked list.
 *		When changes are made (ie, VarSet, VarDelete), the changes are made
 *		to the linked list, and a change counter is incremented.  When the
 *		poco program ends, the library cleanup routine is called, and this
 *		routine dumps all the variables back into the two files, if the
 *		change counter is non-zero.  The logic for deleting a variable is
 *		also pretty cheesy:  we free up the value string, and then set the
 *		name field in the Globalv structure to '\0' to indicate the var has
 *		been deleted.  We don't unlink the var from the list, we just leave
 *		the 'tombstone' entry linked in for simplicity.  When the list is
 *		saved, the tombstones are skipped so that deleted vars don't get
 *		written out to the files.
 ****************************************************************************/

/*----------------------------------------------------------------------------
 * include the usual header files...
 *--------------------------------------------------------------------------*/

#include "errcodes.h"
#include "pocolib.h"
#include "memory.h"
#include "filepath.h"
#include "lstdio.h"

/*----------------------------------------------------------------------------
 * data and constants...
 *--------------------------------------------------------------------------*/

static char permfpath[PATH_SIZE] = "";
static char permfname[] = "aaperm.glv";
static char tempfpath[] = "=:aatemp.glv";

#define CHANGE_COUNT_LIMIT 50	// number of changes before auto-list-compaction
#define PERMVAR_FLAG '$'        // names starting with this outlive resets.

#define MAX_VNAME_LEN	 256	// who needs a variable name bigger than this?
#define MAX_VVALUE_LEN	1536	// 1.5k, because we're only g'teed 2k stack.

typedef struct globalv {
	struct globalv *next;
	char		   *value;
	char		   name[1]; 	// not really 1, just defines start of string.
	} Globalv;

static Globalv	*varlist  = NULL;
static Globalv	*listnext = NULL;
static Boolean	listchangecount = 0;

/*----------------------------------------------------------------------------
 * code...
 *--------------------------------------------------------------------------*/

static char *new_value(char *value)
/*****************************************************************************
 * alloc memory for a string and copy the existing string into it.
 ****************************************************************************/
{
	char	*newvalue;

	if (NULL == (newvalue = pj_malloc(1+strlen(value))))
		return NULL;
	strcpy(newvalue, value);
	return newvalue;
}

static Globalv *new_global_var(char *name, char *value)
/*****************************************************************************
 * alloc a new global var, copy the name and value into it.
 ****************************************************************************/
{
	Globalv *newvar;

	if (NULL == (newvar = pj_malloc(sizeof(Globalv)+strlen(name))))
		return NULL;
	strcpy(newvar->name, name);

	if (NULL == (newvar->value = new_value(value))) {
		pj_free(newvar);
		return NULL;
	}

	newvar->next = varlist;
	varlist = newvar;
	return newvar;
}

static void write_nt_string(FILE *ofile, char *str)
/*****************************************************************************
 * write nullterm'd string (including nullterm char) to file.
 *	resist the temptation to change this to fputs, we have to preserve EOL
 *	chars as they exist in the variable data string.
 ****************************************************************************/
{
	do	{
		fputc(*str, ofile);
		} while (*str++);
}

static Errcode read_nt_string(FILE *ifile, char *str, int maxlen)
/*****************************************************************************
 * read a nullterm'd string of up to maxlen chars.
 *	resist the temptation to change this to fgets, we have to preserve EOL
 *	chars as they exist in the variable data string.
 ****************************************************************************/
{
	int inchar;

	for (;;) {
		if (0 == --maxlen)
			return Err_truncated;
		if (EOF == (inchar = fgetc(ifile)))
			return Err_eof;
		if (0x00 == (*str++ = inchar))
			return Success;
	}
}

static Errcode dump_global_vars(void)
/*****************************************************************************
 * dump list of global vars (if any) to the temp files.
 *	 if we have trouble opening the permenant data file, we return the error
 *	 status to the caller, in case this dump was triggered by a GlobalVarFlush.
 *	 if we have trouble opening the temporary file, we ignore it.
 ****************************************************************************/
{
	Errcode err;
	FILE	*opfile = NULL; 		// file for permenant vars
	FILE	*otfile = NULL; 		// file for temporary vars
	FILE	*to_file;				// holds one of the above
	Globalv *vl;
	Globalv *vlnext;

	if (NULL == varlist)			// no list, punt.
		return Success;

	if (0 == listchangecount)		// unmodified list, just free it.
		goto FREE_LIST;

	if (permfpath[0] == 0x00) { 	// one time, make perm filepathname.
		make_resource_name(permfname, permfpath);
	}

	opfile = fopen(permfpath,"wb");
	if (opfile == NULL)
		err = pj_errno_errcode();

	otfile = fopen(tempfpath,"wb"); // we don't care about errors here.

	for (vl = varlist; vl != NULL; vl = vlnext) {
		if (vl->name[0]) { // skip deleted-var tombstones
			to_file = (vl->name[0] == PERMVAR_FLAG) ? opfile : otfile;
			if (to_file != NULL) {
				write_nt_string(to_file, vl->name);  // we hope these writes
				write_nt_string(to_file, vl->value); // work, but we don't care.
			}
		}
		vlnext = vl->next;
	}

FREE_LIST:

	if (opfile != NULL)
		fclose(opfile);
	if (otfile != NULL)
		fclose(otfile);

	for (vl = varlist; vl != NULL; vl = vlnext) {
		if (vl->name[0]) {
			pj_free(vl->value);
		}
		vlnext = vl->next;
		pj_free(vl);
	}

	listchangecount = 0;
	varlist  = NULL;
	listnext = NULL;
	return err;
}

static Boolean load_a_file(char *fname)
/*****************************************************************************
 * load vars from one of the var files, return TRUE if anything loaded.
 ****************************************************************************/
{
	Boolean anyloaded = FALSE;
	FILE	*ifile;
	char	vname[MAX_VNAME_LEN];
	char	vvalue[MAX_VVALUE_LEN];

	if (NULL == (ifile = fopen(fname, "rb"))) {
		return FALSE;
	}

	for (;;) {
		if (Success != read_nt_string(ifile, vname, MAX_VNAME_LEN))
			goto ERROR_EXIT;
		if (Success != read_nt_string(ifile, vvalue, MAX_VVALUE_LEN))
			goto ERROR_EXIT;
		if (NULL == new_global_var(vname, vvalue))
			goto ERROR_EXIT;
		anyloaded = TRUE;
	}

ERROR_EXIT:

	fclose(ifile);
	return anyloaded;
}

static Errcode load_global_vars(void)
/*****************************************************************************
 * load the global vars from the temp files into a linked list.
 ****************************************************************************/
{
	Boolean anyloaded;
	Globalv *dmyvar;

	if (permfpath[0] == 0x00) { 	// one time, make perm filepathname.
		make_resource_name(permfname, permfpath);
	}

	if (NULL != varlist)		// should never happen, but if it does,
		dump_global_vars(); 	// this will keep everything in sync.

	anyloaded  = load_a_file(permfpath);		   // load permenant vars
	anyloaded |= load_a_file(tempfpath);		   // load temporary vars

	/*------------------------------------------------------------------------
	 * strange dept:
	 *	if we didn't load anything from the files, we add a dummy variable
	 *	then delete it.  this results in the varlist header pointing to the
	 *	tombstone of the deleted var, which is to say, it is not NULL.	the
	 *	net effect is that we won't try to load the files on every GetVar
	 *	call once we've determined that there's nothing to load.
	 *----------------------------------------------------------------------*/

	if (!anyloaded) {
		if (NULL == (dmyvar = new_global_var("x","x"))) // add a dummy var,
			return Err_no_memory;						// punt on failure.
		pj_free(dmyvar->value); 						// free up dummy value.
		dmyvar->name[0] = 0x00; 						// indicate var deleted.
	}

	return Success;
}

static Errcode po_compact_global_vars(void)
/*****************************************************************************
 * save then reload global vars, po_compacts memory from deleted/modified vars.
 ****************************************************************************/
{
	Errcode err;
	if (Success > (err = dump_global_vars()))
		return err;
	return load_global_vars();
}

static Globalv *find_global_var(char *name)
/*****************************************************************************
 * search the linked list for a variable of a given name.
 ****************************************************************************/
{
	Globalv *vl;
	char	firstchar = name[0];

	for (vl = varlist; vl != NULL; vl = vl->next) {
		if (firstchar == vl->name[0] && 0 == strcmp(name, vl->name))
			return vl;
	}
	return NULL;
}

static Errcode po_gvar_get(Popot name, Popot value)
/*****************************************************************************
 * ErrCode GlobalVarGet(char *name, char *value);
 *	 search for the named variable and if found return its value in the
 *	 specified string.	if not found, the return string is unchanged.
 ****************************************************************************/
{
	Globalv *var;
	Errcode err;

	if (NULL == varlist)
		if (Success != (err = load_global_vars()))
			return err;

	if (NULL == name.pt)
		return builtin_err = Err_null_ref;

	if (NULL == (var = find_global_var(name.pt)))
		return Err_not_found;

	if (Popot_bufcheck(&value, 1+strlen(var->value)))
		return builtin_err;

	strcpy(value.pt, var->value);
	return Success;
}

static Errcode po_gvar_set(Popot name, Popot value)
/*****************************************************************************
 * ErrCode GlobalVarSet(char *name, char *value);
 *	 set the named variable to the specified value.
 ****************************************************************************/
{
	Errcode err;
	Globalv *var;
	char	*newvalue;

	if (NULL == varlist)
		if (Success != (err = load_global_vars()))
			return err;

	if (NULL == name.pt || NULL == value.pt)
		return builtin_err = Err_null_ref;

	if (strlen(name.pt) > MAX_VNAME_LEN || strlen(value.pt) > MAX_VVALUE_LEN)
		return builtin_err = Err_truncated;

	if (NULL == (var = find_global_var(name.pt))) {
		if (NULL == new_global_var(name.pt, value.pt))
			return Err_no_memory;
	} else {
		if (strlen(value.pt) <= strlen(var->value)) {	// copy in place if
			strcpy(var->value, value.pt);				// it fits.
		} else {
			if (NULL == (newvalue = new_value(value.pt)))
				return Err_no_memory;
			pj_free(var->value);
			var->value = newvalue;
		}
	}

	if (++listchangecount > CHANGE_COUNT_LIMIT)
		po_compact_global_vars();
	return Success;
}

static Errcode po_gvar_del(Popot name)
/*****************************************************************************
 * Errcode GlobalVarDelete(char *name);
 *	delete the named variable.
 ****************************************************************************/
{
	Errcode err;
	Globalv *var;

	if (NULL == varlist)
		if (Success != (err = load_global_vars()))
			return err;

	if (name.pt == NULL)
		return builtin_err = Err_null_ref;

	if ((*(char *)name.pt) == 0x00)
		return Err_not_found;		// naughty caller passed an empty string!

	if (NULL == (var = find_global_var(name.pt)))
		return Err_not_found;

	pj_free(var->value);
	var->name[0] = 0x00;
	if (++listchangecount)
		po_compact_global_vars();

	return Success;
}

static Errcode po_gvar_next(Popot nameptr, Popot valueptr)
/*****************************************************************************
 *
 ****************************************************************************/
{
	Globalv *cur = listnext;

	if (cur == NULL)
		return Err_not_found;

	if (NULL == nameptr.pt || NULL == valueptr.pt)
		return builtin_err = Err_null_ref;

	*((Popot *)nameptr.pt)	= po_ptr2ppt(cur->name,0);
	*((Popot *)valueptr.pt) = po_ptr2ppt(cur->value,0);

	listnext = cur->next;
	return Success;
}

static Errcode po_gvar_first(Popot nameptr, Popot valueptr)
/*****************************************************************************
 *
 ****************************************************************************/
{
	Errcode err;

	if (NULL == (listnext = varlist)) {
		if (Success > (err = load_global_vars()))
			return err;
		listnext = varlist;
	}

	return po_gvar_next(nameptr, valueptr);
}

/*----------------------------------------------------------------------------
 * library protos...
 *
 * Maintenance notes:
 *	To add a function to this library, add the pointer and prototype string
 *	TO THE END of the following list.  Then go to POCOLIB.H and add a
 *	corresponding prototype to the library structure typedef therein which
 *	matches the name of the structure below.  When creating the prototype
 *	in POCOLIB.H, remember that all arguments prototyped below as pointers
 *	must be defined as Popot types in the prototype in pocolib.h; any number
 *	of stars in the Poco proto still equate to a Popot with no stars in the
 *	pocolib.h prototype.
 *
 *	DO NOT ADD NEW PROTOTYPES OTHER THAN AT THE END OF AN EXISTING STRUCTURE!
 *	DO NOT DELETE A PROTOTYPE EVER! (The best you could do would be to point
 *	the function to a no-op service routine).  Breaking these rules will
 *	require the recompilation of every POE module in the world.  These
 *	rules apply only to library functions which are visible to POE modules
 *	(ie, most of them).  If a specific typedef name exists in pocolib.h, the
 *	rules apply.  If the protos are coded as a generic array of Lib_proto
 *	structs without an explicit typedef in pocolib.h, the rules do not apply.
 *--------------------------------------------------------------------------*/

PolibGlobalv po_libglobalv = {
po_gvar_get,
	"ErrCode GlobalVarGet(char *name, char *value);",
po_gvar_set,
	"ErrCode GlobalVarSet(char *name, char *value);",
po_gvar_del,
	"ErrCode GlobalVarDelete(char *name);",
po_compact_global_vars,
	"ErrCode GlobalVarFlush(void);",
po_gvar_first,
	"ErrCode GlobalVarFirst(char **name, char **value);",
po_gvar_next,
	"ErrCode GlobalVarNext(char **name, char **value);",
};

Poco_lib po_globalv_lib = {
	NULL,								// -> next
	"Global Variable",                  // Library name
	(Lib_proto *)&po_libglobalv,		// pointer to jumptable/protos
	POLIB_GLOBALV_SIZE, 				// number of functions in library
	NULL,								// library init routine
	dump_global_vars,					// library cleanup routine
	};
