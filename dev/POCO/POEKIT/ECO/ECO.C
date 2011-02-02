//Eco.poc - adaptation of em.pog into Poco.
//
//Simulate a little ecosystem.  Got a constant source of lo-level food
//the fountains and green dots), creatures that eat green dots, and
//creatures (harry) that eat the creatures that eat green dots.
//
//Goal - to make the whole thing keep going.
//
//Keyboard commands -
//		f	-  make a fountain - source of green dot algae
//		p	-  make a mutating purplish algae eater
//		h	-  make 'harry' the predator.
//		k	-  kill all creatures
//		q   - exit program
//		

#include "errcodes.h"
#include "ptrmacro.h"
#include "pocorex.h"
#include "pocolib.h"
#include "syslib.h"
#include "gfx.h"
#include "rastlib.h"
#include "math.h"
#include "stdio.h"


/* Macro to help make strings for poco functions */
#define poco_buf(x) ptr2ppt((x),sizeof(*(x)))

#define HLIB_TYPE_1 AA_POCOLIB
#define HLIB_TYPE_2 AA_SYSLIB
#define HLIB_TYPE_3 AA_GFXLIB
#define HLIB_TYPE_4 AA_MATHLIB
#define HLIB_TYPE_5 AA_STDIOLIB
#include <hliblist.h>

/********* Some math stuff *********/
#include <limits.h>

#define PI		3.14159265358979323846
#define Random(x) poernd(x)
#define XYangle(x,y) arctan2(y,x)
#define SquareRoot(x) sqrt(x)
#define DegreesToRadians(a) ((a)*2*PI/360)
#define RadiansToDegrees(a)	((a)*360/(2*PI))


double distance(int x1, int y1, int x2, int y2)
/* Distance between two points. */
{
int dx = x1-x2;
int dy = y1-y2;
return(sqrt((double)dx*dx+(double)dy*dy));
}

#define ArcX(angle,radius) (cos(angle)*(radius))
#define ArcY(angle,radius) (sin(angle)*(radius))


#define CF_DEAD 1
#define CF_DOT 2
#define CF_EATER 4
#define CF_HARRY 8

/*** Creature oriented stuff - stuff to manage a list of objects
 *** which each share the CC_ALL fields (x,y position, speed, age)
 *** and have an evolution routine that gets called once a frame.
 ***/
/* This is the common parts of all creatures. */
#define CC_ALL \
	struct creature *next; \
	struct creature **prev; \
	int x,y; \
	int dx,dy; \
	int age; \
	int flags; \
	int energy; \
	int size; \
	struct creature_class *class;

typedef struct creature
/* Generic creature */
	{
	CC_ALL
	} Creature;

typedef struct creature_class
/* Stuff to organize an entire class of creatures.  This and some
 * structure beginning with CC_ALL is pretty much what you need
 * to define a new class */
	{
	int data_size;					 /* sizeof(Creature) */
	void (*init)(void *me);			 /* creature specific init. stuff */
	void (*evolve)(void *me);		 /* creature tick stuff. */
	void (*see)(void *me);			 /* creature see stuff */
	} Creature_class;

Creature *living_list = NULL;		/* List of all living creatures */

void *spawn(Creature_class *cc, int x, int y, int dx, int dy)
/* Create a new creature.  Allocate data area. Initialize common values, 
 * call creature specific init routine, and hang it on the living_list. */
{
Creature *c;
Creature **p;
Creature *tmp;

if ((c = malloc(cc->data_size)) != NULL)
	{
	memset(c, 0, cc->data_size);
	c->x = x;
	c->y = y;
	c->dx = dx;
	c->dy = dy;
	c->age = 0;
	c->class = cc;
	if (cc->init != NULL)
		cc->init(c);
	p = &living_list;
	tmp = *p;
	*p = c;
	c->prev = p;
	c->next = tmp;
	if (tmp != NULL)
		tmp->prev = &c->next;
	}
return(c);
}

void kill(void *v)
/* Kill a creature.   Remove it from living list and free up data area. */
{
Creature *c = v;

if (c != NULL)
	{
	c->flags |= CF_DEAD;
	}
}

void eat(void *my, void *food)
/* Transfer food energy to my, and kill food. */
{
Creature *myc = my;
Creature *foodc = food;

myc->energy += (foodc->energy)/2;
kill(foodc);
}

