/* expandat.c - The idea is to take an argc,argv pair with potentially some
 * @file words in argv,  and read the file into the argc,argv pair */

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pjstypes.h"
#include "pjecodes.h"
#include "expandat.h"


void gentle_free(void *pt)
/*************************************************************************
 * Free non-NULL pointer. 
 *************************************************************************/
{
if (pt != NULL)
	free(pt);
}

Errcode read_alloc_buf(char *filename, char **pbuf, int *psize)
/*************************************************************************
 * Find out size of file, allocate a buffer that big + 1.  Read in
 * file into buffer.  Put a zero tag at end of buffer.  Return buffer
 * and size of buffer.
 *************************************************************************/
{
Errcode err = Success;
long size = 0;
char *buf = NULL;
int handle = -1;

if ((handle = open(filename, O_RDONLY|O_BINARY)) == -1)
	{
	err = report_error(Err_no_message,
		"Couldn't open %s. %s. Sorry", filename, strerror(errno));
	goto OUT;
	}
size = lseek(handle, 0L, SEEK_END);
lseek(handle, 0L, SEEK_SET);
if ((buf = malloc((size_t)size+1)) == NULL)
	{
	err = report_error(Err_no_memory,
		"Couldn't allocate %ld bytes for %s buffer", size, filename);
	goto OUT;
	}
buf[size] = 0;	/* add zero tag at end */
if (read(handle, buf, size) != size)
	{
	err = report_error(Err_no_message,
		"Couldn't read %s. %s.", filename, strerror(errno));
	goto OUT;
	}
OUT:
	if (err < Success)
		{
		gentle_free(buf);
		buf = NULL;
		size = 0;
		}
	if (handle != -1)
		close(handle);
	*pbuf = buf;
	*psize = size;
	return(err);
}

char *skip_space(char *pt)
/*************************************************************************
 *	Return first non-space character in string, or NULL if all spaces
 *  until end.
 *************************************************************************/
{
while (isspace(*pt))
	++pt;
if (*pt == 0)
	return(NULL);
return(pt);
}

char *skip_to_space(char *pt)
/*************************************************************************
 *	Return first space character in string, or NULL if no spaces
 *  until end.
 *************************************************************************/
{
char c;

for (;;)
	{
	c = *pt;
	if (isspace(c))
		break;
	if (c == 0)
		return(NULL);
	++pt;
	}
return(pt);
}

static int str_count_words(char *pt)
/*************************************************************************
 * Count the number of white-space separated words in string.
 *************************************************************************/
{
int wcount = 0;

for (;;)
	{
	if ((pt = skip_space(pt)) == NULL)
		break;
	++wcount;
	if ((pt = skip_to_space(pt)) == NULL)
		break;
	}
return(wcount);
}

static void str_breakup_words(char **words, char *buf)
/*************************************************************************
 * Convert a white space separated bunch of words in a buf to
 * an array of zero terminated strings words.  Bits of buf that
 * are white space will be overwritten with zero's to accomplish this.
 *************************************************************************/
{
for (;;)
	{
	if ((buf = skip_space(buf)) == NULL)
		break;
	*words++ = buf;
	if ((buf = skip_to_space(buf)) == NULL)
		break;
	*buf++ = 0;
	}
}

typedef struct chopped_words
/** This structure holds a memory character buffer that's
 ** been chopped into words */
 	{
	char *buf; 			/* pointer to memory buffer */
	int wordc; 			/* recieves # of words */
	char **wordv; 		/* recieves start of each word */
	} Chopped_words;


static Errcode buf_to_words(char *buf, Chopped_words *cw)
/*************************************************************************
 * Make convert a text buffer to chopped words.
 *************************************************************************/
{
cw->wordc = str_count_words(buf);
if ((cw->wordv = malloc(cw->wordc * sizeof(char *))) == NULL)
	{
	return(report_error(Err_no_memory
	, "in buf_to_words() %d words", cw->wordv));
	}
str_breakup_words(cw->wordv, buf);
cw->buf = buf;
return(Success);
}

static Errcode file_to_words(char *filename, Chopped_words *fw)
/*************************************************************************
 * Read a file into memory and make it into chopped words.
 *************************************************************************/
{
char *buf = NULL;
Errcode err = Success;
int size;

if ((err = read_alloc_buf(filename, &buf, &size)) < Success)
	goto OUT;
if ((err = buf_to_words(buf, fw)) < Success)
	goto OUT;
OUT:
	if (err < Success)
		gentle_free(buf);
	return(err);
}

static int count_ats(int argc, char *argv[])
/*************************************************************************
 * Count up the number of strings that start with an @ symbol.
 *************************************************************************/
{
int count = 0;

while (--argc >= 0)
	{
	if (**argv == '@')
		++count;
	++argv;
	}
return(count);
}

Errcode expand_ats(int *pargc, char *(*pargv[]))
/*************************************************************************
 * Expand @files in an argc/argv array to be the contents of the
 * @file.  Typical use:
 *		main(int argc, char *argv)
 *		{
 *		if (expand_ats(&argc, &argv) >= Success)
 * 			do_main_code(argc, argv);
 *		}
 *************************************************************************/
{
int argc = *pargc;
char **argv = *pargv;
char **argp;
char *name;
int acount;
Chopped_words *alist, *apt;
int i;
Errcode err = Success;
int new_argc = 0;
char **new_argv;
char **new_argp;

if ((acount = count_ats(argc, argv)) <= 0)
	return(Success);	/* nothing to expand */
if ((alist = calloc(acount, sizeof(*alist))) == NULL)
	{
	return(report_error(Err_no_memory, "expand_ats() acount %d", acount));
	}
apt = alist;
argp = argv;
i = argc;
while (--i >= 0)
	{
	name = *argp++;
	if (name[0] == '@')
		{
		if ((err = file_to_words(name+1, apt)) < Success)
			goto OUT;
		new_argc += apt->wordc;
		++apt;
		}
	else
		{
		new_argc += 1;
		}
	}
if ((new_argp = new_argv = calloc(new_argc, sizeof(char *))) == NULL)
	{
	err = report_error(Err_no_memory, 
		"expand_ats() new_argc %d", new_argc);
	goto OUT;
	}
apt = alist;
argp = argv;
i = argc;
while (--i >= 0)
	{
	name = *argp++;
	if (name[0] == '@')
		{
		memcpy(new_argp, apt->wordv, apt->wordc * sizeof(char *));
		new_argp += apt->wordc;
		++apt;
		}
	else
		{
		*new_argp++ = name;
		}
	}
*pargc = new_argc;
*pargv = new_argv;
OUT:
	gentle_free(alist);
	return(err);
}

