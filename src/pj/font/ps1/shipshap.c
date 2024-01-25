/*

        Convert a PostScript font into an AutoCAD .SHP file

*/

#include "type1.h"

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
    "degree",            /* 127 */          /* AutoCAD-specific */
    "plusminus",         /* 128 */          /* AutoCAD-specific */
    "emptyset",          /* 129 */          /* AutoCAD-specific */
    NULL,                /* 130 */
    NULL,                /* 131 */
    NULL,                /* 132 */
    NULL,                /* 133 */
    NULL,                /* 134 */
    NULL,                /* 135 */
    NULL,                /* 136 */
    NULL,                /* 137 */
    NULL,                /* 138 */
    NULL,                /* 139 */
    NULL,                /* 140 */
    NULL,                /* 141 */
    NULL,                /* 142 */
    NULL,                /* 143 */
    NULL,                /* 144 */
    "quoteleft",         /* 145 */
    "quoteright",        /* 146 */
    NULL,                /* 147 */
    NULL,                /* 148 */
    NULL,                /* 149 */
    NULL,                /* 150 */
    NULL,                /* 151 */
    NULL,                /* 152 */
    NULL,                /* 153 */
    NULL,                /* 154 */
    NULL,                /* 155 */
    NULL,                /* 156 */
    NULL,                /* 157 */
    NULL,                /* 158 */
    NULL,                /* 159 */
    "space",             /* 160 */
    "exclamdown",        /* 161 */
    "cent",              /* 162 */
    "sterling",          /* 163 */
    "currency",          /* 164 */
    "yen",               /* 165 */
    "bar",               /* 166 */
    "section",           /* 167 */
    "dieresis",          /* 168 */
    "copyright",         /* 169 */
    "ordfeminine",       /* 170 */
    "guillemotleft",     /* 171 */
    "logicalnot",        /* 172 */
    "minus",             /* 173 */
    "registered",        /* 174 */
    "hyphen",            /* 175 */
    "ring",              /* 176 */
    "plusminus",         /* 177 */
    "twosuperior",       /* 178 */
    "threesuperior",     /* 179 */
    "acute",             /* 180 */
    "mu",                /* 181 */
    "paragraph",         /* 182 */
    "periodcentered",    /* 183 */
    "cedilla",           /* 184 */
    "onesuperior",       /* 185 */
    "ordmasculine",      /* 186 */
    "guillemotright",    /* 187 */
    "onequarter",        /* 188 */
    "onehalf",           /* 189 */
    "threequarters",     /* 190 */
    "questiondown",      /* 191 */
    "Agrave",            /* 192 */
    "Aacute",            /* 193 */
    "Acircumflex",       /* 194 */
    "Atilde",            /* 195 */
    "Adieresis",         /* 196 */
    "Aring",             /* 197 */
    "AE",                /* 198 */
    "Ccedilla",          /* 199 */
    "Egrave",            /* 200 */
    "Eacute",            /* 201 */
    "Ecircumflex",       /* 202 */
    "Edieresis",         /* 203 */
    "Igrave",            /* 204 */
    "Iacute",            /* 205 */
    "Icircumflex",       /* 206 */
    "Idieresis",         /* 207 */
    "Eth",               /* 208 */
    "Ntilde",            /* 209 */
    "Ograve",            /* 210 */
    "Oacute",            /* 211 */
    "Ocircumflex",       /* 212 */
    "Otilde",            /* 213 */
    "Odieresis",         /* 214 */
    "multiply",          /* 215 */
    "Oslash",            /* 216 */
    "Ugrave",            /* 217 */
    "Uacute",            /* 218 */
    "Ucircumflex",       /* 219 */
    "Udieresis",         /* 220 */
    "Yacute",            /* 221 */
    "Thorn",             /* 222 */
    "germandbls",        /* 223 */
    "agrave",            /* 224 */
    "aacute",            /* 225 */
    "acircumflex",       /* 226 */
    "atilde",            /* 227 */
    "adieresis",         /* 228 */
    "aring",             /* 229 */
    "ae",                /* 230 */
    "ccedilla",          /* 231 */
    "egrave",            /* 232 */
    "eacute",            /* 233 */
    "ecircumflex",       /* 234 */
    "edieresis",         /* 235 */
    "igrave",            /* 236 */
    "iacute",            /* 237 */
    "icircumflex",       /* 238 */
    "idieresis",         /* 239 */
    "eth",               /* 240 */
    "ntilde",            /* 241 */
    "ograve",            /* 242 */
    "oacute",            /* 243 */
    "ocircumflex",       /* 244 */
    "otilde",            /* 245 */
    "odieresis",         /* 246 */
    "divide",            /* 247 */
    "oslash",            /* 248 */
    "ugrave",            /* 249 */
    "uacute",            /* 250 */
    "ucircumflex",       /* 251 */
    "udieresis",         /* 252 */
    "yacute",            /* 253 */
    "thorn",             /* 254 */
    "ydieresis"          /* 255 */
};

