
/* Read Adobe Type 1 Fonts Into Shaper */

#include "jimk.h"
#include "type1.h"
#include "keys.h"

/* Uncomment next lines for debugging */
/*#define DBGAT1*/
/*#define DBGAT2*/
/*#define DBGAT3*/
/*#define DBGAT4*/
/*#define DBGAT5*/
/*#define DBGAT6*/
/*#define SAVEIT*/

extern char gp_buffer[];
extern unsigned char *holebuf;
extern char *strstr();
extern int error_in_font;

/* Local allocated areas */

static unsigned char cs[4];
static unsigned char *csp;
static int csl;
static enum {Unknown, Binary, Hex} filemode = Unknown, forcemode = Unknown;
static FILE *t1handle=NULL;
char *t1pointer= (char*)(&t1handle);
static double skewsin = 0.0;                  /* Sine of ItalicAngle for obliquing */

static int ncdefs;                            /* Number of character definitions */
static unsigned char *chardefs[NCdefs][2];   /* Character definitions */
static int chardl[NCdefs];                    /* Character definition lengths */

static int nsubrs = 0;                /* Number of subroutines */
static unsigned char **subrs = NULL;          /* Subroutine pointer table */
static int *subrl=NULL;                       /* Subroutine lengths */

char t1_fullname[256];          /* Font name */
int t1_chars;                   /* Chars in font */
int t1_width;                   /* Current char width */
int t1_minx,t1_miny,t1_maxx,t1_maxy;
int dbgat3=0;

unsigned char databyte;

static file_section section;

static char *csnames[] = {
    "(0)",
    "hstem",
    "(2)",
    "vstem",
    "vmoveto",
    "rlineto",
    "hlineto",
    "vlineto",
    "rrcurveto",
    "closepath",
    "callsubr",
    "return",
    "escape",
    "hsbw",
    "endchar",
    "(15)",
    "(16)",
    "(17)",
    "(18)",
    "(19)",
    "(20)",
    "rmoveto",
    "hmoveto",
    "(23)",
    "(24)",
    "(25)",
    "(26)",
    "(27)",
    "(28)",
    "(29)",
    "vhcurveto",
    "hvcurveto",

    /* 12 x commands */

    "dotsection",
    "vstem3",
    "hstem3",
    "(12-3)",
    "(12-4)",
    "(12-5)",
    "seac",
    "sbw",
    "(12-8)",
    "(12-9)",
    "(12-10)",
    "(12-11)",
    "div",
    "(12-13)",
    "(12-14)",
    "(12-15)",
    "callothersubr",
    "pop",
    "(12-18)",
    "(12-19)",
    "(12-20)",
    "(12-21)",
    "(12-22)",
    "(12-23)",
    "(12-24)",
    "(12-25)",
    "(12-26)",
    "(12-27)",
    "(12-28)",
    "(12-29)",
    "(12-30)",
    "(12-31)",
    "(12-32)",
    "setcurrentpoint"
};
static int ncsnames = ELEMENTS(csnames);

/* Data stuff from INTERP.C */

#define StackLimit  25

#define Sl(n) if (sp < (n)) {fflush(stdout); continu_line("Stack underflow."); return;}
#define Npop(n) sp -= (n)
#define So(n) if ((sp + (n)) > StackLimit) {fflush(stdout); continu_line("Stack overflow."); return;}
#define Clear() sp = 0
#define Opath() if (!pathopen) { psx = curx; psy = cury; pathopen = TRUE; }
#define Dpath() pathopen = FALSE; Opath()

#define S0  stack[sp - 1]
#define S1  stack[sp - 2]
#define S2  stack[sp - 3]
#define S3  stack[sp - 4]
#define S4  stack[sp - 5]
#define S5  stack[sp - 6]

static long stack[StackLimit];        /* Data stack */
static int sp;                        /* Stack pointer */

#define ReturnStackLimit 10

static unsigned char *rstack[ReturnStackLimit]; /* Return stack */
static int rsp;                       /* Return stack pointer */

static int curx = 0, cury = 0;        /* The current point */
static int psx = 0, psy = 0;          /* Path start co-ordinates */
static int pathopen;                  /* Path open ? */

/* Data stuff from SHIPSHAP.C */


/*  Map of PostScript character names to ISO 8859-1 Latin 1  */