void kill_all()
/* Kill all creatures. */
{
Creature *ll, *next;

for (ll = living_list;ll != NULL;ll = next)
	{
	next = ll->next;
	if ((*(ll->prev) = ll->next) != NULL)
		ll->next->prev = ll->prev;
	free(ll);
	}
}

void evolve()
/* Call evolve routines of all creatures and age them.*/
{
Creature *ll, *next;
int metabolism;
int dx,dy;

/* Go evolve everyone still alive. */
for (ll = living_list; ll != NULL; ll = ll->next)
	{
	if (!(ll->flags&CF_DEAD))
		{
		dx = ll->dx;
		dy = ll->dy;
		metabolism = ll->size+Absval(dx)+Absval(dy);
		/* extract energy proportional to speed and size for metabolism */
		if ((ll->energy -= metabolism) < 0)
		/* If out of energy then die. */
			ll->flags |= CF_DEAD;
		else
			{
			ll->class->evolve(ll);
			ll->x += dx;
			ll->y += dy;
			ll->class->see(ll);
			++ll->age;
			}
		}
	}
/* Remove all the dead ones from the list */
for (ll = living_list;ll != NULL;ll = next)
	{
	next = ll->next;
	if (ll->flags&CF_DEAD)
		{
		if ((*(ll->prev) = ll->next) != NULL)
			ll->next->prev = ll->prev;
		free(ll);
		}
	}
}

#define CreatureX(c) ((c)->x)
#define CreatureY(c) ((c)->y)
#define Cwrite(class,var,creature,val)  (creature->var = (val))
#define Cread(class,var,creature)	(creature->var)

void *ClosestT(int flags, int x, int y)
/* Find the closest creature in the given class */
{
Creature *closest = NULL;
long closestd = LONG_MAX;	/* arbitrary big distance */
long dist;
long dx,dy;

Creature *ll;

for (ll = living_list; ll != NULL; ll = ll->next)
	{
	if (ll->flags == flags)
		{
		dx = x - ll->x;
		dy = y - ll->y;
		dist = dx*dx + dy*dy;
		if (dist < closestd)
			{
			closest = ll;
			closestd = dist;
			}
		}
	}
return(closest);
}

void *ClosestCreature(void *me, int x, int y)
/* Find the closest creature to me. */
{
Creature *closest = NULL;
long closestd = LONG_MAX;	/* arbitrary big distance */
long dist;
long dx,dy;

Creature *ll;

for (ll = living_list; ll != NULL; ll = ll->next)
	{
	if (!(ll->flags&CF_DEAD) && ll != me)
		{
		dx = x - ll->x;
		dy = y - ll->y;
		dist = dx*dx + dy*dy;
		if (dist < closestd)
			{
			closest = ll;
			closestd = dist;
			}
		}
	}
return(closest);
}

/********* Eco stuff *************/
#define BLACK  0		
#define GREEN  1
#define YELLOW 2
#define DARK_GREEN 3
#define RED  4
#define BLUE  5
#define HCOLOR  6	//color of the Harry preditor
#define WHITE  7
#define PURPLE  8	//lowest 'purpler' color
#define IAFRAID  50	//initial radius where purplers get scared of harrys
#define PUPCOUNT  7	// of 'purpler' colors
#define PSPOKES  6	// of spokes in a spoked purpler


#define dspeed (-3)
#define hspeed 4
#define bspeed 2

int time,total;
/********* Some relatively fast graphics *****************/
Popot cpscreen;
LibRast *cscreen;
LibRast *bscreen;


/* There's a Poco bug or this would be a macro... */
void ClearScreen() 
{
cscreen->lib->set_rast(cscreen,0);
}

#define putdot(color,x,y) CPUT_DOT(cscreen,color,x,y)