/*  Standard Adobe encoding vector  */

static char *psmap[224] = {
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
    "quoteleft",         /*  96 */
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
    NULL,                /* 127 */
    NULL,                /* 128 */
    NULL,                /* 129 */
    NULL,                /* 130 */
    NULL,                /* 131 */
    NULL,                /* 132 */
    NULL,                /* 133 */
    NULL,                /* 134 */
    NULL,                /* 135 */
    NULL,                /* 136 */
    NULL,                /* 137 */
    NULL,                /* 138 */
    NULL,                /* 139 */
    NULL,                /* 140 */
    NULL,                /* 141 */
    NULL,                /* 142 */
    NULL,                /* 143 */
    NULL,                /* 144 */
    NULL,                /* 145 */
    NULL,                /* 146 */
    NULL,                /* 147 */
    NULL,                /* 148 */
    NULL,                /* 149 */
    NULL,                /* 150 */
    NULL,                /* 151 */
    NULL,                /* 152 */
    NULL,                /* 153 */
    NULL,                /* 154 */
    NULL,                /* 155 */
    NULL,                /* 156 */
    NULL,                /* 157 */
    NULL,                /* 158 */
    NULL,                /* 159 */
    NULL,                /* 160 */
    "exclamdown",        /* 161 */
    "cent",              /* 162 */
    "sterling",          /* 163 */
    "fraction",          /* 164 */
    "yen",               /* 165 */
    "florin",            /* 166 */
    "section",           /* 167 */
    "currency",          /* 168 */
    "quotesingle",       /* 169 */
    "quotedblleft",      /* 170 */
    "guillemotleft",     /* 171 */
    "guilsinglleft",     /* 172 */
    "guilsinglright",    /* 173 */
    "fi",                /* 174 */
    "fl",                /* 175 */
    NULL,                /* 176 */
    "endash",            /* 177 */
    "dagger",            /* 178 */
    "daggerdbl",         /* 179 */
    "periodcentered",    /* 180 */
    NULL,                /* 181 */
    "paragraph",         /* 182 */
    "bullet",            /* 183 */
    "quotesinglbase",    /* 184 */
    "quotedblbase",      /* 185 */
    "quotedblright",     /* 186 */
    "guillemotright",    /* 187 */
    "ellipsis",          /* 188 */
    "perthousand",       /* 189 */
    NULL,                /* 190 */
    "questiondown",      /* 191 */
    NULL,                /* 192 */
    "grave",             /* 193 */
    "acute",             /* 194 */
    "circumflex",        /* 195 */
    "tilde",             /* 196 */
    "macron",            /* 197 */
    "breve",             /* 198 */
    "dotaccent",         /* 199 */
    "dieresis",          /* 200 */
    NULL,                /* 201 */
    "ring",              /* 202 */
    "cedilla",           /* 203 */
    NULL,                /* 204 */
    "hungarumlaut",      /* 205 */
    "ogonek",            /* 206 */
    "caron",             /* 207 */
    "emdash",            /* 208 */
    NULL,                /* 209 */
    NULL,                /* 210 */
    NULL,                /* 211 */
    NULL,                /* 212 */
    NULL,                /* 213 */
    NULL,                /* 214 */
    NULL,                /* 215 */
    NULL,                /* 216 */
    NULL,                /* 217 */
    NULL,                /* 218 */
    NULL,                /* 219 */
    NULL,                /* 220 */
    NULL,                /* 221 */
    NULL,                /* 222 */
    NULL,                /* 223 */
    NULL,                /* 224 */
    "AE",                /* 225 */
    NULL,                /* 226 */
    NULL,                /* 227 */
    NULL,                /* 228 */
    NULL,                /* 229 */
    NULL,                /* 230 */
    NULL,                /* 231 */
    "Lslash",            /* 232 */
    "Oslash",            /* 233 */
    "OE",                /* 234 */
    "ordmasculine",      /* 235 */
    NULL,                /* 236 */
    NULL,                /* 237 */
    NULL,                /* 238 */
    NULL,                /* 239 */
    NULL,                /* 240 */
    "ae",                /* 241 */
    NULL,                /* 242 */
    NULL,                /* 243 */
    NULL,                /* 244 */
    "dotlessi",          /* 245 */
    NULL,                /* 246 */
    NULL,                /* 247 */
    "lslash",            /* 248 */
    "oslash",            /* 249 */
    "oe",                /* 250 */
    "germandbls",        /* 251 */
    NULL,                /* 252 */
    NULL,                /* 253 */
    NULL,                /* 254 */
    NULL                 /* 255 */
};