static char *isomap[224] = {
    "space",             /*  32 */
    "exclam",            /*  33 */
    "quotedbl",          /*  34 */
    "numbersign",        /*  35 */
    "dollar",            /*  36 */
    "percent",           /*  37 */
    "ampersand",         /*  38 */
    "quoteright",        /*  39 */
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
    NULL,            /* 127 */
    "Ccedilla",            /* 128 */
    "uumlaut",            /* 129 */
    "eacute",            /* 130 */
    "acircumflex",               /* 131 */
    "aumlaut",           /* 132 */
    "agrave",            /* 133 */
    "aring",             /* 134 */
    "ccedilla",          /* 135 */
    "ecircumflex",               /* 136 */
    "eumlaut",           /* 137 */
    "egrave",            /* 138 */
    "iumlaut",           /* 139 */
    "icircumflex",               /* 140 */
    "igrave",            /* 141 */
    "Aumlaut",           /* 142 */
    "Aring",             /* 143 */
    "Eacute",            /* 144 */
    "ae",            /* 145 */
    "AE",        /* 146 */
    "ocircumflex",               /* 147 */
    "oumlaut",           /* 148 */
    "ograve",            /* 149 */
    "ucircumflex",               /* 150 */
    "ugrave",            /* 151 */
    "yumlaut",           /* 152 */
    "Oumlaut",           /* 153 */
    "Uumlaut",           /* 154 */
    "cent",              /* 155 */
    "sterling",          /* 156 */
    "yen",               /* 157 */
    NULL,                /* 158 */
    NULL,                /* 159 */
    "aacute",             /* 160 */
    "iacute",        /* 161 */
    "oacute",              /* 162 */
    "uacute",          /* 163 */
    "ntilde",          /* 164 */
    "Ntilde",               /* 165 */
    NULL,               /* 166 */
    NULL,           /* 167 */
    "questiondown",          /* 168 */
    NULL,         /* 169 */
    NULL,       /* 170 */
    "onehalf",     /* 171 */
    "onequarter",        /* 172 */
    "exclamdown",             /* 173 */
    NULL,        /* 174 */
    NULL,            /* 175 */
    NULL,              /* 176 */
    NULL,         /* 177 */
    NULL,       /* 178 */
    NULL,     /* 179 */
    NULL,             /* 180 */
    NULL,                /* 181 */
    NULL,         /* 182 */
    NULL,    /* 183 */
    NULL,           /* 184 */
    NULL,       /* 185 */
    NULL,      /* 186 */
    NULL,    /* 187 */
    NULL,        /* 188 */
    NULL,           /* 189 */
    NULL,     /* 190 */
    NULL,      /* 191 */
    NULL,            /* 192 */
    NULL,            /* 193 */
    NULL,       /* 194 */
    NULL,            /* 195 */
    NULL,         /* 196 */
    NULL,             /* 197 */
    NULL,                /* 198 */
    NULL,          /* 199 */
    NULL,            /* 200 */
    NULL,            /* 201 */
    NULL,       /* 202 */
    NULL,         /* 203 */
    NULL,            /* 204 */
    NULL,            /* 205 */
    NULL,       /* 206 */
    NULL,         /* 207 */
    NULL,               /* 208 */
    NULL,            /* 209 */
    NULL,            /* 210 */
    NULL,            /* 211 */
    NULL,       /* 212 */
    NULL,            /* 213 */
    NULL,         /* 214 */
    NULL,          /* 215 */
    NULL,            /* 216 */
    NULL,            /* 217 */
    NULL,            /* 218 */
    NULL,       /* 219 */
    NULL,         /* 220 */
    NULL,            /* 221 */
    NULL,             /* 222 */
    NULL,        /* 223 */
    "alpha",            /* 224 */
    "beta",            /* 225 */
    "gamma",       /* 226 */
    "pi",            /* 227 */
    "Sigma",         /* 228 */
    "sigma",             /* 229 */
    "mu",                /* 230 */
    "tau",          /* 231 */
    "Phi",            /* 232 */
    "theta",            /* 233 */
    "omega",       /* 234 */
    "delta",         /* 235 */
    "infinity",            /* 236 */
    "phi",            /* 237 */
    "epsilon",       /* 238 */
    NULL,         /* 239 */
    NULL,               /* 240 */
    "plusminus",            /* 241 */
    NULL,            /* 242 */
    NULL,            /* 243 */
    NULL,       /* 244 */
    NULL,            /* 245 */
    "divide",         /* 246 */
    NULL,            /* 247 */
    "degree",            /* 248 */
    NULL,            /* 249 */
    "bullet",            /* 250 */
    NULL,       /* 251 */
    NULL,         /* 252 */
    "twosuperior",            /* 253 */
    NULL,             /* 254 */
    NULL          /* 255 */
};

