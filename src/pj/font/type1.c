/*

		Read and decrypt an Adobe Type 1 font.
		This code was written originally by John Walker
		using the _Adobe Type 1 Font Format 1.1_ published
		by Addison-Wesley ISBN 0-201-57044-0 as a guide.

		Jim Kent changed it a fair amount,  reformatting the
		indentation and renaming some of the identifiers to
		mix with the local style;  making the output go through
		functions embedded in a structure so the same interpreter 
		could be used for both sizing and drawing the font,  and
		putting in stuff to glue it into Animator Pro's font
		manager.  Also changed from line oriented parsing to token
		oriented parsing to accomodate some PD .PFB files.

		There are four main sections to this file set apart with
		long comment blocks.  One reads the font into memory.
		The second interprets the font language.  The third is concerned
		with directing the output of the interpreter into a form
		useful for PJ.  The final bit is the glue into the PJ
		virtual font system.

*/
#undef DEBUG

#define VFONT_C
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include <setjmp.h>
#include "blockall.h"
#include "errcodes.h"
#include "fontdev.h"
#include "linklist.h"
#include "pjbasics.h"
#include "rastext.h"
#include "render.h"
#include "reqlib.h"
#include "sdot.h"
#include "token.h"
#include "type1.h"
#include "xfile.h"

#define EOS     '\0'

#define X   0
#define Y   1

static void calc_font_bounds(Type1_font *tcd);
static void free_scale_info(Type1_scale_info *si);

#ifdef DEBUG
/*********DEBUGGING STUFF********/
FILE *debug_log;
Boolean in_debug = FALSE;

void open_debug_log()
{
	if (debug_log == NULL)
		debug_log = fopen("H:\\debug.log", "w");
}

void dvprintf(char *fmt, va_list args)
{
open_debug_log();
vfprintf(debug_log,fmt,args);
fflush(debug_log);
}

void dlog(char *fmt, ...)
/* Printf to debug file. */
{
va_list argptr;

va_start(argptr, fmt);
dvprintf(fmt,argptr);
va_end(argptr);
}

void ddlog(char *fmt, ...)
/* Conditional printf to debug file */
{
va_list argptr;

if (in_debug)
	{
	va_start(argptr, fmt);
	dvprintf(fmt,argptr);
	va_end(argptr);
	}
}
#endif /* DEBUG */

/*****************************************************************************
 *****************************************************************************
 ** The Load Section.  A PostScript Type 1 file is composed of a list of
 ** definitions.  Each definition is a keyword followed by data.  
 ** At the start of the file are a bunch of definitions that are about the
 ** font as a whole.  Then we come to the individual letters,   which are
 ** encrypted and in their own little RPN language.
 **
 ** The load section is concerned with verifying that the file is indeed
 ** a PostScript Type 1,  extracting a few things from the 
 ** pre-letter definitions,  and then decrypting
 ** the individual letters and the subroutines they use and sticking them 
 ** in a couple of arrays for later access.
 *****************************************************************************
 ****************************************************************************/

/*****************************************************************************
 * Let's handle errors (not enough memory, bad data in file, etc.) during
 * reading with a setjmp/longjmp.  The longjmp destination will be the
 * highest level read routine (read_font).
 ****************************************************************************/
static jmp_buf type1_load_errhandler; /* Jump buffer for load errors.	*/

static void type1_load_error(char *fmt, ...)
/*
 * format & output an error message, then longjump to error handler.
 */
{
va_list args;

va_start(args, fmt);
varg_continu_box(NULL,fmt,args,NULL);
va_end(args);
longjmp(type1_load_errhandler, Err_reported);
}


/*  Sections of the font file.  */
typedef enum { Header, FontInfo, OtherSubrs, Subrs, CharStrings } file_section;
static file_section section;



#define MAP_SIZE 256

/*  Map of PostScript character names to IBM code page 850 (multilingual) */
static char *isomap[MAP_SIZE] = {
	NULL,				 /*  00 */
	NULL,				 /*  01 */
	NULL,				 /*  02 */
	NULL,				 /*  03 */
	NULL,				 /*  04 */
	NULL,				 /*  05 */
	NULL,				 /*  06 */
	NULL,				 /*  07 */
	NULL,				 /*  08 */
	NULL,				 /*  09 */
	NULL,				 /*  10 */
	"ordmasculine",		 /*  11 */
	"ordfeminine",		 /*  12 */
	NULL,				 /*  13 */
	NULL,				 /*  14 */
	NULL,				 /*  15 */
	NULL,				 /*  16 */
	NULL,				 /*  17 */
	NULL,				 /*  18 */
	NULL,				 /*  19 */
	"paragraph",         /*  20 */
	"section",			 /*  21 */
	NULL,				 /*  22 */
	NULL,				 /*  23 */
	NULL,				 /*  24 */
	NULL,				 /*  25 */
	NULL,				 /*  26 */
	NULL,				 /*  27 */
	NULL,				 /*  28 */
	NULL,				 /*  29 */
	NULL,				 /*  30 */
	NULL,				 /*  31 */
    "space",             /*  32 */
    "exclam",            /*  33 */
    "quotedbl",          /*  34 */
    "numbersign",        /*  35 */
    "dollar",            /*  36 */
    "percent",           /*  37 */
    "ampersand",         /*  38 */
    "quotesingle",        /*  39 */		/* ?"quoteright"? */
    "parenleft",         /*  40 */
    "parenright",        /*  41 */
    "asterisk",          /*  42 */
    "plus",              /*  43 */
    "comma",             /*  44 */
    "hyphen",            /*  45 */
    "period",            /*  46 */
    "slash",             /*  47 */
    "zero",              /*  48 */
    "one",               /*  49 */
    "two",               /*  50 */
    "three",             /*  51 */
    "four",              /*  52 */
    "five",              /*  53 */
    "six",               /*  54 */
    "seven",             /*  55 */
    "eight",             /*  56 */
    "nine",              /*  57 */
    "colon",             /*  58 */
    "semicolon",         /*  59 */
    "less",              /*  60 */
    "equal",             /*  61 */
	"greater",           /*  62 */
    "question",          /*  63 */
    "at",                /*  64 */
    "A",                 /*  65 */
    "B",                 /*  66 */
    "C",                 /*  67 */
    "D",                 /*  68 */
    "E",                 /*  69 */
    "F",                 /*  70 */
    "G",                 /*  71 */
    "H",                 /*  72 */
    "I",                 /*  73 */
    "J",                 /*  74 */
    "K",                 /*  75 */
    "L",                 /*  76 */
    "M",                 /*  77 */
    "N",                 /*  78 */
    "O",                 /*  79 */
    "P",                 /*  80 */
    "Q",                 /*  81 */
    "R",                 /*  82 */
    "S",                 /*  83 */
    "T",                 /*  84 */
    "U",                 /*  85 */
    "V",                 /*  86 */
    "W",                 /*  87 */
    "X",                 /*  88 */
    "Y",                 /*  89 */
    "Z",                 /*  90 */
    "bracketleft",       /*  91 */
    "backslash",         /*  92 */
    "bracketright",      /*  93 */
    "asciicircum",       /*  94 */
    "underscore",        /*  95 */
    "grave",             /*  96 */
	"a",                 /*  97 */
    "b",                 /*  98 */
    "c",                 /*  99 */
    "d",                 /* 100 */
    "e",                 /* 101 */
    "f",                 /* 102 */
    "g",                 /* 103 */
    "h",                 /* 104 */
    "i",                 /* 105 */
    "j",                 /* 106 */
    "k",                 /* 107 */
    "l",                 /* 108 */
    "m",                 /* 109 */
    "n",                 /* 110 */
    "o",                 /* 111 */
    "p",                 /* 112 */
    "q",                 /* 113 */
    "r",                 /* 114 */
    "s",                 /* 115 */
    "t",                 /* 116 */
    "u",                 /* 117 */
    "v",                 /* 118 */
    "w",                 /* 119 */
    "x",                 /* 120 */
    "y",                 /* 121 */
    "z",                 /* 122 */
    "braceleft",         /* 123 */
    "bar",               /* 124 */
    "braceright",        /* 125 */
    "asciitilde",        /* 126 */
    NULL,            	 /* 127 */
    "Ccedilla",          /* 128 80 */
    "udieresis",         /* 129 */
    "eacute",            /* 130 */
    "acircumflex",       /* 131 */
	"adieresis",         /* 132 */
    "agrave",            /* 133 */
    "aring",             /* 134 */
    "ccedilla",          /* 135 */
    "ecircumflex",       /* 136 */
    "edieresis",         /* 137 */
    "egrave",            /* 138 */
    "idieresis",         /* 139 */
    "icircumflex",       /* 140 */
    "igrave",            /* 141 */
    "Adieresis",         /* 142 */
    "Aring",             /* 143 */
    "Eacute",            /* 144 90 */
    "ae",                /* 145 */
    "AE",                /* 146 */
    "ocircumflex",       /* 147 */
    "odieresis",         /* 148 */
    "ograve",            /* 149 */
    "ucircumflex",       /* 150 */
    "ugrave",            /* 151 */
    "ydieresis",         /* 152 */
    "Odieresis",         /* 153 */
    "Udieresis",         /* 154 */
    "oslash",            /* 155 9B */
    "sterling",          /* 156 */
    "Oslash",            /* 157 9D */
    "multiply",          /* 158 9E */	
    "florin",            /* 159 */  
    "aacute",            /* 160 A0 */
    "iacute",       	 /* 161 */
    "oacute",            /* 162 */
    "uacute",            /* 163 */
    "ntilde",            /* 164 */
    "Ntilde",            /* 165 */
    NULL,                /* 166 A6 ??? */	
	NULL,                /* 167 A7 ??? */
    "questiondown",      /* 168 */
    "registered",        /* 169 */
    "logicalnot",        /* 170 */
    "onehalf",           /* 171 */
    "onequarter",        /* 172 */
    "exclamdown",        /* 173 */
    "guillemotleft",                /* 174 AE */
    "guillemotright",                /* 175 AF */
    NULL,                /* 176 B0 ??? */
    NULL,                /* 177 B1 ??? */
    NULL,                /* 178 B2 ??? */
    NULL,                /* 179 B3 ??? */
    NULL,                /* 180 B4 ??? */
    "Aacute",            /* 181 */				
    "Acircumflex",       /* 182 */			
    "Agrave",            /* 183 */
    "copyright",         /* 184 */
    NULL,                /* 185 */				
    NULL,                /* 186 */			
    NULL,                /* 187 */		
    NULL,                /* 188 */	
    "cent",              /* 189 */
    "yen",               /* 190 */
    NULL,                /* 191 */
    NULL,                /* 192 C0 */
    "grave",             /* 193 C1 */		
    "acute",             /* 194 C2 */	
    "circumflex",        /* 195 C3 */
    "tilde",             /* 196 C4 */
    "macron",            /* 197 C5 */
    "atilde",            /* 198 C6 */				
    "Atilde",            /* 199 C7 */
    "dieresis",          /* 200 C8 */
    NULL,                /* 201 C9 */
	"ring",              /* 202 CA */
    "cedilla",           /* 203 CB */
    NULL,                /* 204 CC */
    "hungarumlaut",      /* 205 CD */
    "ogonek",                /* 206 CE */
    "currency",          /* 207 CF */
    "eth",               /* 208 DO */	
    "Eth",               /* 209 D1 */	
    "Ecircumflex",       /* 210 D2 */
    "Edieresis",         /* 211 D3 */
    "Egrave",            /* 212 D4 */
    NULL,                /* 213 D5 ??? */
    "Iacute",            /* 214 D6 */
    "Icircumflex",       /* 215 */
    "Idieresis",         /* 216 */
    NULL,                /* 217 */
    NULL,                /* 218 */
    NULL,                /* 219 */
    NULL,                /* 220 */
    "brokenbar",         /* 221 */
    "Igrave",            /* 222 */
    NULL,                /* 223 */
    "Oacute",            /* 224 E0 */
    "germandbls",        /* 225 */
    "Ocircumflex",       /* 226 */
    "Ograve",            /* 227 */
    "otilde",            /* 228 */
    "Otilde",            /* 229 */
    "mu",                /* 230 */
    "thorn",             /* 231 E7 */
    "Thorn",             /* 232 E8 */
    "Uacute",            /* 233 */
    "Ucircumflex",       /* 234 */
    "Ugrave",            /* 235 */
    "yacute",            /* 236 */
	"Yacute",            /* 237 */
    "macron",            /* 238 */
    "acute",             /* 239 EF */
    "hyphen",            /* 240 F0 */
    "plusminus",         /* 241 */
    NULL,                /* 242 */
    "threequarters",     /* 243 */
    "paragraph",         /* 244 */
    "dotlessi",          /* 245 F5 */
    "divide",            /* 246 */
    "cedilla",           /* 247 F7 */
    "degree",            /* 248 */
    "dieresis",          /* 249 F9 */
    "bullet",            /* 250 */
    "onesuperior",       /* 251 */
    "threesuperior",     /* 252 */
    "twosuperior",       /* 253 */
    NULL,                /* 254 */
    NULL                 /* 255 */
};




