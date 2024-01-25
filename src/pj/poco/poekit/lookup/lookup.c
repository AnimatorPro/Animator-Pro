/*****************************************************************************
 * lookup.c - Demo of using Poco user interface functions from a POE module.
 *
 *	This program will display a scrolling list of all the functions in the
 *	Poco builtin and specified POE libraries.  A double-click on a function
 *	name in the scrolling list will display the complete function prototype.
 *	The scrolling list also contains a type-in field.  You may type a string
 *	into this field and hit ENTER, and a list of function names containing
 *	your string (in any position within the name) will be displayed.  The
 *	type-in string filtering is case-insensitive; typing "bli" will list
 *	"Blit()", "IconBlit()", and "RubLine()".
 *
 *	The parent Poco program, LOOKUP.POC, contains a small stub main()
 *	routine which just calls into this POE module.	All the real work is
 *	done herein.
 *
 *	Associated files:
 *
 *		LOOKUP.POC		- The parent Poco program that invokes this code.
 *		STRFUNCS.ASM	- Some string utilties coded in assembler.
 *		POE.LNK 		- Standard Phar Lap link response file for POEs.
 *		MAKEFILE		- Your typical makefile.
 *
 *	This source code demonstrates several aspects of developing a POE module.
 *	It uses functions and data from a variety of host-provided sources:
 *
 *	  - Poco builtin libraries:
 *			Qtext(), Qlist()
 *
 *	  - SYSLIB host library:
 *			malloc(), free(), strcmp(), etc.
 *
 *	  - The POE interface library:
 *			FindPoe(), ptr2ppt(), builtin_err.
 *
 *	It also demonstrates the gyrations required to manipulate a mixture of
 *	string pointers used in local C code and Popot pointers used by the
 *	builtin library functions.
 ****************************************************************************/

/*----------------------------------------------------------------------------
 * include the usual header files...
 *--------------------------------------------------------------------------*/

#include "errcodes.h"
#include "pocorex.h"
#include "syslib.h"

/*----------------------------------------------------------------------------
 * set up the host libraries we need...
 *--------------------------------------------------------------------------*/

#define HLIB_TYPE_1 AA_POCOLIB
#define HLIB_TYPE_2 AA_SYSLIB
#include <hliblist.h>

/*----------------------------------------------------------------------------
 * prototypes for things that live in strfuncs.asm
 *--------------------------------------------------------------------------*/

extern char *strchr(char *str, char chr);
extern int	stricmp(char *str1, char *str2);
extern char *stristr(char *str, char *substr);

/*----------------------------------------------------------------------------
 * prototypes for a couple 'forward reference' situations in this module
 *--------------------------------------------------------------------------*/

static Errcode do_lookup_dialog(Popot *nameptrs,
								char **protoptrs, int namecount);

extern Lib_proto calls[];	/* not really extern, just need its type known */

/*----------------------------------------------------------------------------
 * some global data...
 *--------------------------------------------------------------------------*/

#define MIN_MAXNAMES	300 	/* the constants are used for parameter */
#define MAX_MAXNAMES   5000 	/* range checking on the parms passed	*/
#define MIN_MAXSTRLEN	 20 	/* in from the parent Poco program. 	*/
#define MAX_MAXSTRLEN	 80

int 	maxnames;				/* max names, passed in from lookup.poc */
int 	maxstrlen;				/* max name length, passed in from lookup.poc */

int 	fullnamecount	= 0;	/* how many names are in the 'full list' arrays */
Popot	*fullnameptrs	= NULL; /* full list of library function names */
char	**fullprotoptrs = NULL; /* full list of library function prototypes */
char	*namestrs		= NULL; /* buffer to hold names distilled from protos */
Popot	*curfullptr;			/* current name pointer when building full list */
char	*curfullstr;			/* current name string when building full list */
char	**curfullproto; 		/* current proto string when building full list */

static int copy_thruchar(char *dest, char *src, char chr)
/*****************************************************************************
 * copy src to dest up through nullterm or specified char.
 ****************************************************************************/
{
	int len = 1;	/* we always copy at least one char */

	for (;;) {
		*dest = *src;
		if (*dest == chr || *dest == '\0')
			return len;
		++dest;
		++src;
		++len;
	}
}