void cline(LibRast *s,SHORT x1, SHORT y1, SHORT x2, SHORT y2, int color)
/* Draw a single pixel wide line in color on Rcel *s */
{
register SHORT   duty_cycle;
SHORT incy;
register SHORT delta_x, delta_y;
register SHORT dots;
void (*dot)(LibRast *r, Pixel color, Coor x, Coor y) = s->lib->cput_dot;


	delta_y = y2-y1;
	delta_x = x2-x1;
	if (delta_y < 0) 
	{
		delta_y = -delta_y;
		incy = -1;
	}
	else
	{
		incy = 1;
	}
	if ((delta_x) < 0) 
	{
		delta_x = -delta_x;
		incy = -incy;
		x1 = x2;
		y1 = y2;
	}
	duty_cycle = (delta_x - delta_y)/2;

	if (delta_x >= delta_y)
	{
		dots = ++delta_x;
		while (--dots >= 0)
		{
			(*dot)(s,color,x1,y1);
			duty_cycle -= delta_y;
			++x1;
			if (duty_cycle < 0)
			{
				duty_cycle += delta_x;	  /* update duty cycle */
				y1+=incy;
			}
		}
	}
	else /* dy > dx */
	{
		dots = ++delta_y;
		while (--dots >= 0)
		{
			(*dot)(s,color,x1,y1);
			duty_cycle += delta_x;
			y1+=incy;
			if (duty_cycle > 0)
			{
				duty_cycle -= delta_y;	  /* update duty cycle */
				++x1;
			}
		}
	}
}

void ccircle(LibRast *s, int xcen, int ycen, int rad, int color)
{
int err;
int derr, yerr, xerr;
int aderr, ayerr, axerr;
register int x,y;
int lasty;
void (*dot)(LibRast *r, Pixel color, Coor x, Coor y) = s->lib->cput_dot;

if (rad <= 0)
	{
	(*dot)(s,color,xcen,ycen);
	return;
	}
err = 0;
x = rad;
lasty = y = 0;
for (;;)
	{
	/* draw 4 quadrandts of a circle */
	(*dot)(s,color,xcen+x,ycen+y);
	(*dot)(s,color,xcen+x,ycen-y);
	(*dot)(s,color,xcen-x,ycen+y);
	(*dot)(s,color,xcen-x,ycen-y);
	axerr = xerr = err -x-x+1;
	ayerr = yerr = err +y+y+1;
	aderr = derr = yerr+xerr-err;
	if (aderr < 0)
		aderr = -aderr;
	if (ayerr < 0)
		ayerr = -ayerr;
	if (axerr < 0)
		axerr = -axerr;
	if (aderr <= ayerr && aderr <= axerr)
		{
		err = derr;
		x -= 1;
		y += 1;
		}
	else if (ayerr <= axerr)
		{
		err = yerr;
		y += 1;
		}
	else
		{
		err = xerr;
		x -= 1;
		}
	if (x < 0)
		break;
	}
}
void cdisk(LibRast *s, int xcen, int ycen, int rad, int color)
{
int err;
int derr, yerr, xerr;
int aderr, ayerr, axerr;
register int x,y;
int lasty;

if (rad <= 0)
	{
	CPUT_DOT(s,color,xcen,ycen);
	return;
	}
err = 0;
x = rad;
lasty = y = 0;
for (;;)
	{
	if (y == 0)
		pj_set_hline(s,color,xcen-x,ycen,x<<1);
	else
		{
		if (lasty != y)
			{
			pj_set_hline(s,color,xcen-x,ycen-y,x<<1);
			pj_set_hline(s,color,xcen-x,ycen+y,x<<1);
			lasty = y;
			}
		}
	axerr = xerr = err -x-x+1;
	ayerr = yerr = err +y+y+1;
	aderr = derr = yerr+xerr-err;
	if (aderr < 0)
		aderr = -aderr;
	if (ayerr < 0)
		ayerr = -ayerr;
	if (axerr < 0)
		axerr = -axerr;
	if (aderr <= ayerr && aderr <= axerr)
		{
		err = derr;
		x -= 1;
		y += 1;
		}
	else if (ayerr <= axerr)
		{
		err = yerr;
		y += 1;
		}
	else
		{
		err = xerr;
		x -= 1;
		}
	if (x < 0)
		break;
	}
}
/******* Display buffering functions *********/
void swap() 
{
pj_blitrect(cscreen, 0, 0, bscreen, 0, 0, bscreen->width, bscreen->height);
}

void preswap(){}
void deswap(){}

/********** Eco simulation stuff ***********/

int mouse_x,mouse_y,mouse_left,mouse_right,key;

int pcount;
int bcount;
int hcount;
int lhcount;
int rcount;
int gcount;
int ave_pspeed;

