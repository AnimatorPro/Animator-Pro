/*

        Read and decrypt an Adobe Type 1 font

*/

#include "type1.h"

#define TRACE

static unsigned char cs[4];
static unsigned char *csp;
static int csl;
static enum {Unknown, Binary, Hex} filemode = Unknown, forcemode = Unknown;
#ifdef TRACE
static int tracing = FALSE;           /* Echo decrypted input */
#endif
int shapedoc = FALSE;                 /* PostScript as comments in shape file */

int bnum = 5;                         /* Line segments per Bezier curve */
double skewsin = 0.0;                 /* Sine of ItalicAngle for obliquing */
int noflex = FALSE;                   /* Suppress Flex if true */

int ncdefs;                           /* Number of character definitions */
unsigned char *chardefs[NCdefs][2];   /* Character definitions */
int chardl[NCdefs];                   /* Character definition lengths */

int nsubrs = 0;                       /* Number of subroutines */
unsigned char **subrs = NULL;         /* Subroutine pointer table */
int *subrl;                           /* Subroutine lengths */

char fullname[256];                   /* Font name */

static file_section section;

static void dcstring( FILE *fp, int l);

char *csnames[] = {
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
int ncsnames = ELEMENTS(csnames);

/*  INBYTE  --  Get the next binary byte from the file, either in
                hexadecimal or binary mode. */

static int inbyte(FILE *fp)
{
    int c, xd = 0, i;

    for (i = 0; i < 2; i++) {
        while (TRUE) {
            if (csl > 0) {
                c = *csp++;
                csl--;
            } else {
                c = getc(fp);
            }  
            if (filemode == Binary || c == EOF)
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
            fprintf(stderr, "Bad hex digit\n");
            return EOF;
        }
        xd = (xd << 4) | c;
    }
    return xd;
}

/*  INLINE  --  Read a line from the file, delimited by any of the
                end of line sequences. */

static int inline( FILE *fp, char *line)
{
    char *lp = line;

    while (TRUE) {
        int ch = getc(fp);

        if (ch == '\r' || ch == '\n') {
            char cn = getc(fp);
                        if (!((ch == '\r' && cn == '\n') || (ch == '\n' && cn == '\r'))) {
                ungetc(cn, fp);
            }
            *lp = EOS;
            return TRUE;
        }

        if (ch == EOF) {
            if (lp != line) {
                *lp = EOS;
                return TRUE;
            }
            return FALSE;
        }
        *lp++ = ch;
    }
}

/*  DECRYPT  --  Perform running decryption of file.  */

static unsigned short int cryptR, cryptC1, cryptC2, cryptCSR;

static void cryptinit( unsigned int key)
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

/*  PROCESS_DEFINITION  --  Blunder through the font by stumbling over
                            key defining words.  */

static void process_definition(char *t)
{
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
                                        fprintf(stderr, "\nToo many CharString definitions.\n");
                } else {
                    chardefs[ncdefs][0] = (unsigned char *) strsave(t);/* Save char name */
                    chardefs[ncdefs++][1] = NULL;       /* Clear definition */
                }
                break;

                default:
                break;
        }
    }
}

/*  RTYPE1  --  Load a type 1 font into memory.  */