static void *loader_alloc(Type1_font *tcd, unsigned nbytes)
/* Alloc some memory for loader.  Bail out if can't. */
{
	char *cp;

	if ((cp = alloc_from_block(&tcd->font_ba, nbytes)) == NULL) {
		type1_load_error("Out of memory.  Couldn't find %d bytes", nbytes);
	}
	return (void *) cp;
}



static char *loader_strdup(Type1_font *tcd, char *s)
/*  STRSAVE  --  Allocate a duplicate of a string.  */
{
	char *c = loader_alloc(tcd, (unsigned) (strlen(s) + 1));

	strcpy(c, s);
	return c;
}


/*****************************************************************************
 * File input.
 ****************************************************************************/

static int (*byte_in)(XFILE *fp);	/* Call indirect to read a byte.
									 * Will be fgetc() or hex_byte_in() */

static int hex_byte_in(XFILE *fp)
/* Read a byte from file in hexadecimal format.  This skips white space,
 * and reads in two hex digits. */
{
    int c, xd = 0, i;

    for (i = 0; i < 2; i++) {
		for (;;) {
			if ((c = xfgetc(fp)) == EOF)
                return c;
            if (isspace(c))
                continue;
            break;
        }
		if (islower(c))
			c = toupper(c);
        if (c >= '0' && c <= '9') {
            c -= '0';
		} else if (c >= 'A' && c <= 'F') {
            c = (c - 'A') + 10;
		} else {
            type1_load_error("Bad hex digit\n");
            return EOF;
        }
        xd = (xd << 4) | c;
    }
    return xd;
}


/*****************************************************************************
 *  DECRYPT  --  Perform running decryption of file.  
 ****************************************************************************/

static unsigned short int cryptR, cryptC1, cryptC2, cryptCSR;

static void crypt_init( unsigned int key)
{
	cryptR = key;
    cryptC1 = 52845;
    cryptC2 = 22719;
}

static unsigned int decrypt(unsigned int cipher)
{
    unsigned int plain = (cipher ^ (cryptR >> 8));

    cryptR = (cipher + cryptR) * cryptC1 + cryptC2;
	return plain;
}

static int decrypt_byte_in(XFILE *fp)
{
	int ch;

	if ((ch = (*byte_in)(fp)) == EOF)
		return EOF;
	else
		return decrypt(ch);
}

static void cstrinit(void)
{
    cryptCSR = 4330;
}

static unsigned int decstr(unsigned int cipher)
{
    unsigned int plain = (cipher ^ (cryptCSR >> 8));

    cryptCSR = (cipher + cryptCSR) * cryptC1 + cryptC2;
    return plain;
}

/*****************************************************************************
 *  PARSER  --  Chop up file a line at a time and decide what to put where...
 ****************************************************************************/


typedef enum 
	{
	TTT_EOF,
	TTT_NAME,
	TTT_NUMBER,
	TTT_OTHER,
	TTT_TOO_LONG,
	} T1_token_type;

typedef struct
	{
	T1_token_type type;
	char string[256];
	int pushback;
	XFILE *file;
	int (*source)(XFILE *f);
	} Type1_token;

static void type1_token_init(Type1_token *tok
, XFILE *file, int (*source)(XFILE *f))
{
	clear_struct(tok);
	tok->file = file;
	tok->source = source;
}


static Boolean continue_number(int ch)
/* Return true if character is a digit */
{
	return isdigit(ch);
}

static Boolean continue_name(int ch)
/* Return true if character can be the second or further character in 
 * a name. */
{
	return ch == '_' || isalnum(ch);
}

static void type1_get_binary(Type1_token *tok, unsigned char *buf, int size)
/* Read in X number of bytes.  Account for any pushed-back characters. 
 * Bails out if not enough bytes left. */
{
	int ch;
	int i = 0;

	if (tok->pushback != 0)
		{
		*buf++ = tok->pushback;
		i = 1;
		tok->pushback = 0;
		}
	for (; i<size; ++i)
		{
		if ((ch = tok->source(tok->file)) == EOF)
			type1_load_error("Could only read %d of %d bytes.\n", i, size);
		*buf++ = ch;
		}
}

static void type1_get_token(Type1_token *token)
/* Read in a token from file and categorize it. 
 * In this case a token is a run of numbers, a letter followed by 
 * letters and numbers, or a single non-alpha-numeric character.  
 * White space serves to separate tokens but is otherwise skipped. 
 * Pass in a "source" function to get next character from file. 
 */
{
	int ch;
	Boolean (*get_next)(int ch);
	int tok_len = sizeof(token->string);
	char *string = token->string;

	/* Get pushed-back character if any. */
	if (token->pushback == 0)
		ch = token->source(token->file);
	else
		{
		ch = token->pushback;
		token->pushback = 0;
		}
	/* Skip leading spaces. */
	for (;;)
		{
		if (ch == EOF)
			{
			token->type = TTT_EOF;
			strcpy(token->string, "<EOF>");	/* For error reporting. */
			return;
			}
		if (!isspace(ch))
			break;
		ch = token->source(token->file);
		}
	if (isdigit(ch))
		{
		get_next = continue_number;
		token->type = TTT_NUMBER;
		}
	else if (ch == '_' || isalpha(ch))
		{
		get_next = continue_name;
		token->type = TTT_NAME;
		}
	else
		{
		*string++ = ch;
		*string = 0;
		token->type = TTT_OTHER;
		return;
		}
	for (;;)
		{
		if (--tok_len <= 0)
			{
			token->type = TTT_TOO_LONG;
			return;
			}
		*string++ = ch;
		ch = token->source(token->file);
		if (!(*get_next)(ch))
			{
			token->pushback = ch;
			*string = EOS;
			return;
			}
		}
}

#ifdef DEBUG
static void debug_type1_get_token(Type1_token *tok)
{

	type1_get_token(tok);
	switch (tok->type)
		{
		case TTT_EOF:
			dlog("<eof>\n");
			break;
		case TTT_TOO_LONG:
			dlog("<too long>\n");
			break;
		default:
			dlog("\t\"%s\"\n", tok->string);
			break;
		}
}
#endif /* DEBUG */

static Errcode type1_check_signature(XFILE *fp)
/* This just verifies that the font begins with %!FontType1 or
 * %!PS-AdobeFont-1.0.  We expect this in the first 128 bytes or so. */
{
	int ch;
	int i;
	char buf[80];
	static char magic1[16] = "PS-AdobeFont-1.0";
	static char magic2[9] = "FontType1";

	for (i=0; i<128; ++i)
		{
		ch = xfgetc(fp);
		if (ch == EOF)
			break;
		if (ch == '%') 
			{
			ch = xfgetc(fp);
            if (ch == EOF)
				break;
            if (ch == '!')
				{
				if (xfgets(buf, sizeof(buf), fp) == NULL)
					break;
				if (strncmp(buf, magic1, sizeof(magic1)) == 0
				||	strncmp(buf, magic2, sizeof(magic2)) == 0)
					return Success;
				}
	        }
		}
	return Err_bad_magic;
}

static void type1_parse_custom_encoding(Type1_font *tcd
, Type1_token *tok, int size)
/* Read in a bunch of custom encoding statements (that are used to 
 * associate letter names with ASCII values basically. */
{
	int char_ix;
	int i;

	tcd->encoding_count = size;
	tcd->encoding = loader_alloc(tcd, size * sizeof(*(tcd->encoding)));
	for (i=0; i<size; ++i)
		{
		type1_get_token(tok);
		if (tok->type == TTT_EOF)
			type1_load_error("Premature end of file in Encoding");
		if (tok->type == TTT_NAME)
			{
			if (strcmp(tok->string, "dup") == 0)
				{
				/* This should be the start of a sequence formmatted:
				 * 		dup NN /name put
				 */
				type1_get_token(tok);			/* get NN into char_ix */
				if (tok->type != TTT_NUMBER)
					goto SYNTAX_ERROR;
				char_ix = atoi(tok->string);
				if (char_ix < 0 || char_ix >= size)
					type1_load_error(
			        "Error parsing encoding vector.\n"
					"Character index %d out of range.\n"
					, char_ix);
				type1_get_token(tok);			/* Skip over /  */
				if (tok->type != TTT_OTHER && tok->string[0] != '/')
					goto SYNTAX_ERROR;
				type1_get_token(tok);			/* Get name into encoding. */
				if (tok->type == TTT_NAME)
					tcd->encoding[char_ix] = loader_strdup(tcd,tok->string);
				type1_get_token(tok);			/* Skip over put */
				}
			else if (strcmp(tok->string, "def") == 0)
				return;
			}
		}
	return;
SYNTAX_ERROR:
	type1_load_error("Syntax error in Encoding");
}

static void type1_parse_encoding(Type1_font *tcd, Type1_token *tok)
/* Parse the encoding statement - which determines how names of letters
 * match up with ascii values. This will either just be StandardEncoding,
 * or it will be an array of name/number pairs. */
{
	long encode_size;

	type1_get_token(tok);
	if (tok->type == TTT_NAME 
	&& ((strcmp(tok->string, "StandardEncoding") == 0)
	|| (strcmp(tok->string, "ISOLatin1Encoding") == 0)))
		{
		tcd->encoding = isomap;
		tcd->encoding_count = 256;
		return;
		}
	if (tok->type == TTT_NUMBER)
		{
		encode_size = atoi(tok->string);
		type1_get_token(tok);
		if (tok->type == TTT_NAME 
		&& strcmp(tok->string, "array") == 0)
			{
			type1_parse_custom_encoding(tcd, tok, encode_size);
			return;
			}
		}
	type1_load_error("Strange /Encoding");
}