char *usermap[224];                   /* User-defined mapping vector */

char **charmap = isomap;              /* Active mapping vector */

FILE *ostream=NULL;

/*  STRSAVE  --  Allocate a duplicate of a string.  */

static char *strsave(s)
  char *s;
{
char *c;

    if((c=(char *) malloc((unsigned) (strlen(s) + 1)))==NULL)
     return(NULL);
    strcpy(c, s);
    return c;
}

/*  INBYTE  --  Get the next binary byte from the file, either in
                hexadecimal or binary mode. */

static int inbyte(FILE *fp,unsigned char *val)
{
    int c, xd = 0, i;
    unsigned char uc;

    for (i = 0; i < 2; i++) {
        while (TRUE) {
            if (csl > 0) {
                c = *csp++;
                csl--;
            } else {
#ifdef DBGAT5
printf("About to read file @ %d\n",ftell(fp));
#endif
                c=getc(fp);
                if(c==EOF)
                        return(0);
            }  
            if (filemode == Binary)
                {
                *val=c;
                return 1;
                }
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
            continu_line("Bad hex digit");
            return 0;
        }
        xd = (xd << 4) | c;
    }
    *val=xd;
    return 1;
}

/*  INLINE  --  Read a line from the file, delimited by any of the
                end of line sequences. */

static int inline(fp, line)
  FILE *fp;
  char *line;
{
    char *lp;

    blankline:
    lp=line;
    while (TRUE) {
        int ch = getc(fp);

        if (ch == '\r' || ch == '\n') {
            char cn = getc(fp);
            if (!((ch == '\r' && cn == '\n') || (ch == '\n' && cn == '\r'))) {
                ungetc(cn, fp);
            }
            *lp = EOS;
            if(strlen(line)==0)
                goto blankline;
            return TRUE;
        }

        if (ch == EOF) {
            if (lp != line) {
                *lp = EOS;
                if(strlen(line)==0)
                    goto blankline;
                return TRUE;
            }
            return FALSE;
        }
        *lp++ = ch;
        *lp=0;
        if(strlen(lp)>254)
                return FALSE;
    }
}

/*  DECRYPT  --  Perform running decryption of file.  */

static unsigned short int cryptR, cryptC1, cryptC2, cryptCSR;

static void cryptinit(key)
  unsigned int key;
{
    cryptR = key;
    cryptC1 = 52845;
    cryptC2 = 22719;
}

static unsigned int decrypt(cipher)
  unsigned int cipher;
{
    unsigned int plain = (cipher ^ (cryptR >> 8));

    cryptR = (cipher + cryptR) * cryptC1 + cryptC2;

    return plain;
}

static void cstrinit()
{
    cryptCSR = 4330;
}

static unsigned int decstr(cipher)
  unsigned int cipher;
{
    unsigned int plain = (cipher ^ (cryptCSR >> 8));

    cryptCSR = (cipher + cryptCSR) * cryptC1 + cryptC2;

    return plain;
}

/*  PROCESS_DEFINITION  --  Blunder through the font by stumbling over
                            key defining words.  */

static int process_definition(t)
  char *t;
{
#ifdef DBGAT2
printf("Process %s\n",t);
#endif
    if (strcmp(t, "FontInfo") == 0) {
        section = FontInfo;
    } else if (strcmp(t, "OtherSubrs") == 0) {
        section = OtherSubrs;
    } else if (strcmp(t, "Subrs") == 0) {
        section = Subrs;
    } else if (strcmp(t, "CharStrings") == 0) {
        section = CharStrings;
    } else {
        switch (section) {

            case CharStrings:
                if (ncdefs >= (NCdefs - 1)) {
                        continu_line("Too many CharString definitions.");
                        return(0);
                } else {
#ifdef DBGAT2
printf("Adding charstring %s\n",t);
#endif
                    chardefs[ncdefs][0] = (unsigned char *) strsave(t);/* Save char name */
                    chardefs[ncdefs++][1] = NULL;       /* Clear definition */
                }
                break;

            default:
                while(kbhit())
                        {
                        if(getch()==ESC)
                                return(0);
                        }
#ifdef DBGAT6
printf("Unknown def %s\nIn section %d\n",t,section);
#endif
                break;
        }
    }
return(1);
}