char *usermap[224];                   /* User-defined mapping vector */

char **charmap = isomap;              /* Active mapping vector */

#define CodeLimit   2048              /* Maximum bytes of shape code per char */

static int code[CodeLimit];           /* Compiled shape code */
static int codep = 0;                 /* Code pointer */
static enum {Up = 2, Down = 1} thepen;/* Pen state indicator */

#define Emit(x) /*printf("Emit(%d)\n", x);*/ if (codep >= CodeLimit) { fprintf(stderr, "\nShape code overflow!\n"); abort(); } else code[codep++] = (x)
#define pen(x) if (thepen != (x)) { Emit(x); thepen = (x); }

#define StackLimit  25                /* Maximum data stack depth */
#define OtherLimit  10                /* Maximum othersubr return values */

#define Sl(n) if (sp < (n)) {fprintf(stderr, "\nStack underflow.\n"); return;}
#define Npop(n) sp -= (n)
#define So(n) if ((sp + (n)) > StackLimit) {fprintf(stderr, "\nStack overflow.\n"); return;}
#define Clear() sp = 0
#define Dpath() psx = curx; psy = cury; pathopen = TRUE

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

static int curx = 0, cury = 0;        /* The current point */
static int psx = 0, psy = 0;          /* Path start co-ordinates */
static int pathopen;                  /* Path open ? */
static int flexing = FALSE;           /* If a Flex in progress ? */
static int flexx, flexy;              /* Flex current position */

/*  GOTO  --  Move or draw to a given location.  This apparently
              simple task is complicated by the fact that the
              displacement may be more than the maximum
              of -127 to +127.  If that's the case, we subdivide the
              range in the middle and recurse. */