static char *add_spaces(char *dest, int count)
/*****************************************************************************
 * add some spaces to a string.
 ****************************************************************************/
{
	while (count--)
		*dest++ = ' ';

	return dest;
}

static void sort_namelist(void)
/*****************************************************************************
 * sort the full list of names (and associated proto pointers).
 * sort sequence is ascending alpha case-insensitive.
 ****************************************************************************/
{
	int 	i;
	Popot	*nptr;
	char	**pptr;
	Popot	*pt1, *pt2;
	Popot	swap;
	short	swaps;
	int 	space, ct;
	int 	count;

	/*
	 * first copy all the proto pointers into the 'min' elements of the name
	 * Popots array.  this will allow us to sort the Popot array and have
	 * the proto pointers tag along automatically.	after the sort is done,
	 * we'll unload the proto pointers back into their own array, and
	 * restore the 'min' elements to point to the name strings.
	 * (tricky, huh?)
	 */

	nptr = fullnameptrs;
	pptr = fullprotoptrs;
	for (i = 0; i < fullnamecount; ++i) {
		nptr->min = *pptr;
		++nptr;
		++pptr;
	}

	/*
	 * now sort the name pointer Popots using a simple shell sort...
	 */

	count = fullnamecount;
	space = count/2;
	if (count < 2)	/* very short arrays are already sorted */
		goto ALREADY_SORTED;
	--count; /* since we look at two elements at once...*/

	for (;;) {
		do	{
			swaps = 0;
			pt2 = pt1 = fullnameptrs;
			pt2 += space;
			ct = count - space + 1;
			while (--ct >= 0) {
				if (stricmp(pt1->pt, pt2->pt) > 0) {
					swaps = 1;
					swap = *pt1;
					*pt1 = *pt2;
					*pt2 = swap;
				}
				pt1++;
				pt2++;
			}
		} while (swaps);
		if ( (space = space/2) == 0)
			break;
	}

ALREADY_SORTED:

	/*
	 * unload the proto pointers to their own array & restore the min ptrs...
	 */

	nptr = fullnameptrs;
	pptr = fullprotoptrs;
	for (i = 0; i < fullnamecount; ++i) {
		*pptr = nptr->min;
		nptr->min = nptr->pt;
		++nptr;
		++pptr;
	}
}

static void add_name(unsigned char *proto)
/*****************************************************************************
 * add a function name to the name pointers and names strings arrays.
 ****************************************************************************/
{
	int len;
	static Boolean warning_shown = 0;
	static Popot ovflow_warning = {"Maximum of %d functions exceeded.  "
								   "Some functions will not appear in the list."
								  };

	/*
	 * make sure we don't overflow our pointer tables, whine if we do...
	 *
	 * (note that we don't check builtin_err after the Qtext() because the
	 * only thing we can do is return, which we are about to do anyway. our
	 * callers check builtin_err upon our return.)
	 */

	if (fullnamecount >= maxnames) {
		if (!warning_shown) {
			++warning_shown;		  /* prevent repeats of the warning message */
			poeQtext(1, sizeof(int), ovflow_warning, maxnames);
		}
		return; /* don't add name */
	}

	/*
	 * record the location of this function name in the name pointers array,
	 * and the location of the full prototype in the corresponding slot of
	 * the proto pointers array...
	 * (the location of this function name is the current location in the
	 * name strings data area; we are about to format the name into this
	 * location.)
	 */

	curfullptr->pt = curfullptr->min = curfullptr->max = curfullstr;
	*curfullproto  = proto;
	++curfullptr;
	++curfullproto;
	++fullnamecount;

	/*
	 * first skip the leading word (hopefully it will be the return value
	 * type in the proto) and any following whitespace or star chars...
	 * (this will turn "FILE     *fopen()" into "fopen", for example.)
	 */

	while (*proto && *proto != ' ' && *proto != '\t')
		++proto;

	while (*proto && (*proto == '*' || *proto == ' ' || *proto == '\t'))
		++proto;

	/*
	 * copy the function name into the big string buffer.  we'll copy up to
	 * the opening paren, or maxstrlen chars.  following the name, we tack
	 * on empty parens to make it look more like a function name.
	 */

	for (len = 0; *proto && *proto != '(' && len < maxstrlen-3; ++len)
		*curfullstr++ = *proto++;

	*curfullstr++ = '(';
	*curfullstr++ = ')';
	*curfullstr++ = '\0';

}

