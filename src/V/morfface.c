
#ifdef MORPH


#include "jimk.h"
#include "morf.h"
#include "poly.h"

extern Poly working_poly;
extern free_poly(Poly *);


typedef struct moo_face
	{
	int pt_count;
	int *pt_ix;
	} Moo_face;

typedef struct moo_point
	{
	Moo_coor xyz[3];
	} Moo_point;

#define M3D_SIZE 256
Moo_point moo_3d[M3D_SIZE];
int m3d_size = M3D_SIZE;
int m3d_next = 0;

clear_all_faces()
{
m3d_next = 0;
}

void *clone_structure(void *s, int size)
{
void *d;

if ((d = begmem(size)) != NULL)
	copy_structure(s, d, size);
return(d);
}


Moo_point *get_moo_3d(ix)
{
return(moo_3d + ix);
}

closest_moo_point(Moo_coor xyz[3], Moo_point *list, int count)
{
int closest_ix = -1;
long closest_dist = 0x7ffffff;
int i, j;
long dist;
long temp;
Moo_point *p;

p = list;
for (i=0; i<count; i++)
	{
	dist = 0;
	for (j=0; j<3; j++)
		{
		temp = p->xyz[j] - xyz[j];
		dist += temp*temp;
		}
	if (dist < closest_dist)
		{
		closest_dist = dist;
		closest_ix = i;
		}
	p++;
	}
return(closest_ix);
}

closest_moo_3d(Moo_coor xyz[3])
{
closest_moo_point(xyz, moo_3d, m3d_next);
}


check_moo_3d_free(int count)
{
if (m3d_next + count < m3d_size)
	return(TRUE);
else
	return(FALSE);
}

next_moo_3d()
{
int ret;

if (m3d_next < m3d_size)
	{
	ret = m3d_next;
	m3d_next += 1;
	return(ret);
	}
else
	return(-1);
}

unget_moo_3d()
{
m3d_next -= 1;
}

static int face_color;

static facedot(x,y)
{
if (x >= 0 && x < XMAX && y >= 0 && y < YMAX)
	{
	render_form->p[y*render_form->bpr+x] = face_color;
	}
}

some_face(Morf_obj *moo, Vector dot)
{
Moo_face *face;
int i;
Moo_point *last, *this;
int *ixs;
face = moo->data;
i = face->pt_count;
ixs = face->pt_ix;
last = get_moo_3d(ixs[i-1]);
while (--i >= 0)
	{
	this = get_moo_3d(*ixs++);
	cline( this->xyz[0], this->xyz[1], last->xyz[0], last->xyz[1], 
		dot);
	last = this;
	}
}

rub_face(Morf_obj *moo)
{
int occ;

occ = vs.ccolor;
vs.ccolor = moo->color;
some_face(moo, sdot);
vs.ccolor = occ;
}

unrub_face(Morf_obj *moo)
{
some_face(moo, copydot);
}

render_face(Morf_obj *moo)
{
int otp;
int odm;
int occ;

odm = vs.draw_mode;
vs.draw_mode = moo->draw_mode;
occ = vs.ccolor;
vs.ccolor = moo->color;
otp = vs.tint_percent;
vs.tint_percent = moo->tint_percent;
render_xy(moo->xmin,moo->ymin,moo->xmax,moo->ymax);
make_render_cashes();
some_face(moo, render_dot);
free_render_cashes();
vs.draw_mode = odm;
vs.ccolor = occ;
vs.tint_percent = otp;
}

draw_face(Morf_obj *moo)
{
render_face(moo);
}


free_face(Morf_obj *moo)
{
Moo_face *face;

if ((face = moo->data) != NULL)
	{
	gentle_freemem(face->pt_ix);
	freemem(face);
	}
}

static face_extents(Morf_obj *moo)
{
Moo_face *f;
Moo_point *p;
int *ixs;
int i;

moo->xmin = render_form->w;
moo->ymin = render_form->h;
moo->xmax = moo->ymax = 0;
f = moo->data;
ixs = f->pt_ix;
i = f->pt_count;
while (--i >= 0)
	{
	p = get_moo_3d(*ixs++);
	if (p->xyz[0] > moo->xmax)
		moo->xmax = p->xyz[0];
	if (p->xyz[1] > moo->ymax)
		moo->ymax = p->xyz[1];
	if (p->xyz[0] < moo->xmin)
		moo->xmin =  p->xyz[0];
	if (p->xyz[1] < moo->ymin)
		moo->ymin =  p->xyz[1];
	}
}

