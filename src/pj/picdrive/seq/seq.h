

struct cel
	{
	SHORT xoff, yoff;
	SHORT width, height;
	SHORT cmap[16];
	SHORT *image;
	long image_size;
	SHORT *mask;
	};
typedef struct cel Cel;

#define CYBER_MAGIC 0xfedb
#define FLICKER_MAGIC 0xfedc

struct seq_header
	{
	USHORT magic;		/* == 0xfedc Flicker 0xfedb Cyber Paint */
	SHORT version;
	long cel_count;
	SHORT speed;
	char resolution; /* 0 lores, 1 medium, 2 hires */
	char flags;
	char reserved[52]; /* extra space all for me */
	char pad[64]; /* extra space I'm not claiming yet */
	};
typedef struct seq_header Seq_header;


/* some bits for seq_header.flags */
#define SEQ_STEREO	1

struct neo_head
{
	SHORT type;  /* 0 for neo, 1 for programmed pictures, 2 for cels? */
	SHORT resolution; /*0 lores, 1 medium, 2 hires*/
	USHORT colormap[16];
	char filename[8+1+3];
	SHORT ramp_seg; /*hibit active, bits 0-3 left arrow, 4-7 right arrow*/
	char ramp_active;  /*hi bit set if actively cycled*/
	char ramp_speed;  /*60hz ticks between cycles*/
	SHORT slide_time;  /*60hz ticks until next picture*/
	SHORT xoff, yoff;  /*upper left corner of cel*/
	SHORT width, height; /*dimensions of cel*/
	char	op;		/* xor this one, copy it? */
	char	compress;	/* compressed? */
	long	data_size;		/* size of data */
	char reserved[30];	/* Please leave this zero, I may expand later */
	char pad[30]; /* You can put some extra info here if you like */
};
typedef struct neo_head Neo_head;

/* defines for neo_head.op */
#define NEO_COPY	0
#define NEO_XOR		1

/* defines for neo_head.compress */
#define NEO_UNCOMPRESSED	0
#define NEO_CCOLUMNS	1

/* handy macro's to find out how much memory a raster line takes up */
#define seq_Mask_line(width) ((((width)+15)>>3)&0xfffe)
#define seq_Mask_block(width, height) (seq_Mask_line(width)*(height))
#define seq_Raster_line(width) (((((width)+15)>>3)&0xfffe)*2)
#define seq_Raster_block(width, height) (seq_Raster_line(width)*(height))
#define seq_LRaster_line(width) (((((width)+15)>>3)&0xfffe)*4)
#define seq_LRaster_block(width, height) (seq_LRaster_line(width)*(height))