static void type1_parse_public_definition(Type1_font *tcd, Type1_token *tok)
/* Cope with /XXXX definitions during the public (unencrypted) part of
 * the font.  Right now we ignore everything except /Encoding. */
{
	type1_get_token(tok);
	if (tok->type != TTT_NAME)
		return;
	if (strcmp(tok->string, "Encoding") == 0)
		type1_parse_encoding(tcd, tok);
}

static void type1_parse_to_eexec(Type1_font *tcd, Type1_token *tok)
/* Read through unencrypted part of the file.  Stop at "eexec" 
 * statement. */
{
	for (;;)
		{
		type1_get_token(tok);
		switch (tok->type)
		  {
		  case TTT_EOF:
		    type1_load_error("No eexec in Type1 font file.");
			break;
		  case TTT_NAME:
		    if (strcmp(tok->string, "eexec") == 0)
				return;
			break;
		  case TTT_OTHER:
			if (tok->string[0] == '/')
			    type1_parse_public_definition(tcd, tok);
			break;
		  default:
		  	break;
		  }
		}
}

static void type1_find_mode(Type1_font *tcd, XFILE *fp)
/* 
   (John Walker's comment on how to tell hex from binary.)
   "Adobe Type 1 Font Format Version 1.1", ISBN 0-201-57044-0 states
   on page 64 that one distinguishes an ASCII from a Hexadecimal
   font file by two tests:

		* The first ciphertext byte must not be an ASCII white space
			  character (blank, tab, carriage return or line feed).
		* At least one of the first 4 ciphertext bytes must not be one
		  of the ASCII hexadecimal character codes (a code for 0-9,
		  A-F, or a-f).  These restrictions can be satisfied by adjusting
		  the random plaintext bytes as necessary.

   Well, notwithstanding this statement, Adobe's own Helvetica Bold
   Narrow Oblique file furnished with Adobe Type Manager for Windows
   has a carriage return as the first byte after the eexec invocation.
   Consequently, I turned off recognition of a hex file by the
   presence of a carriage return. */
{
	char cs[4];
	long encrypt_start;
	int i;
	(void)tcd;

	encrypt_start = xftell(fp);
	cs[0] = xfgetc(fp);


	if (cs[0] == ' ' || cs[0] == '\t' ||
		/* cs[0] == '\r' || */
		cs[0] == '\n') 
		{
		byte_in = hex_byte_in;
        } 
	else 
		{
		for (i = 1; i < 4; i++) 
			{
			cs[i] = xfgetc(fp);
            }
		byte_in = hex_byte_in;
		for (i = 0; i < 4; i++) 
			{
			if (!((cs[i] >= '0' && cs[0] <= '0') ||
				  (cs[i] >= 'A' && cs[0] <= 'F') ||
				  (cs[i] >= 'a' && cs[0] <= 'f'))) 
				{
				byte_in = xfgetc;
				break;
                }
            }
        }

	xfseek(fp, encrypt_start, XSEEK_SET);/* Reread encrypted random bytes as
										 * the decrypter depends on everything
										 * from encrypt_start on going through
										 * byte_in(). */
}

#ifdef DEBUG
static void dump_encrypted_part(Type1_font *tcd, XFILE *fp)
/* Run file through decryptor and write it to a debugging file. */
{
	XFILE *out = xfopen("H:decrypt", "wb");
	long encrypt_start = xftell(fp);
	int ch;

    crypt_init(55665);
	while ((ch = decrypt_byte_in(fp)) != EOF)
		xfputc(ch, out);
	xfclose(out);
	xfseek(fp, encrypt_start, XSEEK_SET);
}
#endif /* DEBUG */

static int type1_get_number(Type1_token *tok)
/* Get next token,  make sure it's a number,  and return the
 * atoi'd value of number. */
{
	type1_get_token(tok);
	if (tok->type != TTT_NUMBER)
		type1_load_error("Expecting number got %s\n", tok->string);
	return atoi(tok->string);
}

static void type1_force_symbol(Type1_token *tok, char *symbol)
/* Get next token.  Verify that it matches symbol. */
{
	type1_get_token(tok);
	if (tok->type != TTT_NAME || strcmp(tok->string, symbol) != 0)
		type1_load_error("Expecting %s got %s\n", symbol, tok->string);
}

static void type1_skip_to(Type1_token *tok, char *symbol)
/* Skip tokens until come to one that matches symbol. */
{
	for (;;)
		{
		type1_get_token(tok);
		if (tok->type == TTT_EOF)
			type1_load_error("End of file looking for %s\n", symbol);
		if (strcmp(tok->string, symbol) == 0)
			return;
		}
}


static void type1_force_RD(Type1_token *tok)
/* Make sure that the next bit in the input is either "RD" or "-|" 
 * Also skip the following white space.  */
{
	unsigned char buf[4];

	type1_get_token(tok);
	if (strcmp(tok->string, "RD") == 0)
		{
		type1_get_binary(tok, buf, 1);	/* Skip white space. */
		return;
		}
	if (tok->string[0] == '-')
		{
		type1_get_binary(tok, buf, 2);
		if (buf[0] == '|')
			return;
		}
	type1_load_error("Expecting RD or -|\n");
}

static void type1_read_encoded_buffer(Type1_token *token
, unsigned char *str, int size)
/* Read in bytes from file and (doubly) decrypt.  
 * You may wonder what happens to any pushed-back characters.
 * Well, this is only called in contexts where there will be no
 * pushbacks. */
{
	int i;

	cstrinit();	/* Initialize string decryption. */
	/* We'll throw out the first 4 characters,  only using them
	 * to cycle the decryptor.  */
	for (i=0; i<4; ++i)	/* Decrypt next three. */
		decstr(token->source(token->file));
	/* Now read in and decrypt (again) the string. */
	while (--size >= 0)
		*str++ = decstr(token->source(token->file));
}

static unsigned char *alloc_and_read_RD_string(Type1_font *tcd
,	Type1_token *tok, int binary_size)
{
	unsigned char *buf;

	type1_force_RD(tok);
	buf = loader_alloc(tcd, binary_size);
	type1_read_encoded_buffer(tok, buf, binary_size);
	return buf;
}

static void type1_get_subrs(Type1_font *tcd, Type1_token *tok)
{
/* The Subrs format should be of the form:
 *	NN array 
 *		dup NN NN RD xxxxxxx NP
 *				...
 *		dup NN NN RD xxxxxxx NP
 */
	int sub_count;
	int sub_ix;
	int binary_size;
	unsigned char **subrs;
	int i;

	tcd->sub_count = sub_count = type1_get_number(tok);
	tcd->subrs = subrs = loader_alloc(tcd, sub_count * sizeof(*subrs));
	type1_force_symbol(tok, "array");
	for (i=0; i<sub_count; ++i)
		{
		type1_skip_to(tok, "dup");
		sub_ix = type1_get_number(tok);
		if (sub_ix < 0 || sub_ix >= sub_count)
			type1_load_error("\nSubr %d out of range (0-%d).\n"
			, sub_ix, sub_count);
		binary_size = type1_get_number(tok) - 4;
		subrs[sub_ix] = alloc_and_read_RD_string(tcd, tok, binary_size);
		}
}

static void type1_get_char_strings(Type1_font *tcd, Type1_token *tok)
{
/* The CharStrings format should be of the form:
 *	NN dict dup begin 
 *		/name NN RD xxxxxxx ND
 *				...
 *		/name NN RD xxxxxxx ND
 */
	int letter_count;
	char **letter_names;
	unsigned char **letter_defs;
	int letter_ix = 0;
	int binary_size;

	tcd->letter_count = letter_count = type1_get_number(tok);
	tcd->letter_names = letter_names 
	= loader_alloc(tcd, letter_count * sizeof(*letter_names));
	tcd->letter_defs = letter_defs
	= loader_alloc(tcd, letter_count * sizeof(*letter_defs));
	for (;;)
		{
		type1_get_token(tok);
		if (tok->type == TTT_EOF)
			return;		/* Oh heck, probably have most of the font by now. */
		else if (tok->type == TTT_NAME && strcmp(tok->string, "end") == 0)
			return;
		else if (tok->type == TTT_OTHER && tok->string[0] == '/')
			{
			type1_get_token(tok);
			if (tok->string[0] == '.')	
				{
				/* Here hopefully all we are doing is converting
				 * ".notdef" to "notdef" */
				type1_get_token(tok);
				}
			if (tok->type == TTT_NUMBER)
				{	/* bocklin.pfb has a character with a missing name.
					 * Well, I geuss we kludge around it here... */
				letter_names[letter_ix] = loader_strdup(tcd, "");
				binary_size = atoi(tok->string);
				}
			else
				{
				letter_names[letter_ix] = loader_strdup(tcd, tok->string);
				binary_size = type1_get_number(tok);
				}
			letter_defs[letter_ix] 
			= alloc_and_read_RD_string(tcd, tok, binary_size);
			if (++letter_ix >= letter_count)
				return;
			}
		}
}

static void rtype1(Type1_font *tcd, XFILE *fp)
/*  RTYPE1  --  Load a type 1 font into memory.  */
{
	int i;
	Errcode err;
	Type1_token tok;

	section = Header;

	if ((err = type1_check_signature(fp)) < Success)
		type1_load_error("Can't find !%FontType1 in .PFB file.");
	type1_token_init(&tok, fp, xfgetc);
	type1_parse_to_eexec(tcd, &tok);
	if (tcd->encoding == NULL)
		type1_load_error("No /Encoding array.");
    for (i = 0; i < 6; i++) 
		xfgetc(fp); /* Beats me, but there's 6 trash bytes */
	type1_find_mode(tcd, fp);
#ifdef DEBUG
	dump_encrypted_part(tcd, fp);
#endif /* DEBUG */
    crypt_init(55665);
    /* Now burn the first four plaintext bytes. */
	for (i = 0; i < 4; i++)
		(void) decrypt_byte_in(fp);
	type1_token_init(&tok, fp, decrypt_byte_in);
	for (;;)
		{
		type1_get_token(&tok);
		switch (tok.type)
		  {
		  case TTT_EOF:
			goto DONE;
		  case TTT_OTHER:
		    if (tok.string[0] == '/')
				{
				type1_get_token(&tok);
				if (tok.type == TTT_NAME)
					{
					if (strcmp(tok.string, "Subrs") == 0)
						type1_get_subrs(tcd, &tok);
					else if (strcmp(tok.string, "CharStrings") == 0)
						{
						type1_get_char_strings(tcd, &tok);
						goto DONE;
						}
					}
				}
			break;
		  default:
		    break;
		  }
	}
DONE:
	if (tcd->letter_defs == NULL)
		type1_load_error("No CharStrings!\n");
}

static void tcd_freez(Type1_font **ptcd)
/* Free up memory associated with Type1_font. */
{
Type1_font *tcd;

if (ptcd != NULL && (tcd = *ptcd) != NULL)
	{
	destroy_block_allocator(&tcd->font_ba);
	free_scale_info(&tcd->scale);
	pj_freez(ptcd);
	}
}

static Errcode read_font(XFILE *fp, Type1_font **ptcd)
{
Errcode err = Success;
Type1_font *tcd;

if ((tcd = pj_zalloc(sizeof(*tcd))) == NULL)
	err = Err_no_memory;
else
	{
	construct_block_allocator(&tcd->font_ba, 512L, pj_zalloc, pj_free);
	if (Success != (err = setjmp(type1_load_errhandler)))
		{	/* Got here via longjmp. */
		tcd_freez(&tcd);
		}
	else
		{
		rtype1(tcd, fp);
		}
	}
*ptcd = tcd;
return err;
}