int rtype1(FILE *fp)
{
    char line[256], token[256], ltoken[256], stoken[256], ptoken[256];
    char *tokenp;
    int i, ic, inEncoding = FALSE;

    ncdefs = 0;
        nsubrs = 0;
    subrs = NULL;
        section = Header;
    fullname[0] = ptoken[0] = stoken[0] = ltoken[0] = token[0] = EOS;

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
#ifdef TRACE
    if (tracing) {
        printf("%s\n", line);
    }
#endif

    while (inline(fp, line)) {
#ifdef TRACE
        if (tracing) {
            printf("%s\n", line);
        }
#endif
        if (strstr(line, "/FullName ") != NULL) {
            char *cp = strchr(line, '(');

            if (cp != NULL) {
                char *ep = strchr(cp, ')');

                if (ep != NULL) {
                    *ep = EOS;
                    strcpy(fullname, cp + 1);
                }
            }
        }
        if (strstr(line, "/ItalicAngle") != NULL) {
				double iangle = 0.0;

			sscanf(strstr(line, "/ItalicAngle") + 13, "%lg", &iangle);
			skewsin = tan(M_PI * (iangle / -180.0));
/* fprintf(stderr, "This font is obliqued %g degrees.\n", -iangle); */
		}
		else if (strstr(line, "/FontBBox") != NULL) {
			char *cp = strchr(line, '{');
			puts(line);
			if (cp != NULL) {
				char *ep = strchr(cp, '}');
				struct bounding_box{int xmin,ymin,xmax,ymax;} t1_bounds;
				if (ep != NULL) {
					*ep = EOS;
					if(sscanf(cp+1,"%d %d %d %d"
					, &t1_bounds.xmin,&t1_bounds.ymin
					, &t1_bounds.xmax,&t1_bounds.ymax)!=4) {
						puts("Invalid font bounding box");
					} else {

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
            } else {
                char *dname = strchr(line, '/');

                if (dname != NULL) {
                    if (dname[1] != '.') {
                        char token[80];

                        if (sscanf(dname + 1, "%s ", token) > 0) {
                            int n;

                                dname--;
                            while (isspace(*dname) && (dname > line)) {
                                dname--;
                            }
                            while (!isspace(*dname) && (dname > line)) {
                                dname--;
                            }
                            n = atoi(dname);
                            if (n < 32 || n > 255) {
                                fprintf(stderr,
         "Error parsing encoding vector.  Character index %d out of range.\n",
                                        n);
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
/* fprintf(stderr, "Using font-specific encoding vector.\n"); */
                inEncoding = TRUE;
                for (i = 0; i < (256 - 32); i++)
                    usermap[i] = NULL;
            }
        }
                if (strstr(line, "eexec") != NULL)
            break;
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
            /* cs[0] == '\r' || */
            cs[0] == '\n') {
            filemode = Hex;
            csl = 1;
        } else {
            for (i = 1; i < 4; i++) {
                cs[i] = getc(fp);
            }
            csl = 4;
                filemode = Hex;
            for (i = 0; i < 4; i++) {
                if (!((cs[i] >= '0' && cs[0] <= '0') ||
                      (cs[i] >= 'A' && cs[0] <= 'F') ||
                      (cs[i] >= 'a' && cs[0] <= 'f'))) {
                    filemode = Binary;
                    break;
                }
            }
        }
    }
    csp = cs;

    cryptinit(55665);

    /* Now burn the first four plaintext bytes. */

    for (i = 0; i < 4; i++) {
        (void) decrypt(inbyte(fp));
    }
    tokenp = token;
    while ((ic = inbyte(fp)) != EOF) {
        int dc = decrypt(ic);

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
                        process_definition(ltoken + 1);
                }
                if (strcmp(ltoken, "array") == 0 && section == Subrs &&
                    subrs == NULL) {
                    int l = (nsubrs = atoi(stoken)) * sizeof(char *);

                    subrs = (unsigned char **) alloc(l);
                        subrl = (int *) alloc(nsubrs * sizeof(int));
                    memset(subrs, 0, l);
                }
            }
        } else {
            *tokenp++ = dc;
        }
#ifdef TRACE
        if (tracing) {
            if (dc == '\r')
                putchar('\n');
            else
                putchar(dc);
        }
#endif

        if (isspace(dc) &&
                        ((strcmp(ltoken, "-|") == 0) || (strcmp(ltoken, "RD") == 0))) {
            int l = atoi(stoken), j;
            cstrinit();

#ifdef TRACE
        if (tracing)
            printf(" <<(%d)", l);
#endif
            for (j = 0; j < 4; j++) {
                (void) decstr(decrypt(inbyte(fp)));
                }

            /* Process the charstring depending on the section it's
                   encountered within. */

            switch (section) {
                case CharStrings:
                    if (ncdefs > 0 && chardefs[ncdefs - 1][1] == NULL) {
                        unsigned char *csd = alloc(l - 4);

                        chardefs[ncdefs - 1][1] = csd;
                        l -= 4;
                        chardl[ncdefs - 1] = l;
                        while (l-- > 0) {
                            *csd++ = decstr(decrypt(inbyte(fp)));
                        }
                    } else {
                        fprintf(stderr, "\nUnexpected data in CharStrings.\n");
                        dcstring(fp, l - 4);
                    }
                    break;

                case Subrs:
                    j = atoi(ptoken); /* Subr number */
                    if (j < 0 || j >= nsubrs) {
                                                fprintf(stderr, "\nSubr %d out of range (0-%d).\n",
                            j, nsubrs);
                    } else {
                        if (l > 4) {
                            unsigned char *csd = alloc(l - 4);

                            subrs[j] = csd;
                            l -= 4;
                            subrl[j] = l;
                            while (l-- > 0) {
                                *csd++ = decstr(decrypt(inbyte(fp)));
                            }
                        }
                    }
                    break;

                default:
                        dcstring(fp, l - 4);
                        break;
            }
#ifdef TRACE
            if (tracing)
                printf(">> ");
#endif
            ltoken[0] = EOS;
        }
    }
    return TRUE;
}

/*  DCSTRING  --  Decode charstring.  */

#define fetchcs()  (l--, decstr(decrypt(inbyte(fp))))

static void dcstring( FILE *fp, int l)
{
    while (l > 0) {
        int c = fetchcs();

        if (c < 32) {
            /* Command */
            if (c == 12) {
                /* Two byte command */
                c = fetchcs() + 32;
            }
            if (c < ELEMENTS(csnames)) {
    printf(" %s", csnames[c]);
            } else {
    printf(" (%d)", c);
            }
        } else {
            long n;

            if (c <= 246) {
                n = c - 139;
            } else if (c <= 250) {
                n = ((c - 247) << 8) + fetchcs() + 108;
            } else if (c < 255) {
                n = -((c - 251) << 8) - fetchcs() - 108;
            } else {
                char ba[4];

                ba[0] = fetchcs();
                ba[1] = fetchcs();
                ba[2] = fetchcs();
                ba[3] = fetchcs();
                n = (((((ba[0] << 8) | ba[1]) << 8) | ba[2]) << 8) | ba[3];
            }
    printf(" %d", n);
        }
    }
}

/*  LOADVECTOR  --  Load custom mapping vector from file.  */

static int loadvector(char *f)
{
        FILE *fp = fopen(f, "rb");         /* Open file */
    char line[256];
    int i, lineno = 0, errors = 0;

    if (fp == NULL) {
        fprintf(stderr, "Unable to open mapping vector file %s\n", f);
        return FALSE;
    }

    for (i = 0; i < (256 - 32); i++)
         usermap[i] = NULL;

    while (fgets(line, 255, fp) != NULL) {
        char *cp;

        lineno++;
        if ((cp = strchr(line, ';')) != NULL) {
            *cp = EOS;
        }

        cp = line;
        while ((*cp != EOS) && isspace(*cp)) {
            cp++;
        }
        if (*cp != EOS) {
            int n, ok = TRUE;
            char token[80];

            if (*cp == '\"') {
                n = cp[1];
                if ((cp[2] != '\"') || (sscanf(cp + 3, " %s", token) < 1)) {
                    ok = FALSE;
                }
            } else if (sscanf(cp, "%d %s", &n, token) < 2) {
                ok = FALSE;
            }
            if (!ok) {
                fprintf(stderr, "\n%s\n", line);
                fprintf(stderr, "Syntax error on line %d of vector file %s\n",
                    lineno, f);
                errors++;
            } else {
                if (n < 32 || n > 255) {
                    fprintf(stderr, "\n%s\n", line);
                    fprintf(stderr,
                        "Character index out of range in line %d of %s\n",
                        lineno, f);
                }
                usermap[n - 32] = strsave(token);
            }
        }
    }

    fclose(fp);
    return errors == 0;
}

/*  USAGE  --  Print how to call information.  */

#define PR(x) fprintf(stderr, x)

static void usage(void)
{
    PR("Usage:  readfont [options] inputfile outputfile\n");
    PR("        Options:\n");
    PR("           -B     Force binary file mode\n");
    PR("           -C     Dump character and subroutine code\n");
    PR("           -D     Embed PostScript in .shp for debugging\n");
    PR("           -H     Force hexadecimal file mode\n");
#ifdef TRACE
    PR("           -L     List file as it is read\n");
#endif
    PR("           -N     Print names of characters defined in file\n");
    PR("           -Q     Make quick-generating font\n");
    PR("           -Sn    Use n line segments per Bezier curve\n");
    PR("           -T     Interactive test mode\n");
    PR("           -U     Print this message\n");
    PR("           -Vfile Use mapping vector in file\n");
}

/*  MAIN  --  Main program.  */

int main( int argc, char *argv[])
{
    int i;
    char fname[256];
        FILE *fp, *ofp;
    char *ifname = NULL;
    char *ofname = NULL;
    int dumpdefs = FALSE, dumpcode = FALSE, testmode = FALSE;

    for (i = 1; i < argc; i++) {
        char *cp, opt;

        cp = argv[i];
        if (*cp == '-') {
            opt = *(++cp);
            if (islower(opt))
                opt = toupper(opt);
            switch (opt) {

                case 'B':             /* -B:  Force binary file mode */
                    forcemode = Binary;
                    break;

                case 'C':             /* -C:  Dump char and subr code */
                    dumpcode = TRUE;
                    break;

                case 'D':             /* -D:  Embed code as comments in shape
                                              file for debugging. */
                    shapedoc = TRUE;
                    break;

                case 'H':             /* -H:  Force hex file mode */
                    forcemode = Hex;
                    break;

#ifdef TRACE
                case 'L':             /* -L:  List decrypted input */
                    tracing = TRUE;
                    break;
#endif

                case 'N':             /* -N:  Dump names of defined chars */
                    dumpdefs = TRUE;
                    break;

                case 'Q':             /* -Q:  Quick-generating font */
                    noflex = TRUE;
                    break;

                case 'S':             /* -Sn:  Use n line segments to
                                               approximate Bezier curves */
                    bnum = atoi(cp + 1);
                    break;

                case 'T':             /* -T:  Interactive test mode */
                    testmode = TRUE;
                    break;

                case 'U':
                case '?':
                    usage();
                    return 0;

                case 'V':             /* -Vfile:  Load custom mapping vector */
                    if (loadvector(cp + 1)) {
                        charmap = usermap;
                        break;
                    }
                    return 2;
            }
        } else {
            if (ofname != NULL) {
                PR("Too many file names.\n");
                return 2;
            }
            if (ifname == NULL) {
                ifname = cp;
            } else {
                ofname = cp;
            }
        }
     }
    if (ifname == NULL) {
        PR("No input file specified.\n");
        usage();
        return 2;
    }

        fp = fopen(ifname, "rb");          /* Open input file */
    if (fp == NULL) {
        fprintf(stderr, "Cannot open input file %s.\n", ifname);
        return 2;
    }
    rtype1(fp);
    fclose(fp);

    if (dumpdefs) {
       int i;
        for (i = 0; i < ncdefs; i++) {
            printf("%s\n", chardefs[i][0]);
        }
        printf("\n");
        return 0;
    }

    if (dumpcode) {
       int i;
        for (i = 0; i < ncdefs; i++) {
            printf("===> /%s: ", chardefs[i][0]);
            dumpchars(chardefs[i][1], chardl[i]);
            printf("\n");
        }

        printf("\nDefined subroutines:\n");
        for (i = 0; i < nsubrs; i++) {
            if (subrs[i] != NULL) {
                printf("===> %d: ", i);
                dumpchars(subrs[i], subrl[i]);
                printf("\n");
            }
        }
        return 0;
    }

    if (testmode) {
        char t[132];

        while (fprintf(stderr, "\nTest: "), gets(t) != NULL) {
            int i;

            if (strlen(t) > 0) {
                for (i = 0; i < ncdefs; i++) {
                    if (strcmp(t, chardefs[i][0]) == 0) {
                        exchars(chardefs[i][1]);
                        i = -1;
                        break;
                    }
                }
                if (i > 0) {
                    printf("Duh....");
                }
            }
        }
        return 0;
    }

    if (ofname != NULL) {
                ofp = fopen(ofname, "wb");
        if (ofp == NULL) {
            fprintf(stderr, "Unable to open output file %s.\n", ofname);
            return 2;
        }
    } else {
        ofp = stdout;
    }
        shipshape(ofp);
    fclose(ofp);

    return 0;
}

/*  ALLOC  --  Allocate storage and fail if it can't be obtained.  */

void *alloc(unsigned nbytes)
{
    char *cp;

    if ((cp = malloc(nbytes)) == NULL) {
        fprintf(stderr, "\nOut of memory!!!\n");
        abort();
    }
    return (void *) cp;
}

/*  STRSAVE  --  Allocate a duplicate of a string.  */

char *strsave( char *s)
{
    char *c = (char *) alloc((unsigned) (strlen(s) + 1));

    strcpy(c, s);
    return c;
}