static void add_library(Lib_proto *plib, int count)
/*****************************************************************************
 * add all the functions from a library to the names/protos lists.
 *
 * note that it is possible for the function pointer in a Lib_proto to be
 * NULL.  this indicates that it isn't really a function prototype, so we
 * just ignore such entries. (er, don't ask why; just trust me on this one.)
 ****************************************************************************/
{
	int i;

	for (i = 0; i < count; ++i, ++plib) {
		if (plib->func != NULL) {
			add_name(plib->proto);
			if (builtin_err)
				return;
		}
	}

}

static Errcode build_function_list(void)
/*****************************************************************************
 * build the lists of all lib function names and protos, sort the lists.
 ****************************************************************************/
{
	Lib_proto *lib;
	int 	   libcount;

	/*
	 * init the global 'cur' vars to indicate empty lists...
	 */

	curfullptr	  = fullnameptrs;
	curfullstr	  = namestrs;
	curfullproto  = fullprotoptrs;
	fullnamecount = 0;

	/*
	 * build the name and proto lists for the builtin library functions...
	 */

	libcount = FindPoe("poco$builtin", &lib);   /* get first builtin lib */
	if (libcount < Success)
		return libcount;

	do	{										/* loop thru builtin libs... */
		add_library(lib, libcount);
		if (builtin_err)
			return builtin_err;
		libcount = FindPoe(NULL, &lib); 		/* get next builtin lib */
	} while (libcount > 0);

	/*
	 * build the name and proto lists for the loaded library functions...
	 * (note that we filter ourselves out of the loaded library listing.)
	 */

	libcount = FindPoe("poco$loaded", &lib);    /* get first loaded lib */
	if (libcount < Success)
		return libcount;

	do {
		if (lib != calls) { 					/* as long as it's not us...*/
			add_library(lib, libcount);
			if (builtin_err)
				return builtin_err;
		}
		libcount = FindPoe(NULL, &lib); 		/* get next loaded lib */
	} while (libcount > 0);

	/*
	 * go sort the names (and associated proto pointers)...
	 */

	sort_namelist();

	return Success;
}

static void show_proto(char *proto)
/*****************************************************************************
 * put up a Qtext() display box with the full function prototype in it.
 ****************************************************************************/
{
	int  len;
	int  indent_count;
	char formatted_proto[1024];
	char *pfmt = formatted_proto;
	char *ptmp;
	static Popot prompt = {"Syntax is:                         \n\n%s\n"};
									/*	^^ this whitespace ^^ is intentional */

	/*
	 * pretty up the proto for display...
	 *	- copy the first word (the return value type, one presumes).
	 *	- insert a single space.
	 *	- skip additional spaces between the return type and function name.
	 *	- copy the function name, up thru the opening paren.
	 *	- if the function has 1 or 2 args, copy the rest of the line as is.
	 *	- if the function has more than 2 args, format them 1-per-line,
	 *	  indented under the name of the function.
	 *
	 *	while formatting, we put a gutter of 2 spaces before and after each
	 *	line of formatted text, to keep the display from jamming up against
	 *	the sides of the box, since that can be kinda ugly.
	 */

	pfmt = add_spaces(pfmt, 2); 					/* nice white gutter.	  */

	while (*proto && *proto != ' ' && *proto != '\t')
		*pfmt++ = *proto++; 						/* copy return type.	  */

	*pfmt++ = ' ';                                  /* add one space.         */

	indent_count = 1 + (pfmt - formatted_proto);	/* remember indent. 	  */

	while (*proto && (*proto == ' ' || *proto == '\t'))
		++proto;									/* skip other spaces.	  */

	if (NULL != (ptmp = strchr(proto, ','))) {      /* if multiple args...    */

		if (NULL == strchr(ptmp+1, ','))            /* if only two args,      */
			goto COPY_FULLINE;						/* do it as one line.	  */

		len = copy_thruchar(pfmt, proto, '(');      /* copy function name     */
		pfmt += len;								/* up thru opening paren. */
		proto += len;
		pfmt = add_spaces(pfmt,2);					/* add trailing gutter.   */
		*pfmt++ = '\n';                             /* args go on later lines.*/
		*pfmt++ = ' ';                              /* extra space looks good.*/

		while (NULL != strchr(proto, ',')) {        /* while more args left...*/
			pfmt = add_spaces(pfmt, indent_count);	/* indent the arg.		  */
			len = copy_thruchar(pfmt, proto, ',');  /* copy arg thru comma.   */
			pfmt += len;
			proto += len;
			pfmt = add_spaces(pfmt,2);				/* add trailing gutter.   */
			*pfmt++ = '\n';                         /* next arg on next line. */
		}
		pfmt = add_spaces(pfmt, indent_count);		/* indent last arg line.  */
	}

COPY_FULLINE:

	while (*proto)									/* copy rest of line.	  */
		*pfmt++ = *proto++;

	pfmt = add_spaces(pfmt,2);						/* add trailing gutter.   */

	*pfmt = '\0';                                   /* nullterm display str.  */

	/*
	 * display string is all formatted, show it to the user...
	 *
	 * note that our callers will check the status of builtin_err; we don't
	 * check it here because the only thing we could do is return, which
	 * we do right after the Qtext call anyway.
	 */

	poeQtext(1,sizeof(char *), prompt, formatted_proto);

}

