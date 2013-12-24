/*****************************************************************************
 *
 * pp.c - C preprocessor.
 *
 *	08/25/90	(Ian)
 *				Changed all occurances of freemem() and gentle_freemem() to
 *				po_freemem() and poc_gentle_freemem().
 *	09/05/90	(Ian)
 *				Rewrote the prep_line() routine, and added a few support
 *				routines for it, including pp_define().
 *				Also, added support for #error, and made #line a see-and-
 *				ignore directive.
 *	09/06/90	(Ian)
 *				Added support for ANSI builtin macros (__TIME__, etc).
 *				Also added __POCO__ and NULL.  NULL is a strange case: it's
 *				not really a macro, and we don't substitute anything for it,
 *				but by having it defined as TSFL_ISBUILTIN, we ensure that a
 *				user can't #undef or redefine it (which would break Poco.)
 *	09/07/90	(Ian)
 *				Added support for #if.
 *	09/08/90	(Ian)
 *				Added support for #elif, extra #if/#elif/#else error checking.
 *	09/19/90	(Ian)
 *				Fixed a bug where a number such as 0x80 could get mistaken
 *				for a #defined symbol with the name x80 during expansion.
 *				Fixed a bug in pp_define() which was causing a symbol with
 *				a null value to contain garbage (eg, #define xyz\n wasn't
 *				always working right.)
 *	10/02/90	(Ian)
 *				Fixed bug in detecting EOF inside conditional.	We used to
 *				complain if we were out_of_it when EOF was detected.  Now we
 *				complain when ifdef_stack is not NULL at EOF, since this is
 *				the acurate indicator of whether #endif's are still needed.
 *	10/05/90	(Ian)
 *				Converted sprintf/pp_say_fatal pairs to pp_say_fatal w/formatting.
 *	10/21/90	(Ian)
 *				Added logic to po_pp_next_line() to get a line from the library
 *				protos if such is available, or from the input stream if not.
 *				See comment block for po_pp_next_line() for more details.
 *	10/25/90	(Ian)
 *				Added pp_pragma() routine, and new_file_stack_entry() routine,
 *				plus converted things so that the File_stack structure is
 *				shared by both files and builtin libraries.  Added pragma
 *				keywords 'macrosub' and 'nomacrosub', and logic in the
 *				pp_next_line() routine to feed lines to the macro substitutor
 *				or not, accordingly (basically rewrote pp_next_line()). Also,
 *				added handling of #pragma poco library <libname>, but we
 *				still need a function on the pj side to load libraries for us.
 *	10/28/90	(Ian)
 *				Added #pragma echo command, only when DEVELOPMENT is defined,
 *				so that our test/verification programs can talk to us.
 *	10/30/90	(Ian)
 *				Fixed a glitch in prev_file_stack() that was causing spurious
 *				EOF in #if errors.
 *	12/12/90	(Ian)
 *				Changed syntax of #pragma echo to #pragma poco echo. It still
 *				only works if DEVELOPMENT is #define'd.
 *				Also, removed 'strlwr()' call at head of #pragma handling
 *				routine; for echo, we don't want to lowercase the whole line
 *				because it screws our autotesting rexx program (besides, it
 *				just wasn't a smart thing to do.)  Now the stricmp() case-
 *				insensitive compare function is used to check the keywords,
 *				and the non-keyword part of the line is left alone.
 *	12/14/90	(Ian)
 *				Added #pragma poco stacksize NNNN.
 *	02/07/91	(Ian)
 *				Added new builtin macros:
 *					Array_els()
 *					PATH_SIZE
 *	03/20/91	(Ian)
 *				Changed logic for searching for #include files so that it can
 *				also be used for searching for library (poe) files.  Also,
 *				our host now passes in pathnames which include the trailing
 *				backslash.	As a side effect, this fixed a (previously
 *				unrealized) bug in which a #include with a mega-long filename
 *				would have overwritten the stack and crashed us.
 *	04/03/91	(Ian)
 *				Fixed bug in pp_count_qstring and pp_copy_qstring that would
 *				cause a glitch on a string such as "\\", the 2nd backslash
 *				would be seen (incorrectly) as escaping the string delimiter.
 *	04/14/91	(Peter)
 *				Re did pp_pragma() to handle more complex syntax and added an
 *				optional id_string to the #pragma library entry. renamed
 *				pp_chop_filename() to pp_chop_string() and added arg optionally
 *				to allow "<>" delimiters for include names.
 *	05/01/91	(Ian)
 *				Added code to handle #pragma poco eof.	When this pragma
 *				is encountered, we pretend we found the normal EOF at that
 *				point.	This allows massive comment blocks at the end of a
 *				program with absolutely no compile-time performance hit.
 *	08/17/92	(Ian)
 *				Changed pp_say_fatal() so that it sets the new global error
 *				line number (in the pcb) to the preprocessor's idea of the
 *				current line number.  This makes reporting of errors in
 *				preprocessing report the correct line number, since otherwise
 *				the line number comes from "the current token", a concept that
 *				means nothing when handling preprocessor statements.
 *	09/14/92	(Ian)
 *				Fixed pp_say_fatal() to check for t.file_stack being NULL.
 ****************************************************************************/

#include <time.h>
#include <string.h>
#include <ctype.h>
#include "poco.h"
#include "stdtypes.h"
#include "linklist.h"
#include "filepath.h"

/*----------------------------------------------------------------------------
 * Some macros...
 *--------------------------------------------------------------------------*/

#ifndef iscsymf
  #define iscsymf(x) (isalpha(x) || (x) == '_')
#endif

#ifndef iscsym
  #define iscsym(x)  (isalnum(x) || (x) == '_')
#endif

#define TOKEN_MAX		SZTOKE
#define LINE_MAX		SZTOKE
#define MAX_MACRO_PARMS 32
#define RECURSE_LIMIT	10000
#define free_string(s)	if (s != NULL) po_freemem(s)

/*----------------------------------------------------------------------------
 * error message literals...
 *--------------------------------------------------------------------------*/

static Poco_cb *ppcb; /* Used by pp_fatal to hide pcb to keep ppeval generic. */

static char recurse_detected[]	= "infinite loop detected during macro substitution";

static char macro_overflow[]	= "buffer size exceeded during macro expansion";
static char macro_oneline[] 	= "end of line found before end of parameters for macro";
static char macro_parmexceed[]	= "maximum number of macro parameters exceeded";
static char macro_name[]		= "name of macro";
static char macro_expect_name[] = "expecting name of macro";
static char macro_parmname[]	= "name of macro parameter";
static char macro_needparm[]	= "expecting parameter(s) for macro %s";
static char macro_redefined[]	= "macro redefined with non-identical value";
static char macro_builtin[] 	= "a builtin macro cannot be un-defined";
static char macro_toomany[] 	= "too many";
static char macro_toofew[]		= "not enough";
static char macro_parmcount[]	= "%s parameters for macro %s";
static char comma_or_rparen[]	= "comma or )";

