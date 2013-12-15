
/* geometry.c - Some linear algebra to help do rotations.  */

#include "jimk.h"

void act_rot_offset();

void find_conjugates(struct ado_setting *function)

/*find_conjugates(void)
  the guy passed to this is the op or function portion of the act with
  the axis  - function6-8 - already filled in. It fills in function3&4,
  with the time parameter theta = function5 set by whoever...
  */
{
Short_xyz axis;
SHORT rotate_op[9];
SHORT theta;

zero_structure(rotate_op, 9*sizeof(SHORT) );
pj_copy_structure( &function->spin_axis, &axis, 3*sizeof(SHORT) );

theta = arctan( 0, SCALE_ONE) - arctan( axis.x, axis.y);
while (theta > TWOPI/2 ) theta -= TWOPI;
while (theta < -TWOPI/2) theta += TWOPI;

rotate_op[5] = function->itheta1 = theta;
act_rot_offset(&axis, rotate_op, SCALE_ONE);

theta = arctan( 0, SCALE_ONE) - arctan( axis.y, axis.z);
while (theta > TWOPI/2 ) theta -= TWOPI;
while (theta < -TWOPI/2) theta += TWOPI;

function->itheta2 = theta;
}


static void act_rot_offset(register Short_xyz *point,
			        register SHORT *op,
					SHORT scale)
{
register SHORT theta;
register SHORT s,c;
SHORT x,y,z;

theta =  op[3];
if (theta)
	{
	s = isin(theta);
	c = icos(theta);


	x = point->x;
	y = point->y;

	point->x = itmult(x,c) + itmult(y,s);
	point->y = itmult(y,c) + itmult(x,-s);
	}
theta =  op[4];
if (theta)
	{
	s = isin(theta);
	c = icos(theta);

	y = point->y;
	z = point->z;

	point->y = itmult(y,c) + itmult(z,s);
	point->z = itmult(z,c) + itmult(y,-s);
	}
theta =  op[5];
if (theta)
	{
	theta = itmult(theta, scale);
	s = isin(theta);
	c = icos(theta);

	x = point->x;
	y = point->y;

	point->x = itmult(x,c) + itmult(y,s);
	point->y = itmult(y,c) + itmult(x,-s);
	}
theta =  -op[4];
if (theta)
	{
	s = isin(theta);
	c = icos(theta);

	y = point->y;
	z = point->z;

	point->y = itmult(y,c) + itmult(z,s);
	point->z = itmult(z,c) + itmult(y,-s);
	}
theta =  -op[3];
if (theta)
	{
	s = isin(theta);
	c = icos(theta);


	x = point->x;
	y = point->y;

	point->x = itmult(x,c) + itmult(y,s);
	point->y = itmult(y,c) + itmult(x,-s);
	}
}



