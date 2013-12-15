/* Next available MSG number is   5 */
/*
      (C) Copyright 1985-1991 by Autodesk, Inc.

******************************************************************************
*                                                                            *
* The information contained herein is confidential, proprietary to Autodesk, *
* Inc., and considered a trade secret as defined in section 499C of the      *
* penal code of the State of California.  Use of this information by anyone  *
* other than authorized employees of Autodesk, Inc. is granted only under a  *
* written non-disclosure agreement, expressly prescribing the scope and      *
* manner of such use.                                                        *
*                                                                            *
**************************************************************************** */

/*     PWIDGET.C

  Functions specific to the Swiss (i.e. parallel) Hardware Lock.

                         Hardware Lock Overview


WARNING ABOUT OS/2 HARDWARE LOCK CHECKSUMING CODE:
Please read the comments in /os2/os2mach.c regarding the checksuming code.


I.  What is the Purpose of the Hardware Lock?

The hardware lock is used by AutoCAD to ensure that the running copy was
obtained through legitimate channels.  Obviously, nothing can completely
achieve this goal.  As with most security measures, this one is
primarily concerned with "amateur" protection violations.  Someone who
is willing to sit down with an ICE for a couple of weeks can almost
certainly produce a copy of AutoCAD which does not require a hardware
lock at all.  For the time being though, it is fairly unlikely that
anyone would go to very extensive efforts to defeat the lock since the
U.S. (English) version of AutoCAD will run without a lock installed.

As a secondary goal, the harware lock implementation should facillitate
detection of anyone who attempts (and more importantly succeeds) at
running a protected version of AutoCAD without a lock.

The tertiary goal is to lay the groundwork for a better implementation
(i.e., one requiring a higher level of sophistication to defeat) in the
future.



II.  What is a Hardware Lock?

The hardare lock is (literally) a black box which is connected to a
parallel port of a machine.  It has two DB-25 connectors on it, one to be
connected to the machine and one to be connected to whatever else would
normally have been attached to the port.

Most of the time, the hardware lock device is completely transparent to
both the hardware and the software to which it is connected.  When
stimulated by the right host machine signals, the hardware lock becomes
active (excited?).
The data absorbed by an active hardware lock is compressed, encrypted
and returned to the host machine by the hardware lock.  The encryption
key is a manufacturing (bonding) option of the lock.

An active hardware lock can be deactivated at any time by the host
machine at which time the lock reverts to transparent mode.


III.  What Does the AutoCAD User See of the Lock?

Except for training versions of AutoCAD (which do not require the lock),
the AutoCAD user will purchase a hardware lock bundled together with the
software and documentation.  Before bringing up AutoCAD, the lock must
be installed on one of the parallel ports supported by a machine.
machine.
AutoCAD will not even  permit an initial configuration without the lock
installed.
Once the initial check for the presense of a hardware lock is passed,
periodic additional checks will occur during AutoCAD use.  If the
hardware lock is absent during one of the subsequent checks, it will be
treated as an I/O error.  The user will have the options to ABORT or to
correct the problem and RETRY the validation.

Aside from these interactions and perhaps a small performance
degredation due to the periodic validation of the lock, the user should
see no other changes in his normal operational procedures.

      changed for TOPLOCK               Jiri Zeman
                                        Dezember 24, 1987
      tampering code improved
      and adapted                       Dieter Voegtli
                                        August 15, 1988

      Some serial port specific variables and functions are left as
      they are, because they have to interface with other modules.

      The following changes were made to let crash several existing
      'lock removal programs':
      Addressing pointers to tamper modified.
      Function challenge(), first occurence
      of xor-ing tamper: bit 0x100 included, too.
                                        Dieter Voegtli
                                        Mai 29, 1989
*/

#include        "astdio.h"
#include        "mcdefs.h"

#include        "imath.h"




#ifdef  STATIC
#undef  STATIC
#endif
#define STATIC  static                /* For almost all cases, force STATICs
                                         to be static */

#ifdef  WDEBUG
#undef  WDEBUG                        /* This should only be defined due to
                                         DEBUG below */
#endif

#ifdef  DEBUG
#ifdef  PRODUCTION
#undef  DEBUG                         /* Don't allow debug compilation for
                                         PRODUCTION */
#else
#define WDEBUG
#undef  STATIC
#define STATIC                        /* For DEBUG, non-PRODUCTION
                                         compilations only */
#endif
#endif



       /* Don't make it easy to find the
          I/O addresses:  */

static int io_data[] = {~(0x3BC), ~(0x378), ~(0x278)};
static int io_stat[] = {~(0x3BD), ~(0x379), ~(0x279)};
static int io_cmd[] = {~(0x3BE), ~(0x37A), ~(0x27A)};

int *xio_data = io_data + ELEMENTS(io_data);
int *xio_stat = io_stat + ELEMENTS(io_stat);
int *xio_cmd = io_cmd + ELEMENTS(io_cmd);

