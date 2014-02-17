#include "pjbasics.h"
#include "softmenu.h"
#include "ftextf.h"
#include "errcodes.h"


Errcode soft_menu_err(Errcode err, int class, char *symname)
{
	return(softerr(err, "!%s%s", "cant_load2", smu_class_names[class],symname));
}

char *soft_string(char *key, char *buf, int size)
/* Reads in text from resource into fixed sized buffer */
{
Errcode err;

	if ((err = smu_string(&smu_sm, key, buf, size)) < Success)
		soft_menu_err(err, SMU_TEXT_CLASS, key);
	return(buf);
}

#ifdef SLUFFED
Errcode soft_strings(char *symname, Smu_strings *s)
/* Reads in strings class into smu_strings structure */
{
Errcode err;

if ((err = smu_get_strings(&smu_sm, symname, s)) < Success)
	err = soft_menu_err(err, SMU_STRINGS_CLASS, symname);
return(err);
}
#endif /* SLUFFED */

#ifdef SLUFFED
Errcode soft_scatters(char *symname, Smu_strings *s, 
	char **scatters[], int count)
/* This reads in a list of strings from the resource file.  It takes
   an array of pointers to strings, and puts the strings into place.
   A common use would be for a panel menu which contains several
   strings. */
{
Errcode err;
int i;
char **pt;

if ((err = soft_strings(symname, s)) < Success)
	goto OUT;
if (s->count != count)
	{
	if (s->count < count)
		err = Err_not_enough_fields;
	else
		err = Err_too_many_fields;
	smu_free_strings(s);
	goto OUT;
	}
for (i=0; i<count; ++i)
	{
	pt = *scatters++;
	*pt = s->strings[i];
	}
OUT:
return(err = soft_menu_err(err, SMU_STRINGS_CLASS, symname));
}
#endif /* SLUFFED */

Errcode soft_name_scatters(char *symname, Smu_name_scats *nscats,
		unsigned int num_scats, void **allocd, USHORT flags)
/* If fill all is true it will abort error if all the symbols are not found */
{
	return(smu_name_scatters(&smu_sm,symname,nscats,num_scats,
							 allocd, flags));
}
Errcode soft_load_text(char *key, char **ptext)
/*******
 * Loads text from softmenu file in global smu_sm and reports any errors
 ******/
{
Errcode err;

	if((err = smu_load_text(&smu_sm, key, ptext)) < Success)
		return(soft_menu_err(err, SMU_TEXT_CLASS, key));
	return(Success);
}
Errcode soft_load_ftext_type(char *key,va_list *pargs,
						    char **pformats,char **ptext)
/************************************************************
 * This handles the input side of a softmenu function using "ftext" type text.
 * A function like the one below may have the "key" field optionally be a
 * ftext formats string "!%d" beginning with a '!' and followed by '%' type
 * format specifiers, followed by an argument that is the soft menu text key
 * or may just be the soft menu key with no formats.
 * called like func("!%d", "key_name", ...)
 * 		  or   func("key_name",... )
 * The formats string specifies the type and order of the '...' args which are
 * ignored if a formats string is not present. The 'key' argument must 
 * immediately precede the '...' args for this to work.
 *
 * sample use:
 *	
 *	func(arg1,arg2,key,...)
 *	{
 *	char *formats;
 *	va_list args;
 *
 *		va_start(args,key);
 *		if(soft_load_ftext_type(key,&args,&formats,&text) >= Success)
 *		{
 *			ftext_subroutine(arg1,arg2,formats,text,args);
 *			smu_free_text(&text);
 *		}
 *		va_end(args);
 *	}
 *
 ************************************************************/

{
	if((*pformats = ftext_format_type(&key,pargs)) == NULL)
		*pformats = "";
	return(soft_load_text(key, ptext));
}
