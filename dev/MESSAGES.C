#include <stdarg.h>
#include "jimk.h"
#include "errcodes.h"
#include "ptrmacro.h"
#include "commonst.h"
#include "softmenu.h"
#include "ftextf.h"


void top_textf(char *fmt,...)
/* Print a line in a little window on top of the screen (or bottom if
 * the cursor is near the top */
{
va_list args;
char *formats;

	if(!vs.dcoor)
		return;
	va_start(args, fmt);
	formats = ftext_format_type(&fmt,&args);
	ttextf(fmt,args,formats);
	va_end(args);
}
void soft_top_textf(char *key,...)
{
va_list args;

	if(!vs.dcoor)
		return;
	va_start(args,key);
	soft_ttextf(key,&args);
	va_end(args);
}
/******* canned messages and error reports *******/

Errcode outta_memory(void)
/* Put up the old 'out of memory' error box */
{
	return(softerr(Err_no_memory,NULL));
}
void *begmem(unsigned size)
/* Ask for memory and squawk if it's not available (then return NULL) */
{
void *pt;

	if (size == 0L)
	{
		softerr(Err_no_message,"alloc_zero");
		return(NULL);
	}
	if((pt = pj_malloc(size)) == NULL)
	{
		outta_memory();
		return(NULL);
	}
	return(pt);
}

Errcode cant_create(Errcode err,char *name)
{
	return(softerr(err, "!%s", "cant_create", name ));
}
void truncated(char *filename)
/* Announce file isn't as big as it's supposed to be.  Checks for NULL
   name argument in case we forgot who we're writing somewhere along
   the line... */
{
	if(filename == NULL)
		filename = empty_str;

	soft_continu_box("!%s","truncated",filename);
}
Boolean overwrite_old(char *name)
/* Make sure use knows they're overwriting an old file and give 'em a
   chance to abort it. */
{
	if (!pj_exists(name) )
		return(1);
	return(soft_yes_no_box("!%s", "over_old", name ));
}
Boolean really_delete(char *name)
{
	return(soft_yes_no_box("!%s","really_del", name));
}
#ifdef TESTING
Errcode dump_box(char *msg, void *mem, int memsize)
/* Display a (bunch of) continue boxes filled with a memory dump */
{
static char *maybenext[] = { "Next block", "Cancel", NULL };

long *lmem = mem;
char *textbuf;
char *buf;
long size;
int count;

	if(memsize <= 0)
		return(continu_box("%s\n Zero dumpsize!", msg));

	if((textbuf = pj_malloc(18*41)) == NULL)
		return(softerr(Err_no_memory,NULL));

	count = 0;

	while(memsize > 0)
	{
		size = Min((16*16),memsize);
		memsize -= size;

		buf = textbuf;
		buf += sprintf(buf, "%%s");

		while(size >= 16)
		{
			buf += sprintf(buf, "\n%02X |%08X %08X %08X %08X|", 
								count++, lmem[0],lmem[1],lmem[2],lmem[3]); 
			size -= 16;
			lmem += 4;
		}
		if(size)
			buf += sprintf(buf,"\n%02X |");

		while(size >= 4)
		{
			buf += sprintf(buf,"%08X ", *lmem);
			++lmem;
			size -= 4;
		}
		while(size > 0)
		{
			buf += sprintf(buf,"%2X", *((char *)lmem));
			lmem = OPTR(lmem,1);
			--size;
		}
		--buf;
		if(*buf == ' ')
			*buf++ = '|';
		else if(*buf != '|')
		{
			*(++buf) = '|';
			*(++buf) = 0;
		}

		if(memsize)
		{
			if(multi_box(maybenext, textbuf, msg) == 1)
				continue;
		}
		else
			continu_box(textbuf, msg);

		break;
	}
	pj_free(textbuf);
}
#endif /* TESTING */