static unsigned irand(), parcalc();   /* Forward references */

int ccnt = 0;

static unsigned parity = 0;           /* 1 for odd parity else 0      */
static unsigned baudrate = ~0;
static unsigned parbit = 0;           /* Mask for parity bit or zero
                                         if no parity                 */
static unsigned stopbit;              /* Mask for stopbit(s) and all
                                         higher bits                  */
static unsigned maxdata;              /* Max. legal value for portdata*/
extern short otimeron;                /* Non-zero to time-out ouput
                                         operations                   */
static short lasterror;               /* TRUE if some kind of an error
                                         occurred on the last hardware lock
                                         challenge and we want to ensure a
                                         message (rather than a lock-up) if
                                         something goes wrong this time */
extern short tamper[2];               /* Flags to try to detect code
                                         tampering.  A full description is
                                         just above "chktamp()". This code
                                         does many "noise" (don't care)
                                         perturbations to "tamper[0]".  The
                                         ones that count are the ones in the
                                         comments. */
                                      /* Now the changed values: (DV) */

#define FIFTY_THREE 53
static short *tamperp1 = tamper + 83, *tamperp2 = tamper + 201;

#define tamper  (tamperp1[-83])
#define tamper2 (tamperp2[-200])


extern char sent2[];                  /* Serial-nr-chk-failed flg (EDINIT) */
extern short goodl;                   /* Flag to cause HWL not-installed
                                         message */
extern (*chkwidgp) ();                /* Pointer to routine chkwidg in
                                         _PAGEIO.C */

STATIC int upd(), challenge(), chktamp();

/* All functions in this file are accessed from outside of the file
   via the following vector.  We don't want any direct code references
   since they are a bit too easy to find with a debugger. */
int (*(widgetp[])) () = {
     challenge,
     upd,
     chktamp
    };


/*  The "tamper" mechanism is an attempt to prevent software pirates
    from alterring an executable copy of AutoCAD to execute (reliably)
    without a hardware lock.  Obviously, no security measure can prevail
    against someone who is sufficiently (skilled and) determined.  This
    mechanism ought to be pretty hard to defeat--at least without a
    source listing.

    The general handling of the "tamper" cells in the hardware lock code is
    to set a "condition" in tamper[0] before calling a routine and then have
    that routine (or something it calls) clear the "condition".  The idea
    being that anyone who short-circuits a part of the code is likely to
    miss the setting of a bit somewhere before the calls.  Even if a
    potential pirate does notice the bit fiddling with tamper, it is so
    complicated (from looking at the generated code) that it would be
    difficult to figure out what to replace the existing code with.  While
    each "condition" is logically just one bit of information, the
    representations are quite varied and there are a number of intentionally
    misleading manipulations.  There is no rule for routines which return a
    failing status.  They may or may not clear "condition"s which they clear
    on a "succeed" return--see individual routine comments.

    There are three places in AutoCAD where the current value of "tamper[0]"
    is validated:  COMMAND, ZOOMC and EDTERM.  "Tamper[0]" is valid if and
    only if no condition other than possibly conditions I and L are set.
    The actions are not nice in any case.

    The "conditions" currently used in "tamper[0]" are defined by the
    following tables.  The single letter condition names have no purpose
    other than to cross reference between the two tables.  For example,
    condition D is true whenever bit 0x0008 of tamper[0] can be exclusive
    or'ed with bit 0x0400 and the result of the XOR is a zero.  Routine
    "challenge()" reverses the condition and must therefore only be called
    after setting the condition, i.e., after ensuring that the two bits have
    the same value.  Note that logically it is a null operation to
    compliment (or to interchange) both bits 0x0008 and 0x0400.  Such null
    operations occur frequently during the execution of the hardware lock
    challenge code.

    Tamper[1] is just a running logical OR of tamper[0].  This is used to
    tell whether or not condition L was set just before a call to chkwidg.
    Condition L in tamper[1] gets cleared within chkwidg if challenge
    returns a failing status and condition L was also set in tamper[0].
    We've had several attempts to defeat the hardware lock by simply forcing
    a successful return from challenge at the call in chkwidg.  This defends
    against that particular attack.

    Condition           Principle Operation on this Condition
    ---------   ------------------------------------------------------------
        A       Cleared by "ioww()" in MSDOS.C
        B       Cleared by "upd()" in WIDGET.C
        C       Cleared by "check()" in WIDGET.C
        D       Reversed by "challenge()" in WIDGET.C.  This condition is
                always reversed by "challenge()", even on a failing
                challenge.
        E       Cleared by "irand()" in WIDGET.C
        F       Never cleared!  Set occasionally in "getpage()" in _PAGEIO.C
                if the pointer "chkwidgp" has the wrong value or if "cntwidg"
                is out of bounds.  Also used internally in CHKTAMP.
        G       Cleared by "chall()" in WIDGET.C.
        H       Set in "check()" of WIDGET.C (on rare occasions) when it
                deliberately gives a fail return.  If the fail status
                makes it all the back out to "chkwidg" or "wdginit" in
                _PAGEIO.C, the condition is cleared and the challenge is
                tried again.
        I       Garbage--condition is set, reset and reversed all over.
        J       Never set.  Cleared occasionally at random spots.
        K       Set if condition L is set on a successful return from
                "challenge" to "chkwidg" in _PAGEIO.C.
        L       Set occasionally at random by MSDOS.C and COMMAND.C.  If
                set, then "check()" in WIDGET.C will fail.  Cleared on a
                failed return from "challenge" to "chkwidg" in _PAGEIO.C.
                (In this case, there is also an automatic retry.)  If
                "chkwidg" gets a succeed return and the 0x8000 bit is set,
                _PAGEIO.C sets condition K.

    Condition       Definition of "Set" or "True" state for the Condition
    ---------   ------------------------------------------------------------
        A       The compliment of bit 0x0001.
        B       Bit 0x0002.
        C       Bit 0x0004.
        D       Bit 0x0008 XOR bit 0x0400.
        E       Bit 0x0010 XOR bit 0x0800.
        F       Bit 0x0020.
        G       The compliment of (bit 0x0040 XOR bit 0x2000).
        H       Bit 0x0080.
        I       Bit 0x0100 XOR bit 0x0200.
        J       Bit 0x1000.
        K       The compliment of bit 0x4000.
        L       Bit 0x8000.

If the hardware lock is not configured, then "_paginit()" forces the value of
"tamper[0]" to a valid state before any harm is done.  The initially true
conditions are:  A, B, C, D, E, G and I.  For non-hardware lock versions
of AutoCAD, this gets changed to just condition I during initializations. */