static Errcode find_ascii_values(Type1_font *tcd)
/*****************************************************************************
 * Go through and build up an ascii-ordered array of character definitions.
 ****************************************************************************/
{
	char *name;
	char *ascii_name;
	char **map = tcd->encoding;
	char **names = tcd->letter_names;
	unsigned char **defs = tcd->letter_defs;
	int i,ascii_val;
	int def_ix;
	int def_count;
	int matches = 0;

/* The character defs are stored in the font file in *roughly* ascii order.
 * The logic in this routine takes some advantage of this by starting
 * the search for the next letter where the search for the current letter
 * left off. */
	def_ix = 0;		/* Initialize search starting position. */
	def_count = tcd->letter_count;
	for (ascii_val=0; ascii_val<tcd->encoding_count; ++ascii_val)
		{
		if ((ascii_name = *map++) != NULL)
			{
			i = def_count;
			while (--i >= 0)
				{
				if (++def_ix >= def_count)
					def_ix = 0;
				if ((name = names[def_ix]) != NULL)
					{
					if (strcmp(name, ascii_name) == 0)
						{
						tcd->ascii_defs[ascii_val] = defs[def_ix];
						++matches;
						break;
						}
					}
				}
			}
		}
	if (matches > 0)
		return Success;
	else
		{
		return Err_not_found;
		}
}

static Errcode type1_load_font(char *file_name, Type1_font **ptcd)
/*****************************************************************************
 * Load a type1 font into memory and do everything short of scaling to
 * prepare it for display.
 ****************************************************************************/
{
	Errcode err;
	XFILE *file;

	err = xffopen(file_name, &file, XREADONLY);
	if (err < Success)
		return err;

	err = read_font(file, ptcd);
	if (err >= Success) {
		err = find_ascii_values(*ptcd);
		if (err >= Success)
			calc_font_bounds(*ptcd);
	}

	xffclose(&file);
	return err;
}



/*****************************************************************************
 *****************************************************************************
 ** The Interpreter Section.  This section deals with interpreting the
 ** little reverse-polish-notation language that describes the letters in
 ** the font.  
 *****************************************************************************
 ****************************************************************************/



enum cscommand {
/*  Charstring command op-codes.  */
	Unused_0,
	Hstem,
	Unused_2,
	Vstem,
	Vmoveto,
	Rlineto,
	Hlineto,
	Vlineto,
	Rrcurveto,
	Closepath,
    Callsubr,
    Return,
    Escape,
    Hsbw,
    Endchar,
    Unused_15,
    Unused_16,
    Unused_17,
    Unused_18,
    Unused_19,
    Unused_20,
    Rmoveto,
	Hmoveto,
    Unused_23,
    Unused_24,
    Unused_25,
    Unused_26,
    Unused_27,
    Unused_28,
    Unused_29,
    Vhcurveto,
    Hvcurveto,

    /* 12 x commands */

    Dotsection,
    Vstem3,
    Hstem3,
    Unused_12_3,
    Unused_12_4,
    Unused_12_5,
    Seac,
    Sbw,
    Unused_12_8,
    Unused_12_9,
    Unused_12_10,
    Unused_12_11,
    Div,
    Unused_12_13,
    Unused_12_14,
    Unused_12_15,
    Callothersubr,
    Pop,
    Unused_12_18,
    Unused_12_19,
    Unused_12_20,
    Unused_12_21,
	Unused_12_22,
    Unused_12_23,
    Unused_12_24,
    Unused_12_25,
    Unused_12_26,
    Unused_12_27,
    Unused_12_28,
    Unused_12_29,
    Unused_12_30,
    Unused_12_31,
    Unused_12_32,
    Setcurrentpoint
};

#ifdef DEBUG
static char *cscommand_string[] = {
/*  Charstring command op-codes.  */
	"Unused_0",
	"Hstem",
	"Unused_2",
	"Vstem",
	"Vmoveto",
	"Rlineto",
	"Hlineto",
	"Vlineto",
	"Rrcurveto",
	"Closepath",
    "Callsubr",
    "Return",
    "Escape",
    "Hsbw",
    "Endchar",
    "Unused_15",
    "Unused_16",
    "Unused_17",
    "Unused_18",
    "Unused_19",
    "Unused_20",
    "Rmoveto",
	"Hmoveto",
    "Unused_23",
    "Unused_24",
    "Unused_25",
    "Unused_26",
    "Unused_27",
    "Unused_28",
    "Unused_29",
    "Vhcurveto",
    "Hvcurveto",

    /* 12 x commands */

    "Dotsection",
    "Vstem3",
    "Hstem3",
    "Unused_12_3",
    "Unused_12_4",
    "Unused_12_5",
    "Seac",
    "Sbw",
    "Unused_12_8",
    "Unused_12_9",
    "Unused_12_10",
    "Unused_12_11",
    "Div",
    "Unused_12_13",
    "Unused_12_14",
    "Unused_12_15",
    "Callothersubr",
    "Pop",
    "Unused_12_18",
    "Unused_12_19",
    "Unused_12_20",
    "Unused_12_21",
	"Unused_12_22",
    "Unused_12_23",
    "Unused_12_24",
    "Unused_12_25",
    "Unused_12_26",
    "Unused_12_27",
    "Unused_12_28",
    "Unused_12_29",
    "Unused_12_30",
    "Unused_12_31",
    "Unused_12_32",
    "Setcurrentpoint",
};

char *type1_command_string(unsigned a)
{
if (a < Array_els(cscommand_string))
	return cscommand_string[a];
else
	return "OpTooBig";
}
#endif /* DEBUG */

#define StackLimit  25
#define OtherLimit  10                /* Maximum othersubr return values */

#define Npop(n) sp -= (n)
#define Clear() sp = 0



#define S0  stack[sp - 1]
#define S1  stack[sp - 2]
#define S2  stack[sp - 3]
#define S3  stack[sp - 4]
#define S4  stack[sp - 5]
#define S5  stack[sp - 6]

static long stack[StackLimit];        /* Data stack */
static int sp;                        /* Stack pointer */
static long osres[OtherLimit];        /* Results from othersubrs */
static int orp;                       /* Othersubr result pointer */

#define ReturnStackLimit 10

static unsigned char *rstack[ReturnStackLimit]; /* Return stack */
static int rsp;                       /* Return stack pointer */

static int curx, cury;			      /* The current point */
static int flexing;			          /* If a Flex in progress ? */
static int flexx, flexy;              /* Flex current position */
static Boolean pathopen;                  /* Path open ? */

static int bnum;                  	  /* Line segments per Bezier curve */

static int pcount;

#define AddPoint(output,x,y) {output->shape_point(output,x,y); ++pcount;}
#define ClosePath(output) {if (pcount) output->shape_close(output);pcount=0;}

static void Opath(Type1_output *output)
/*----------------------------------------------------------------------*
 * Start new closed shape if not in the middle of one already.
 *----------------------------------------------------------------------*/
{
        if (!pathopen)
        {
                pathopen = TRUE;
				pcount = 0;
                output->shape_open(output,curx,cury);
        }
}

static void Dpath(Type1_output *output)
/*----------------------------------------------------------------------*
 * Close current shape if any and start a new one.
 *----------------------------------------------------------------------*/
{
		if (pathopen)
        {
                ClosePath(output);
                pathopen = FALSE;
        }
        Opath(output);
}


/*  BEZIER  --  Evaluate a Bezier curve defined by four control
                points.  */


static Errcode draw_bezier(Type1_output *output, long x0, long y0
, long x1, long y1, long x2, long y2, long x3, long y3, int n)
/*----------------------------------------------------------------------*
 *	Draw a bezier curve.
 *----------------------------------------------------------------------*/
{
	int i;
	double ax, bx, cx, ay, by, cy;
	double t, dt = 1.0/n;

	ax = -x0 + 3 * x1 - 3 * x2 + x3;
	bx = 3 * x0 - 6 * x1 + 3 * x2;
	cx = 3 * (x1 - x0);

	ay = -y0 + 3 * y1 - 3 * y2 + y3;
	by = 3 * y0 - 6 * y1 + 3 * y2;
	cy = 3 * (y1 - y0);

	t = dt;
	for (i = 1; i <= n; i++) 
		{
        double vx, vy;

		vx = x0 + t * (cx + t * (bx + t * ax));
		vy = y0 + t * (cy + t * (by + t * ay));
        /* vy = ay * t * t * t + by * t * t + cy * t + y0; */
        t += dt;
		AddPoint(output,vx,vy);
        }
	curx = x3;
	cury = y3;
	return Success;
}


static void othersubr(Type1_output *output, int procno, int nargs, int argp)
/*----------------------------------------------------------------------*
 * Interpret an "other subroutine".  I'm not 100% sure what all this
 * can be.  There are some predefined ones for all fonts,  but potentially they
 * can reside in the file too?
 *----------------------------------------------------------------------*/
{
    static int flexp;                 /* Flex argument pointer */
    static int flexarg[8][2];
    (void)nargs;

    orp = 0;                          /* Reset othersubr result pointer */

	switch (procno) {
        case 0:                       /* Flex */
			draw_bezier(output,
				   flexarg[0][X], flexarg[0][Y],
				   flexarg[2][X], flexarg[2][Y],
				   flexarg[3][X], flexarg[3][Y],
				   flexarg[4][X], flexarg[4][Y], bnum);
			draw_bezier(output,
				   flexarg[4][X], flexarg[4][Y],
				   flexarg[5][X], flexarg[5][Y],
				   flexarg[6][X], flexarg[6][Y],
				   flexarg[7][X], flexarg[7][Y], bnum);
            osres[orp++] = stack[argp + 3];
            osres[orp++] = stack[argp + 2];
            flexing = FALSE;          /* Terminate flex */
            break;

        case 1:                       /* Flex start */
            flexing = TRUE;           /* Mark flex underway */
            flexx = curx;
            flexy = cury;
            flexp = 0;
            /* Note fall-through */
        case 2:                       /* Flex argument specification */
            flexarg[flexp][X] = flexx;
            flexarg[flexp++][Y] = flexy;
            break;

        case 3:                       /* Hint replacement */
			osres[orp++] = 3;		  /* This eventually results in
									   * subroutine 3 being called.
									   * Since subroutine 3 does nothing
									   * but return, one can only guess
									   * what Adobe had in mind designing
									   * this. */
            break;

        default:
    /*        fprintf(stderr, "\nCall to undefined othersubr %d\n",
                procno); */
			break;
    }
}


/*  EXCHARS  --  Execute charstring.  */