static char incl_name_missing[] = "missing or malformed name of file for #include";
static char incl_open[] 		= "can't open source file %s";

static char pragma_unknown[]	= "'%s' is not a valid poco pragma";
static char lib_name_missing[]	= "missing or malformed filename for library pragma";
static char lib_cant_find[] 	= "can't find POE library module %s";
static char lib_open_failed[]	= "can't load POE library module %s";
static char stksz_value_bad[]	= "stacksize value must be in kbytes, between 4 and 64";
static char unexpected_tok[]	= "Unexpected \"%s\"";
static char unexpected_eol[]	= "Unexpected end of line.";
static char if_defined_syntax[] = "syntax error in '#if defined' statement";

static char ppcmd_unknown[] 	= "unknown preprocessor command '%s'";

static char forced_fatal[]		= "fatal error forced by #error directive...";

static char else_unmatched[]	= "#else/#elif without preceeding #if/#ifdef";
static char endif_unmatched[]	= "#endif without preceeding #if/#ifdef";
static char else_multiple[] 	= "only one #else is allowed per #if/#ifdef";
static char elif_after_else[]	= "#elif cannot follow a #else directive";

static char eof_in_conditional[]= "EOF inside #if/#ifdef";
static char eol_in_conditional[]= "end of library prototypes inside #if/#ifdef";

void pp_say_fatal(char *fmt, ...)
/*****************************************************************************
 * complain about something and die.
 * this is used by all parts of the preprocessor, instead of calling
 * po_say_fatal() directly.  this lets us set the error line number to the
 * proper line, instead of taking it from curtoken.
 ****************************************************************************/
{
	char	sbuf[512];
	va_list args;

	va_start(args, fmt);
	vsprintf(sbuf, fmt, args);
	va_end(args);

	if (ppcb->t.file_stack != NULL) {
		ppcb->error_line_number = ppcb->t.file_stack->line_count;
	}
	po_say_fatal(ppcb, sbuf);
}

static char *pp_findfile(Names *idirs, char *fname)
/*****************************************************************************
 * search include paths for file, return pointer to combined path/filename.
 *	note that the host which passes us the list of include directories has
 *	total control over the search path(s), even to the degree of preventing
 *	the inclusion of drive/path spec in the name specified in the poco code.
 *	if the host will allow device/path names to be specified, a path in the
 *	include list MUST be a null string ("") (this would typically be the
 *	first path in the list).  if no paths are a null string, all file searches
 *	will be rooted in the dirs specified in the include list.
 ****************************************************************************/
{
	static char path[PATH_SIZE];
	FILE		*f;
	int 		namelen;

	if (0 == (namelen = strlen(fname))) 	/* naughty naughty user...	  */
		return NULL;						/* ...can't fool us that easy */

	while (idirs != NULL)
		{
		if (namelen + strlen(idirs->name) < PATH_SIZE)
			{
			sprintf(path, "%s%s", idirs->name, fname);
			if (NULL != (f = fopen(path, "r")))
				{
				fclose(f);
				return path;
				}
			}
		idirs = idirs->next;
		}

	return NULL;
}

static Text_symbol *pp_in_hash_list(char *word, Text_symbol **list)
/*****************************************************************************
 * find symbol in symbol table.
 ****************************************************************************/
{
Text_symbol *ts;

ts = list[po_hashfunc(word)];
while (ts != NULL)
	{
	if (ts->name[0] == *word)	/* quick-test 1st char before strcmp call */
		if (po_eqstrcmp(ts->name, word) == 0)
			return(ts);
	ts = ts->next;
	}
return NULL;
}

static void add_to_hash(Text_symbol *hash, Text_symbol **table)
/*****************************************************************************
 * add a symbol to the symbol table.
 ****************************************************************************/
{
table += po_hashfunc(hash->name);
hash->next = *table;
*table = hash;
}

static void pp_add_new_ts(Poco_cb *pcb, char *name,
							char *value, SHORT pcount, Ts_flags flags)
/*****************************************************************************
 * allocate a new Text_symbol, fill in the fields, add to hash table.
 ****************************************************************************/
{
register Text_symbol *ts;

ts = po_memalloc(pcb, sizeof(Text_symbol));
ts->name	  = name;
ts->value	  = value;
ts->parmcount = pcount;
ts->flags	  = flags;
add_to_hash(ts, pcb->t.define_list);
}

static void free_a_ts(Text_symbol *ts)
/*****************************************************************************
 * free a text_symbol structure, and its attachments.
 ****************************************************************************/
{
if (!(ts->flags & TSFL_ISBUILTIN))	/* The name and value strings for		*/
	{								/* builtins are not dynamically alloc'd.*/
	free_string(ts->name);
	free_string(ts->value);
	}
po_freemem(ts);
}

static void unhash(char *name, Text_symbol **table)
/*****************************************************************************
 * remove a name from the symbol table.
 ****************************************************************************/
{
register struct text_symbol *hash, *last;

table += po_hashfunc(name);
hash = *table;
if (hash == NULL)
	return;
if (po_eqstrcmp(hash->name, name) == 0)
	{
	*table = hash->next;
	free_a_ts(hash);
	return;
	}
last = hash;
while ((hash = last->next) != NULL)
	{
	if (po_eqstrcmp(hash->name, name) == 0)
		{
		last->next = hash->next;
		free_a_ts(hash);
		return;
		}
	last = hash;
	}
}


static void free_hash_list(register struct text_symbol **hs)
/*****************************************************************************
 * free everything in the symbol table, by walking the hash table & its links.
 ****************************************************************************/
{
register struct text_symbol *ts, *next;
register int i;

i = HASH_SIZE;
while (--i >= 0)
	{
	next = *hs;
	while ((ts = next) != NULL)
		{
		next = ts->next;
		free_a_ts(ts);
		}
	*hs++ = NULL;
	}
}

static void new_file_stack_entry(Poco_cb *pcb, void *fp, char *fname, Fsflags flags)
/*****************************************************************************
 *
 ****************************************************************************/
{
File_stack *new_filep;

new_filep = po_memalloc(pcb, sizeof(File_stack));

new_filep->source.file = fp;
new_filep->name 	   = po_clone_string(pcb, fname);
new_filep->line_count  = 0;
new_filep->flags	   = flags;
new_filep->pred 	   = pcb->t.file_stack;
pcb->t.file_stack	   = new_filep;

}