/*  CHKTAMP--The "second-order" tamper detection mechanism.  CHKTAMP is
    called occasionally to ensure that our tamper resistant mechanisms
    are still functioning.  Its basic operation is to call "chkwidg" but
    to watch the values of "tamper" both before and after the call.  If
    there is a statistically (very) improbable run of identical input or
    output values, then we declare it to be the result of tampering.  To
    try to ensure that it's not a "random" event, "chktamp()" perturbs
    the values of tamper just before the call to "chkwidg()".  This stuff
    all works for a non-hardware lock version of AutoCAD because the
    perturbation of tamper sets the 0x0004 bit occasionally.  Even when
    the hardware lock code is turned off, doing this will cause "chkwidg"
    to alter the returned value in tamper[0].  */
STATIC
/*FCN*/chktamp()
{
    static (**ppchkwidg) () = &chkwidgp - FIFTY_THREE;
    static char fifty_three = FIFTY_THREE;

#define chkwidg()     (*ppchkwidg[fifty_three])()
    static char *basep = sent2 + FIFTY_THREE;
    union {
        long l;
        char c[sizeof(long)];
    } sercheck;
    static short *goodlp = &goodl - FIFTY_THREE;
    static char inarow = 0, samein = 0;
    static short intamper = ~0, outamper = 0;
    static short xormask[] = {
        0x0008 | 0x0100 | 0x0400, 0x0008 | 0x0040 | 0x0400 | 0x2000, 0x0200,
        0x0001 | 0x0004 | 0x0040 | 0x0100, 0x0100 | 0x8000
    };
    int i;
    unsigned u;

    if (intamper == tamper) {
        samein++;
    } else {
        intamper = tamper;
        samein = 0;
    }
    if (inarow != 0)
        tamper ^= xormask[inarow - 1];
    tamper2 |= tamper;

    chkwidg();

    if (tamper != outamper) {
        /* We didn't get the same value in tamper[0] as the last time */
        inarow = 0;
    } else if (inarow >= ELEMENTS(xormask)) {
#ifdef  WDEBUG
        erabt(/*MSG0*/"!WIDGET 1 %d %04x %04x %04x",
              inarow, tamper, tamper2, intamper);
#endif
        tamper |= 0x0020;             /* Tamper violation!--report it next
                                         time */
        inarow = 0;
    } else
        inarow++;
    if (samein >= 15 || (outamper | intamper) & 0x0020) {
#ifdef WDEBUG
        erabt(/*MSG0*/"!WIDGET 2 %d %04x %04x %04x",
              samein, outamper, intamper, tamper);
#endif
        /* We've got a second-order tamper violation!  While "tamper" may
           contain a "valid" value, the sequence of values we observed
           indicates that someone is stuffing a value into cell "tamper[0]"
           someplace.  Given that this is happening, it's pointless to set a
           tamper bit to cause a delayed abort. We get them by faking a
           serial number violation!  A ZOOM or PAN command will result in a
           rapid demise if the serial number is bad.  COMMAND.C also checks
           it occasionally. */
        /* Cause "Invalid drawing file pointer 35" [0x35==FIFTY_THREE] */
        sercheck.l = 3;
        for (i = sizeof(sercheck.l); i--;)
            basep[i - sercheck.l] = sercheck.c[i];
        /* Did tamperrer miss the obvious?  Here's another way to cause a
           delayed abort.  Routine docmd() decrements goodl if it's greater
           than 0 until it gets to 1 and then causes a nasty abort, possibly
           with an intentionally hung system! */
        u = goodlp[fifty_three];
        goodlp[fifty_three] = (u == 0 ? 100 /* Die in 100 commands! */
                                      : u < 3 ? u    /* Will die soon! */
                                              : u <= 53 ? u - 1 /* sooner */
                                                        : 53);
    }
#ifdef  WDEBUG
    if (tamper2 & 0x8000)
        erabt(/*MSG0*/"!WIDGET 3 %04x %04x %04x %04x",
              tamper, tamper2, intamper, outamper);
#endif
    outamper = tamper | ((tamper2 & 0x8000) >> 10);
#undef  chkwidg
}