static Errcode type1_exchars(Type1_font *tcd, int *pwidth
,	Type1_output *output, int bezier_points, unsigned char *cp
,   int ix, int iy
)
/* Main interpreter routine. Returns Errcode or the width of character. */
{
#define So(n) if ((sp + (n)) > StackLimit) {err = Err_stack; goto ERROR;}
#define Sl(n) if (sp < (n)) {err = Err_stack; goto ERROR;}
	Errcode err = Success;
	int sub;
	int sidebear[2], charwid[2];   /* Character sidebearing and width */

	bnum = bezier_points;			  /* Set how many points in bezier curve. */
	sp = rsp = 0;                     /* Reset stack pointer */
	pathopen = FALSE;
	flexing = FALSE;          

	for (;;)
		{
		int c = *cp++;

		if (c < 32) 
			{
			/* Command */
			if (c == 12) 
				{
				/* Two byte command */
				c = *cp++ + 32;
	            }

#ifdef DEBUG
			ddlog("%s %d %d %d %d %d\n", type1_command_string(c)
			, S0, S1, S2, S3, S4);
#endif /* DEBUG */
			switch (c)
				{
				/* Commands for Starting and Finishing */
				case Endchar:     /* 14: End character */
					Clear();
					goto OUT;
				case Hsbw:        /* 13:  Set horizontal sidebearing */
					Sl(2);
					curx = sidebear[X] = S1 + ix;
					cury = sidebear[Y] = 0 + iy;
					*pwidth = charwid[X] = S0;
					charwid[Y] = 0;
					Clear();
					break;
				case Seac:        /* 12-6:  Standard encoding accented char */
					{
					int lwidth;
					unsigned char *base, *accent;
					unsigned base_ix = S1, accent_ix = S0;
					int xoff = S3, yoff = S2, asb = S4; 
					int fx = curx, fy = cury;
					Sl(5);
					if (base_ix < BYTE_MAX)
						{
						if ((base = tcd->ascii_defs[base_ix]) != NULL)
							{
							if ((err = type1_exchars(tcd, &lwidth, output
							,bezier_points, base, ix, iy)) < Success)
								{
								goto ERROR;
								}
							}
						}
					if (accent_ix < BYTE_MAX)
						{
						if ((accent = tcd->ascii_defs[accent_ix]) != NULL)
							{
							if ((err = type1_exchars(tcd, &lwidth, output
							,bezier_points, accent, ix+xoff+fx-asb
							, iy+fy+yoff)) 
							< Success)
								goto ERROR;
							}
						}
					Clear();
					goto OUT;
					}
				case Sbw:         /* 12-7:  Sidebearing point (x-y) */
					Sl(4);
					curx = sidebear[X] = S3 + ix;
					cury = sidebear[Y] = S2 + iy;
					*pwidth = charwid[X] = S1;
					charwid[Y] = S0;
					Clear();
					break;

				/* Path Construction Commands */
				case Closepath:       /* 9:  Close path */
					if (pathopen)
						ClosePath(output);
					pathopen = FALSE;
					Clear();
					break;
				case Hlineto:         /* 6: Horizontal line to */
					Sl(1);
					Opath(output);
					curx += S0;
					AddPoint(output,curx, cury);
					Clear();
					break;
				case Hmoveto:         /* 22:  Horizontal move to */
					Sl(1);
					if (flexing)
						flexx += S0;
					else
						{
						curx += S0;
						Dpath(output);
						}
					Clear();
					break;
				case Hvcurveto:       /* 31:  Horizontal-vertical curve to */
					Sl(4);
					Opath(output);
					draw_bezier(output, curx, cury, curx + S3, cury,
					   curx + S3 + S2, cury + S1,
					   curx + S3 + S2, cury + S1 + S0, bnum);
					Clear();
					break;
				case Rlineto:         /* 5:  Relative line to */
					Sl(2);
					Opath(output);
					curx += S1;
					cury += S0;
					AddPoint(output,curx, cury);
					Clear();
					break;
				case Rmoveto:         /* 21:  Relative move to */
					Sl(2);
					if (flexing) 
						{
						flexx += S1;
						flexy += S0;
						}
					else
						{
						curx += S1;
						cury += S0;
						Dpath(output);
						}
					Clear();
					break;
				case Rrcurveto:       /* 8:  Relative curve to */
					Sl(6);
					Opath(output);
					draw_bezier(output, curx, cury, curx + S5, cury + S4,
					   curx + S5 + S3, cury + S4 + S2,
					   curx + S5 + S3 + S1, cury + S4 + S2 + S0, bnum);
					Clear();
					break;
				case Vhcurveto:       /* 30:  Vertical-horizontal curve to */
					Sl(4);
					Opath(output);
					draw_bezier(output, curx, cury, curx, cury + S3,
					   curx + S2, cury + S3 + S1,
					   curx + S2 + S0, cury + S3 + S1, bnum);
					Clear();
					break;
				case Vlineto:         /* 7:  Vertical line to */
					Sl(1);
					Opath(output);
					cury += S0;
					AddPoint(output,curx,cury);
					Clear();
					break;
				case Vmoveto:         /* 4:  Vertical move to */
					Sl(1);
					if (flexing)
						flexy += S0;
					else
						{
						cury += S0;
						Dpath(output);
						}
					Clear();
					break;

				/*  Hint Commands  */
				case Dotsection:      /* 12-0:  Dot section */
					Clear();
					break;
				case Hstem:           /* 1:  Horizontal stem zone */
					Sl(2);
					Clear();
					break;
				case Hstem3:          /* 12-2:  Three horizontal stem zones */
					Sl(6);
					Clear();
					break;
				case Vstem:           /* 3:  Vertical stem zone */
					Sl(2);
					Clear();
					break;
				case Vstem3:          /* 12-1:  Three vertical stem zones */
					Sl(6);
					Clear();
					break;
				/* Arithmetic command */

				case Div:             /* 12 12:  Divide */
					Sl(2);
					S1 = (S1 + (S0 / 2)) / S0;
					Npop(1);
					break;

				/* Subroutine Commands */
				case Callothersubr:   /* 12 16:  Call other subroutine */
					Sl(2);
					Sl(2 + S1);
					othersubr(output, S0, S1, sp - (3 + S1));
					Npop(2 + S1);
					break;
				case Callsubr:        /* 10:  Call subroutine */
					Sl(1);
					if (rsp >= ReturnStackLimit) 
						{
						err = Err_stack;
						goto ERROR;
						}
					rstack[rsp++] = cp;
					sub = S0;
					Npop(1);
					if (sub < 0 || sub >= tcd->sub_count) 
						{
						err = Err_function_not_found;
						goto ERROR;
						}
					if (tcd->subrs[sub] == NULL)
						{
						err = Err_null_ref;
						goto ERROR;
						}
					/* Set instruction pointer to subr code */
					cp = tcd->subrs[sub];  
					break;
				case Pop:      /* 12 17:  Return argument from othersubr */
					So(1);
					if (orp <= 0)
						{
						err = Err_stack;
						goto ERROR;
						}
					stack[sp++] = osres[--orp];
					break;
				case Return:   /* 11:  Return from subroutine */
					if (rsp < 1) 
						{
						err = Err_stack;
						goto ERROR;
						}
					cp = rstack[--rsp]; /* Restore pushed call address */
					break;
				case Setcurrentpoint: /* 12 33:  Set current point */
					/* Since we only do flexing, which is done elsewhere,
					 * ignore this. */
					Sl(2);
					curx = S1 + ix;
					cury = S0 + iy;
					Clear();
					break;
				}
			} 
		else 
			{
			long n;

			if (c <= 246)
				{
				n = c - 139;
				} 
			else if (c <= 250) 
				{
				n = ((c - 247) << 8) + *cp++ + 108;
				} 
			else if (c < 255) 
				{
				n = -((c - 251) << 8) - *cp++ - 108;
				} 
			else 
				{
				unsigned char a[4];

				a[0] = *cp++;
				a[1] = *cp++;
				a[2] = *cp++;
				a[3] = *cp++;
				n = (((((a[0] << 8) | a[1]) << 8) | a[2]) << 8) | a[3];
				}
			if (sp >= StackLimit) 
				{
				err = Err_stack;
				goto ERROR;
				} 
			else 
				{
				stack[sp++] = n;
				}
			}
	    }
OUT:
	ClosePath(output);
ERROR:
	return err;
#undef So
}

static Errcode type1_interp_char(Type1_font *tcd,	int *pwidth
,	Type1_output *output, int bezier_points, unsigned char *cp)
/* Returns Errcode or the width of character. */
{
	Errcode err,terr;

	if ((err = output->letter_open(output)) >= Success)
		{
		terr = type1_exchars(tcd,pwidth,output,bezier_points,cp,0,0);
		if (terr >= Success)
			err = output->letter_close(output);
		if (err >= Success)	/* Give precedence to letter_close error. */
			err = terr;
		}
	return err;
}

/*****************************************************************************
 *****************************************************************************
 * Output section - a couple of ways of using the interpreter above.  One
 * for finding the bounds of characters,  and one for actually drawing them.
 *****************************************************************************
 ****************************************************************************/


/*****************Help figure out how much space letter uses*****************/

static void init_bounding_box(Type1_box *b)
/*----------------------------------------------------------------------*
 * Set up bounding box so that any incoming point will set the
 * min and the max.
 *----------------------------------------------------------------------*/
{
	b->xmin = b->ymin = INT_MAX;
	b->xmax = b->ymax = INT_MIN;
}

static void point_into_bounding_box(Type1_box *b, double x, double y)
/*----------------------------------------------------------------------*
 * Update the bounding box with a new point.
 *----------------------------------------------------------------------*/
{
	if (x < b->xmin)
		b->xmin = x;
	if (x > b->xmax)
		b->xmax = x;
	if (y < b->ymin)
		b->ymin = y;
	if (y > b->ymax)
		b->ymax = y;
}

static void box_into_bounding_box(Type1_box *dest
,	Type1_box *new)
/*----------------------------------------------------------------------*
 * Update the bounding box to include a new box.
 *----------------------------------------------------------------------*/
{
	if (new->xmin < dest->xmin)
		dest->xmin = new->xmin;
	if (new->xmax > dest->xmax)
		dest->xmax = new->xmax;
	if (new->ymin < dest->ymin)
		dest->ymin = new->ymin;
	if (new->ymax > dest->ymax)
		dest->ymax = new->ymax;
}

/*********************Stuff to position font on screen************************/
static int sf_xoff, sf_yoff;		/* Upper left corner of current char. */
static double sf_scalex, sf_scaley;	/* How to scale current character. */
static int sf_bezier_points = 8;	/* How many points to put in bezier. */

/* Floating point arithmetic near zero rounds badly.  So we have the
   ROUNDING_KLUDGE to compensate. */
#define ROUNDING_KLUDGE 50

#define X_TO_SCREEN(fo,x) (sf_xoff + (int)((x)*sf_scalex))
/*
#define Y_TO_SCREEN(fo,y) (sf_yoff - (int)((y + ROUNDING_KLUDGE )*sf_scaley))
*/

static int Y_TO_SCREEN(Type1_output *fo, double y)
{
(void)fo;
y = (y + ROUNDING_KLUDGE) * sf_scaley;
return sf_yoff - (int)y;
}

static void set_sf_pos(double xoff, double yoff, double scalex, double scaley)
/*
 * Set up letter scaling and positioning machinery
 */
{
sf_xoff = xoff;
sf_yoff = yoff + ROUNDING_KLUDGE*scaley;
sf_scalex = scalex;
sf_scaley = scaley;
}

/*********************Stuff for finding bounds of letter*********************/
static Type1_box bounds_box;
static int bounds_points;

static Errcode bounds_letter_open(Type1_output *fo)
{
	(void)fo;

	init_bounding_box(&bounds_box);
	return Success;
}

static Errcode bounds_close(Type1_output *fo)
{
	(void)fo;

	return Success;
}

static Errcode bounds_add_point(Type1_output *fo, double x, double y)
{
	(void)fo;

	point_into_bounding_box(&bounds_box, x, -y);
	++bounds_points;
	return Success;
}