static void prev_file_stack_entry(Poco_cb *pcb)
/*****************************************************************************
 *
 ****************************************************************************/
{
File_stack *fs = pcb->t.file_stack;


if (fs->flags & FSF_ISFILE)
	fclose(fs->source.file);

free_string(fs->name);
pcb->t.file_stack = fs->pred;
po_freemem(fs);

fs = pcb->t.file_stack;

if (fs == NULL && pcb->t.ifdef_stack != NULL)
	{
	if (fs->flags & FSF_ISFILE)
		pp_say_fatal(eof_in_conditional);
	else
		pp_say_fatal(eol_in_conditional);
	}
}

static char *pp_strtrim(char *line)
/*****************************************************************************
 * trim leading and trailing whitespace from a sequence of characters.
 ****************************************************************************/
{
	char *first;
	char *last = NULL;

	while (isspace(*line))
		++line;
	if (*line == '\0')
		return line;
	first = line;
	while (*line)
		{
		if (!isspace(*line))
			last = line;
		++line;
		}
	*++last = '\0';
	return first;
}

static SHORT pp_in_parm_array(char **parms, SHORT pcount, char *word)
/*****************************************************************************
 * search an array of pointers to strings for a matching string.
 ****************************************************************************/
{
	int i;

	for(i = 1; i <= pcount; i++)
		{
		if (*parms[i] == *word)
			if (0 == po_eqstrcmp(parms[i], word))
				return i;
		}
	return 0;
}

static SHORT pp_count_qstring(Poco_cb *pcb, char *in, char delim)
/*****************************************************************************
 * count chars in quoted string, escaped quotes are handled.
 * the return value is the length of the string, including the closing delim.
 ****************************************************************************/
{
register SHORT	c;
register SHORT	lastc  = 0;
register SHORT	count  = 0;

for (;;)
	{
	++count;
	c = *in++;
	if (c == '\\' && lastc == '\\') /* don't let a slash-slash-delim screw us:*/
		lastc = ~c; 				/* make 2nd slash invisible when checking */
	else if (c == delim && lastc != '\\')      /* the delim next time around. */
		break;
	else lastc = c;
	}
return count;
}

static void pp_copy_qstring(Poco_cb *pcb, char **out, char **in, char delim)
/*****************************************************************************
 * copy quoted string thru terminating quote, escaped quotes are handled.
 ****************************************************************************/
{
char			*inptr	= *in;
char			*outptr = *out;
register SHORT	c;
register SHORT	lastc = 0;

for (;;)
	{
	*outptr++ = c = *inptr++;
	if (c == 0)
		pp_say_fatal(macro_oneline);
	if (c == '\\' && lastc == '\\') /* don't let a slash-slash-delim screw us:*/
		lastc = ~c; 				/* make 2nd slash invisible when checking */
	else if (c == delim && lastc != '\\')      /* the delim next time around. */
		break;
	else lastc = c;
	}
*in  = inptr;
*out = outptr;
}

static void pp_copy_pstring(Poco_cb *pcb, char **out, char **in)
/*****************************************************************************
 * copy parenthesized string thru terminating paren, nested parens are handled.
 ****************************************************************************/
{
char			*inptr	= *in;
char			*outptr = *out;
register SHORT	c;
register SHORT	pcount = 0;

for (;;)
	{
	*outptr++ = c = *inptr++;
	if (c == 0)
		pp_say_fatal(macro_oneline);
	if (c == '(')
		++pcount;
	else if (c == ')')
		{
		if (--pcount == 0)
			break;
		}
	}
*in  = inptr;
*out = outptr;
}

static char *pp_join_strings(char *dest, char *s)
/*****************************************************************************
 * concatenate two strings, don't null-term the destination.
 ****************************************************************************/
{
register char c;

while ((c = *s++) != 0)
	*dest++ = c;
return dest;
}

static char *pp_expand(Poco_cb		 *pcb,
					  register char  *outbuf,
					  char			 *savbuf,
					  Text_symbol	 *ts,
					  register SHORT bufrspace,
					  SHORT 		 wordlen)