/*  RTYPE1  --  Load a type 1 font into memory.  */

static char line[256], token[256], ltoken[256], stoken[256], ptoken[256];
static char *tokenp;
static int i, inEncoding = FALSE;
static unsigned char ic;

int rtype1(int hdronly)
{
int ix;
FILE *fp=t1handle;

    charmap = isomap;         /* Reset mapping vector */
    if(hdronly)
     {
     ncdefs = 0;
     nsubrs = 0;
     subrs = NULL;
     section = Header;
     t1_fullname[0] = ptoken[0] = stoken[0] = ltoken[0] = token[0] = EOS;

     /* Initialize chardefs array */

     for(ix=0; ix<NCdefs; ++ix)
      chardefs[ix][0]=chardefs[ix][1]=NULL;

     /* Skip any prefix before the font header. */

     while (TRUE) {
        int ch = getc(fp);

        if (ch == EOF)
            return FALSE;
        if (ch == '%') {
            ch = getc(fp);
            if (ch == EOF)
                return FALSE;
            if (ch == '!')
                break;
        }
     }

     strcpy(line, "%!");
     inline(fp, line + 2);

     while (inline(fp, line)) {
        if (strstr(line, "/FullName ") != NULL) {
            char *cp = strchr(line, '(');

            if (cp != NULL) {
                char *ep = strchr(cp, ')');

				if (ep != NULL) {
					*ep = EOS;
//                    if(strlen(cp+1)>255)
//                        {
//                        continu_line("Font name too long");
//                        return FALSE;
//                        }
					strcpy(t1_fullname, cp + 1);
				}
			}
		 }

		t1_minx=t1_miny=0;
		t1_maxx=t1_maxy=1000;

		if (strstr(line, "/FontBBox ") != NULL) {
			char *cp = strchr(line, '{');

			if (cp != NULL) {
				char *ep = strchr(cp, '}');

				if (ep != NULL) {
					*ep = EOS;
					strcpy(gp_buffer, cp + 1);
					if(sscanf(gp_buffer,"%d %d %d %d",
						&t1_minx,&t1_miny,&t1_maxx,&t1_maxy)!=4) {
								continu_line("Invalid font bounding box");
								t1_minx=t1_miny=0;
								t1_maxx=t1_maxy=1000;
					}
				}
			}
		 }

		 /* If font contains a custom encoding vector, load the encoding
		   vector into the user-defined encoding. */

		 if (inEncoding) {
			if (strstr(line, " def") != NULL) {
				inEncoding = FALSE;
				charmap = usermap;
#ifdef DBGAT2
printf("Using custom encoding\n");
getch();
#endif
			} else {
				char *dname = strchr(line, '/');

				if (dname != NULL) {
					if (dname[1] != '.') {
						char token[80];

						if (sscanf(dname + 1, "%s ", token) > 0) {
							int n;
							if(strlen(token)>79)
								{
								printf("Token name too long\n");
								return FALSE;
								}

                            dname--;
                            while (isspace(*dname) && (dname > line)) {
                                dname--;
                            }
                            while (!isspace(*dname) && (dname > line)) {
                                dname--;
                            }
                            n = atoi(dname);
                            if (n < 32 || n > 255) {
                                /* No action -- out of range */
                            } else {
                                usermap[n - 32] = strsave(token);
                            }
                        }
                    }
				}
            }
         }

        /* If no explicit encoding vector has been loaded from a file and
           the font contains an encoding vector of its own, use the vector
           from the to map the characters into font slots. */

         if ((strstr(line, "/Encoding") != NULL) && (charmap != usermap)) {
            int i;

            if (strstr(line, "StandardEncoding") == NULL) {
                inEncoding = TRUE;
                for (i = 0; i < (256 - 32); i++)
                    usermap[i] = NULL;
            }
        }
        if (strstr(line, "eexec") != NULL)
            break;
     }

     /* We're reading only the header -- return OK! */

     t1_chars=224;
     return(TRUE);
     }

    /* O.K., we're now into the encrypted portion of the file.
       Determine if it's ASCII or binary and process accordingly. */

    for (i = 0; i < 6; i++) {
        (void) getc(fp);              /* Beats me, but there's 6 trash bytes */
    }

    cs[0] = getc(fp);

    /* "Adobe Type 1 Font Format Version 1.1", ISBN 0-201-57044-0 states
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
       presence of a carriage return.  Note also the "-B" and "-H"
       command line switches that you can use to override the automatic
       recognition of file format. */

    if (forcemode != Unknown) {
        filemode = forcemode;
        csl = 1;
    } else {
        if (cs[0] == ' ' || cs[0] == '\t' ||
            /* cs[0] == '\r' || */      /* OLD COMMENTED OUT CODE */
            cs[0] == '\r' ||
            cs[0] == '\n') {
            filemode = Hex;
#ifdef DBGAT2
printf("Filemode is Hex\n");
#endif
            csl = 1;
        } else {
            for (i = 1; i < 4; i++) {
                cs[i] = getc(fp);
            }
            csl = 4;
#ifdef DBGAT2
printf("CS:%c%c%c%c%c\n",cs[0],cs[1],cs[2],cs[3],cs[4]);
#endif
            filemode = Hex;
            for (i = 0; i < 4; i++) {
                if (!((cs[i] >= '0' && cs[0] <= '0') ||
                      (cs[i] >= 'A' && cs[0] <= 'F') ||
                      (cs[i] >= 'a' && cs[0] <= 'f'))) {
                    filemode = Binary;
#ifdef DBGAT2
printf("Filemode=Binary\n");
#endif
                    break;
                }
            }
        }
    }
    csp = cs;

#ifdef DBGAT2
sprintf(gp_buffer,"File mode determined to be %d",filemode);
show_prompt(gp_buffer);
#endif

    cryptinit(55665);

    /* Now burn the first four plaintext bytes. */

    for (i = 0; i < 4; i++) {
        if(inbyte(fp,&ic)==0)
                return(FALSE);
        (void) decrypt(ic);
    }
    tokenp = token;
    while (inbyte(fp,&ic)) {
        int dc = decrypt(ic);
#ifdef DBGAT4
printf("Got Char:%c, Filepos:%d\n",dc,ftell(fp));
#endif
#ifdef SAVEIT
{
char byte;

byte=dc;
if(ostream==NULL)
 ostream=fopen("e:\pfb.txt","wb");
fwrite(&dc,1,1,ostream);
}
#endif

        if (isspace(dc)) {
            if (tokenp > token) {
                *tokenp = EOS;

                strcpy(ptoken, stoken);
                strcpy(stoken, ltoken);
                strcpy(ltoken, token);
                tokenp = token;
                if (strcmp(ltoken, "closefile") == 0) {
                    return TRUE;
                }
                if (ltoken[0] == '/') {
#ifdef DBGAT4
printf("Ltoken:%s\nFilepos:%d\n",ltoken,ftell(fp));
#endif
                    if(process_definition(ltoken + 1)==0)
                        return(FALSE);
                }
                if (strcmp(ltoken, "array") == 0 && section == Subrs &&
                    subrs == NULL) {
                    int l = (nsubrs = atoi(stoken)) * sizeof(char *);

                    subrs = (unsigned char **) malloc(l);
                    subrl = (int *) malloc(nsubrs * sizeof(int));
                    memset(subrs, 0, l);
                }
            }
        } else {
            if(dc=='/' && tokenp!=token)
                {
                tokenp=token;
                }
            *tokenp++ = dc;
            *tokenp=0;
            if(strlen(token)>254)
                return(FALSE);
        }

        if (isspace(dc) &&
            ((strcmp(ltoken, "-|") == 0) || (strcmp(ltoken, "RD") == 0))) {
            int l = atoi(stoken), j;
            cstrinit();

            for (j = 0; j < 4; j++) {
                if(inbyte(fp,&ic)==0)
                        return(FALSE);
                (void) decstr(decrypt(ic));
            }

            /* Process the charstring depending on the section it's
               encountered within. */

            switch (section) {
                case CharStrings:
#ifdef DBGAT2
printf("In charstrings section\n");
#endif
                    if (ncdefs > 0 && chardefs[ncdefs - 1][1] == NULL) {
                        unsigned char *csd = malloc(l - 4);

                        chardefs[ncdefs - 1][1] = csd;
                        l -= 4;
                        chardl[ncdefs - 1] = l;
                        while (l-- > 0) {
                            if(inbyte(fp,&ic)==0)
                                return(FALSE);
                            *csd++ = decstr(decrypt(ic));
                        }
                    } else {
                        continu_line("Unexpected data in CharStrings.");
                        return(FALSE);
                    }
                    break;

                case Subrs:
#ifdef DBGAT2 
printf("In subrs section\n");
#endif
                    j = atoi(ptoken); /* Subr number */
                    if (j < 0 || j >= nsubrs) {
                        sprintf(gp_buffer, "Subr %d out of range (0-%d).",
                            j, nsubrs);
                        continu_line(gp_buffer);
                    } else {
                        if (l > 4) {
                            unsigned char *csd = malloc(l - 4);

                            subrs[j] = csd;
                            l -= 4;
                            subrl[j] = l;
                            while (l-- > 0) {
                                if(inbyte(fp,&ic)==0)
                                        return(FALSE);
                                *csd++ = decstr(decrypt(ic));
                            }
                        }
                    }
                    break;

                default:
                    break;
            }
            ltoken[0] = EOS;
        }
    }
    return TRUE;
}