/*  UPD -- Update parameters relevant to the hardware lock  */
STATIC
/*FCN*/upd(hwlport, dbits, par, baud)
  char *hwlport;
  unsigned dbits, par, baud;
{
    maxdata = (1 << dbits) - 1;
    stopbit = ~maxdata;
    if (par) {
        par += parcalc(baud);         /* Reverse low-bit of "par" if "baud"
                                         has odd parity in its low 8 bits.
                                         This is compensated for in
                                         "challenge()".  */
        parbit = maxdata + 1;
        stopbit ^= parbit;
    } else
        parbit = 0;
    parity = par & 1;
    tamper &= ~(0x0002 | 0x0100 | 0x8000); /* Clear 0x0002 and 0x8000 */
    baudrate = baud;
    lasterror = TRUE;                 /* Ensure a message */
}


/*  CHALLENGE -- Give the hardware lock two tries to come up with the right
    response.
    Also note that this routine always reverses
    condition D (see comments in _PAGEIO.C) even on a failing return.
 */
STATIC int
/*FCN*/challenge()
{
    unsigned char timestart;
    unsigned count;
    int retsav;
    int retval = BAD;
    short svotimeron = otimeron;

#ifdef WDEBUG
    printf(/*MSG0*/"CALLED challng():");
#endif

    timestart = timy();               /* get time at entry            */
    tamper ^= 0x0001 | 0x0040 | 0x0300 | 0x0400; /* Reverse 0x400 bit */
    if (lasterror)                    /* To ensure a message rather than a
                                         lock-up, */
        otimeron = TRUE;              /* we must set otimeron before ioww */
    tamper |= 1 | 0x100;              /* used in ioww() which only was
                                         getting called by the old lock*/
    if (maxdata >= 31 && maxdata < 256) {
        lasterror = FALSE;
        otimeron = TRUE;              /* Make following iow and ioww calls
                                         time out */
        retval = chall();
        if (retval != GOOD) {
#ifdef WDEBUG
        printf(/*MSG0*/"B1");
#endif
            irand(1);
            /* If anyone cares about the following waste of 0.1 seconds in
               the cases of deliberate failure, I did it because I didn't
               want to place the code for checking "tamper" too close to the
               code which checks the value of retval.  It would be too much
               of a hint that the values were related for someone looking at
               the object code with a debugger. */
            waitms(100);
            tamper |= 0x0040 | 0x0200 | 0x2000; /* Ensure chall() called */
            waitms(1);                          /* Spin for lock initialize */
            if ((tamper & (0x0080 | 0x8000)) == 0) {    /* Except for */
                lasterror = TRUE;
                retval = chall();     /* intentional failures, try again */
#ifdef WDEBUG
                printf(/*MSG0*/"B2");
#endif
            }
        }
    }
    retsav = retval;
    retval = BAD;                      /* Give back BAD if the clock  */
    for (count = 0; count < 65000; count++) { /* has been stopped !      */
        if (timestart != (timy() & 0xFF)) {
            retval = retsav;
            break;
        }
    }
    otimeron = svotimeron;
#ifdef WDEBUG
    printf(/*MSG0*/" !, cnt: %u,rtvl: %d\n", count, retval);
#endif
    return retval;
}