/*****************************************************************************
 * perform macro substitution.
 ****************************************************************************/
{
char	*rv;
char	*line;
char	*template;
PPToken pptok;

rv = line = outbuf;
template = ts->value;

if (ts->flags & TSFL_ISSPECIAL) 	/* If it requires special handling...	*/
	{
	time_t	timenow;				/* time value return by time()			*/
	char	*ctimestr;				/* -> string returned by ctime()		*/
	char	dtlbuf[PATH_SIZE];		/* buffer for filename/date/time/line#	*/

	pptok = *template;
	template = dtlbuf;

	switch (pptok)
		{
		case PPTOK_SFILE:
			sprintf(dtlbuf, "\"%s\"", pcb->t.file_stack->name);
			goto SIMPLE_SUBSTITUTE;

		case PPTOK_SLINE:
			sprintf(dtlbuf, "%ld", pcb->t.file_stack->line_count);
			goto SIMPLE_SUBSTITUTE;

		case PPTOK_SDATE:
		case PPTOK_STIME:
			timenow  = time(NULL);
			ctimestr = ctime(&timenow);
			if (pptok == PPTOK_STIME)
				sprintf(dtlbuf, "\"%8.8s\"", &ctimestr[11]);
			else
				sprintf(dtlbuf, "\"%6.6s %4.4s\"", &ctimestr[4], &ctimestr[20]);
			goto SIMPLE_SUBSTITUTE;

		case PPTOK_SNULL:
			rv += wordlen;	/* NULL is a no-op, basically */
			break;
		}
	}

else if (ts->flags & TSFL_HASPARMS) 	/* If macro has parameters...		*/
	{
	register SHORT	c;
	register SHORT	lastc;
	char			*parmbuf;
	char			*parmwrk;
	char			*parms[MAX_MACRO_PARMS];
	char			*thisparm;
	SHORT			pcount;

	/*
	 * aquire a buffer to hold the parameter values.  note that a smallblk
	 * buffer is plenty...it is 512 bytes, which is also the max line length,
	 * so we know all the parms can fit in the buffer, and we do no checking
	 * as we copy the parms into it.
	 *
	 * ANSI allows whitespace between the macro name and the opening paren,
	 * skip any intervening space, then ensure an opening paren is present.
	 */
	parmbuf = po_cache_malloc(pcb, &pcb->smallblk_cache);

	line = po_skip_space(line+wordlen);
	if (line == NULL || *line != '(')
		{
		pp_say_fatal(macro_needparm, ts->name);
		}
	else
		++line;

	/*
	 * gather the parameter values from the macro invokation.
	 * we just copy the parms from the input line (which will later be the
	 * output line) into the parm buffer, one after another.  we save a
	 * pointer to each parameter value in the buffer into an array, for use
	 * during the actual substitution.
	 */
	pcount	 = 0;
	thisparm = parmbuf;
	parmwrk  = parmbuf;

	if (ts->parmcount == 0) {	// if it's a macro that 'has parms', but really
		if (*line++ == ')')     // just has empty parens, then skip all the
			goto ENDOFPARMS;	// parm-gathering groodah.
		else
			pp_say_fatal(macro_parmcount, macro_toomany);
	}

	for (;;)
		{
		switch (*line)
			{
			case '\0':
				pp_say_fatal(macro_oneline);
				break;
			case '"':
				pp_copy_qstring(pcb, &parmwrk, &line, '"');
				break;
			case '\'':
				pp_copy_qstring(pcb, &parmwrk, &line, '\'');
				break;
			case '(':
				pp_copy_pstring(pcb, &parmwrk, &line);
				break;
			case ')':
			case ',':
				*parmwrk++ = '\0';
				if (++pcount >= MAX_MACRO_PARMS)
					pp_say_fatal(macro_parmcount, macro_toomany);
				parms[pcount] = pp_strtrim(thisparm);
				thisparm = parmwrk;
				if (*line++ == ')')
					goto ENDOFPARMS;
				break;
			default:
				*parmwrk++ = *line++;
				break;
			}
		}

ENDOFPARMS:

	/*
	 * make sure we got the right number of parameters.
	 *
	 * save the portion of the line that remains after the closing paren of
	 * the macro, so that we can splice it back on after the expansion.
	 */
	if (pcount != ts->parmcount)
		{
		pp_say_fatal(macro_parmcount,
				( (pcount < ts->parmcount) ? macro_toofew : macro_toomany),
				ts->name);
		}

	strcpy(savbuf, line);

	/*
	 * run through the expansion template, plugging in parameter values
	 * as we go.  this routine uses the 'magic values' placed into the
	 * template string during #define processing.  a value of 0x01 in the
	 * string indicates that the next byte in the string is the index number
	 * of a parm value to be substituted; the 0x01 and index byte are
	 * replaced by the text of the parm specified in the invokation.  a value
	 * of 0x02 is similar, except that it indicates that the parm value must
	 * be quoted as it is copied (ie, ANSI '#' stringizing).
	 *
	 * HACK ALERT!
	 *	note that there is a major shortcut here in checking for buffer
	 *	overflow:  if we do overflow the output buffer, it will overflow
	 *	into the save buffer; not a big deal.  after processing a logical
	 *	chunk of info (a parm, or all the template stuff between two parms)
	 *	we check for overflow and die if need be.
	 *	this whole concept works because the two buffers involved are
	 *	pcb->t.line_b1 and line_b2, which are adjacent (and must remain so).
	 */
	for (;;)
		{
		while ((c = *template++) > PPTOK_PARMQ)
			{
			*outbuf++ = c;				/* Copy template text between parms */
			--bufrspace;
			}

		if (bufrspace <= 0) 			/* If overflow, die */
			pp_say_fatal(macro_overflow);

		if (c == 0) 					/* If end of template, exit loop. */
			break;

		else if (c == PPTOK_PARMN)		/* Normal parameter substitution */
			{
			thisparm = parms[*template++];
			while (0 != (c = *thisparm++))
				{
				*outbuf++ = c;
				--bufrspace;
				}
			}

		else if (c == PPTOK_PARMQ)		/* Stringized parameter substitution*/
			{
			*outbuf++ = '"';
			--bufrspace;
			thisparm = parms[*template++];
			lastc = 0;
			while (0 != (c = *thisparm++))
				{
				if ((c == '"' || c == '\\') && lastc != '\\')
					{
					*outbuf++ = '\\';
					--bufrspace;
					}
				*outbuf++ = lastc = c;
				--bufrspace;
				}
			*outbuf++ = '"';
			--bufrspace;
			}
		if (bufrspace <= 0)
			pp_say_fatal(macro_overflow);
		}

	/*
	 * make sure the substitution left us enough room to put the tail back.
	 */
	if (bufrspace < strlen(savbuf))
		pp_say_fatal(macro_overflow);
	strcpy(outbuf, savbuf);

	po_freemem(parmbuf);
	}

else	/* Simple macro (no parms)... */
	{
	register SHORT diff;
	register SHORT len;

SIMPLE_SUBSTITUTE:

	/*
	 * for a macro without parameters, processing is pretty simple...
	 * if the replacement value is shorter than the original macro name,
	 * we just plug the replacement text over the top of the macro name in
	 * the line, replacing any extra chars with spaces.
	 * if the replacement value is longer than the original name, we save
	 * the tail of the line, plug in the replacement value, then splice on
	 * the tail we saved.
	 */
	len = strlen(template); 		/* Subtract len of token we're replacing*/
	diff = len - wordlen;			/* from length of replacement value.	*/
	if (diff <= 0)					/* If replacement will fit in-place...	*/
		{							/* copy replacement over top of original*/
		line = pp_join_strings(line, template);
		while (diff++ < 0)			/* Wipe out rest of original token with */
			*line++ = ' ';          /* spaces. (Is this cheating or what?)  */
		}
	else							/* It's simple, but won't fit in-place. */
		{
		if ((bufrspace - diff) < 0) /* Make sure we won't overflow buffer.  */
			pp_say_fatal(macro_overflow);
		strcpy(savbuf, line+wordlen);			  /* Save input line tail,	*/
		line = pp_join_strings(line, template);   /* put new value in line, */
		strcpy(line, savbuf);					  /* copy tail after new.	*/
		}
	}

return rv;
}


static UBYTE *prep_line(Poco_cb *pcb, UBYTE *line_buf, UBYTE *word_buf, int bsize)
/*****************************************************************************
 * drive macro substitution on a line before it gets returned to the tokenizer.
 ****************************************************************************/
{
register UBYTE	c;
SHORT			ttype;
SHORT			bufrspace = bsize - 2;
SHORT			len;
USHORT			loopcount = 0;
register UBYTE	 *in	   = line_buf;
UBYTE			 *nxtchr;
Text_symbol 	*ts;

for (;;)
	{
	if (++loopcount > RECURSE_LIMIT)
		pp_say_fatal(recurse_detected);
	if ('\0' == (c = *in++))
		break;
	if (c == '\'' || c == '"')
		{
		len = pp_count_qstring(pcb, in, c);
		bufrspace -= len;
		in += len;
		}
	else if (c == '0' && (*in == 'x' || *in == 'X'))
		{
		++in;			/* don't let 0x... get mixed up with a #defined     */
		bufrspace -=2;	/* symbol with the name 'x...' !                    */
		}
	else if (!iscsymf(c))
		{
		--bufrspace;
		}
	else
		{
		nxtchr = tokenize_word(--in, word_buf, NULL, NULL, &ttype, TRUE);
		if (NULL != (ts = pp_in_hash_list(word_buf, pcb->t.define_list)))
			{

#ifdef DEBUG_PP
			printf("b: %s\n", pcb->t.line_b1);
			nxtchr = pp_expand(pcb, in, word_buf, ts,
								bufrspace, strlen(word_buf));
			printf("a: %s\n", pcb->t.line_b1);
#else
			nxtchr = pp_expand(pcb, in, word_buf, ts,
								bufrspace, strlen(word_buf));
#endif
			}
		bufrspace -= nxtchr - in;
		in = nxtchr;
		}
	}
return line_buf;
}