static void goTo(x, y)
  int x, y;
{
#ifdef OLDWAY
    if (x != curx || y != cury) {
        int dx = x - curx,
            dy = y - cury;

        if (dx < -128 || dx > 127 ||
            dy < -128 || dy > 127) {
            int mx = curx + dx / 2,
                my = cury + dy / 2;

            goTo(mx, my);
            goTo(x, y);
        } else {
            Emit(8);                  /* X-Y displacement vector */
            Emit(dx);
            Emit(dy);
            curx = x;
            cury = y;
        }
    }
#else
    int scurx = curx + cury * skewsin + 0.499,
        sx = x + y * skewsin + 0.499;

    if (x != curx || y != cury) {
        int dx = sx - scurx,
            dy = y - cury;

        if (dx < -128 || dx > 127 ||
            dy < -128 || dy > 127) {
            if (thepen == Up) {
                Emit(4);
                Emit(100);
                Emit(8);
                Emit(dx / 100);
                Emit(dy / 100);
                Emit(3);
                Emit(100);
                if ((dx % 100) != 0 || (dy % 100) != 0) {
                    Emit(8);
                    Emit(dx % 100);
                    Emit(dy % 100);
                }
                curx = x;
                cury = y;
            } else {
#ifndef OLDWAY
                int mx = curx + (x - curx) / 2,
                    my = cury + dy / 2;

                goTo(mx, my);
                goTo(x, y);
#else
                int maxd = max(abs(dx), abs(dy)),
                    sfac = maxd / 64;

                Emit(4);
                Emit(sfac);
                Emit(8);
                Emit((dx / 2) / sfac);
                Emit((dy / 2) / sfac);
                Emit(8);
                Emit((dx / 2) / sfac);
                Emit((dy / 2) / sfac);
                Emit(3);
                Emit(sfac);
                Emit(8);
                Emit(dx % sfac);
                Emit(dy % sfac);
                curx = x;
                cury = y;
#endif
            }
        } else {
            Emit(8);                  /* X-Y displacement vector */
            Emit(dx);
            Emit(dy);
            curx = x;
            cury = y;
        }
    }
#endif
}

/*  BEZIER  --  Evaluate a Bezier curve defined by four control
                points.  */

