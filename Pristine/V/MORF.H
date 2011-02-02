
typedef short Moo_coor;
#define BIG_MOO_COOR 0x7fff

typedef struct morf_obj
	{
	UBYTE type;
	UBYTE visible;
	short color;
	short draw_mode;
	short tint_percent;
	short xmin,ymin,xmax,ymax;
	void *data;
	} Morf_obj;
#define MOT_FACE 0

typedef struct morf_vfuns
	{
	int (*draw_moo)();
	int (*free_moo)();
	int (*save_moo)();
	int (*load_moo)();
	} Morf_vfuns;
#define MOL_SIZE 16

extern draw_moos();
extern Morf_obj *new_moo();
extern Morf_obj *get_moo(int ix);
