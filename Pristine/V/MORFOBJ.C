
#ifdef MORPH

#include "jimk.h"
#include "morf.h"

static Morf_obj *morf_list[MOL_SIZE];
static int mol_size = MOL_SIZE;

draw_moos()
{
int i;

clear_form(render_form);
for (i=0; i<mol_size; i++)
	if (morf_list[i] != NULL)
		{
		draw_moo(morf_list[i]);
		}
}

extern draw_face(), free_face(), save_face(), load_face();

static Morf_vfuns moo_vtable[] = {
	{
	draw_face,
	free_face,
	save_face,
	load_face,
	},
};

Morf_obj *new_moo()
{
return(begmemc(sizeof(Morf_obj)));
}

Morf_obj *get_moo(int ix)
{
return(morf_list[ix]);
}

count_moos()
{
int i;
int count = 0;

for (i=0; i<mol_size; i++)
	{
	if (morf_list[i] != NULL)
		count++;
	}
return(count);
}

free_moos()
{
int i;

for (i=0; i<mol_size; i++)
	{
	free_moo(morf_list[i]);
	morf_list[i] = NULL;
	}
}

clear_all_moos()
{
free_moos();
}

add_new_moo(Morf_obj *moo)
{
int i;

for (i=0; i<mol_size; i++)
	if (morf_list[i] == NULL)
		{
		morf_list[i] = moo;
		return(TRUE);
		}
return(FALSE);
}


draw_moo(Morf_obj *moo)
{
(*moo_vtable[moo->type].draw_moo)(moo);
}

free_moo(Morf_obj *moo)
{
if (moo != NULL)
	{
	(*moo_vtable[moo->type].free_moo)(moo);
	freemem(moo);
	}
}

struct moo_header
	{
	short type;
	short color, ink, strength, flags;
	};

save_moo(Morf_obj *moo, int file)
{
struct moo_header mh;

mh.type = moo->type;
mh.color = moo->color;
mh.ink = moo->draw_mode;
mh.strength = moo->tint_percent;
mh.flags = 0;
if (jwrite(file, &mh, sizeof(mh)) < sizeof(mh))
	return(0);
return((*moo_vtable[moo->type].save_moo)(moo, file));
}

load_moo(Morf_obj *moo, int file)
{
struct moo_header mh;

if (jread(file, &mh, sizeof(mh)) < sizeof(mh))
	{
	truncated("Face8");
	return(0);
	}
moo->type = mh.type;
moo->color = mh.color;
moo->draw_mode = mh.ink;
moo->tint_percent = mh.strength;
return((*moo_vtable[moo->type].load_moo)(moo, file));
}


#endif MORPH