static Moo_face *face_poly(Poly *p)
{
int *ixs;
Moo_face *f;
Moo_point *mp;
int i;
int ix;
LLpoint *lp;

i = p->pt_count;
lp = p->clipped_list;
if (check_moo_3d_free(i))
	{
	if ((f = begmem(sizeof(*f))) != NULL)
		{
		if ((ixs = begmem(i * sizeof(*ixs))) != NULL)
			{
			f->pt_count = i;
			f->pt_ix = ixs;
			while (--i >= 0)
				{
				ix = next_moo_3d();
				*ixs++ = ix;
				mp = get_moo_3d(ix);
				mp->xyz[0] = lp->x;
				mp->xyz[1] = lp->y;
				mp->xyz[2] = lp->z;
				lp = lp->next;
				}
			return(f);
			}
		else
			freemem(f);
		}
	}
return(NULL);
}

morf_new_face()
{
Morf_obj *new;

make_poly();
if ((new = new_moo()) != NULL)
	{
	new->type = MOT_FACE;
	new->color = vs.ccolor;
	new->tint_percent = vs.tint_percent;
	new->draw_mode = vs.draw_mode;
	if ((new->data = face_poly(&working_poly)) == NULL)
		{
		freemem(new);
		}
	else
		{
		face_extents(new);
		if (!add_new_moo(new))
			free_moo(new);
		else
			draw_face(new);
		}
	}
poly_nopoints(&working_poly);
}

get_shift()
{
union regs r;

r.b.ah = 2;
sysint(0x16,&r,&r);
return(r.b.al);
}

make_face(Morf_obj *moo, int pt_max)
{
Moo_face *face;
Moo_point *p;
int cix;
int shfix;
int i;
int shf;
Moo_coor xyz[3];

face = moo->data;
cix = next_moo_3d();
face->pt_count = 1;
face->pt_ix[0] = cix;
if (cix >= 0)
	{
	for (;;)
		{
		rub_face(moo);
		wait_input();
		unrub_face(moo);
		shf = get_shift();
		shf &= 3;
		xyz[0] = grid_x;
		xyz[1] = grid_y;
		xyz[2] = 0;
		if (shf)	/* if shifted snap to closest */
			{
			shfix = closest_moo_point(xyz, moo_3d, m3d_next-1);
			if (shfix >= 0)
				{
				p = get_moo_3d(shfix);
				for (i=0; i<3; i++)
					xyz[i] = p->xyz[i];
				}
			}
		p = get_moo_3d(cix);
		for (i=0; i<3; i++)		/* update current point with cursor pos. */
			p->xyz[i] = xyz[i];
		if (RJSTDN || key_hit)
			{
			unget_moo_3d();
			face->pt_count -= 1;
			break;
			}
		if (PJSTDN)
			{
			if (shf && shfix >= 0)
				{
				face->pt_ix[face->pt_count-1] = shfix;
				}
			else
				{
				if ((cix = next_moo_3d()) < 0)	/* out of mem! */
					break;
				p = get_moo_3d(cix);
				for (i=0; i<3; i++)
					p->xyz[i] = xyz[i];
				}
			face->pt_ix[face->pt_count] = cix;
			if ((face->pt_count+= 1) >= pt_max)
				break;		/* too many points */
			}
		}
	}
}

morf_make_face()
{
Morf_obj *new;
Moo_face *face;
int ixs_buf[M3D_SIZE];

save_undo();
if ((new = new_moo()) != NULL)
	{
	if ((face = begmem(sizeof(*face))) == NULL)
		{
		freemem(new);
		}
	else
		{
		face->pt_ix = ixs_buf;
		face->pt_count = 0;
		new->type = MOT_FACE;
		new->data = face;
		new->color = vs.ccolor;
		new->draw_mode = vs.draw_mode;
		new->tint_percent = vs.tint_percent;
		make_face(new, M3D_SIZE);
		if (face->pt_count == 0)
			{
			freemem(face);
			}
		else
			{
			if ((face->pt_ix = 
				clone_structure(face->pt_ix, face->pt_count * sizeof(int) ))
				== NULL)
				{
				freemem(face);
				}
			else
				{
				face_extents(new);
				if (!add_new_moo(new))
					free_moo(new);
				else
					draw_face(new);
				}
			}
		}
	}
}

extern UBYTE circ3_cursor[];

