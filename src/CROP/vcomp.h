

struct sriff_head
	{
	WORD xoff, yoff;
	WORD width, height;
	WORD depth;
	WORD ViewMode;
	WORD frame_count;
	WORD jiffies_frame;	/* # of jiffies each frame takes */
	WORD frames_written;	/* may be less than frame count if file trunc */
	WORD reserved[19];
	};
typedef struct sriff_head Sriff_head;

struct riff_head
	{
	char iff_type[4];	/* == RIFF */
	long iff_size;
	WORD xoff, yoff;
	WORD width, height;
	WORD depth;
	WORD ViewMode;
	WORD frame_count;
	WORD jiffies_frame;	/* # of jiffies each frame takes */
	WORD frames_written;	/* may be less than frame count if file trunc */
	WORD reserved[19];
	};
typedef struct riff_head Riff_head;

struct comp_size
	{
	WORD comp;
	UWORD size;
	};
typedef struct comp_size Comp_size;

#define VCOMP_NONE	0
#define VCOMP_VRUN	1
#define VCOMP_SKIP	2

struct vcomp_iff
	{
	char iff_type[4];
	long iff_size;
	WORD xoff, yoff;
	WORD width, height;
	WORD depth;
	WORD ViewMode;
	struct comp_size comps[8];
	WORD hold_time;	/*in jiffies */
	WORD reserved[1];
	WORD cmap[32];
	};
typedef struct vcomp_iff Vcomp_iff;

struct vcomp_head
	{
	WORD xoff, yoff;
	WORD width, height;
	WORD depth;
	WORD ViewMode;
	struct comp_size comps[8];
	WORD hold_time;	/*in jiffies */
	WORD reserved[1];
	WORD cmap[32];
	};
typedef struct vcomp_head Vcomp_head;


struct vcomp_info
	{
	Vcomp_head header;
	struct BitMap bitmap;
	};
typedef struct vcomp_info Vcomp_info;

Vcomp_info *read_vrun();
struct iff_chunk *gulp_vrun();

struct riff_list
	{
	struct riff_list *next;
	struct BitMap BitMap;
	WORD cmap[32];
	};

#define MAXRUN 127
