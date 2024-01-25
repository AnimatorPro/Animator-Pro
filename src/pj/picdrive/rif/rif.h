#ifndef RIF_H
#define RIF_H

typedef struct riff_head
{
	char iff_type[4];	/* == RIFF */
	int32_t iff_size;
	SHORT xoff, yoff;
	SHORT width, height;
	SHORT depth;
	SHORT ViewMode;
	SHORT frame_count;
	SHORT jiffies_frame;	/* # of jiffies each frame takes */
	SHORT frames_written;	/* may be less than frame count if file trunc */
	SHORT reserved[19];
} Riff_head;
STATIC_ASSERT(rif, sizeof(Riff_head) == 64);

typedef struct comp_size
{
	SHORT comp;
	USHORT size;
} Comp_size;
STATIC_ASSERT(rif, sizeof(Comp_size) == 4);

#define VCOMP_NONE	0
#define VCOMP_VRUN	1
#define VCOMP_SKIP	2

typedef struct vcomp_iff
{
	char iff_type[4];
	int32_t iff_size;
	SHORT xoff, yoff;
	SHORT width, height;
	SHORT depth;
	SHORT ViewMode;
	struct comp_size comps[8];
	SHORT hold_time;	/*in jiffies */
	SHORT reserved[1];
	USHORT cmap[32];
} Vcomp_iff;
STATIC_ASSERT(rif, sizeof(Vcomp_iff) == 120);

struct rcel;

extern Errcode
conv_bitmaps(UBYTE *planes[], int pcount, int bpr, int width, int height,
		struct rcel *screen);

extern UBYTE *
decode_vplane(UBYTE *comp, UBYTE *plane, int BytesPerRow);

extern UBYTE *
decode_vkplane(UBYTE *comp, UBYTE *plane, int BytesPerRow, int *ytable);

#endif /* RIF_H */