//Creature attatched to the mouse.
typedef struct
	{
	CC_ALL
	} mouser;
static void evolve_mouser(mouser *my);
static void see_mouser(mouser *my);
Creature_class class_mouser = 
	{sizeof(mouser), NULL, evolve_mouser, see_mouser};
Creature *mouse;


//Food for red,blue, and purple creatures.
typedef struct
	{
	CC_ALL
	} greendot;
static void init_greendot(greendot *my);
static void evolve_greendot(greendot *my);
static void see_greendot(greendot *my);
Creature_class class_greendot = 
	{sizeof(greendot), init_greendot,evolve_greendot,see_greendot};

//Fountain swirls around releasing green-dot food
typedef struct
	{
	CC_ALL
	double angle;
	double angular_velocity;
	int radius;
	} fountain;
static void init_fountain(fountain *my);
static void evolve_fountain(fountain *my);
static void see_fountain(fountain *my);
Creature_class class_fountain = 
	{sizeof(fountain), init_fountain, evolve_fountain, see_fountain};


//purpler - the evolving creature.  Shows up as green to purple depending
//on the value of it's afraid variable, which also controls when it
//starts fleeing harrys.  Will be a circle or spoked wheel depending on
//its stop variable, which also controls if it stops after eating.
//The size of its children and the size it reproduces at also mutate.
//
typedef struct
	{
	CC_ALL
	int fed;
	int wait_after_food;
	int stop;
	int ax,ay;
	int afraid;
	int mycolor;
	int repro_energy;
	int child_energy;
	int pspeed;
	int spin;
	int theta;
	} purpler;
static void init_purpler(purpler *my);
static void evolve_purpler(purpler *my);
static void see_purpler(purpler *my);
Creature_class class_purpler = 
	{sizeof(purpler),init_purpler,evolve_purpler,see_purpler};

//Harry - the predator creature.  Orange circle
typedef struct
	{
	CC_ALL
	int fed;
	int wait_after_food;
	int resting;
	int ax,ay;
	int hunt_size;
	int mycolor;
	int repro_energy;
	int child_energy;
	int pspeed;
	int spin;
	int theta;
	} harry;
static void init_harry(harry *my);
static void evolve_harry(harry *my);
static void see_harry(harry *my);
Creature_class class_harry = 
	{sizeof(harry), init_harry, evolve_harry, see_harry};


void rad_draw_spoked_wheel(int color,int x,int y,int rad,
	double theta,int spokes)
//draw_spoked_wheel in radians
{
int i;
double t;

for (i=0; i<spokes; ++i)
	{
	t = theta + i*2.0*PI/spokes;
	cline(cscreen, x, y, x+cos(t)*rad, y+sin(t)*rad, color );
	}
}

void draw_spoked_wheel(int color,int x,int y,int rad,int theta,int spokes)
{
rad_draw_spoked_wheel(color,x,y,rad,DegreesToRadians(theta),spokes);
}


int mutate_color(int color, int off)
/* Change a color in the purple range */
{
color = color+off/2;
if (color < PURPLE)
	color = PURPLE;
if (color >= PURPLE+PUPCOUNT)
	color = PURPLE+PUPCOUNT-1;
return(color);
}

void split_purpler(int x,int y,int dx,int dy,
	int afraid,int mycolor,int repro_energy,
	int pspeed,int child_energy, int spin, int stop, int size,
	int wait_after_food)
/* go breed a new purpler.  Mutate it a little. */
{
purpler *new;
int t, off;

new = spawn(&class_purpler, x, y, -dx, -dy);
if (new)
	{
	off = Random(11)-5;
	new->afraid = afraid + off;
	new->mycolor = mutate_color(mycolor,off);
	t = repro_energy + Random(6*32+1)-3*32;
	if (t < 1)
		t = 1;
	new->size = size + Random(5)-2;
	new->repro_energy = t;
	new->pspeed = pspeed + Random(3)-1;
	new->energy = child_energy;
	child_energy = child_energy + Random(5) - 2;
	if (child_energy < 0)
		child_energy = 0;
	if (child_energy > repro_energy)
		child_energy = repro_energy;
	new->child_energy = child_energy;
	new->spin = spin + Random(5)-2;
	new->wait_after_food = wait_after_food + Random(5)-2;
	t = stop;
	if (Random(8) == 0)	//1 in  8 chance of flip-flopping spokiness
		t = !t;
	new->stop = t;
	}
}