static void pp_ifdef(Poco_cb *pcb, char *line, char *word_buf, SHORT positive)
/*****************************************************************************
 * process #ifdef or #ifndef line.
 ****************************************************************************/
{
register Conditional *con;
SHORT				 ttype;

if (NULL == (line = tokenize_word(line, word_buf, NULL, NULL, &ttype,TRUE)) ||
	ttype != TOK_UNDEF)
	{
	po_expecting_got_str(pcb, macro_name, word_buf);
	}

con = po_memalloc(pcb, sizeof(Conditional));
if (NULL != pp_in_hash_list(word_buf, pcb->t.define_list))
	con->state = positive;
else
	con->state = !positive;
if (!con->state)
	pcb->t.out_of_it++;
con->next = pcb->t.ifdef_stack;
pcb->t.ifdef_stack = con;
}

static Boolean pp_if(Poco_cb *pcb, char *line, char *word_buf)
/*****************************************************************************
 * process #if line - returns TRUE/FALSE, depending on expression evaluation.
 ****************************************************************************/
{
register SHORT	c;
SHORT			len;
SHORT			ttype;
USHORT			loopcount = 0;
SHORT			bufrspace = LINE_MAX -2;
register char	*in 	  = line;
char			*nxtchr;
Text_symbol 	*ts;

/*
 * first, we prepare the line for evaluation.
 * the following loop is similar to the loop in prep_line(), except that
 * we handle defined and defined(), and we replaced any TOK_UNDEF items
 * that are not macros with '0'.
 */
for (;;)
	{
	if (++loopcount > RECURSE_LIMIT)
		pp_say_fatal(recurse_detected);
	if ('\0' == (c = *in++))
		break;
	if (c == '\'' || c == '"')
		{
		len = pp_count_qstring(pcb, in, c);
		bufrspace -= len;
		in += len;
		}
	else if (c == '0' && (*in == 'x' || *in == 'X'))
		{
		++in;			/* don't let 0x... get mixed up with a #defined     */
		bufrspace -=2;	/* symbol with the name 'x...' !                    */
		}
	else if (!iscsymf(c))
		{
		--bufrspace;
		}
	else
		{
		nxtchr = tokenize_word(--in, word_buf, NULL, NULL, &ttype, TRUE);
		len = strlen(word_buf);
		if (NULL == (ts = pp_in_hash_list(word_buf, pcb->t.define_list)))
			{
			if (0 == po_eqstrcmp(word_buf, "defined"))
				{
				if (NULL == (nxtchr = tokenize_word(nxtchr, word_buf, NULL, NULL, &ttype, TRUE)))
					pp_say_fatal(if_defined_syntax);
				if (ttype == '(')
					{
					if (NULL == (nxtchr = tokenize_word(nxtchr, word_buf, NULL,
														NULL, &ttype, TRUE)))
						pp_say_fatal(if_defined_syntax);
					if (NULL == (nxtchr = po_skip_space(nxtchr)) ||
						*nxtchr != ')')
						pp_say_fatal(if_defined_syntax);
					else
						++nxtchr;
					}
				len = nxtchr - in;
				poco_stuff_bytes(in, ' ', len);
				ts = pp_in_hash_list(word_buf, pcb->t.define_list);
				*in = (ts == NULL) ? '0' : '1';
				}
			else
				{
				poco_stuff_bytes(in, ' ', len);               /* Unknown identifier,  */
				*in = '0';                          /* replace with 0       */
				}
			}
		else
			{
			nxtchr = pp_expand(pcb, in, word_buf, ts, bufrspace, len);
			}
		bufrspace -= nxtchr - in;
		in = nxtchr;
		}
	}

return po_pp_eval(line, word_buf);
}


