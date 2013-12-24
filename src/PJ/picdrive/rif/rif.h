

struct sriff_head
	{
	SHORT xoff, yoff;
	SHORT width, height;
	SHORT depth;
	SHORT ViewMode;
	SHORT frame_count;
	SHORT jiffies_frame;	/* # of jiffies each frame takes */
	SHORT frames_written;	/* may be less than frame count if file trunc */
	SHORT reserved[19];
	};
typedef struct sriff_head Sriff_head;

struct riff_head
	{
	char iff_type[4];	/* == RIFF */
	long iff_size;
	SHORT xoff, yoff;
	SHORT width, height;
	SHORT depth;
	SHORT ViewMode;
	SHORT frame_count;
	SHORT jiffies_frame;	/* # of jiffies each frame takes */
	SHORT frames_written;	/* may be less than frame count if file trunc */
	SHORT reserved[19];
	};
typedef struct riff_head Riff_head;

struct comp_size
	{
	SHORT comp;
	USHORT size;
	};
typedef struct comp_size Comp_size;

#define VCOMP_NONE	0
#define VCOMP_VRUN	1
#define VCOMP_SKIP	2

struct vcomp_iff
	{
	char iff_type[4];
	long iff_size;
	SHORT xoff, yoff;
	SHORT width, height;
	SHORT depth;
	SHORT ViewMode;
	struct comp_size comps[8];
	SHORT hold_time;	/*in jiffies */
	SHORT reserved[1];
	USHORT cmap[32];
	};
typedef struct vcomp_iff Vcomp_iff;

struct vcomp_head
	{
	SHORT xoff, yoff;
	SHORT width, height;
	SHORT depth;
	SHORT ViewMode;
	struct comp_size comps[8];
	SHORT hold_time;	/*in jiffies */
	SHORT reserved[1];
	SHORT cmap[32];
	};
typedef struct vcomp_head Vcomp_head;