/*  BEZIER  --  Evaluate a Bezier curve defined by four control
                points.  */

static short *a1buf;
int a1lastx,a1lasty;

static void bezier(x0, y0, x1, y1, x2, y2, x3, y3)
  int x0, y0, x1, y1, x2, y2, x3, y3;
{
*a1buf++ = 1;
*a1buf++ = x0;
*a1buf++ = y0;
*a1buf++ = x1;
*a1buf++ = y1;
*a1buf++ = x2;
*a1buf++ = y2;
*a1buf++ = x3;
*a1buf++ = y3;
a1lastx=x3;
a1lasty=y3;

#ifdef DBGAT3
if(dbgat3)
 printf("%d %d to %d %d\n",x0,y0,x3,y3);
#endif
}

/*  EXCHARS  --  Execute charstring.  */

static int exchars(unsigned char *cp,short *outbuf)
{
    int sub;

    a1buf=outbuf;
    sp = rsp = 0;                     /* Reset stack pointer */
    pathopen = FALSE;

    while (TRUE) {
        int c = *cp++;

        if (c < 32) {
            /* Command */
            if (c == 12) {
                /* Two byte command */
                c = *cp++ + 32;
            }

            switch (c) {

                /* Commands for Starting and Finishing */

                case Endchar:         /* 14: End character */
#ifdef DBGAT3
if(dbgat3)
 printf("Endchar\n");
#endif
                    *a1buf++ = 0;                       
                    a1lastx=a1lasty=0;
                    Clear();
                    goto exdone;

                case Hsbw:            /* 13:  Set horizontal sidebearing */
                    Sl(2);
                    curx=a1lastx=S1;
                    cury=a1lasty=0;
                    t1_width=S0;
                    Clear();
                    break;

                case Seac:            /* 12-6:  Standard encoding accented char */
                    Sl(5);
                    Clear();
                    goto exdone;

                case Sbw:             /* 12-7:  Sidebearing point (x-y) */
                    Sl(4);
                    curx = a1lastx = S3;
                    cury = a1lasty = S2;
                    t1_width=S0;
                    Clear();
                    break;

                /* Path Construction Commands */

                case Closepath:       /* 9:  Close path */
#ifdef DBGAT3
if(dbgat3)
 printf("Closepath\n");
#endif
                    *a1buf++ = 2;
                    pathopen = FALSE;
                    Clear();
                    break;

                case Hlineto:         /* 6: Horizontal line to */
                    Sl(1);
#ifdef DBGAT3
if(dbgat3)
 printf("Hlineto ");
#endif
                    bezier(curx,cury,curx,cury,curx+S0,cury,curx+S0,cury);
                    curx = curx + S0;
                    Clear();
                    break;

                case Hmoveto:         /* 22:  Horizontal move to */
                    Sl(1);
#ifdef DBGAT3
if(dbgat3)
 printf("Hmoveto ");
#endif
                    curx += S0;
                    Clear();
                    break;

                case Hvcurveto:       /* 31:  Horizontal-vertical curve to */
                    Sl(4);
#ifdef DBGAT3
if(dbgat3)
 printf("Hvcurveto ");
#endif
                    bezier(curx, cury, curx + S3, cury,
                           curx + S3 + S2, cury + S1,
                           curx + S3 + S2, cury + S1 + S0);
                    curx+=(S3+S2);
                    cury+=(S1+S0);
                    Clear();
                    break;

                case Rlineto:         /* 5:  Relative line to */
                    Sl(2);
#ifdef DBGAT3
if(dbgat3)
 printf("Rlineto ");
#endif
                    bezier(curx,cury,curx,cury,
                        curx+S1,cury+S0,curx+S1,cury+S0);
                    curx += S1;
                    cury += S0;
                    Clear();
                    break;

                case Rmoveto:         /* 21:  Relative move to */
                    Sl(2);
#ifdef DBGAT3
if(dbgat3)
 printf("Rmoveto %d %d\n",curx+S1,cury+S0);
#endif
                    curx += S1;
                    cury += S0;
                    Clear();
                    break;

                case Rrcurveto:       /* 8:  Relative curve to */
                    Sl(6);
#ifdef DBGAT3
if(dbgat3)
 printf("Rrcurveto ");
#endif
                    bezier(curx, cury, curx + S5, cury + S4,
                           curx + S5 + S3, cury + S4 + S2,
                           curx + S5 + S3 + S1, cury + S4 + S2 + S0);
                    curx+=(S5+S3+S1);
                    cury+=(S4+S2+S0);
                    Clear();
                    break;

                case Vhcurveto:       /* 30:  Vertical-horizontal curve to */
                    Sl(4);
#ifdef DBGAT3
if(dbgat3)
 printf("Vhcurveto ");
#endif
                    bezier(curx, cury, curx, cury + S3,
                           curx + S2, cury + S3 + S1,
                           curx + S2 + S0, cury + S3 + S1);
                    curx+=(S2+S0);
                    cury+=(S3+S1);
                    Clear();
                    break;

                case Vlineto:         /* 7:  Vertical line to */
                    Sl(1);
#ifdef DBGAT3
if(dbgat3)
 printf("Vlineto ");
#endif
                    bezier(curx,cury,curx,cury,curx,cury+S0,curx,cury+S0);
                    cury += S0;
                    Clear();
                    break;

                case Vmoveto:         /* 4:  Vertical move to */
                    Sl(1);
#ifdef DBGAT3
if(dbgat3)
 printf("Vmoveto %d %d\n",curx,cury+S0);
#endif
                    cury += S0;
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
                    Npop(2 + S1);
                    break;

                case Callsubr:        /* 10:  Call subroutine */
                    Sl(1);
                    if (rsp >= ReturnStackLimit) {
                        fflush(stdout);
                        continu_line("Return stack overflow.");
                        return(0);
                    }
                    rstack[rsp++] = cp;
                    sub = S0;
                    Npop(1);
                    if (sub < 0 || sub >= nsubrs) {
                        fflush(stdout);
                        continu_line("Subr number out of range in call.");
                        return(0);
                    }
                    if (subrs[sub] == NULL) {
                        fflush(stdout);
                        continu_line("Subr undefined.");
                        return(0);
                    }
                    cp = subrs[sub];  /* Set instruction pointer to subr code */
                    break;

                case Pop:             /* 12 17:  Return argument from othersubr */
                    So(1);
                    stack[sp++] = 3;  /************/
                    break;

                case Return:          /* 11:  Return from subroutine */
                    if (rsp < 1) {
                        fflush(stdout);
                        continu_line("Return stack underflow.");
                        return(0);
                    }
                    cp = rstack[--rsp]; /* Restore pushed call address */
                    break;

                case Setcurrentpoint: /* 12 33:  Set current point */
#ifdef DBGAT3
if(dbgat3)
 printf("Setcurrent ");
#endif
                    Sl(2);
                    bezier(a1lastx,a1lasty,a1lastx,a1lasty,curx,cury,curx,cury);
                    Clear();
                    break;
            }

        } else {
            long n;

            if (c <= 246) {
                n = c - 139;
            } else if (c <= 250) {
                n = ((c - 247) << 8) + *cp++ + 108;
            } else if (c < 255) {
                n = -((c - 251) << 8) - *cp++ - 108;
            } else {
                char ba[4];

                ba[0] = *cp++;
                ba[1] = *cp++;
                ba[2] = *cp++;
                ba[3] = *cp++;
                n = (((((ba[0] << 8) | ba[1]) << 8) | ba[2]) << 8) | ba[3];
            }
            if (sp >= StackLimit) {
                fflush(stdout);
                continu_line("Data stack overflow.");
            } else {
                stack[sp++] = n;
            }
        }
    }

exdone:
if(a1buf==outbuf)
 return(0);
return(1);
}