static void pp_define(Poco_cb *pcb, char *line, char *wrkbuf)
/*****************************************************************************
 * process #define line.
 ****************************************************************************/
{
	SHORT		ttype;
	SHORT		parmnum;
	SHORT		pcount = 0;
	Ts_flags	flags  = 0;
	char		*name;
	char		*parms[MAX_MACRO_PARMS];
	char		*value;
	char		*sharp;
	Text_symbol *old;

	/*
	 * isolate the name of the macro and validate it...
	 */
	line = tokenize_word(line, wrkbuf, NULL, NULL, &ttype, TRUE);
	if (line == NULL || ttype != TOK_UNDEF)
		po_expecting_got_str(pcb, macro_name, wrkbuf);
	name = wrkbuf;
	wrkbuf += 1 + strlen(name);

	/*
	 * if there are parameters, tokenize them and store pointers to
	 * their names in the parms array...
	 * an array is used instead of a linked list because we will later save
	 * time & space by refering to the parms by index number rather than name.
	 */
	if (*line && *line++ == '(')
		{
		flags |= TSFL_HASPARMS;
		line = tokenize_word(line, wrkbuf, NULL, NULL, &ttype, TRUE);
		while (ttype != TOK_RPAREN)
			{
			if (ttype != TOK_UNDEF)
				po_expecting_got_str(pcb, macro_parmname, wrkbuf);
			if (++pcount >= MAX_MACRO_PARMS)
				pp_say_fatal(macro_parmexceed);
			parms[pcount] = wrkbuf;
			wrkbuf += 1 + strlen(wrkbuf);
			line = tokenize_word(line, wrkbuf, NULL, NULL, &ttype, TRUE);
			if (ttype != ',' && ttype != TOK_RPAREN)
				po_expecting_got_str(pcb, comma_or_rparen, wrkbuf);
			else
				if (ttype != TOK_RPAREN)
					line = tokenize_word(line, wrkbuf, NULL, NULL, &ttype, TRUE);
			}
		}

	/*
	 * process the macro value string.
	 * if the macro has parameters, each TOK_UNDEF item from the value
	 * string is compared to all the parameter names.  if a match is found,
	 * a 'magic number' is placed into the value string, followed by the
	 * index of the parm (ie, its relative position within the parm list).
	 * during substitution, we use the magic/index pairs to plug in the
	 * parms specified on the invokation.
	 */
	value = wrkbuf;

	sharp = NULL;
	ttype = ~TOK_UNDEF; 	/* Initial tok type is anything but TOK_UNDEF */
	for(;;)
		{
		if (NULL == (line = po_skip_space(line)))
			{
			*wrkbuf = '\0';     /* tie off last value token */
			break;
			}
		if (iscsymf(*line) && ttype == TOK_UNDEF)	/* Last tok & this tok	*/
			*wrkbuf++ = ' ';                        /* both UNDEF, add space*/
		line = tokenize_word(line, wrkbuf, NULL, NULL, &ttype, TRUE);
		if (ttype == '#')
			{
			if (*line == '#')
				{
				++line; 					/* double sharp - go backwards	*/
				while (*(wrkbuf-1) == ' ')  /* in value string until we are */
					--wrkbuf;				/* just past last non-blank tok.*/
				}
			else
				{
				if (iscsymf(*line)) 		/* single sharp - remember where*/
					sharp = wrkbuf; 		/* only if any chance its a parm*/
				}
			}
		else
			{
			if (ttype == TOK_UNDEF && pcount != 0 &&
				(0 != (parmnum = pp_in_parm_array(parms, pcount, wrkbuf))))
				{
				if (sharp != NULL)
					{
					wrkbuf = sharp; 		/* parm preceded by a sharp,	*/
					*wrkbuf++ = PPTOK_PARMQ;/* set magic # followed 		*/
					*wrkbuf++ = parmnum;	/* by parameter number. 		*/
					}
				else
					{
					*wrkbuf++ = PPTOK_PARMN;/* parm without a sharp, set	*/
					*wrkbuf++ = parmnum;	/* magic # and parm number. 	*/
					}
				}
			else
				{
				wrkbuf += strlen(wrkbuf); /* plain ol' token in buf.    */
				}
			sharp = NULL;	/* one way or another we're done with the # now.*/
			}
		}

	/*
	 * check for redefinition of a macro...
	 * note that we have to do this *after* the value has been completely
	 * processed, because ANSI says the values must be 'identical except for
	 * whitespace', so we can't just do a straight strcmp() of the whole line.
	 * also, ANSI says the parameters on a redefine must match exactly in
	 * both count and names; we care about count, but not the names:
	 *	 #define x(a,b)  xxx	// these two are
	 *	 #define x(x,y)  xxx	// equal in our eyes
	 */

	if (NULL != (old = pp_in_hash_list(name, pcb->t.define_list)))
		{
		if (old->parmcount == pcount && 0 == po_eqstrcmp(old->value, value))
			return;
		else
			pp_say_fatal(macro_redefined);
		}

	/*
	 * its a new one, build the symbol, add it to the pp symbol table.
	 */

	 pp_add_new_ts(pcb, po_clone_string(pcb, name),
						po_clone_string(pcb, value),
						pcount, flags);


}

static void pp_undef(Poco_cb *pcb, char *line, char *word_buf)
/*****************************************************************************
 * process #undef line.
 ****************************************************************************/
{
SHORT		ttype;
Text_symbol *ts;

if (NULL == (line = tokenize_word(line, word_buf, NULL, NULL, &ttype, TRUE)))
	pp_say_fatal(macro_expect_name);

if (NULL != (ts = pp_in_hash_list(word_buf, pcb->t.define_list)))
	{
	if (ts->flags & TSFL_ISBUILTIN)
		pp_say_fatal(macro_builtin);
	unhash(word_buf, pcb->t.define_list);
	}
}

static char *pp_chop_string(Poco_cb *pcb, char *line, char *word_buf,
							Boolean include_name)
/*****************************************************************************
 *
 ****************************************************************************/
{
char c;
Boolean already_tried_expansion = FALSE;

	for(;;)
	{
		if (NULL == (line = po_skip_space(line)))
			goto eoline;

		c = *line;
		if ( c == '"')
			break;
		else if (c == '<')
		{
			if(!include_name)
				goto unexpected;
			c = '>';
			break;
		}

		if(already_tried_expansion)
			goto unexpected;

		prep_line(pcb, line, word_buf, LINE_MAX);
		already_tried_expansion = TRUE;
	}

	if(*(line = po_chop_to(++line, word_buf, c)) == 0)
		goto eoline;

	return(++line);

unexpected:
	word_buf[0] = c; /* the unexpected char */
	return(NULL);
eoline:
	word_buf[0] = 0;
	return(NULL);
}