static Type1_output bounds_output = 
	{
	bounds_letter_open,
	bounds_close, 
	bounds_add_point, 
	bounds_close, 
	bounds_add_point,
	NULL
	};

static Errcode bounds_interpret(Type1_font *tcd, unsigned char *def
,	Type1_box *bounds)
/************************************************************************
 * 
 ************************************************************************/
{
	int width;

	bounds_points = 0;
	type1_interp_char(tcd, &width, &bounds_output, sf_bezier_points, def);
	if (bounds_points > 0)
		*bounds = bounds_box;
	return width;
}

static void calc_font_bounds(Type1_font *tcd)
/************************************************************************
 *  Figure out the (unscaled) size of each letter in font.
 ************************************************************************/
{
	unsigned char **ascii_defs = tcd->ascii_defs;
	Type1_box *pbounds = tcd->letter_bounds;
	int *pwidth = tcd->letter_width;
	int width, widest;
	unsigned char *def;
	int i;

	init_bounding_box(&tcd->font_bounds);
	i = BYTE_MAX;
	widest = 0;
	while (--i >= 0)
		{
		if ((def = *ascii_defs++) != NULL)
			{
			width = *pwidth = bounds_interpret(tcd, def, pbounds);
			if (width > widest)
				widest = width;
			box_into_bounding_box(&tcd->font_bounds, pbounds);
			}
		++pwidth;
		++pbounds;
		}
	tcd->font_widest = widest;

}

/******************** Bitplane Stuff ************************/

static unsigned char empty_pixel = 0;
static Type1_bitplane empty_bitplane =
/* This is a small empty bitplane. */
	{
	&empty_pixel,1,
	1,1,
	0,0
	};

static Type1_bitplane *alloc_type1_bitplane(Block_allocator *ba
,  Type1_box *bounds)
{
	Type1_bitplane *bits;
	int width, height;

	if ((bits = alloc_from_block(ba, sizeof(*bits))) == NULL)
		return NULL;
	bits->x = bounds->xmin;
	bits->y = bounds->ymin;
	bits->width = width = bounds->xmax - bounds->xmin + 1;
	bits->height = height = bounds->ymax - bounds->ymin + 1;
	bits->bpr = Bitmap_bpr(width);
	if ((bits->bits = alloc_from_block(ba, height*bits->bpr))
	== NULL)
		return NULL;
	return bits;
}

static Errcode type1_bits_hline(SHORT yoff, SHORT xstart, SHORT xend, void *data)
/* Draw a horizontal line on a bitplane. */
{
	Type1_bitplane *bits = data;

	set_bit_hline(bits->bits, bits->bpr, yoff, xstart, xend);
	return Success;
}

static void type1_bits_dot(SHORT x, SHORT y, void *data)
/* Draw a dot on a bitplane. */
{
	Type1_bitplane *bits = data;

	bits->bits[y*bits->bpr + (x>>3)] |= bit_masks[x&7];
}

static void type1_bits_outline(Poly *poly, int xoff, int yoff
, Boolean closed, Type1_bitplane *bits)
/* Draw a polygon outline on a bitplane. */
{
	int count = poly->pt_count;
	LLpoint *this, *next;
	(void)closed;

	this = poly->clipped_list;
	while (--count >= 0)
	{
		next = this->next;
		pj_cline(this->x-xoff, this->y-yoff, next->x-xoff, next->y-yoff
		, type1_bits_dot, bits);
		this = next;
	}
}


/*********************Stuff to draw a filled letter**************************/

static Block_allocator fill_ba;

typedef struct shape_list
	{
	struct shape_list *next;
	LLpoint *points;
	int point_count;
	} Shape_list;

static Shape_list *fill_shape_list;
static Type1_box fill_bounds;


static Errcode fill_point(Type1_output *fo, double x, double y)
/*----------------------------------------------------------------------*
 * Add a point to a filled shape.
 *----------------------------------------------------------------------*/
{
	LLpoint *p;
	int sx,sy;

	if ((p = alloc_from_block(&fill_ba, sizeof(*p))) == NULL)
		return Err_no_memory;
	p->next = fill_shape_list->points;
	fill_shape_list->points = p;
	p->x = sx = X_TO_SCREEN(fo,x);
	p->y = sy = Y_TO_SCREEN(fo,y);
	point_into_bounding_box(&fill_bounds, sx, sy);
	++fill_shape_list->point_count;
	return Success;
}

static Errcode fill_shape_open(Type1_output *fo, double x, double y)
/*----------------------------------------------------------------------*
 * Start a new filled shape.
 *----------------------------------------------------------------------*/
{
	Shape_list *s;

	if ((s = alloc_from_block(&fill_ba, sizeof(*s))) == NULL)
		return Err_no_memory;
	s->next = fill_shape_list;
	s->points = NULL;
	s->point_count = 0;
	fill_shape_list = s;
	return fill_point(fo,x,y);
}

static Errcode fill_shape_close(Type1_output *fo)
/*----------------------------------------------------------------------*
 * Finish up a filled shape.
 *----------------------------------------------------------------------*/
{
	LLpoint *last_point;
	(void)fo;

	last_point = slist_last((Slnode *)(fill_shape_list->points));
	last_point->next = fill_shape_list->points;
	return Success;
}

static Errcode fill_letter_open(Type1_output *fo)
/*----------------------------------------------------------------------*
 * Start a filled letter.
 *----------------------------------------------------------------------*/
{
	(void)fo;

	fill_shape_list = NULL;
	construct_block_allocator(&fill_ba, 512L, pj_malloc, pj_free);
	init_bounding_box(&fill_bounds);
	return Success;
}


static Boolean find_shape_bounds(Type1_box *bounds, Shape_list *shapes)
/*----------------------------------------------------------------------*
 * Make up a bounding box that contains every point in every shape in
 * the shape list.
 *----------------------------------------------------------------------*/
{
	Type1_box b;
	LLpoint *points;
	int point_count;
	int x,y;
	Boolean got_shape = FALSE;

	init_bounding_box(&b);
	while (shapes != NULL)
		{
		/* Ignore dotty input. */
		if ((point_count = shapes->point_count) > 1)
			{
			got_shape = TRUE;
			points = shapes->points;
			while (--point_count >= 0)
				{
				x = points->x;
				y = points->y;
				if (x < b.xmin)
					b.xmin = x;
				if (x > b.xmax)
					b.xmax = x;
				if (y < b.ymin)
					b.ymin = y;
				if (y > b.ymax)
					b.ymax = y;
				points = points->next;
				}
			}
		shapes = shapes->next;
		}
	*bounds = b;
	return got_shape;
}


static Errcode output_shape_list(Type1_output *fo, Shape_list *shape_list)
/*----------------------------------------------------------------------*
 * Render shape list to screen.
 *----------------------------------------------------------------------*/
{
	Poly poly;
	Type1_box bounds;
	Type1_bitplane bits, *pbits;
	Type1_bits_out *bits_out = fo->data;
	Shape_list *shapes;
	Errcode err = Success;

/* If there's no shape data use the empty bitplane. */
	if (!find_shape_bounds(&bounds, shape_list))
		{
		*(bits_out->pbits) = &empty_bitplane;
		return Success;
		}
/* Allocate a single bit-plane buffer big enough for whole shape. */
	if ((*(bits_out->pbits) = pbits 
	= alloc_type1_bitplane(bits_out->ba, &bounds)) == NULL)
		return Err_no_memory;
	bits = *pbits;	/* get a local copy of bitplane for speed. */
/* Add all the shapes to the on-off bitplane. */
	for (shapes = shape_list; shapes != NULL; shapes = shapes->next)
		{
		/* kludge around dotty input. */
		if ((poly.pt_count = shapes->point_count) > 1)
			{
			poly.clipped_list = shapes->points;
			fill_add_shape(&poly, bits.bits, bits.bpr, bits.x, bits.y);
			}
		}
/* Fill on-off bitplane. */
	err = fill_on_off(bits.bpr, bits.width, bits.height, 0, 0
	, bits.bits, type1_bits_hline, &bits);
/* Draw outline of polygon to clean up edges. */
	if (err >= Success)
		{
		for (shapes = shape_list; shapes != NULL; shapes = shapes->next)
			{
			/* kludge around dotty input. */
			if ((poly.pt_count = shapes->point_count) > 1)
				{
				poly.clipped_list = shapes->points;
				type1_bits_outline(&poly,bits.x,bits.y,TRUE,&bits);
				}
			}
		}
	return err;
}

static Errcode fill_letter_close(Type1_output *fo)
/*----------------------------------------------------------------------*
 * Finish up a filled letter.
 *----------------------------------------------------------------------*/
{
	Errcode err;

	err = output_shape_list(fo, fill_shape_list);
	destroy_block_allocator(&fill_ba);
	fill_shape_list = NULL;
	return err;
}

static Type1_output fill_output = 
	{
	fill_letter_open, 
	fill_letter_close, 
	fill_shape_open, 
	fill_shape_close,
	fill_point,
	NULL
	};

static int fill_scale_interpret(Type1_font *tcd	/* Font definition */
, unsigned char *def						/* Code to interpret */
, int xoff, int yoff						/* Upper left corner of result */
, double scalex, double scaley				/* How to scale char */
, Type1_bits_out *bits_out)					/* Where to put bitplane result */
/************************************************************************
 * Call interpreter to actually draw a filled character.
 ************************************************************************/
{
	int width;

	set_sf_pos(xoff, yoff, scalex, scaley);
	fill_output.data = bits_out;
	type1_interp_char(tcd, &width, &fill_output, sf_bezier_points, def);
	return width*scalex;
}

static int fill_interpret(Type1_font *tcd	/* Font definition */
, unsigned char *def						/* Code to interpret */
, int xoff, int yoff						/* Upper left corner of result */
, Type1_bits_out *bits_out)					/* Where to put bitplane result */
/************************************************************************
 * Call interpreter to actually draw a filled character using scale
 * defined in tcd.
 ************************************************************************/
{
	return fill_scale_interpret(tcd, def
	, xoff, yoff, tcd->scale.scalex, tcd->scale.scaley, bits_out);
}

/******************* Simple debugging outline to screen output ****************/
#ifdef DEBUG
static Errcode simple_letter_open(Type1_output *fo)
{
return Success;
}

static Errcode simple_letter_close(Type1_output *fo)
{
return Success;
}

double simple_cx, simple_cy;	/* Offset of point in letter. */
double simple_sx, simple_sy;	/* Offset of letter in screen. */
double simple_fx, simple_fy;	/* Offset of first point in shape. */

static Errcode simple_dot(int x, int y, Raster *r)
/* Draw a dot in color 255. */
{
	pj_put_dot(r, 255, x, y);
	return Success;
}

static void simple_line(Raster *r, int x0, int y0, int x1, int y1)
/* Draw a line in color 255. */
{
	pj_cline(x0, y0, x1, y1, simple_dot, r);
}

static Errcode simple_shape_open(Type1_output *fo, double x, double y)
{
simple_fx = simple_cx = x;
simple_fy = simple_cy = y;
return Success;
}


static Errcode simple_shape_close(Type1_output *fo)
{
simple_line(fo->data, X_TO_SCREEN(fo,simple_cx), Y_TO_SCREEN(fo,simple_cy)
, X_TO_SCREEN(fo, simple_fx), Y_TO_SCREEN(fo,simple_fy));
return Success;
}

