/* pocotur.c - the stuff to implement the poco turtle */

#include "ptrmacro.h"
#include "errcodes.h"
#include "pocodraw.h"
#include "pocoface.h"
#include "pocolib.h"
#include "vpsubs.h"
#include <math.h>

extern Errcode builtin_err;

/** Turtle graphics **/
static double xtur, ytur, atur;
static bool pendown;

/*****************************************************************************
 * void Home(void)
 ****************************************************************************/
void po_tur_home(void)
{
	xtur = fli_screen_width()>>1;
	ytur = fli_screen_height()>>1;
	atur = 0;
	pendown = 1;
}


/*****************************************************************************
 * void Right(double angle)
 ****************************************************************************/
static void po_tur_right(double degrees)
{
	atur += degrees;
}


/*****************************************************************************
 * void Left(double angle)
 ****************************************************************************/
static void po_tur_left(double degrees)
{
	atur -= degrees;
}


/*****************************************************************************
 * void MoveTo(double x, double y, double angle)
 ****************************************************************************/
static void po_tur_set_position(double x, double y, double degrees)
{
	xtur = x;
	ytur = y;
	atur = degrees;
}

/*****************************************************************************
 * void Where(double *x, double *y, double *angle)
 ****************************************************************************/
static void po_tur_get_position(Popot x, Popot y, Popot degrees)
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

/*****************************************************************************
 * void Move(double amount)
 ****************************************************************************/
static void po_tur_forward(double distance)
{
	int lx, ly;
	double radians;

	lx = xtur;
	ly = ytur;
	radians = atur*2.0*PI/360.0;
	xtur += cos(radians)*distance;
	ytur += sin(radians)*distance;
	if (pendown) {
		po_ink_line(lx, ly, (int)xtur, (int)ytur);
	}
}


/*****************************************************************************
 * void Back(double amount)
 ****************************************************************************/
static void po_tur_backward(double distance)
{
	po_tur_forward(-distance);
}


/*****************************************************************************
 * void PenUp(void)
 ****************************************************************************/
static void po_tur_pen_up(void)
{
	pendown = false;
}


/*****************************************************************************
 * void PenDown(void)
 ****************************************************************************/
static void po_tur_pen_down(void)
{
	pendown = true;
}


/*****************************************************************************
 * Boolean IsDown(void)
 ****************************************************************************/
static bool po_tur_get_pen(void)
{
	return pendown;
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