static void pp_pragma(Poco_cb *pcb, char *line, char *word_buf)
/*****************************************************************************
 * process #pragma line. A little state machine to parse pragma lines
 ****************************************************************************/
{
SHORT ttype;
int state = 0;			/* state switch */
Boolean keep_quotes = TRUE; /* keep quotes on strings */
Boolean end_ok = TRUE;
char *fatal;		 /* fatal error text */
int want_pp_string = 0; /* 0 == non string token,
						 * 1 == allow "<>" include delimiter as well as "",
						 * 2 == quoted string only */
char tbuf[PATH_SIZE];  /* temp buffer */

	for(;;)
	{
		if(want_pp_string)
		{
			if((line = pp_chop_string(pcb,line,word_buf,
									  want_pp_string == 1)) == NULL)
			{
				if(!end_ok)
					goto unexpected;
			}
		}
		else
		{
			if((line = tokenize_word(line, word_buf,NULL,NULL,&ttype,
									 keep_quotes)) == NULL)
			{
				if(!end_ok)
				{
					word_buf[1] = 0;
					goto unexpected;
				}
			}
			else if(!end_ok && word_buf[0] == ';')
			{
				goto unexpected;
			}
		}

		switch(state)
		{
		/*** top level case following "#pragma" must find "poco"
			 or line ignored ***/

			case 0:
			{
				if (0 != strcmp("poco", word_buf))
					return;
				state = 1;
				break;
			}

		/*** second level case #pragma poco ???? ***/
			case 1:
			{
				if(0 == strcmp("library", word_buf))
				{
					want_pp_string = 1; /* want an include name */
					end_ok = FALSE;
					state = 3;
				}
				else if (0 == strcmp("eof", word_buf))
				{
					prev_file_stack_entry(pcb); // pretend we hit EOF
					goto pragma_done;
				}
				else if (0 == strcmp("macrosub", word_buf))
				{
					pcb->t.file_stack->flags |= FSF_MACSUB;
					goto pragma_done;
				}
				else if (0 == strcmp("nomacrosub", word_buf))
				{
					pcb->t.file_stack->flags &= ~FSF_MACSUB;
					goto pragma_done;
				}
				else if(0 == strcmp("stacksize", word_buf))
				{
					state = 2;
				}
				else if (0 == strcmp("echo", word_buf))
				{
					keep_quotes = FALSE;
					state = 5;
				}
				else
				{
					fatal = pragma_unknown;
					goto fatal_error;
				}
				break;
			}
		/************* begin 3rd level cases ************/

		/*** stacksize cases ***/
			case 2:
			{
			long wrksize = 1024 * atol(word_buf);

				if(wrksize >= POCO_STACKSIZE_MIN
					  && wrksize <= POCO_STACKSIZE_MAX)
				{
					pcb->run.stack_size = wrksize;
					goto pragma_done;
				}
				pp_say_fatal(stksz_value_bad);
			}
		/***** library cases *****/
			case 3: /* want a library name */
			{
			char *path;

				if(0 == po_eqstrcmp(word_buf, "poco$builtin"))
				{
					path = word_buf;
				}
				else
				{
					word_buf[sizeof(tbuf)-1] = 0; /* in case we are too big */
					if (NULL == (path = pp_findfile(pcb->t.include_dirs,
														word_buf)))
					{
						fatal = lib_cant_find;
						goto fatal_error;
					}
				}
				want_pp_string = 2; /* quotes only */
				end_ok = TRUE;	 /* we can finish here */
				strcpy(tbuf,path);
				state = 4;
				break;
			}
			case 4: /* we have a lib name and may be sent an id_string */
			{
			Poco_lib *lib;
			char *id_string;

				if(line != NULL)
				{
					id_string = word_buf;
				}
				else if(word_buf[0] && word_buf[0] != ';')
				{
					goto unexpected;
				}
				else
					id_string = NULL;

				if(NULL == (lib = po_open_library(pcb, tbuf, id_string)))
				{
					word_buf = tbuf;
					fatal = lib_open_failed;
					goto fatal_error;
				}
				new_file_stack_entry(pcb, lib, lib->name, FSF_ISLIB);
				goto pragma_done;
			}
		/**** echo cases ****/
			case 5:
			{
				fprintf(pcb->t.err_file, "%s\n", word_buf);
				goto pragma_done;
			}
		/**** default should never happen ****/
			default:
				goto pragma_done;
		}
	}

pragma_done:
	return;
unexpected:
	if(word_buf[0])
		fatal = unexpected_eol;
	else
		fatal = unexpected_tok;
fatal_error:
	pp_say_fatal(fatal, word_buf);
	return;
}

static void pp_include(Poco_cb *pcb, char *line, char *word_buf)
/*****************************************************************************
 * process #include line.
 ****************************************************************************/
{
FILE		*fp;
char		*path;

if (NULL == pp_chop_string(pcb, line, word_buf, TRUE))
	pp_say_fatal(incl_name_missing);

if (NULL == (path = pp_findfile(pcb->t.include_dirs, word_buf)))
	pp_say_fatal(incl_open, word_buf);

if (NULL == (fp = fopen(path, "r")))
	pp_say_fatal(incl_open, word_buf);

new_file_stack_entry(pcb, fp, path, FSF_ISFILE|FSF_MACSUB);

}

static void feed_preproc(Poco_cb *pcb, char *line, char *word_buf)
/*****************************************************************************
 * feed a line to the preprocessor, we know it starts with a sharp character.
 ****************************************************************************/
{
SHORT ttype;

line = po_skip_space(++line);	/*skip to first token */

/* sharp followed by all white space is compiler comment*/

if (line != NULL)
	{

	line = tokenize_word(line, word_buf, NULL, NULL, &ttype, TRUE);

/* define */

	if (po_eqstrcmp(word_buf, "define") == 0 && pcb->t.out_of_it == 0)
		{
		pp_define(pcb, line, word_buf);
		}

/* include */

	else if (po_eqstrcmp( word_buf, "include") == 0 && pcb->t.out_of_it == 0)
		{
		pp_include(pcb, line, word_buf);
		}

/* undef */

	else if (po_eqstrcmp(word_buf, "undef") == 0 && pcb->t.out_of_it == 0)
		{
		pp_undef(pcb, line, word_buf);
		}

/* error */

	else if (po_eqstrcmp(word_buf, "error") == 0 && pcb->t.out_of_it == 0)
		{
		line = po_skip_space(line);
		if (line != NULL)
			pp_say_fatal("#error: %s", line);
		else
			pp_say_fatal(forced_fatal);
		}

/* line */

	else if (po_eqstrcmp(word_buf, "line") == 0 && pcb->t.out_of_it == 0)
		{
		/* allow but ignore #line directives */
		}

/* pragma */

	else if (po_eqstrcmp(word_buf, "pragma") == 0 && pcb->t.out_of_it == 0)
		{
		pp_pragma(pcb, line, word_buf);
		}

/* ifdef */

	else if (po_eqstrcmp(word_buf, "ifdef") == 0)
		{
		pp_ifdef(pcb, line, word_buf, TRUE);
		}

/* ifndef */

	else if (po_eqstrcmp(word_buf, "ifndef") == 0)
		{
		pp_ifdef(pcb, line, word_buf, FALSE);
		}

/* if */

	else if (po_eqstrcmp(word_buf, "if") == 0)
		{
		register Conditional *con;

		con = po_memalloc(pcb, sizeof(Conditional));
		con->else_state = FALSE;
		con->next = pcb->t.ifdef_stack;
		pcb->t.ifdef_stack = con;

		if (0 == (con->state = pp_if(pcb,line, word_buf)))
			pcb->t.out_of_it++;
		}

/* elif */

	else if (po_eqstrcmp(word_buf, "elif") == 0)
		{
		register Conditional *con;

		if (NULL == (con = pcb->t.ifdef_stack))
			pp_say_fatal(else_unmatched);

		if (con->else_state == PP_NO_ELSE_SEEN)
			{
			if (con->state)
				{
				con->state = FALSE;
				con->else_state = PP_DONE_ELIF;
				++pcb->t.out_of_it;
				}
			else
				{
				if (0 != (con->state = pp_if(pcb,line, word_buf)))
					--pcb->t.out_of_it;
				}
			}
		else if (con->else_state == PP_DONE_ELSE)
			pp_say_fatal(elif_after_else);
		}

/* else */

	else if (po_eqstrcmp(word_buf, "else") == 0)
		{
		register Conditional *con;

		if (NULL == (con = pcb->t.ifdef_stack))
			pp_say_fatal(else_unmatched);

		if (con->else_state == PP_NO_ELSE_SEEN)
			{ /* C *almost* has the grace of APL.  Consider the next line...*/
			pcb->t.out_of_it += (con->state = !con->state) ? -1 : 1;
			}
		else if (con->else_state == PP_DONE_ELSE)
			pp_say_fatal(else_multiple);

		con->else_state = PP_DONE_ELSE;
		}

/* endif */

	else if (po_eqstrcmp(word_buf, "endif") == 0)
		{
		register Conditional *con;

		if ((con = pcb->t.ifdef_stack) == NULL)
			pp_say_fatal(endif_unmatched);

		if (!con->state)
			--pcb->t.out_of_it;
		pcb->t.ifdef_stack = con->next;
		po_freemem(con);
		}

/* unknown pp command */

	else if (pcb->t.out_of_it == 0)
		{
		pp_say_fatal(ppcmd_unknown, word_buf);
		}
	}
}



