/* pocotur.c - the stuff to implement the poco turtle */

#include "ptrmacro.h"
#include "errcodes.h"
#include "pocoface.h"
#include "pocolib.h"
#include <math.h>

extern Errcode builtin_err;

/** Turtle graphics **/
static double xtur, ytur, atur;
static Boolean pendown;

void po_tur_home(void)
/*****************************************************************************
 * void Home(void)
 ****************************************************************************/
{
xtur = fli_screen_width()>>1;
ytur = fli_screen_height()>>1;
atur = 0;
pendown = 1;
}

static void po_tur_right(double degrees)
/*****************************************************************************
 * void Right(double angle)
 ****************************************************************************/
{
atur += degrees;
}

static void po_tur_left(double degrees)
/*****************************************************************************
 * void Left(double angle)
 ****************************************************************************/
{
atur -= degrees;
}


static void po_tur_set_position(double x, double y, double degrees)
/*****************************************************************************
 * void MoveTo(double x, double y, double angle)
 ****************************************************************************/
{
xtur = x;
ytur = y;
atur = degrees;
}

static void po_tur_get_position(Popot x, Popot y, Popot degrees)
/*****************************************************************************
 * void Where(double *x, double *y, double *angle)
 ****************************************************************************/
{

if (x.pt == NULL || y.pt == NULL || degrees.pt == NULL)
	{
	builtin_err = Err_null_ref;
	return;
	}
vass(x.pt,double) = xtur;
vass(y.pt,double) = ytur;
vass(degrees.pt,double) = atur;
}

#define  PI 3.141592

static void po_tur_forward(double distance)
/*****************************************************************************
 * void Move(double amount)
 ****************************************************************************/
{
int lx,ly;
double radians;

lx = xtur;
ly = ytur;
radians = atur*2.0*PI/360.0;
xtur += cos(radians)*distance;
ytur += sin(radians)*distance;
if (pendown)
	po_ink_line(lx, ly, (int)xtur, (int)ytur);
}

static void po_tur_backward(double distance)
/*****************************************************************************
 * void Back(double amount)
 ****************************************************************************/
{
po_tur_forward(-distance);
}

static void po_tur_pen_up(void)
/*****************************************************************************
 * void PenUp(void)
 ****************************************************************************/
{
pendown = FALSE;
}

static void po_tur_pen_down(void)
/*****************************************************************************
 * void PenDown(void)
 ****************************************************************************/
{
pendown = TRUE;
}

static Boolean po_tur_get_pen(void)
/*****************************************************************************
 * Boolean IsDown(void)
 ****************************************************************************/
{
return(pendown);
}

/*----------------------------------------------------------------------------
 * library protos...
 *
 * Maintenance notes:
 *	To add a function to this library, add the pointer and prototype string
 *	TO THE END of the following list.  Then go to POCOLIB.H and add a
 *	corresponding prototype to the library structure typedef therein which
 *	matches the name of the structure below.  When creating the prototype
 *	in POCOLIB.H, remember that all arguments prototyped below as pointers
 *	must be defined as Popot types in the prototype in pocolib.h; any number
 *	of stars in the Poco proto still equate to a Popot with no stars in the
 *	pocolib.h prototype.
 *
 *	DO NOT ADD NEW PROTOTYPES OTHER THAN AT THE END OF AN EXISTING STRUCTURE!
 *	DO NOT DELETE A PROTOTYPE EVER! (The best you could do would be to point
 *	the function to a no-op service routine).  Breaking these rules will
 *	require the recompilation of every POE module in the world.  These
 *	rules apply only to library functions which are visible to POE modules
 *	(ie, most of them).  If a specific typedef name exists in pocolib.h, the
 *	rules apply.  If the protos are coded as a generic array of Lib_proto
 *	structs without an explicit typedef in pocolib.h, the rules do not apply.
 *--------------------------------------------------------------------------*/

PolibTurtle po_libturtle = {
po_tur_forward,
	"void    Move(double amount);",
po_tur_backward,
	"void    Back(double amount);",
po_tur_left,
	"void    Left(double angle);",
po_tur_right,
	"void    Right(double angle);",
po_tur_pen_up,
	"void    PenUp(void);",
po_tur_pen_down,
	"void    PenDown(void);",
po_tur_get_pen,
	"Boolean IsDown(void);",
po_tur_set_position,
	"void    MoveTo(double x, double y, double angle);",
po_tur_get_position,
	"void    Where(double *x, double *y, double *angle);",
po_tur_home,
	"void    Home(void);",
};

Poco_lib po_turtle_lib =
	{
	NULL,
	"Turtle Graphics",
	(Lib_proto *)&po_libturtle, POLIB_TURTLE_SIZE,
	};