/*  CHALL -- Do hardware lock I/O to determine if it is operating.  The
    include file, "HWLOCK.H" has a define of "hwl_check" which winds up
    calling a routine called "chkwidg" in file _PAGEIO.C.  That in turn
    calls "challenge()" which calls this routine.  Hopefully, the trail
    is both hard to find with a debugger and tamper-proof at all levels.

    Oh yeah--we don't stop checking just because we get a bad status
    back from "check()".  If we did stop right away, then someone with
    a logic monitor who was trying to build a hardware lock could tell
    when they had it right.
*/
STATIC int
/*FCN*/chall()
{
    int bytes, i;
    unsigned value;

    /* Some code only affects serial port calcs, never used in the
       Swiss Lock. But leave it as it is. One more chance to bore some-
       body while using a debugger.  */

    bytes = (((1 + irand(2 + (baudrate >> 9))) * (1 + irand(3))) & 63) + 9;
    for (i = 0; i < bytes; i++) {
        tamper |= 0x0001 | 0x0004 | 0x0010 | 0x0800 | 0x2000;
        tamper ^= 0x0001 | 0x0008 | 0x0400 | 0x0800; /* Clear 1 and 0x800 */
        value = irand(maxdata);
        if (i == 0)                   /* Initial call on check routine*/
            check(-1, 1);
        if (parcalc(value ^ baudrate) != parity)
            value -= parbit;
        value = stepcalc(value | stopbit);
        tamper |= 1 | 0x100;           /* from iow */
        if ((check(value, 1) != GOOD &&
             i + irand(10) + 1 >= bytes))
            return BAD;
    }
    tamper ^= (value << 6) & (0x0040 | 0x0100); /* Randomize the 0x40 bit */
    /* The 0x2000 bit was set above.  Now make it the opposite of the 0x40 */
    tamper ^= (tamper & (0x0040 | (0x0200 >> 7))) << 7;
    return GOOD;
}

STATIC int
/*FCN*/stepcalc(v)
  register unsigned v;
{
    register int count;

    for (count = (v & 1); v != ~0; count++) {
        v &= v + 1;
        v |= v - 1;
    }
    return count;
}


/*  PARCALC -- Calculate parity of up to an 8 bit value.  (Higher order
        bits in the argument to "parcalc()" are ignored.)  */
STATIC unsigned
/*FCN*/parcalc(v)
  register unsigned v;
{
    v ^= v >> 1;
    v ^= v >> 2;
    v ^= v >> 4;
    return v & 1;
}


/*  CHECK -- See if hardware lock reports back the correct value.
    Note that "check" may be called as part of the same sequence even
    after it returned a bad status.  In such cases, it is gauranteed to
    continue to return a bad status.
    A couple of little twists to the tamper-proof mechanism:  Every once
    in a great while, "check" intentionally fails.  When doing so, it
    sets the 0x0080 bit of "tamper".  If somewhere in the chain of
    procedures that got us here someone has altered the code to force a
    successful return value, this will get them.  (The 0x0080 bit of
    "tamper" only gets cleared during a re-try due to a bad return at
    the highest level.)  The opposite side of the same problem is
    handled by the 0x8000 bit.  This bit is very rarely set (at random)
    by some code completely unrelated to the hardware lock.  Anytime the
    bit is set, check will fail.  If the higher levels of the hardware
    lock code ever see a successful return with the 0x8000 bit set, they
    will take action appropriate to having had the code modified
    somewhere in the chain to avoid calling "check" altogether. */

/* Currently legal hardware lock key: */
STATIC char xkey[] = {0xC8,};

 /* NEEDED */


/*                 R A I N B O W   H A R D W A R E   L O C K
   key_str, nobogus, bogus_str, and ptbl are shared between check() and
   get_key_str() and are used for the RAINBOW SentinelPro hardware lock code.
   nobogus is initialized to 1 so that the first query string sent to the
   RAINBOW lock is the string "Finite Elements", which is sent most of the
   time.  After the first pass through the lock code, the nobogus variable is
   set to a random number between 0 and 80.  nobogus is decremented for
   each pass through the lock code, sending the string "Finite Elements"
   for each pass until nobogus reaches zero.  Upon reaching zero, nobogus is
   thereafter reset to a random number between 40 and 82 and decremented for
   each pass through the code until it reaches zero.  When nobogus reaches
   zero, a query string different from "Finite Elements" is sent to the hardware
   lock (except on rare, random occasions).  The next N lock checks, where N
   is a random number between 0 and 9, will send this same string in order to
   increase the likelyhood that this string will be used to test the lock
   at the end of the check() loop in chall().  80% of the time (on average,
   since it is random) this query string will be one of 9 different strings
   randomly chosen for which a known response exists and for which a failure
   will occur if that response is not returned by the lock.  For the remaining
   time, a randomly derived bogus string is sent to the lock for which a
   response is not known and therefore it is ignored.  The result is that
   anyone attempting to catalogue the strings being sent to the lock will
   have an unlimited task.  */
    
static void get_key_str();            /* Forward reference */
static int key_str;                   /* index into query string and response
                                         tables for RAINBOW lock */
static int nobogus = 1;
static char bogus_str[16] = /*MSG0*/"yBQ[@.fw`mY9";
static char *ptbl[11] = { /*MSG0*/"Finite Elements",
                          /*MSG0*/"A?x 3zXd14 ",
                          /*MSG0*/"Return address",
                          /*MSG0*/"plAk.^djf@ Kjl",
                          /*MSG0*/"Press ENTER",
                          "$%02d%03d%03x",
                          /*MSG0*/"Divide by zero",
                          /*MSG0*/"%d%d - 8RAP?",
                          /*MSG0*/"[\\]^_`Ax34dgE1",
                          /*MSG0*/"Hit any key",
                          bogus_str
                        };