static void bezier(long x0, long y0, long x1, long y1, long x2, long y2, long x3, long y3, int n)
{
    int i;
    double ax, bx, cx, ay, by, cy, t;

    ax = -x0 + 3 * x1 - 3 * x2 + x3;
    bx = 3 * x0 - 6 * x1 + 3 * x2;
        cx = 3 * (x1 - x0);

    ay = -y0 + 3 * y1 - 3 * y2 + y3;
    by = 3 * y0 - 6 * y1 + 3 * y2;
    cy = 3 * (y1 - y0);

    pen(Down);
#ifdef OLDWAY
    t = 0;
    for (i = 0; i <= n; i++) {
#else
    t = 1.0 / n;
    for (i = 1; i <= n; i++) {
#endif
        int vx, vy;

        vx = (ax * t * t * t + bx * t * t + cx * t + x0) + 0.5;
        vy = (ay * t * t * t + by * t * t + cy * t + y0) + 0.5;
        t += 1.0 / n;
        goTo(vx, vy);
    }
    curx = x3;
    cury = y3;
}

/*  OTHERSUBR  --  Execute an "othersubr" procedure.  */

static void othersubr(procno, nargs, argp)
  int procno, nargs, argp;
{
    static int flexp;                 /* Flex argument pointer */
    static int flexarg[8][2];
    static int spsx, spsy;            /* Path start save cells */

    orp = 0;                          /* Reset othersubr result pointer */

    switch (procno) {
        case 0:                       /* Flex */
            if (curx != flexarg[0][X] || cury != flexarg[0][Y]) {
                pen(Up);
                goTo(flexarg[0][X], flexarg[0][Y]);
            }

            /* If Flex is being suppressed in order to generate a
               quick displaying font, just draw a line to replace
               the shallow curve. */

            if (noflex) {
                pen(Down);
                goTo(flexarg[7][X], flexarg[7][Y]);
            } else {
                bezier(flexarg[0][X], flexarg[0][Y],
                       flexarg[2][X], flexarg[2][Y],
                       flexarg[3][X], flexarg[3][Y],
                       flexarg[4][X], flexarg[4][Y], bnum);
                bezier(flexarg[4][X], flexarg[4][Y],
                       flexarg[5][X], flexarg[5][Y],
                       flexarg[6][X], flexarg[6][Y],
                       flexarg[7][X], flexarg[7][Y], bnum);
            }
            osres[0] = stack[argp + 1];
            osres[1] = stack[argp + 2];
            psx = spsx;               /* Restore original path start */
            psy = spsy;
            flexing = FALSE;          /* Terminate flex */
            break;

        case 1:                       /* Flex start */
            flexing = TRUE;           /* Mark flex underway */
            flexx = curx;
            flexy = cury;
            spsx = psx;               /* Save current subpath start */
            spsy = psy;
            flexp = 0;
            /* Note fall-through */
        case 2:                       /* Flex argument specification */
            flexarg[flexp][X] = flexx;
            flexarg[flexp++][Y] = flexy;
            break;

        case 3:                       /* Hint replacement */
            osres[0] = 3;             /* Null hint replacement subr */
            break;

        default:
            fprintf(stderr, "\nCall to undefined othersubr %d\n",
                procno);
    }
}

/*  FINDVEC  --  Find a character by name given its standard encoding
                 vector index.  */

static unsigned char *findvec(i)
  int i;
{
    int j;

    for (j = 0; j < ncdefs; j++) {
        if (strcmp(chardefs[j][0], psmap[i - 32]) == 0) {
            return chardefs[j][1];
        }
    }
    return NULL;
}


/*  COMPSHAPE --  Execute charstring and compile a shape definition.  */

static void compshape(cp, fp, subshape)
  unsigned char *cp;
  FILE *fp;
  int subshape;
{
    int sub;
    char sdoc[90];
    int sidebear[2], charwid[2];      /* Character sidebearing and width */

    sp = rsp = 0;                     /* Reset stack pointer */
    pathopen = FALSE;                 /* No path open */
    flexing = FALSE;                  /* No flex in progess */

    sdoc[0] = EOS;
    while (TRUE) {
        int c = *cp++;

        if (c < 32) {
            /* Command */
            if (c == 12) {
                /* Two byte command */
                c = *cp++ + 32;
            }
            if (shapedoc) {
                if (strlen(sdoc) > 60) {
                    fprintf(fp, "; %s\n", sdoc);
                    sdoc[0] = EOS;
                }
                if (c < ncsnames) {
                    sprintf(sdoc + strlen(sdoc), " %s", csnames[c]);
                } else {
                    sprintf(sdoc + strlen(sdoc), " (%d)", c);
                }
            }

            switch (c) {

                /* Commands for Starting and Finishing */

                case Endchar:         /* 14: End character */
                    Emit(6);          /* Pop location */
                    curx = cury = 0;
                    pen(Up);
                    goTo(charwid[X], charwid[Y]);
                    if (!subshape) {
                        Emit(4);
                        Emit(10);
                    }
                    Clear();
                    if (shapedoc) {
                        fprintf(fp, "; %s\n\n", sdoc);
                    }
                    return;

                case Hsbw:            /* 13:  Set horizontal sidebearing */
                    Sl(2);
                    Emit(5);          /* Save location */
                    if (!subshape) {
                        Emit(3);      /* Establish global scale factor */
                        Emit(10);
                    }
                    curx = cury = 0;
                    if (S1 != 0) {
                        pen(Up);
                        goTo(sidebear[X] = S1, 0);
                    }
                    charwid[X] = S0;
                    charwid[Y] = 0;
                    Clear();
                    break;

                case Seac:            /* 12-6:  Standard encoding accented char */
                    Sl(5);
/* fprintf(stderr, "Seac(%d: (%d %d) %d %d)\n", S4, S3, S2, S1, S0); */
                    {   int asb = S4,
                            adx = S3,
                            ady = S2,
                            bchar = S1,
                            achar = S0;
                        unsigned char *bc = findvec(bchar);

                        if (bc != NULL) {
                            compshape(bc, fp, TRUE);
                        } else {
                            fprintf(stderr, "Bogus base char of %d in seac.\n",
                                bchar);
                        }

                        bc = findvec(achar);

                        if (bc != NULL) {
                            Emit(6);  /* Pop character origin position */
                            Emit(5);  /* Push it back on the stack */
                            curx = cury = 0;
                            pen(Up);
                            goTo(adx - (asb - sidebear[X]), ady);
                            compshape(bc, fp, TRUE);
                        } else {
                            fprintf(stderr,
                                "Bogus accent char of %d in seac.\n", achar);
                        }
                    }
                    Emit(6);          /* Pop location */
                    curx = cury = 0;
                    pen(Up);
                    goTo(charwid[X], charwid[Y]);
                    Emit(4);          /* Then turn it off again. */
                    Emit(10);
                    Clear();
                    if (shapedoc) {
                        fprintf(fp, "; %s\n\n", sdoc);
                    }
                    return;

                case Sbw:             /* 12-7:  Sidebearing point (x-y) */
                    Sl(4);
                    Emit(5);          /* Save location */
                    Emit(3);
                    Emit(10);
                    curx = cury = 0;
                    if (S3 != 0 || S2 != 0) {
                        pen(Up);
                        goTo(sidebear[X] = S3, sidebear[Y] = S2);
                    }
                    charwid[X] = S1;
                    charwid[Y] = S0;
                    Clear();
                    break;

                /* Path Construction Commands */

                case Closepath:       /* 9:  Close path */
                    if (!pathopen) {
                        fprintf(stderr, "\nClosepath, yet no path open.\n");
                    } else {
                        int scurx = curx,
                            scury = cury;

                        /* This little dance is required because
                           closepath doesn't change the current point. */

                        Emit(5);      /* Push current location */
                        pen(Down);
                        goTo(psx, psy);
                        Emit(6);      /* Pop current location */
                        curx = scurx; /* Restore our memory of where we are */
                        cury = scury;
                    }
                    pathopen = FALSE;
                    Clear();
                    break;

                case Hlineto:         /* 6: Horizontal line to */
                    Sl(1);
                    pen(Down);
                    goTo(curx + S0, cury);
                    Clear();
                    break;

                case Hmoveto:         /* 22:  Horizontal move to */
                    Sl(1);
                    if (flexing) {
                        flexx += S0;
                    } else {
                        pen(Up);
                        goTo(curx + S0, cury);
                        Dpath();
                    }
                    Clear();
                    break;

                case Hvcurveto:       /* 31:  Horizontal-vertical curve to */
                    Sl(4);
                        bezier(curx, cury, curx + S3, cury,
                           curx + S3 + S2, cury + S1,
                           curx + S3 + S2, cury + S1 + S0, bnum);
                    Clear();
                    break;

                case Rlineto:         /* 5:  Relative line to */
                    Sl(2);
                    pen(Down);
                    goTo(curx + S1, cury + S0);
                    Clear();
                    break;

                case Rmoveto:         /* 21:  Relative move to */
                    Sl(2);
                    if (flexing) {
                        flexx += S1;
                        flexy += S0;
                    } else {
                        pen(Up);
                        goTo(curx + S1, cury + S0);
                        Dpath();
                    }
                    Clear();
                    break;

                case Rrcurveto:       /* 8:  Relative curve to */
                    Sl(6);
                    bezier(curx, cury, curx + S5, cury + S4,
                           curx + S5 + S3, cury + S4 + S2,
                           curx + S5 + S3 + S1, cury + S4 + S2 + S0, bnum);
                    Clear();
                    break;

                case Vhcurveto:       /* 30:  Vertical-horizontal curve to */
                    Sl(4);
                    bezier(curx, cury, curx, cury + S3,
                           curx + S2, cury + S3 + S1,
                           curx + S2 + S0, cury + S3 + S1, bnum);
                    Clear();
                    break;

                case Vlineto:         /* 7:  Vertical line to */
                    Sl(1);
                    pen(Down);
                    goTo(curx, cury + S0);
                    Clear();
                    break;

                case Vmoveto:         /* 4:  Vertical move to */
                    Sl(1);
                    if (flexing) {
                        flexy += S0;
                    } else {
                        pen(Up);
                        goTo(curx, cury + S0);
                        Dpath();
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
                    othersubr(S0, S1, sp - (2 + S1));
                    Npop(2 + S1);
                    break;

                case Callsubr:        /* 10:  Call subroutine */
                    Sl(1);
                    if (rsp >= ReturnStackLimit) {
                        fprintf(stderr, "\nReturn stack overflow.\n");
                        return;
                    }
                    rstack[rsp++] = cp;
                    sub = S0;
                    Npop(1);
                    if (sub < 0 || sub >= nsubrs) {
                        fprintf(stderr, "\nSubr number %d out of range in call.\n",
                            sub);
                        return;
                    }
                    if (subrs[sub] == NULL) {
                        fprintf(stderr, "\nSubr %d is undefined.\n", sub);
                        return;
                    }
                    cp = subrs[sub];  /* Set instruction pointer to subr code */
                    break;

                case Pop:             /* 12 17:  Return argument from othersubr */
                    So(1);
                    stack[sp++] = osres[orp++];
                    break;

                case Return:          /* 11:  Return from subroutine */
                    if (rsp < 1) {
                        fprintf(stderr, "\nReturn stack underflow.\n");
                        return;
                    }
                    cp = rstack[--rsp]; /* Restore pushed call address */
                    break;

                case Setcurrentpoint: /* 12 33:  Set current point */
                    Sl(2);
                    if (S1 != curx || S0 != cury) {
                        pen(Up);
                        goTo(S1, S0);
                    }
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
            if (shapedoc) {
                if (strlen(sdoc) > 60) {
                    fprintf(fp, "; %s\n", sdoc);
                    sdoc[0] = EOS;
                }
                sprintf(sdoc + strlen(sdoc), " %d", n);
            }
            if (sp >= StackLimit) {
                fprintf(stderr, "\nData stack overflow.\n");
            } else {
                stack[sp++] = n;
            }
        }
    }
}

/*  COMPOUT  --  Output compiled shape.  */

static int compout(fp, i, c)
  FILE *fp;
  int i, c;
{
    int j;

    for (j = 0; j < ncdefs; j++) {
        if (strcmp(chardefs[j][0], charmap[i - 32]) == 0) {

            /* Found the character! */

            if (shapedoc) {
                fprintf(fp, "\n;\t\t\t%s\n\n", charmap[i - 32]);
            }
            codep = 0;                /* Reset code pointer */
            thepen = Down;            /* Mark pen as down */
            compshape(chardefs[j][1], fp, FALSE);
            if (codep > 0) {
                char chname[80], outline[82];
                int k;

                if (islower(*charmap[i - 32])) {
                    strcpy(chname, charmap[i - 32]);
                } else {
                    sprintf(chname, "upper_case%c%s",
                        tolower(*charmap[i - 32]), charmap[i - 32] + 1);
                }
                fprintf(fp, "*%d,%d,%s\n", c, codep + 1, chname);
                outline[0] = EOS;
                for (k = 0; k < codep; k++) {
                    if (strlen(outline) > 70) {
                        fprintf(fp, "%s\n", outline);
                        outline[0] = EOS;
                    }
                    sprintf(outline + strlen(outline), "%d,", code[k]);
                }
                fprintf(fp, "%s0\n", outline);
            }
            return TRUE;
        }
    }
    return FALSE;
}

/*  SHIPSHAPE  --  Map a PostScript font into the AutoCAD encoding
                   and export it as a .SHP file.  */

void shipshape(fp)
  FILE *fp;
{
    int i;

    fprintf(fp, "*0,4,%s\n", fullname);
    fprintf(fp, "64,16,2,0\n");
    fprintf(fp, "*10,13,line_feed\n2,4,10,8,(0,-12),14,8,(12,12),3,10,0\n");

    /* Now loop through the standard character positions, look up
       the corresponding PostScript character definition by name,
       and execute its defining code to compile the shape definition. */

    for (i = 32; i < 256; i++) {
        if (charmap[i - 32] != NULL) {
            if (!compout(fp, i, i)) {
                int substituted = FALSE;

                if (strcmp(charmap[i - 32], "emptyset") == 0) {
                    int j;

                    for (j = 32; j < 256; j++) {
                        if (charmap[j - 32] != NULL &&
                            strcmp(charmap[j - 32], "Oslash") == 0) {
                            if (compout(fp, j, i)) {
                                substituted = TRUE;
                            }
                            break;
                        }
                    }
                }
                if (!substituted) {
                    fprintf(fp,
                        "; Warning: character %d (%s) not defined in font.\n",
                        i, charmap[i - 32]);
                    if (!compout(fp, '?', i))
                        compout(fp, ' ', i);
                }
            }
        }
    }
}
