/* Pogo.h - stuff to implement Pogo functionality into Poco with
 * a minimum of change to the pogo code.
 */

#ifndef __POCO__
/* If we're test compiling under Turbo C, include some stuff
 * that's predefined for Poco. */
#include <stdio.h>
#include <stdlib.h>
#define TRUE 1
#define FALSE 0
#endif /* __POCO__ */

/******* Math oriented stuff ********/
#include <limits.h>

#define PI		3.14159265358979323846
#define Random(x) rnd(x)
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
	int new; \
	int is_dead; \
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
	} Creature_class;

Creature *living_list = NULL;		/* List of all living creatures */

void *spawn(Creature_class *cc, int x, int y, int dx, int dy)
/* Create a new creature.  Allocate data area. Initialize common values, 
 * call creature specific init routine, and hang it on the living_list. */
{
Creature *c;
Creature **p;
Creature *tmp;

if ((c = calloc(1,cc->data_size)) != NULL)
	{
	c->x = x;
	c->y = y;
	c->dx = dx;
	c->dy = dy;
	c->age = 0;
	c->new = TRUE;
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
	c->is_dead = TRUE;
	}
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

/* Go evolve everyone */
for (ll = living_list; ll != NULL; ll = ll->next)
	{
	if (!ll->is_dead)
		{
		ll->class->evolve(ll);
		++ll->age;
		ll->new = FALSE;
		}
	}
/* Remove all the dead ones from the list */
for (ll = living_list;ll != NULL;ll = next)
	{
	next = ll->next;
	if (ll->is_dead)
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

void *ClosestT(Creature_class *class, int x, int y)
/* Find the closest creature in the given class */
{
Creature *closest = NULL;
long closestd = LONG_MAX;	/* arbitrary big distance */
long dist;
long dx,dy;

Creature *ll;

for (ll = living_list; ll != NULL; ll = ll->next)
	{
	if (!ll->is_dead && ll->class == class)
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
	if (!ll->is_dead && ll != me)
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

/****** Graphics functions **********/

/* There's a Poco bug or this would be a macro... */
void ClearScreen() {Clear();}

void gtext(int color,int x, int y, char *text)
{
SetColor(color);
Text(x,y,text);
}

void gnumber(int color, int x, int y, int digits, int number)
{
char buf[20];

sprintf(buf, "%*d", digits, number);
SetColor(color);
Text(x,y,buf);
}

/******* Display buffering functions *********/
void swap() {}
void preswap(){}
void deswap(){}