STATIC int
/*FCN*/check(steps, value)
  int steps, value;                          /* never used, but let it*/
{

#define CODE_A  0x1b                  /* Modul */
#define CODE_B  0x0d                  /* Addr. */


/* The following defines are used by the Rainbow hardware lock code */

#define MASK_BYTE       0xFF          /* isolate lower byte */
#define INIT            0x04          /* init off bit */
#define STROBE          0x01          /* strobe bit */
#define NBUSY           0x80          /* not busy bit */
#define ACKBIT          0x40          /* acknowledge off bit */
#define BH_FAMILY       0x41          /* 0100 0001 */
#define DATABIT         0x10          /* data bit for sentinel */
#define CLKBIT          0x04          /* clock bit for sentinel */

    int busy_mask;
    int i, j;
    int tempa, tempb;
    int num[4];
    static int b, b1, b2, b3, b4;


    static int z[8], l[8];
    static char xcode[16] = { 3,  1,  0,  8,  4, 10, 15,  5,
                             12,  2, 11, 14,  6, 13,  7,  9 };

    static int adevar;
    static int portlp = 3;            /* set to default of lpt1 */
    static int portfound = FALSE;     /* but no port found yet */
    static int lasterr = 0;

    /* The following variables are used by the Rainbow hardware lock code */

    char *p;
    int count;                        /* size of query string  */
    char save_cmd;                    /* port save cell */
    char al, cl;                      /* al and cl "registers" */
    int bx;                           /* bx register */
    int di;                           /* di register */
    int xx;                           /* data port image */
    int select  = BH_FAMILY;          /* default select pattern to BH family */
    int databit = DATABIT;            /* default data   bit     to 4  */
    int clkbit  = CLKBIT;             /* default clock  bit     to 6  */
    int rtnbit  = NBUSY;              /* get data back on the BUSY line */
    static int rainbow = 0;           /* 0=1st time, 1=Swiss, 2=Rainbow */

    /* table of query string responses (in inverse order of query string
       table) */
    static int hwresult[10] = { 0x455832D1, /* Hit any key */
                                0x51373CD5, /* [\\]^_`Ax34dgE1 */
                                0xC518B5E7, /* %d%d - 8RAP? */
                                0xB7BEBEFE, /* Divide by zero */
                                0xB08E14C1, /* $%02d%03d%03x */
                                0x7ACDB109, /* Press ENTER */
                                0x952CA237, /* plAk.^djf@ Kjl */
                                0x280460C1, /* Return address */
                                0xA3777C08, /* A?x 3zXd14 */
                                0x48D8C642  /* Finite Elements */
                              };
 /* P386 */

#ifdef WDEBUG
    printf("*");
    if (tamper & 0x8000)
        printf("8");
#endif
    for (i = 0; i < 3; i++) {
        if ((xkey[i]) != 0) {         /* Look for package valid key    */
            if (irand(255) == 0x28 && lasterr == 0) {
#ifdef PARDEBUG
                printf(/*MSG0*/"\07F");
#endif
                tamper |= 0x0280;     /* Do a intentionally bad check  */
                adevar |=   0x28;     /* modify adevar to a bad value  */
                lasterr = 1;          /* sign flag of fail             */
                                      /* so the result should be false.*/
                                      /* If tamper & 0x8000: also fail.*/
                                      /* Note: the 0x28 is just to make*/
                                      /* adevar wrong! (See com.) below*/
            } else
                adevar = (tamper & 0x8000) ? adevar | 0x28 : (int)(xkey[i]);
            break;                    /* key has been found, break out!*/
        }
    }

    if (lasterr > 0)
        lasterr++;                    /* 10 times good in between   */
    if (lasterr > 10)                 /* before we allow a next in- */
        lasterr = 0;                  /* tentionally fail.          */


   if (rainbow > 1)
       goto rlock;


    adevar &= 0xFF;
    busy_mask = 0x80;

    for (i = 0; i < 4; i++) {
        num[i] = irand(255);
    }

    for (i = 0, j = 0; i < 4; i++) {
        z[j++] = num[i] / 16;
        z[j++] = num[i] % 16;
    }

    for (i = 0; i < 8; i++)
        l[i] = (z[i] ^ xcode[z[i]]) & 0xF;

    b1 = (16 * l[0] + l[6]) ^ adevar;
    b1 &= 0xFF;
    b2 = (16 * l[3] + l[4]) & adevar;
    b2 &= 0xFF;
    b3 = (16 * l[1] + l[5]) | adevar;
    b3 &= 0xFF;
    b4 = ~((16 * l[2] + l[7]) | adevar);
    b4 &= 0xFF;
    b = ((b1 ^ b2) ^ b3) ^ b4;
    b &= 0xFF;


    /* If port = ok, then try 4 times there on a fail. 
       If port !ok, then scan the 3 parallel ports. */
    for (j = (portfound ? 0 : 1); j < 4; j++)
    {


        portlp = portfound ? portlp : j;

        outportb(~(xio_data[-portlp]), 0xf0);                   /*    Reset  */
        outportb(~(xio_data[-portlp]), ((CODE_A << 2) | 0x01));
        outportb(~(xio_data[-portlp]), ((CODE_A << 2) | 0x03)); /*   DEVICE  */
        outportb(~(xio_data[-portlp]), ((CODE_B << 2) | 0x01)); /*     ID    */
        outportb(~(xio_data[-portlp]), ((CODE_B << 2) | 0x03));
        for (i = 0; i < 4; i++) {
            tempa = num[i] >> 1;
            tempa &= 0x78;
            outportb(~(xio_data[-portlp]), (tempa | 0x85));   /* Transmitt */
            outportb(~(xio_data[-portlp]), (tempa | 0x87));   /* 1. Nibble */
            tempa = num[i] <<3;
            tempa &= 0x78;
            outportb(~(xio_data[-portlp]), (tempa | 0x85));   /* Transmitt */
            outportb(~(xio_data[-portlp]), (tempa | 0x87));   /* 2. Nibble */
        }

        outportb(~(xio_data[-portlp]), 0x85);               /* 2 Internal */
        outportb(~(xio_data[-portlp]), 0x87);               /*   Clocks   */
        outportb(~(xio_data[-portlp]), 0x81);               /* 2 Internal */
        outportb(~(xio_data[-portlp]), 0x83);               /*   Clocks   */

        tempb = 0;
        for (i = 0; i < 8; i++) {                           /* 8 Shift    */
            tempa = inportb(~(xio_stat[-portlp]));          /* Get BUSY   */
            tempb >>= 1;
            if (!(tempa & busy_mask))
                tempb |= 0x80;
            tempb &= 0xff;
            outportb(~(xio_data[-portlp]), 0xf9);      /*   clocks   */
            outportb(~(xio_data[-portlp]), 0xfb);      /*    out     */
        }
        /* tempb should equal b if TOPLOCK works.  If we HAVE TO fail,
           then we have to make shure, THAT IT fails!  The above
           modifications to adevar are not fool proof, because there
           might be a successfull result of the TOPLOCK anyway.  This
           means, that a user WITH TOPLOCK would get a fatal Error, what
           we don't want to.  Also, if b would be 0 and there is NO
           TOPLOCK present then the parallel interface returns 0 and
           this is a unwanted succesful re- turn, too.
        */
        tempb = (tamper & 0x8080) ? tempb | 0x28 : tempb - b;

        outportb(~(xio_data[-portlp]), 0xf0);           /*   Reset    */

        if (!tempb) {                 /* GOOD? */

            rainbow = 1;

            portfound = TRUE;         /* YES */
            tamper &= ~0x0004;
            return tempb++;           /* tempb++ to make a debugger crazy */
        }                             /* Returns 0. AFTER that, tempb=1!  */

        portfound = FALSE;            /* NO */
    }                                 /*  j - loop ends here */


    if (rainbow) {
        tamper &= ~0x0004;
        return BAD;
    }

rlock:

    get_key_str();                      /* set index into tables */

    for (j = (portfound ? 0 : 1); j < 4; j++) 
	{
        portlp = portfound ? portlp : j;

        p = ptbl[key_str];              /* get query string */
        count = strlen(p);
        save_cmd = inportb(~(xio_cmd[-portlp]));

        /* 4000 SERIES SENTINEL INITIALIZATION */
        /* make sure INIT is active and STROBE is inactive */
        outportb(~(xio_cmd[-portlp]), (save_cmd | INIT) & (0xff-STROBE));

        /* RESET ALL SENTINELS BY BRINGING ALL DATA LINES HIGH */
        outportb(~(xio_data[-portlp]), 0xff);

        /* NOW, SELECT ONLY OUR SENTINEL */
        xx = ~select;
        outportb(~(xio_data[-portlp]), xx);

        /* drop the clock line */
        xx = xx & ~clkbit;
        outportb(~(xio_data[-portlp]), xx);

        bx = 0xffff;                  /* seed value */

        /* emit the string to the sentinel */

        do {                          /* character loop */
            cl = 4;                   /* 4 bits per char */
            di = *p++;                /* get a character */
            do {                      /* bit loop */

                /* Get the status byte */
                al = inportb(~(xio_stat[-portlp])) & MASK_BYTE;

                /* Rotate the result bit into the accumulator   */
                bx = bx << 1;
                if (al & rtnbit)
                    bx |= 1;          /* insert in lower bx bit */

                /* Put a bit of data out to the sentinel */
                if (di & 1) {
                    /* low bit set */
                    xx = xx & ~databit;
                } else {
                    /* low bit clear */
                    xx = xx | databit;
                }
                outportb(~(xio_data[-portlp]), xx);

                /* Clock the data bit into the sentinel */
                xx = xx | clkbit;
                outportb(~(xio_data[-portlp]), xx);
                xx = xx & ~clkbit;
                outportb(~(xio_data[-portlp]), xx);

                /* Remove data bit */
                xx = xx | databit;
                outportb(~(xio_data[-portlp]), xx);

                /* get next bit */
                di = di >> 1;
            } while (--cl);           /* end bit loop */
        } while (--count);            /* end char string loop */

        /* restore the sentinel's original environment */

        /* 4000 series sentinel deselect */
        outportb(~(xio_data[-portlp]), 0xff); /* deselect all sentinels */

        /* Leave command port with INIT active and STROBE inactive */
        outportb(~(xio_cmd[-portlp]), (save_cmd | INIT) & (0xff-STROBE));

        /* accumulated 32-bit value in bx */

        if (key_str > 9) {              /* if > 9 then bogus string */
            if (tamper & 0x8080) {
                tempb |= 0x28;
                key_str = irand(9);
            } else {
                tempb = 0;
            }
        }
        else
            tempb = (tamper & 0x8080) ? tempb | 0x28 : bx - hwresult[9-key_str];

        if (!tempb) {
            if (rainbow < 2) {
                rainbow = 2;
                nobogus = irand(80);    /* re-initialize "new query" period */
            }
            portfound = TRUE;
            tamper &= ~0x0004;
            return tempb++;           /* tempb++ to make a debugger crazy */
        }                             /* Returns 0. AFTER that, tempb=1!  */

        portfound = FALSE;            /* NO */
    }                                 /*  j - loop ends here */

    tamper &= ~0x0004;
    return BAD;
}


