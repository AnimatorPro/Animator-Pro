/* #define TRACE */

/*

        Interpret character string definition of a PostScript Type 1 font.

        I got this code from Carey Clutts, who got it from John Walker I
        believe who got it "off the net".

*/

#include "type1.h"

#define StackLimit  25

#define Sl(n) if (sp < (n)) {fflush(stdout); fprintf(stderr, "\nStack underflow.\n"); return;}
#define Npop(n) sp -= (n)
#define So(n) if ((sp + (n)) > StackLimit) {fflush(stdout); fprintf(stderr, "\nStack overflow.\n"); return;}
#define Clear() sp = 0
//#define Opath() if (!pathopen) { psx = curx; psy = cury; pathopen = TRUE; ShapeOpen(); ShapePoint(curx,cury); }
//#define Dpath() pathopen = FALSE; Opath()



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
static int sidebear[2], charwid[2];   /* Character sidebearing and width */
static int pathopen;                  /* Path open ? */


/* Here are the routines responsible for drawing. */
void ShapeOpen()
{
        printf("{\n");
}

void ShapeClose()
{
        printf("};\n");
}

void ShapePoint(double x, double y)
{
        printf("\t{%f, %f},\n", x, y);
}

void Opath()
/************************************************************************
 * Start new closed shape if not in the middle of one already.
 ************************************************************************/
{
        if (!pathopen)
        {
                psx = curx;     // Remember first coordinate.
                psy = cury;
                pathopen = TRUE;
                ShapeOpen();
                ShapePoint(curx,cury);
        }
}

void Dpath()
/************************************************************************
 * Close current shape if any and start a new one.
 ************************************************************************/
{
        if (pathopen)
        {
                ShapeClose();
                pathopen = FALSE;
        }
        Opath();
}

/*  DUMPCHARS  --  Dump charstring.  */

void dumpchars(unsigned char *cp,  int l)
{
        while (l-- > 0) {
        int c = *cp++;

        if (c < 32) {
                /* Command */
                if (c == 12) {
                /* Two byte command */
                c = *cp++ + 32;
                l--;
                }
                if (c < ncsnames) {
                                printf(" %s", csnames[c]);
            } else {
                printf(" (%d)", c);
            }

            switch (c) {
                case Endchar:
                    return;

                case Seac:
                    return;
            }

        } else {
                long n;

                if (c <= 246) {
                n = c - 139;
                } else if (c <= 250) {
                n = ((c - 247) << 8) + *cp++ + 108;
                l--;
                } else if (c < 255) {
                n = -((c - 251) << 8) - *cp++ - 108;
                l--;
                } else {
                char ba[4];

                ba[0] = *cp++;
                ba[1] = *cp++;
                ba[2] = *cp++;
                ba[3] = *cp++;
                l -= 4;
                n = (((((ba[0] << 8) | ba[1]) << 8) | ba[2]) << 8) | ba[3];
                }
                        printf(" %d", n);
        }
        }
}

/*  BEZIER  --  Evaluate a Bezier curve defined by four control
                points.  */

static void bezier(long x0, long y0
, long x1, long y1, long x2, long y2, long x3, long y3, int n)
{
        int i;
        double ax, bx, cx, ay, by, cy, t;
        double dt = 1.0/n;
        ax = -x0 + 3 * x1 - 3 * x2 + x3;
        bx = 3 * x0 - 6 * x1 + 3 * x2;
        cx = 3 * (x1 - x0);

        ay = -y0 + 3 * y1 - 3 * y2 + y3;
        by = 3 * y0 - 6 * y1 + 3 * y2;
        cy = 3 * (y1 - y0);

        t = dt;
        for (i = 1; i <= n; i++) {
        double vx, vy;

        vx = ax * t * t * t + bx * t * t + cx * t + x0;
        vy = ay * t * t * t + by * t * t + cy * t + y0;
        t += dt;
        ShapePoint(vx,vy);
        }
        curx = x3;
        cury = y3;
}

/*  OTHERSUBR  --  Execute an "othersubr" procedure.  */

static void othersubr(int procno, int nargs, int argp)
{
#ifdef SOON
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
#endif /* SOON */
}

/*  EXCHARS  --  Execute charstring.  */