/*  COMPOUT  --  Output compiled shape.  */

static int compout(unsigned char *dest,int i)
{
int j;
char chname[80], outline[82];
int k;

#ifdef DBGAT1
dump_at1();
#endif
#ifdef DBGAT2
printf("ncdefs:%d\n",ncdefs);
#endif

/* Make sure it's a valid ID */

if(charmap[i-32]==NULL)
 return(FALSE);

/* Look for char by name */

for (j = 0; j < ncdefs; j++)
 {
/*
printf("Comparing:i:%d j:%d [%s][%s]\n",
        i,j,(char *)chardefs[j][0],(char *)charmap[i - 32]);
*/
 if (strcmp((char *)chardefs[j][0],(char *)charmap[i - 32]) == 0)
  {
  /* Found the character! */
#ifdef DBGAT7
printf("Char %d (%c) found at %d,%p\n",i,i,j,chardefs[j][1]);
#endif
  if(exchars(chardefs[j][1],(short *)dest)==0)
        return FALSE;
  return TRUE;
  }
 }

/* Look for char by number */

if(chardefs[i-32][1]==NULL)
 return FALSE;

if(exchars(chardefs[i-32][1],(short *)dest)==0)
        return FALSE;
return TRUE;
}

/* Open the Adobe Type1 file */