static void init_purpler(purpler *my)  { my->flags = CF_EATER; }

static void evolve_purpler(purpler *my)
/* Evolution routine for purpler creatures.  (They sloppily copy some
 * of their state variables to their descendants. */
{
Creature *food;			// Closest food creature
Creature *monster;		// Closest predator
Creature *nearc;
int dist;
int gx,gy;
int acc = 2;

pcount = pcount+1;
my->theta = my->theta + my->spin;
food = ClosestT(CF_DOT,my->x,my->y);
if (food)
	{
	gx = food->x;
	gy = food->y;
	dist = distance(my->x,my->y,gx,gy);
	if (dist < my->size)
		{
		if (my->stop)
			{
			my->dx = 0;
			my->dy = 0;
			my->fed = my->age;
			}
		eat(my,food);
		}
	}
nearc = food;
monster = ClosestT(CF_HARRY,my->x,my->y);
if (monster)
	{
	int monx,mony;

	monx = monster->x;
	mony = monster->y;
	dist = distance(my->x,my->y,monx,mony);
	if (dist < my->afraid)
		{
		nearc = monster;
		gx = monx;
		gy = mony;
		}
	}
if (nearc != NULL &&
	(!my->stop || nearc != food || my->fed + my->wait_after_food < my->age))
	{
	if (gx > my->x)
		my->ax = acc;
	else
		my->ax = -acc;
	if (gy > my->y)
		my->ay = acc;
	else
		my->ay = -acc;
	if (nearc != food)/* Reverse direction if monster is closer than food */
		{
		my->ax = -my->ax;
		my->ay = -my->ay;
		}
	my->dx = my->dx + my->ax;
	my->dy = my->dy + my->ay;
	if (my->dx > my->pspeed)
		my->dx = my->pspeed;
	if (my->dx < -my->pspeed)
		my->dx = -my->pspeed;
	if (my->dy > my->pspeed)
		my->dy = my->pspeed;
	if (my->dy < -my->pspeed)
		my->dy = -my->pspeed;
	}
if (my->energy > my->repro_energy)	// Enough energy to reproduce?? 
	{
	split_purpler(my->x,my->y,my->dx,my->dy,
		my->afraid,my->mycolor,my->repro_energy,
		my->pspeed,my->child_energy,my->spin, my->stop, my->size,
		my->wait_after_food);
	if (my->stop)
		{
		my->dx = 0;
		my->dy = 0;
		}
	my->energy = my->energy - my->child_energy;
	}
}

static void see_purpler(purpler *my)
{
draw_spoked_wheel(my->mycolor,my->x,my->y,my->size,my->theta,(my->energy>>7)+2);
if (my->stop)
	ccircle(cscreen,my->x,my->y,my->size,my->mycolor);
}

static void init_greendot(greendot *my) 
{
my->flags = CF_DOT;
my->energy = 400;
my->size = 2;
}

static void evolve_greendot(greendot *my)
{
gcount = gcount+1;
}

static void see_greendot(greendot *my)
{
putdot(GREEN, my->x, my->y);
}

static void init_fountain(fountain *my)
{
my->angular_velocity = DegreesToRadians(Random(360));
}

static void evolve_fountain(fountain *my)
{
int rx,ry;

rx = my->x+ArcX(my->angle,my->radius);
ry = my->y+ArcY(my->angle,my->radius);
spawn(&class_greendot,rx,ry,ArcX(my->angle,dspeed), ArcY(my->angle,dspeed));
my->angle = my->angle + my->angular_velocity;
}

static void see_fountain(fountain *my)
{
rad_draw_spoked_wheel(DARK_GREEN,my->x,my->y,3,my->angle,3);
}


static void init_harry(harry *my) 
{
my->flags = CF_HARRY;
}

void split_harry(int x,int y,int dx,int dy,
	int hunt_size,int mycolor,int repro_energy,
	int pspeed,int child_energy, int spin, int size,
	int wait_after_food)