void exchars(unsigned char *cp)
{
    int sub;
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
#ifdef TRACE
            if (c < ncsnames) {
    printf(" %s", csnames[c]);
            } else {
        printf(" (%d)", c);
            }
#endif

            switch (c) {

                /* Commands for Starting and Finishing */

                case Endchar:         /* 14: End character */
                    printf("UCS Origin %d,%d,0\n",
                        charwid[X], charwid[Y]);
                    Clear();
                        return;

                case Hsbw:            /* 13:  Set horizontal sidebearing */
                        Sl(2);
                        curx = sidebear[X] = S1;
                        cury = sidebear[Y] = 0;
                        charwid[X] = S0;
                        charwid[Y] = 0;
                        Clear();
                        break;

                case Seac:            /* 12-6:  Standard encoding accented char */
                        Sl(5);
                        Clear();
                        return;

                case Sbw:             /* 12-7:  Sidebearing point (x-y) */
                        Sl(4);
                        curx = sidebear[X] = S3;
                        cury = sidebear[Y] = S2;
                        charwid[X] = S1;
                        charwid[Y] = S0;
                        Clear();
                        break;

                /* Path Construction Commands */

                case Closepath:       /* 9:  Close path */
                        if (!pathopen) {
                        fflush(stdout);
                                                fprintf(stderr, "\nClosepath, yet no path open.\n");
                        } else {
                                ShapeClose();
                        }
                        pathopen = FALSE;
                        Clear();
                        break;

                case Hlineto:         /* 6: Horizontal line to */
                        Sl(1);
                        Opath();
                        curx = curx + S0;
                        ShapePoint(curx, cury);
                        Clear();
                        break;

                case Hmoveto:         /* 22:  Horizontal move to */
                        Sl(1);
                        curx += S0;
                        Dpath();
                        Clear();
                        break;

                case Hvcurveto:       /* 31:  Horizontal-vertical curve to */
                        Sl(4);
                        Opath();
                        bezier(curx, cury, curx + S3, cury,
                           curx + S3 + S2, cury + S1,
                           curx + S3 + S2, cury + S1 + S0, bnum);
                        Clear();
                        break;

                case Rlineto:         /* 5:  Relative line to */
                        Sl(2);
                        Opath();
                        curx += S1;
                        cury += S0;
                        ShapePoint(curx, cury);
                        Clear();
                        break;

                case Rmoveto:         /* 21:  Relative move to */
                        Sl(2);
                        curx += S1;
                        cury += S0;
                        Dpath();
                        Clear();
                        break;

                case Rrcurveto:       /* 8:  Relative curve to */
                        Sl(6);
                        Opath();
                        bezier(curx, cury, curx + S5, cury + S4,
                           curx + S5 + S3, cury + S4 + S2,
                           curx + S5 + S3 + S1, cury + S4 + S2 + S0, bnum);
                        Clear();
                        break;

                case Vhcurveto:       /* 30:  Vertical-horizontal curve to */
                        Sl(4);
                        Opath();
                        bezier(curx, cury, curx, cury + S3,
                           curx + S2, cury + S3 + S1,
                           curx + S2 + S0, cury + S3 + S1, bnum);
                        Clear();
                        break;

                case Vlineto:         /* 7:  Vertical line to */
                        Sl(1);
                        Opath();
                        cury = cury + S0;
                        ShapePoint(curx,cury);
                        Clear();
                        break;

                case Vmoveto:         /* 4:  Vertical move to */
                        Sl(1);
                        cury += S0;
                        Dpath();
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
                        fflush(stdout);
                                                fprintf(stderr, "\nReturn stack overflow.\n");
                        return;
                        }
                        rstack[rsp++] = cp;
                        sub = S0;
                        Npop(1);
                        if (sub < 0 || sub >= nsubrs) {
                        fflush(stdout);
                                                fprintf(stderr, "\nSubr number %d out of range in call.\n",
                                sub);
                        return;
                        }
                        if (subrs[sub] == NULL) {
                        fflush(stdout);
                                                fprintf(stderr, "\nSubr %d is undefined.\n", sub);
                        return;
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
                                                fprintf(stderr, "\nReturn stack underflow.\n");
                        return;
                        }
                        cp = rstack[--rsp]; /* Restore pushed call address */
                        break;

                case Setcurrentpoint: /* 12 33:  Set current point */
                        Sl(2);
                        Clear();
                        printf(">>>Setcurrentpoint<<<\n"); //DEBUG!
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
#ifdef TRACE
    printf(" %d", n);
#endif
            if (sp >= StackLimit) {
                fflush(stdout);
                fprintf(stderr, "\nData stack overflow.\n");
            } else {
                stack[sp++] = n;
            }
        }
    }
}
