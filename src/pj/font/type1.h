typedef struct type1_box
	{
	int xmin,ymin,xmax,ymax;
	} Type1_box;

#define BYTE_MAX 256

typedef struct type1_bitplane
/* This is a two dimensional bit array that defines a hard edged raster
 * character.
 */
	{
	unsigned char *bits;
	int	bpr;					 /* Bytes per row. */
	int width, height;
	int x,y;
	} Type1_bitplane;

typedef struct
/* This is a two dimensional byte array that defines a soft edged
 * rasterized character.
 */
	{
	unsigned char *channel;
	int bpr;
	int width, height;
	int x,y;
	} Type1_alpha;

typedef struct type1_scale_info
/* This is the size-specific part of the font.  We build this up
 * every time the font is resized. */
	{
	double scalex, scaley;
	Type1_bitplane *bits[BYTE_MAX]; /* Sharp edged chars in ASCII order */
	Type1_alpha *alphas[BYTE_MAX];	/* Smooth edged chars in ASCII order */
	int width[BYTE_MAX];
	int widest;
	Type1_box max_bounds;
	int right_overlap;
	Boolean unzag_flag;				/* Do alpha channel stuff? */
	int unzag_shrink;				/* Amount to shrink to get alpha. */
	Block_allocator ba;
	} Type1_scale_info;

#define NCdefs  300                   /* Number of Charstring definitions */


typedef struct type1_char_defs
/* This is all the info on the font that is the same regardless of the
 * size we render it at. */
	{
	unsigned char **letter_defs;	 	 /* Character definitions */
	char **letter_names;				 /* Character names. */
	int letter_count;					 /* Number of letters. */
	unsigned char *ascii_defs[BYTE_MAX]; /* Character defs in ASCII order */
	int sub_count;                    	 /* Number of subroutines */
	unsigned char **subrs;         		 /* Subroutine definitions */
	char **encoding;              		 /* Active mapping vector. */
	int  encoding_count;				 /* Number of entries in encoding */
	Block_allocator font_ba;			 /* Private memory manager. */
	Type1_box letter_bounds[BYTE_MAX];	 /* Bounds of letters (unscaled) */
	Type1_box font_bounds;				 /* Unscaled bounds of font. */
	int letter_width[BYTE_MAX];			 /* Width of letters (unscaled) */
	int font_widest;					 /* Width of widest (unscaled) */
	Type1_scale_info scale;				 /* Size stuff. */
	} Type1_font;


typedef struct type1_poly_out
	{
	/* This guy tells you how to draw a polygon. */
	EFUNC hline;		/* How to draw horizontal lines. */
	EFUNC outline;		/* How to draw an outline. */
	void *data;			/* Data for hline and outline. */
	} Type1_poly_out;

typedef struct type1_bits_out
	{
	Type1_bitplane **pbits;
	Block_allocator *ba;
	} Type1_bits_out;

typedef struct type1_output
	{
	/* This guy tells the interpreter where to send drawing commands. */
	Errcode (*letter_open)(struct type1_output *fo);
	Errcode (*letter_close)(struct type1_output *fo);
	Errcode (*shape_open)(struct type1_output *fo, double x, double y);
	Errcode (*shape_close)(struct type1_output *fo);
	Errcode (*shape_point)(struct type1_output *fo, double x, double y);
	void *data;		/* Hang data specific implementations of letter_open, etc.
					 * might want here. */
	} Type1_output;