static Errcode do_sublist_dialog(Popot *nameptrs, char **protoptrs,
								 int namecount, char *substr)
/*****************************************************************************
 * build & display a sublist of functions with names containing the substring.
 ****************************************************************************/
{
	Errcode err;
	Popot	*subnameptrs   = NULL;
	char	**subprotoptrs = NULL;
	Popot	*cursubname;
	char	**cursubproto;
	Popot	*curname;
	char	**curproto;
	int 	subcount;
	int 	i;
	static Popot notfound = {"No function names contain the string\n"
							 " '%s' "
							};

	/*
	 * get a couple new pointer arrays to hold the sublists...
	 * the arrays are full-sized since a worst-case substring could end
	 * up including every function in the full list (like, if the substring
	 * is a space or a paren...)
	 */

	err = Err_no_memory;

	if (NULL == (subnameptrs = zalloc(namecount*sizeof(Popot))))
		goto ERROR_EXIT;
	if (NULL == (subprotoptrs = zalloc(namecount*sizeof(char *))))
		goto ERROR_EXIT;
	subcount = 0;

	/*
	 * filter the full list into the sublist, keeping only items from the
	 * full list which contain the requested substring...
	 *
	 * note that we don't have to sort the sublist since it was extracted
	 * from a sequential walk of the already-sorted full list.
	 */

	curname 	= nameptrs;
	curproto	= protoptrs;
	cursubname	= subnameptrs;
	cursubproto = subprotoptrs;
	for (i = 0; i < namecount; ++i) {
		if (NULL != stristr(curname->pt, substr)) {
			*cursubname  = *curname;
			*cursubproto = *curproto;
			++cursubname;
			++cursubproto;
			++subcount;
		}
		++curname;
		++curproto;
	}

	/*
	 * if the sublist came up empty, whine at the user and exit...
	 */

	if (subcount == 0) {
		poeQtext(1,sizeof(char *), notfound, substr);
		err = builtin_err;	/* in case Qtext croaked for some reason */
		goto ERROR_EXIT;
	}

	/*
	 * if we made it to here, we built a good sublist, recursively invoke
	 * the do_lookup_dialog() routine to display it.  note that we can
	 * recurse to any depth, limited only by available memory for list arrays.
	 * (the recursion is actually pointless, from a functionality standpoint,
	 * but we sort of get it for free, so let's call it a feature!)
	 */

	err = do_lookup_dialog(subnameptrs, subprotoptrs, subcount);

ERROR_EXIT:

	if (subnameptrs != NULL)
		free(subnameptrs);
	if (subprotoptrs != NULL)
		free(subprotoptrs);

	return err;

}

 Errcode do_lookup_dialog(Popot *nameptrs, char **protoptrs, int namecount)