/* go breed a new harry.  Mutate it a little. */
{
harry *new;
int t, off;

new = spawn(&class_harry, x, y, -dx, -dy);
if (new)
	{
	off = Random(11)-5;
	new->hunt_size = hunt_size + off;
	new->mycolor = mutate_color(mycolor,off);
	t = repro_energy + Random(6*32+1)-3*32;
	new->size = size + Random(5)-2;
	new->repro_energy = t;
	new->pspeed = pspeed + Random(3)-1;
	new->energy = child_energy;
	child_energy = child_energy + Random(5) - 2;
	if (child_energy < 0)
		child_energy = 0;
	if (child_energy > repro_energy)
		child_energy = repro_energy;
	new->child_energy = child_energy;
	new->spin = spin + Random(5)-2;
	new->wait_after_food = wait_after_food + Random(5)-2;
	}
}


static void evolve_harry(harry *my)
{
Creature *food;			// Closest food creature
int dist;
int gx,gy;
int acc = 3;

hcount = hcount+1;
ave_pspeed = ave_pspeed + my->pspeed;
my->theta = my->theta + my->spin;
if (my->fed + my->wait_after_food < my->age)
	{
	my->size = my->hunt_size;
	my->resting = FALSE;
	food = ClosestT(CF_EATER,my->x,my->y);
	if (food)
		{
		gx = food->x;
		gy = food->y;
		dist = distance(my->x,my->y,gx,gy);
		if (dist < my->size)
			{
			my->dx = 0;
			my->dy = 0;
			my->fed = my->age;
			eat(my,food);
			}
		else
			{
			if (gx > my->x)
				my->ax = acc;
			else
				my->ax = -acc;
			if (gy > my->y)
				my->ay = acc;
			else
				my->ay = -acc;
			my->dx = my->dx + my->ax;
			my->dy = my->dy + my->ay;
			if (my->dx > my->pspeed)
				my->dx = my->pspeed;
			if (my->dx < -my->pspeed)
				my->dx = -my->pspeed;
			if (my->dy > my->pspeed)
				my->dy = my->pspeed;
			if (my->dy < -my->pspeed)
				my->dy = -my->pspeed;
			}
		}
	}
else	/* resting after lunch */
	{
	my->size = my->hunt_size>>2;
	my->resting = TRUE;
	}
if (my->energy > my->repro_energy)	// Enough energy to reproduce?? 
	{
	split_harry(my->x,my->y,my->dx,my->dy,
		my->hunt_size,my->mycolor,my->repro_energy,
		my->pspeed,my->child_energy,my->spin, my->size,
		my->wait_after_food);
	my->dx = 0;
	my->dy = 0;
	my->energy = my->energy - my->child_energy;
	}
}

static void see_harry(harry *my)
{
if (my->resting)
	{
	ccircle(cscreen,my->x,my->y,my->size,HCOLOR);
	ccircle(cscreen,my->x,my->y,my->size/2,HCOLOR);
	}
else
	draw_spoked_wheel(HCOLOR,my->x,my->y,my->size,my->theta,(my->energy>>7)+2);
}

static void random_purpler()
{
purpler *pp;

pp = spawn(&class_purpler, mouse_x, mouse_y, 0, 0);
if (pp!=NULL)
	{
	Cwrite(purpler,afraid,pp,44);
	Cwrite(purpler,size,pp,10);
	Cwrite(purpler,mycolor,pp,PURPLE+PUPCOUNT/2);
	Cwrite(purpler,repro_energy,pp,50*32);
	Cwrite(purpler,pspeed,pp,2);
	Cwrite(purpler,child_energy,pp,20*32);
	Cwrite(purpler,wait_after_food,pp,10);
	Cwrite(purpler,energy,pp,20*32);
	Cwrite(purpler,spin,pp,5);
	Cwrite(purpler,stop,pp,Random(2) );
	}
}

static void random_harry()
{
harry *pp;

pp = spawn(&class_harry, mouse_x, mouse_y, 0, 0);
if (pp!=NULL)
	{
	Cwrite(harry,hunt_size,pp,20);
	Cwrite(harry,mycolor,pp,PURPLE+PUPCOUNT/2);
	Cwrite(harry,repro_energy,pp,30*32*3);
	Cwrite(harry,pspeed,pp,6);
	Cwrite(harry,child_energy,pp,12*32*3);
	Cwrite(harry,wait_after_food,pp,20);
	Cwrite(harry,energy,pp,12*32*3);
	Cwrite(harry,spin,pp,5);
	}
}