static int dataread=0;

int
at1_open(char *fname)
{
int ix;

if((t1handle=fopen(fname,"rb"))==NULL)
 {
 cant_open_font();
 return(0);
 }
dataread=0;
for(ix=0; ix<224; ++ix)
 usermap[ix]=NULL;
return(rtype1(1));
}

/* Point to a character's font data in memory */

void *
at1_get_char(int ch)
{
int j,cix,sub;
int dbg3hold=0;

if(dataread==0)
 {
 please_wait("Loading font");
 if(rtype1(0)==0)
  {
  report("");
  continu_line("Invalid Adobe Type1 font");
  error_in_font=1;
  return(NULL);
  }
 report("");
 dataread=1;
 }

#ifdef DBGAT3
if(ch=='i')
 {
 dbg3hold=dbgat3;
 dbgat3=0;
 }
#endif

/* Look up the appropriate character */
/* Adobe fonts don't include chars < 32 */

if((cix=ch-32)<0)
 goto donthave;

#ifdef DBGAT1
printf("ch in at1_get_char:%d\n",ch);
#endif

if(compout(holebuf,ch)==0)
 {
#ifdef DBGAT1
printf("1\n");
#endif
 sub=FALSE;
 if(charmap[cix]!=NULL)
  {
  if (strcmp(charmap[cix], "emptyset") == 0)
   {
   for (j = 32; j < 256; j++)
    {
    if (charmap[j - 32] != NULL && strcmp(charmap[j - 32], "Oslash") == 0)
     {
#ifdef DBGAT1
printf("2\n");
#endif
     if (compout(holebuf, j))
      sub = TRUE;
     goto gotit;
     }
    }
   }
  }
 if (sub==FALSE)
  {
  donthave:

#ifdef DBGAT1
printf("Haven't got char %c (%d/%X)\n",ch,ch,ch);
#endif

#ifdef DBGAT3
if(dbg3hold)
 dbgat3=dbg3hold;
#endif

  return(NULL);
  }
 }

gotit:
#ifdef DBGAT3
if(dbg3hold)
 dbgat3=dbg3hold;
#endif

return(holebuf);
}