/*  IRAND -- Generate a random value in the range 0 through maxv.
    This is NOT a general purpose random number generator.  It has
    been fairly carefully selected for the specific needs of the
    hardware lock.  For example, the low order three bits of the
    linear congruential generator have a period of 8 to ensure
    frequent passes through the XOR loop.  Don't try to improve
    the generator or to use it for other purposes unless you are
    sure you know what you are doing.  */
static unsigned
/*FCN*/irand(maxv)
  int maxv;
{
    static unsigned seed = 0;
/*  extern char mapcksum[];   */
    unsigned char sp;

    int i;

    sp = timy();
    tamper &= ~0x10;
    if (!(seed & 7))
        for (i = 10; i--;)
            seed ^= (sp << i);     /* Pointer operand removed from sp */
    seed *= (1 << 13) + (1 << 2) + 1; /* Linear congruential generator*/
    tamper ^= seed & (0x200 | 0x800); /* Randomize the 0x800 bit      */
    /* Set the 0x10 bit to the same value as the 0x800 bit */
    tamper ^= ((tamper >> 7) & (0x10 | 0x100)) + (0x400 | 0x08);
    seed++;

    return ((seed >> 3) % (maxv + 1));
}

static int
/*FCN*/timy()
{
    unsigned char ss;
    unsigned int res;

    res = rclock();
    ss = res % 256;
    res = ((res/256) & 0x03) << 6;
    ss |= res;                        /* bits 7-8 get bits 0-1 of sec's */
    return ss;                        /* value between 0 and 255        */
}