static void evolve_mouser(mouser *my)
{
my->x = mouse_x;
my->y = mouse_y;
}

static void see_mouser(mouser *my)
{
draw_spoked_wheel(WHITE,my->x,my->y,5,0,4);
}



//set up color map for simulation
static void crcolors()
{
int i,r,g,b;

poeSetColorMap(GREEN,0,255,0);
poeSetColorMap(YELLOW,255,255,0);
poeSetColorMap(DARK_GREEN,0,150,0);
poeSetColorMap(RED,255,0,0);
poeSetColorMap(BLUE,0,0,255);
poeSetColorMap(HCOLOR,255, 160, 0);
poeSetColorMap(WHITE,255, 255, 255);

r = 128;
g = 0;
b = 255;
for (i=PURPLE; i<PURPLE+PUPCOUNT/2; ++i)
	{
	poeSetColorMap(i, r, g, b);
	r = r + 255/PUPCOUNT;
	}
for (i=PURPLE+PUPCOUNT/2; i<=PURPLE+PUPCOUNT; ++i)
	{
	poeSetColorMap(i, r, g, b);
	b = b - 255/PUPCOUNT;
	}
}

//Start of the main code 

void eco_main()
{
/* macro to make poco pointer. */
Rasthdr rhead;
Errcode err;
Rcel *ccel = NULL;
char emessage[] = "Evolve #%d G %d P %d H %d R %d B %d ALL %d";
char errmsg1[] = "Couldn't open double-buffer raster.";
char quitmsg[] = "Quit this simulation?";


cpscreen = poeGetPicScreen();
bscreen = GetPicScreen();
copy_rasthdr(bscreen, &rhead);
if ((err = pj_rcel_bytemap_alloc(&rhead, &ccel, COLORS)) < Success)
	{
	poeQerror(0,0,err,poco_buf(errmsg1));
	goto OUT;
	}
cscreen = (LibRast *)ccel;
crcolors();			//set up the color map
					//spawn the initial creatures
if ((mouse = spawn(&class_mouser,0,0,0,0)) == NULL)
	goto OUT;
time = 0;
preswap();			//set up double buffering
poeSetAbort(FALSE);
poeHideCursor();
for (;;)
	{
	poePollInput(poco_buf(&mouse_x),poco_buf(&mouse_y),
		poco_buf(&mouse_left),poco_buf(&mouse_right),
		poco_buf(&key));
	key &= 0xff;
	Random(2);	//just to jiggle random values
	time = time+1;
	// check keyboard and maybe create some new creatures
	if (key == 'h')
		random_harry();
	else if (key == 'p')
		random_purpler();
	else if (key == 'f')
		{
		fountain *f;
		printf("Fountain!");
		if ((f = spawn(&class_fountain, mouse_x, mouse_y,0,0)) != NULL)
			{
			f->radius = Random(100);
			}
		}
	else if (key == 'g')
		{
		fountain *f;
		if ((f = spawn(&class_fountain, mouse_x, mouse_y,0,0)) != NULL)
			{
			f->radius = Random(100)+100;
			}
		}
	else if (key == 'd')
		kill(ClosestCreature(mouse,mouse_x,mouse_y));
	else if (key == 'k')
		{
		kill_all();
		mouse = spawn(&class_mouser,0,0,0,0);
		time = 0;
		}
	else if (key == 'q' || key == 0x1b || key == 'Q')
		{
		if (poeQquestion(0,0,poco_buf(quitmsg)))
			break;
		}
	//set up variables to keep track of how many of what creature around.
	hcount = 0;
	rcount = 0;
	bcount = 0;
	pcount = 0;
	gcount = 0;
	ClearScreen();
	//let everyone live 1 tick
	evolve();
	//and display results in text
	poeprintf(7, 28, poco_buf(emessage),
		time, gcount, pcount, 
		hcount, rcount, bcount, hcount+pcount+rcount+bcount);
	swap();
	}
OUT:
kill_all();
pj_rcel_free(ccel);
}
/*----------------------------------------------------------------------------
 * Setup rexlib/pocorex interface structures...
 *--------------------------------------------------------------------------*/

static Lib_proto calls[] = {
	{eco_main,	   "void    eco_main(void);"},
};

Setup_Pocorex(NULL, NULL, "Eco Simulation v0.2", calls);