/* Close up the Adobe type1 file and free any memory allocated */

at1_close(void)
{
int ix;
Name_list *n;

dataread=0;

#ifdef SAVEIT
if(ostream)
 {
 fclose(ostream);
 ostream=NULL;
 }
#endif

if(t1handle)
 fclose(t1handle);
t1handle=NULL;

for(ix=0; ix<ncdefs; ++ix)
 {
 if(chardefs[ix][0])
  free(chardefs[ix][0]);
 if(chardefs[ix][1])
  free(chardefs[ix][1]);
 chardefs[ix][0]=NULL;
 chardefs[ix][1]=NULL;
 }

for(ix=0; ix<224; ++ix)
 {
 if(usermap[ix])
  free(usermap[ix]);
 }

if(subrl)
 free(subrl);
subrl=NULL;

if(subrs)
 {
 for(ix=0; ix<nsubrs; ++ix)
  {
  if(subrs[ix])
   free(subrs[ix]);
  }
 free(subrs);
 }
subrs=NULL;
}

#ifdef FROON
dump_at1()
{
int ix;

for(ix=0; ix<224; ++ix)
 {
 printf("Charmap [%d]=",ix);
 if(charmap[ix]==NULL)
  printf("NULL\n");
 else
  printf("%s\n",charmap[ix]);
 }
getch();
}
#endif