see_morf_points()
{
Moo_point *p;
int i;

p = moo_3d;
i = m3d_next;
while (--i >= 0)
	{
	a1blit(16, 5, 0, 0, circ3_cursor+12, 2, 
		p->xyz[0]-8, p->xyz[1]-2, render_form->p,
		render_form->bpr, vs.ccolor);
	p++;
	}
}

morf_link_face()
{
Moo_coor xyz[3];
int ix;
Moo_point *p;

for (;;)
	{
	wait_click();
	if (PJSTDN)
		{
		xyz[0] = grid_x;
		xyz[1] = grid_y;
		xyz[2] = 0;
		if ((ix = closest_moo_3d(xyz)) >= 0)
			{
			p = get_moo_3d(ix);
			a1blit(16, 5, 0, 0, circ3_cursor+12, 2, 
				p->xyz[0]-8, p->xyz[1]-2, render_form->p,
				render_form->bpr, vs.ccolor);
			}
		}
	else
		break;
	}
}

#define POLYH_MAGIC 0xFACE

struct polyh_header
	{
	unsigned short type;
	short pt_count;
	short moo_count;
	};

struct face_header
	{
	int pt_count;
	int flags;
	};

save_face(Morf_obj *moo, int file)
{
struct face_header fh;
Moo_face *face;
long size;

face = moo->data;
fh.pt_count = face->pt_count;
fh.flags = 0;
if (jwrite(file, &fh, sizeof(fh)) < sizeof(fh))
	return(0);
size = face->pt_count * sizeof(face->pt_ix[0]);
if (jwrite(file, face->pt_ix, size) < size)
	return(0);
return(1);
}

load_face(Morf_obj *moo, int file)
{
struct face_header fh;
Moo_face *face;
long size;

if ((face = begmem(sizeof(*face))) == NULL)
	return(0);
if (jread(file, &fh, sizeof(fh)) < sizeof(fh))
	{
	freemem(face);
	truncated("Faceload8");
	return(0);
	}
face->pt_count = fh.pt_count;
size = fh.pt_count * sizeof(face->pt_ix[0]) ;
if ((face->pt_ix = lbegmem(size)) == NULL)
	{
	freemem(face);
	return(0);
	}
if (jread(file, face->pt_ix, size) < size)
	{
	freemem(face);
	freemem(face->pt_ix);
	truncated("Faceload10");
	return(0);
	}
moo->data = face;
return(1);
}

s_faces(int file)
{
struct polyh_header ph;
int i;
long size;
Morf_obj *moo;

ph.type = POLYH_MAGIC;
ph.pt_count = m3d_next;
ph.moo_count = count_moos();
if (jwrite(file, &ph, sizeof(ph)) < sizeof(ph))
	{
	truncated("FACE");
	return(0);
	}
size = m3d_next*sizeof(moo_3d[0]);
if (jwrite(file, moo_3d, size) < size)
	{
	truncated("FACE1");
	return(0);
	}
for (i=0; i<ph.moo_count; i++)
	{
	moo = get_moo(i);
	if (!save_moo(moo))
		{
		return(0);
		}
	}
return(1);
}

save_faces(char *name)
{
int jfile;
int ok;

if ((jfile = jcreate(name)) == 0)
	{
	cant_create(name);
	return(0);
	}
ok = s_faces(jfile);
jclose(jfile);
if (!ok)
	{
	/* jdelete(name); */
	}
return(ok);
}

l_faces(int file)
{
struct polyh_header ph;
int i;
long size;
Morf_obj *moo;

clear_all_moos();
if (jread(file, &ph, sizeof(ph)) < sizeof(ph))
	{
	truncated("FACE4");
	return(0);
	}
if (ph.type != POLYH_MAGIC)
	{
	continu_line("Not a good face file");
	return(0);
	}
if (ph.pt_count > m3d_size)
	{
	continu_line("Too many points in face file");
	return(0);
	}
size = ph.pt_count*sizeof(moo_3d[0]);
if (jread(file, moo_3d, size) < size)
	{
	truncated("FACE5");
	return(0);
	}
m3d_next = ph.pt_count;
for (i=0; i<ph.moo_count; i++)
	{
	if ((moo = new_moo()) == NULL)
		return(0);
	if (!load_moo(moo, file))
		{
		truncated("Face6");
		freemem(moo);
		return(0);
		}
	if (!add_new_moo(moo))
		return(0);
	}
return(1);
}


load_faces(char *name)
{
int jfile;
int ok;

if ((jfile = jopen(name, 0)) == 0)
	{
	cant_find(name);
	return(0);
	}
l_faces(jfile);
jclose(jfile);
}



#endif MORPH