static Errcode simple_point(Type1_output *fo, double x, double y)
{
simple_line(fo->data, X_TO_SCREEN(fo,simple_cx), Y_TO_SCREEN(fo,simple_cy)
, X_TO_SCREEN(fo, x), Y_TO_SCREEN(fo,y));
simple_cx = x;
simple_cy = y;
return Success;
}


static Type1_output simple_output = 
	{
	simple_letter_open, 
	simple_letter_close, 
	simple_shape_open, 
	simple_shape_close,
	simple_point,
	};

static int simple_interpret(Type1_font *tcd	/* Font definition */
, unsigned char *def						/* Code to interpret */
, int xoff, int yoff						/* Upper left corner of result */
, Raster *screen)							/* Where to draw char */
/************************************************************************
 * Call interpreter to actually draw a simple character.
 ************************************************************************/
{
	int width;

	set_sf_pos(xoff, yoff, tcd->scale.scalex, tcd->scale.scaley);
	simple_output.data = screen;
	type1_interp_char(tcd, &width, &simple_output, sf_bezier_points, def);
	return width*tcd->scale.scalex;
}

#endif /* DEBUG */


/*******************Stuff to scale font*******************************/

static void free_scale_info(Type1_scale_info *si)
/*-----------------------------------------------------------------------
 * Free up resources associated with one size of font.
 *----------------------------------------------------------------------*/
{
	destroy_block_allocator(&si->ba);
}

static int find_right_overlap(Type1_font *tcd, double scalex)
/*----------------------------------------------------------------------*
 * Find out the maximum difference between the right edge of a letter in
 * font and the width the same letter.  (This will be useful in telling
 * us how far to the right of the ostensible string width we need to
 * erase when undrawing.)
 *----------------------------------------------------------------------*/
{
	int *pwidth = tcd->scale.width;
	Type1_box *pbox = tcd->letter_bounds;
	unsigned char **ascii_defs = tcd->ascii_defs;
	int i = BYTE_MAX;
	int font_right_overlap = 0;
	int char_right_overlap;

	while (--i >= 0)
		{
		if (*ascii_defs++ != NULL)
			{
			if ((char_right_overlap = pbox->xmax * scalex - *pwidth) 
			> font_right_overlap)
				font_right_overlap = char_right_overlap;
			}
		++pwidth;
		++pbox;
		}
	return font_right_overlap;
}

static void scale_box(double scalex, double scaley
, Type1_box *in, Type1_box *out)
{
	out->xmin = in->xmin * scalex;
	out->ymin = in->ymin * scaley;
	out->xmax = in->xmax * scalex;
	out->ymax = in->ymax * scaley;
}

static void scale_type1_font(Type1_font *tcd, double scalex, double scaley)
/*----------------------------------------------------------------------*
 *  Set up font for a particular size.
 *----------------------------------------------------------------------*/
{
	int *lwidth = tcd->letter_width;
	int *pscaled = tcd->scale.width;
	int i;
	long mem_block_size;

	tcd->scale.scalex = scalex;
	tcd->scale.scaley = scaley;
	tcd->scale.widest = tcd->font_widest * scaley;
	scale_box(scalex, scaley, &tcd->font_bounds, &tcd->scale.max_bounds);
	i = BYTE_MAX;
	while (--i >= 0)
		*pscaled++ = *lwidth++ * scalex;
	tcd->scale.right_overlap = find_right_overlap(tcd, scalex);
	/* Set up # of points in bezier to correspond roughly with
	 * resolution. Also set up memory block size so at small resolutions
	 * we allocate in 512 blocks, but at larger resolutions allocate
	 * for each letter individually. */
	if (scalex < 0.02)
		{
		sf_bezier_points = 8;
		tcd->scale.unzag_shrink = 8;
		mem_block_size = 512;
		}
	else if (scalex < 0.10)
		{
		sf_bezier_points = 16;
		tcd->scale.unzag_shrink = 4;
		mem_block_size = 10*1024;
		}
	else
		{
		sf_bezier_points = 32;
		tcd->scale.unzag_shrink = 2;
		mem_block_size = 64L*1024;
		}
	free_scale_info(&tcd->scale);
	construct_block_allocator(&tcd->scale.ba, mem_block_size
	,	pj_zalloc, pj_free);
	clear_mem(tcd->scale.bits, sizeof(tcd->scale.bits));
	clear_mem(tcd->scale.alphas, sizeof(tcd->scale.alphas));

}

static void set_type1_height(Type1_font *tcd, int height)
/************************************************************************
 *  Scale font so it will be a specific height.  Scale X and Y the same
 *  amount.
 ************************************************************************/
{
	double scale;
	double unscaled_height;

	unscaled_height = tcd->font_bounds.ymax - tcd->font_bounds.ymin + 1;
	scale = (double)height/unscaled_height;
	scale_type1_font(tcd, scale, scale);
}

/*****************************************************************************
 *****************************************************************************
 ** Alpha channel handling section.
 *****************************************************************************
 ****************************************************************************/

static Errcode shrink_bitplane_to_alpha(Block_allocator *ba
,  Type1_bitplane *source, int shrink, Type1_alpha **palpha)
/*****************************************************************************
 * This allocates an alpha channel and shrinks bitplane into it. 
 ****************************************************************************/
{
	Type1_alpha *alpha;
	int y_remainder;

	/* Allocate alpha structure. */
	if ((alpha = alloc_from_block(ba, sizeof(*alpha))) == NULL)
		return Err_no_memory;

	/* Shrink x offset of alpha channel and width. */
	alpha->x = (source->x + shrink/2)/shrink;
	alpha->width = (source->width + shrink-1) / shrink;

	/* Shrink y offset and height of alpha channel.  This may include a
	 * fractional element which gets passed to the shrinker routine.
	 */
	alpha->y = source->y / shrink;
	y_remainder = source->y % shrink;
	alpha->height = (source->height + y_remainder + shrink-1) / shrink;

	/* Allocate the pixels in alpha channel.
	 */
	alpha->bpr = alpha->width * sizeof(alpha->channel[0]);
	if ((alpha->channel = alloc_from_block(ba, alpha->height*alpha->bpr))
	== NULL)
		return Err_no_memory;

	/* Now copy in the shrunken bits. */
	bitmask_to_alpha_channel(alpha->channel, shrink
	, source->bits, source->width, source->height, source->bpr
	, y_remainder);

	*palpha = alpha;
	return Success;
}


typedef void (*Talpha_blit)(UBYTE *alpha, int abpr, int x, int y, int w, int h
, Rcel *r, Pixel oncolor);

#ifdef UNUSED
void mask_alpha_blit(UBYTE *alpha, int abpr, int x, int y, int w, int h
, Rcel *r, Pixel oncolor)
/*****************************************************************************
 * draw opaque color transparently through alpha channel data onto a raster...
 ****************************************************************************/
{
int 	curx;
int 	endx;
UBYTE	curalpha;
UBYTE	*palpha;
Rgb3 dest;
Cmap *cmap = r->cmap;
Rgb3 *ctab = cmap->ctab;

endx = x + w;

while (h--) 
	{
	palpha	= alpha;
	for (curx = x; curx < endx; ++curx) 
		{
		curalpha = *palpha++;
		if (curalpha) 
			{
			if (curalpha == 255) 
				{
				pj_put_dot(r, oncolor, curx, y);
				} 
			else 
				{
				alpha_blend(&ctab[pj_get_dot(r, curx, y)]
				, &ctab[oncolor], &dest, curalpha);
				pj_put_dot(r, closestc(&dest, ctab, cmap->num_colors)
				, curx, y);
				}
			}
		}
	alpha += abpr;
	++y;
	}
}
#endif /* UNUSED */

static Errcode new_alpha_image(Type1_font *tcd
, unsigned char letter, Type1_alpha **palpha)
/*****************************************************************************
 * Return a new alpha channel for letter in *palpha. Alpha channel
 * is allocated in tcd->scale.ba. 
 ****************************************************************************/
{
Errcode err;				/* Did this routine work?  Store result here. */
Type1_bitplane *bits;		/* Points to temporary (big) bitplane. */
Block_allocator local_ba;	/* Where we store our temp bitplane. */
int shrink = tcd->scale.unzag_shrink;	
static Type1_bits_out output;	/* Parameter struct for fill_interpret() */
Type1_alpha *alpha;			/* Our brand new alpha channel for char. */

construct_block_allocator(&local_ba, 512L, pj_zalloc, pj_free);
output.ba = &local_ba;
output.pbits = &bits;
if ((err = fill_scale_interpret(tcd, tcd->ascii_defs[letter], 0, 0, 
shrink*tcd->scale.scalex, shrink*tcd->scale.scaley, &output)) >= Success)
	{
	bits->y -= shrink*tcd->scale.scaley * tcd->font_bounds.ymin;
	if ((err = shrink_bitplane_to_alpha(&tcd->scale.ba, bits, shrink, &alpha))
	>= Success)
		{
		*palpha = alpha;
		}
	}
destroy_block_allocator(&local_ba);
return err;
}

static Errcode get_alpha_image(Type1_font *tcd
, unsigned char letter, Type1_alpha **palpha)
/*****************************************************************************
 * Checks cache for rasterized alpha, and returns quickly if it's there.
 * Otherwise calls new_alpha_image to put it in the cache.
 ****************************************************************************/
{
	Type1_alpha *alpha;
	Errcode err;

	/* Check cache and return quickly with result if it's there. */
	if ((alpha = tcd->scale.alphas[letter]) == NULL)
		{
		if ((err = new_alpha_image(tcd,letter,&alpha)) < Success)
			return err;
		else
			tcd->scale.alphas[letter] = alpha;
		}
	*palpha = alpha;
	return Success;
}

static Errcode soft_gftext(Raster *rast,
			Vfont *v,
			register unsigned char *s,
			int x,int y,
			Pixel color,
			Talpha_blit alpha_blit)
/*****************************************************************************
 * Draw a oversampled text string in the font in opaque ink.   
 ****************************************************************************/
{
	Type1_font *tcd = v->font;
	Type1_alpha *alpha;
	unsigned char c;
	unsigned char *def;
	Errcode err;

	while ((c = *s++) != EOS)
		{
		/* Crude clipping for speed */
		if (x >= rast->width || y >= rast->height)	
			break;
		/* Do tabs here. */
		if (c == '\t')
			{
			x += v->tab_width;
			}
		else if ((def = tcd->ascii_defs[c]) != NULL)
			{
			if ((err = get_alpha_image(tcd, c, &alpha)) < Success)
				return err;
			(*alpha_blit)(alpha->channel, alpha->bpr, x + alpha->x
			, y + alpha->y, alpha->width, alpha->height, (Rcel *)rast, color);
			x += tcd->scale.width[c] + v->spacing;
			}
		}
	return Success;
}

#ifdef DEBUG
void alpha_blit(UBYTE *alpha, int abpr, int x, int y, int w, int h, Rcel *r)
/*****************************************************************************
 * Blit alpha channel straight to the screen.  Will get something meaningful
 * looking if the screen color map happens to be greyscale. Part of 
 * alpha-shrinky test scaffolding.
 ****************************************************************************/
{
int 	curx;
int 	endx;
UBYTE	curalpha;
UBYTE	*palpha;

endx = x + w;

while (h--) 
	{
	palpha	= alpha;
	for (curx = x; curx < endx; ++curx) 
		{
		curalpha = *palpha++;
		pj_put_dot(r, curalpha, curx, y);
		}
	alpha += abpr;
	++y;
	}
}