void po_free_pp(Poco_cb *pcb)
/*****************************************************************************
 * free all memory allocated by the preprocessor.
 ****************************************************************************/
{
register File_stack *fp;

while ((fp = pcb->t.file_stack) != NULL)/*only meaningful during error abort*/
	{
	free_string(fp->name);
	if (fp->flags & FSF_ISFILE)
		fclose(fp->source.file);
	pcb->t.file_stack = fp->pred;
	po_freemem(fp);
	}

po_freelist(&pcb->t.ifdef_stack);	/*this also only meaningful during abort*/

free_hash_list(pcb->t.define_list);
}

Boolean po_init_pp(Poco_cb *pcb, char *filename)
/*****************************************************************************
 * fire up the preprocessor, do pre-defined symbols & #include for main file.
 ****************************************************************************/
{
char wrkstr[256];
register Names *cldefs;

ppcb = pcb;

pcb->t.out_of_it = 0;

/*
 * do pre-defined symbols passed to us by our parent...
 */

cldefs = pcb->t.pre_defines;
while (cldefs)
	{
	sprintf(wrkstr, "#define %s", cldefs->name);
	feed_preproc(pcb, wrkstr, pcb->t.line_b2);
	cldefs = cldefs->next;
	}

/*
 * do our special pre-defines that can't be #undef'd or redefined...
 */

pp_add_new_ts(pcb, "__POCO__", VRSN_STR,0, TSFL_ISBUILTIN);
pp_add_new_ts(pcb, "__FILE__", "\001",  0, TSFL_ISBUILTIN+TSFL_ISSPECIAL);
pp_add_new_ts(pcb, "__LINE__", "\002",  0, TSFL_ISBUILTIN+TSFL_ISSPECIAL);
pp_add_new_ts(pcb, "__DATE__", "\003",  0, TSFL_ISBUILTIN+TSFL_ISSPECIAL);
pp_add_new_ts(pcb, "__TIME__", "\004",  0, TSFL_ISBUILTIN+TSFL_ISSPECIAL);
pp_add_new_ts(pcb, "NULL",     "\005",  0, TSFL_ISBUILTIN+TSFL_ISSPECIAL);
#if defined(DEVELOPMENT)
  pp_add_new_ts(pcb, "cdecl",  "\0",    0, TSFL_ISBUILTIN);
#endif


/*
 * do our more mundane pre-defines; these could be undef'd or redef'd by
 * the poco program if it really wanted/needed to.	these things are mostly
 * provided as a convenience...
 */

sprintf(wrkstr, "#define PATH_SIZE %d", (int)PATH_SIZE);
feed_preproc(pcb, wrkstr, pcb->t.line_b2);
feed_preproc(pcb, "#define Array_els(a) (sizeof((a))/sizeof((a)[0]))", pcb->t.line_b2);
feed_preproc(pcb, "#define TRUE  1", pcb->t.line_b2);
feed_preproc(pcb, "#define FALSE 0", pcb->t.line_b2);

/*
 * do #include of root file...
 * do #pragma for builtin libraries.
 */

sprintf(wrkstr, "#include <%s>", filename);
feed_preproc(pcb, wrkstr, pcb->t.line_b2);

if (pcb->builtin_lib != NULL)
	feed_preproc(pcb, "#pragma poco library <poco$builtin>", pcb->t.line_b2);

return TRUE;
}

char *po_pp_next_line(Poco_cb *pcb)
/*****************************************************************************
 * get next input line, feed to preproc or prep_line, return prep'd line.
 *
 *	 to process the builtin libraries, the compile_poco routine will init
 *	 the preprocessor (po_init_pp), and will then set library_protos.lib to
 *	 the pointer passed by our parents.  the po_init_pp call opens the main
 *	 input file, but does not cause a read from it.  thus, when po_pp_next_line
 *	 is called the first time, it will begin getting data from the builtin
 *	 library protos first (if our parent passed a non-NULL lib pointer).
 *	 this has done away with the old indirection method of obtaining lines
 *	 (wherein pcb->next_line was a pointer to either po_pp_next_line or a
 *	 routine to get a proto line).	now, this routine is the sole source of
 *	 input from the parser's point of view; herein we decide whether to get
 *	 a line from a library proto or from the input stream.
 *
 * NOTE:
 *	 proto lines are subject to the following restrictions:
 *	   - preprocessor macro substitution IS NOT DONE on proto lines!
 *	   - proto lines cannot be completely blank
 *	   - proto lines cannot contain comments (except #-type lines, which can)
 *	   - proto lines may contain preprocessor commands, providing the sharp
 *		 character is in the first column
 *	   - proto lines CANNOT contain a "#pragma library" statement, nested
 *		 library processing is not allowed
 *	   - if the proto line is a function prototype, it must have a non-NULL
 *		 function pointer in the 'func' field
 *	   - if a proto line is not a function prototype (eg, a typedef,
 *		 a #define, etc), the 'func' pointer should be NULL
 ****************************************************************************/
{
File_stack	*fs;
char		*instring;

	do	{
		if ((fs=pcb->t.file_stack) == NULL) 	/* all out of input */
			return NULL;

		if (fs->flags & FSF_ISLIB)
			instring = po_get_libproto_line(pcb);
		else
			instring = po_get_csource_line(pcb);

		if (instring == NULL)
			{
			prev_file_stack_entry(pcb);
			}
		else
			{
			pcb->t.line_buf = instring = po_skip_space(instring);
			if (instring != NULL)
				{
				if (*instring == '#')
					{
					feed_preproc(pcb, instring, pcb->t.line_b2);
					instring = NULL;
					}
				else
					{
					if (pcb->t.out_of_it)
						instring = NULL;
					else
						{
						if (fs->flags & FSF_MACSUB)
							{
							instring = prep_line(pcb, pcb->t.line_b1,
												  pcb->t.line_b2, LINE_MAX);
							instring = po_skip_space(instring);
							}
						}
					}
				}
			}
		} while (instring == NULL);

	return(pcb->t.line_buf = instring);
}