/*****************************************************************************
 * drive the display of and interaction with a function list.
 ****************************************************************************/
{
	Errcode err;
	Boolean ctinue;
	int 	choice;
	int 	lastpos = 0;
	char	choicestr[MAX_MAXSTRLEN];
	Popot	ppt_choicestr	= array2ppt(choicestr);
	Popot	ppt_nameptrs	= ptr2ppt(nameptrs,   sizeof(Popot)*namecount);
	Popot	ppt_choice		= var2ppt(choice);
	Popot	ppt_lastpos 	= var2ppt(lastpos);
	Popot	ppt_prompt		= str2ppt("Select Library Function to Look Up:");

	/*
	 * dialog loop...
	 * - show the list of functions
	 * - if the user picks CANCEL, exit the loop.
	 * - if the user selects a function from the list, call the
	 *	 format-and-display routine.
	 * - if the user types in a string which is not in the list, call the
	 *	 sublist dialog routine.
	 */

	for (;;) {

		/*
		 * start with a clean type-in string; display the list...
		 */

		choicestr[0] = '\0';
		ctinue = poeQlist(ppt_choicestr, ppt_choice,
						  ppt_nameptrs, namecount,
						  ppt_lastpos, ppt_prompt);

		/*
		 * if a builtin_error occurred, return it to our caller.  if the user
		 * picked CANCEL, exit the loop.
		 */

		if (builtin_err != Success)
			return builtin_err;

		if (!ctinue)
			break;

		/*
		 * if a list item was selected, format and display it, else go build
		 * and display a sublist based on the string the user typed in.
		 *
		 * (if we get a non-match indicated by choice < 0, but we didn't
		 * get a CANCEL status from the Qlist call, we must have a case
		 * where the user typed in something not in the list and hit ENTER.)
		 *
		 * if we got a non-match, and the type-in string is empty, the user
		 * just hit ENTER without typing anything.	in this case, treat it
		 * like a CANCEL.
		 *
		 * we check builtin_err upon return from the display or sublist
		 * dialogs, and bail out if necessary.
		 */

		if (choice >= 0) {
			show_proto(protoptrs[choice]);
			break;
		} else {
			if (choicestr[0] == '\0')
				break;
			if (Success != (err = do_sublist_dialog(nameptrs, protoptrs,
													namecount, choicestr)))
				return err;
		}

		if (builtin_err != Success)
			return builtin_err;
	}

	return Success;
}

Errcode poe_main(int mnames, int mlen)
/*****************************************************************************
 * allocate full-list pointer and string arrays, build list, invoke dialogs.
 ****************************************************************************/
{
	Errcode err;

	/*
	 * validate the values passed in from the poco program and assign them
	 * to the corresponding global vars...
	 */

	if (mnames < MIN_MAXNAMES || mnames > MAX_MAXNAMES)
		return builtin_err = Err_parameter_range;
	if (mlen < MIN_MAXSTRLEN || mlen > MAX_MAXSTRLEN)
		return builtin_err = Err_parameter_range;

	maxnames  = mnames;
	maxstrlen = mlen;

	/*
	 * allocate the string and pointer arrays...
	 */

	err = Err_no_memory;

	if (NULL == (fullnameptrs = zalloc(maxnames*sizeof(Popot))))
		goto ERROR_EXIT;
	if (NULL == (namestrs = malloc(maxnames*maxstrlen)))
		goto ERROR_EXIT;
	if (NULL == (fullprotoptrs = zalloc(maxnames*sizeof(char *))))
		goto ERROR_EXIT;

	if (Success != (err = build_function_list()))
		goto ERROR_EXIT;

	err = do_lookup_dialog(fullnameptrs, fullprotoptrs, fullnamecount);

ERROR_EXIT:

	if (fullnameptrs != NULL)
		free(fullnameptrs);
	if (namestrs != NULL)
		free(namestrs);
	if (fullprotoptrs != NULL)
		free(fullprotoptrs);

	return err;
}

/*----------------------------------------------------------------------------
 * Setup rexlib/pocorex interface structures...
 *--------------------------------------------------------------------------*/

static Lib_proto calls[] = {
	{ poe_main, "ErrCode __lookup__poemain(int maxnames, int maxnamelen);"},
};

Setup_Pocorex(NOFUNC,NOFUNC,"Library Lookup Utility", calls);