Errcode shrinky_blit(Type1_bitplane *bits
, int x, int y, Rcel *rast, Pixel color)
/*
 * Test scaffolding for alpha-channeled fonts. 
 */
{
UBYTE *channel;
int shrink = 4;
int y_remainder = bits->y % shrink;
int width = bits->width, height = bits->height;
int nwidth = (width+shrink-1)/shrink;
int nheight = (height+y_remainder+shrink-1)/shrink;


if ((channel = pj_malloc(nwidth * nheight * sizeof(Pixel) ))
== NULL)
	return Err_no_memory;
bitmask_to_alpha_channel(channel, shrink
, bits->bits, width, height,  bits->bpr, y_remainder);
mask_alpha_blit(channel, nwidth
, x+bits->x/shrink, y+bits->y/shrink, nwidth, nheight, rast, color);
//alpha_blit(channel, nwidth, x+bits->x/shrink
//, y+bits->y/shrink, nwidth, nheight, rast);

pj_free(channel);
}
#endif /* DEBUG */




/*****************************************************************************
 *****************************************************************************
 ** PJ Vfont glue section.  This bit folds the type1 stuff into the
 ** protocol shared by all types of fonts that PJ uses.
 *****************************************************************************
 ****************************************************************************/


/****************************Vfont functions*********************************/

static void attatch_type1_scale(Vfont *vfont, Type1_font *tcd);

static void vfont_free(Vfont *v)
{
	tcd_freez((Type1_font **)&v->font);
}

#ifdef DEBUG
void type1_debug()	// Just a convenient break-point
{
	in_debug = TRUE;
}
#endif /* DEBUG */


static Errcode get_bit_image(Type1_font *tcd
, unsigned char letter, Type1_bitplane **pbits)
/*****************************************************************************
 * Checks cache for bitmap, and returns quickly if it's there.
 * Otherwise calls PS Type II interpreter to make a bitmap  
 * and puts it in the cache.
 ****************************************************************************/
{
	static Type1_bits_out output;
	Type1_bitplane *bits;
	Errcode err;

	if ((bits = tcd->scale.bits[letter]) != NULL)
		{
		*pbits = bits;
		return Success;
		}
	output.ba = &tcd->scale.ba;
	output.pbits = &tcd->scale.bits[letter];
#ifdef DEBUG
	ddlog(">>> %c [%#x] <<<\n", letter, letter);
#endif /* DEBUG */
	if ((err = fill_interpret(tcd
	, tcd->ascii_defs[letter], 0, 0, &output)) 
	>= Success)
		{
		bits = *pbits = *(output.pbits);
		bits->y -= tcd->scale.max_bounds.ymin;
		return Success;
		}
	return err;
}

static Errcode vfont_gftext(Raster *rast,
			Vfont *v,
			register unsigned char *s,
			int x,int y,
			Pixel color,Text_mode tmode,
			Pixel bcolor)
/*****************************************************************************
 * Draw a text string in the font.  
 * For each character this guy checks to see if 
 * it's already been rendered as a bitmap.  If not vfont_gftext calls the 
 * PS Type II interpreter to make a bitmap.  Then he draws the bitmap.
 ****************************************************************************/
{
	Type1_font *tcd = v->font;
	Type1_bitplane *bits;
	unsigned char c;
	unsigned char *def;
	Errcode err;

	if (tcd->scale.unzag_flag)
		{
		if (tmode == TM_RENDER)
			{
			return soft_gftext(rast, v, s, x, y, color
			, render_mask_alpha_blit);
			}
		}
	while ((c = *s++) != EOS)
		{
			/* Crude clipping for speed */
		if (x >= rast->width || y >= rast->height)	
			break;
		if (c == '\t')
			{
			x += v->tab_width;
			}
		else if ((def = tcd->ascii_defs[c]) != NULL)
			{
			if ((err = get_bit_image(tcd, c, &bits)) < Success)
				return err;
			blit_for_mode(tmode
			, bits->bits, bits->bpr, 0, 0
			, rast, x+bits->x, y+bits->y
			, bits->width, bits->height, color, bcolor);
			x += tcd->scale.width[c] + v->spacing;
			}
		}
	return Success;
}

static int vfont_char_width(Vfont *v, UBYTE *s)
{
	Type1_font *tcd = v->font;

	return (tcd->scale.width[s[0]] + v->spacing);
}

static Boolean vfont_in_font(Vfont *v, int c)
{
	Type1_font *tcd = v->font;

	return ( c >= 0 && c < BYTE_MAX && tcd->ascii_defs[c] != NULL);
}

static Errcode vfont_scale_font(Vfont *v, int height)
{
	Type1_font *tcd = v->font;

	set_type1_height(tcd, height);
	attatch_type1_scale(v, tcd);
	return Success;
}

static Errcode vfont_change_unzag(Vfont *v, Boolean unzag)
{
Type1_font *tcd = v->font;
tcd->scale.unzag_flag = unzag;
scale_type1_font(tcd, tcd->scale.scalex, tcd->scale.scaley);
return Success;
}

/****************************Font_dev functions******************************/

static Errcode check_type1_font(char *name)
/* Verify it's a Post-script font by looking for the !% signature in the
 * first 128 bytes, and making sure the file suffix starts with a 'p' */
{
XFILE *f;
char *suff = pj_get_path_suffix(name);
Errcode err;

if (!(suff[1] == 'p' || suff[1] == 'P'))
	return Err_suffix;
if ((err = xffopen(name, &f, XREADONLY)) < Success)
	return err;
err = type1_check_signature(f);
xffclose(&f);
return err;
}

static void attatch_type1_scale(Vfont *vfont, Type1_font *tcd)
/*----------------------------------------------------------------------*
 * Fill out scale related bits of Vfont structure from relevant fields 
 * of Type1_font.
 *----------------------------------------------------------------------*/
{
vfont->widest_image = tcd->scale.max_bounds.xmax 
- tcd->scale.max_bounds.xmin + 1;
vfont->widest_char = tcd->scale.widest;
vfont->tab_width = vfont->widest_char*TABEXP;
vfont->image_height = tcd->scale.max_bounds.ymax 
- tcd->scale.max_bounds.ymin + 1;
vfont->line_spacing = vfont->image_height + vfont->leading;
if (tcd->scale.max_bounds.xmin < 0)
	vfont->left_overlap = -tcd->scale.max_bounds.xmin;
vfont->right_overlap = tcd->scale.right_overlap;
scan_init_vfont(vfont);
}

static void attatch_type1_font(Vfont *vfont, Type1_font *tcd)
/*----------------------------------------------------------------------*
 * Fill out Vfont structure from relevant fields of Type1_font.
 *----------------------------------------------------------------------*/
{
clear_struct(vfont);
vfont->type = TYPE1FONT;
vfont->font = tcd;
vfont->close_vfont = vfont_free;
vfont->gftext = vfont_gftext;
vfont->char_width = vfont_char_width;
vfont->scale_font = vfont_scale_font;
vfont->in_font = vfont_in_font;
vfont->change_unzag = vfont_change_unzag;
vfont->flags = VFF_SCALEABLE;
}

#ifdef DEBUG

static char *letter_name(Type1_font *tcd, int ascii_ix)
/* Returns name of ascii letter in this font. */
{
unsigned char *def = tcd->ascii_defs[ascii_ix];
int i;
if (def == NULL)
	return "(null)";
for (i=0; i<tcd->letter_count; ++i)
	{
	if (tcd->letter_defs[i] == def)
		return tcd->letter_names[i];
	}
return "(not found)";
}

static Boolean is_used(Type1_font *tcd, unsigned char *def)
/*
 * Finds out if a def is part of ascii def.
 */
{
int i;
for (i=0; i<BYTE_MAX; ++i)
	{
	if (tcd->ascii_defs[i] == def)
		return TRUE;
	}
return FALSE;
}

static void print_unused(Type1_font *tcd)
/*
 * Print out names of letters that aren't assigned to an ascii_def
 */
{
int i;

dlog("\nUnused letters:\n");
for (i=0; i<tcd->letter_count; ++i)
	{
	if (!is_used(tcd, tcd->letter_defs[i]))
		dlog("%s\n", tcd->letter_names[i]);
	}
}

static void test_type1(Type1_font *tcd)
{
	int i;
	unsigned char *def;
	char *name;
	int xoff = 0;
	int width;

	dlog("encoding_count, vector = %d %p\n"
	, tcd->encoding_count, tcd->encoding);
	dlog("Got %d letters\n", tcd->letter_count);
	dlog("\n");
	dlog("letter_defs & letter_names\n");
	for (i=0; i<tcd->letter_count; ++i)
		{
		def = tcd->letter_defs[i];
		name = tcd->letter_names[i];
		if (name != NULL)
			{
			dlog("%d: %s %p\n", i, name, def);
			}
		else 
			{
			dlog("%d: (null) %p\n", i, def);
			}
		}
	dlog("\n");
	dlog("ascii_defs & width\n");
	for (i=0; i<BYTE_MAX; ++i)
		{
		char c;

		if (isprint(i))
			c = i;
		else
			c = '.';
		dlog("%c %#x :: %s at %p width %d/%d\n"
		, c, i, letter_name(tcd, i),
		tcd->ascii_defs[i], tcd->scale.width[i], tcd->letter_width[i]);
		}
	print_unused(tcd);
	for (i='a'; i<'m'; ++i)
		{
		if ((tcd->ascii_defs[i]) != NULL)
			{
			width = simple_interpret(tcd,tcd->ascii_defs[i]
			, xoff,200,(Raster *)vb.pencel);
			dlog("%c %d %d\n", i, xoff, width);
			xoff += width;
			}
		}
}

static void show_unused(Type1_font *tcd, Raster *rast, Block_allocator *ba)
{
Type1_bitplane *bits;
Type1_bits_out output;
int i;
unsigned char *def;
char *name;
Errcode err;

output.ba = ba;
output.pbits = &bits;
for (i=0; i<tcd->letter_count; ++i)
	{
	def = tcd->letter_defs[i];
	name = tcd->letter_names[i];
	if (def != NULL && !is_used(tcd, def))
		{
		if ((err = fill_interpret(tcd, def, 0, 0, &output)) > Success)
			{
			pj_set_rast(rast, 0);
			pj_mask2blit(bits->bits, bits->bpr, 0, 0
			, rast, 100+bits->x, 100+bits->y
			, bits->width, bits->height, 31, 8);
			gftext(rast,get_sys_font(),name,0,150,31,TM_MASK2,0);
			dos_wait_key();
			}
		else
			{
			errline(err, "Couldn't interpret %s", name);
			break;
			}
		}
	}
}


void test_font(Raster *rast, Vfont *v)
{
}
#endif /* DEBUG */


static Errcode load_vfont(char *title, Vfont *vfont, SHORT height
, SHORT unzag_flag)
{
	Type1_font *tcd;
	Errcode err;

	if (height <= 0)
		height = 100;		/* Set default height. */
	if ((err = type1_load_font(title, &tcd)) >= Success)
		{
		attatch_type1_font(vfont, tcd);
		tcd->scale.unzag_flag = unzag_flag;
		vfont_scale_font(vfont, height);
		vfont->spacing = 1;
		}
#ifdef DEBUG
	test_type1(tcd);
#endif /* DEBUG */
	return err;
}

Font_dev type1_font_dev = {
NULL,
"Postscript Type 1",
"*.PF?",
check_type1_font,
load_vfont,
TYPE1FONT,
0
};