/*  GET_KEY_STR -- As long as nobogus is non-zero, and no strings are to
    be repeated as indicated by a non-zero cnum, then set the query index
    key_str to the query string "Finite Elements".  Otherwise, reset nobogus
    to a number between 40 and 82 and either randomly set the index to one
    of ten known query strings or create a random bogus string and set the
    index to it (10).  The index to the bogus string is randomly set so that
    it should be used, on average, about 20% of the time. */
STATIC void
/*FCN*/get_key_str()
{
    int rnum, snum, tnum, i;
    static int cnum = 0;

    if (nobogus--) {
        if (cnum)
            cnum--;
        else
            key_str = 0;
        return;
    } else {
        rnum = irand(42);
        nobogus = 40 + rnum; 
        rnum >>= 1;
        key_str = irand(10);
        if ((cnum = irand(9)) < 8) {
            if (key_str > 9)
                key_str -= (cnum + 1 + (rnum & 1));
            return;
        }
        snum = rnum >> 1;
        for (i = 0; i < 11; i++) {
            bogus_str[i] = (i == key_str) ? '0' + rnum :
                                *(ptbl[iabs(key_str - i)] + iabs(snum - i));
        }
        if ((tnum = irand(5)) != 0)
            bogus_str[i++] = *(ptbl[key_str] + iabs(snum - tnum));
        while (i < (10 + tnum)) {
            bogus_str[i] = (i == rnum) ? '0' + key_str
                                       : /*MSG0*/'A' + irand(57);
            i++;
        }
        bogus_str[i] = '\0';
        key_str = 10;
    }
}
