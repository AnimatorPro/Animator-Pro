/*****************************************************************************
 * test stub for tiff driver...supplies noop functions for pj_ functions.
 ****************************************************************************/


#include "tiff.h"

#define RASType void

#define IO_READ  0
#define IO_WRITE 1
#define IO_TRUECOLOR 2


extern Pdr			rexlib_header;
static Pdr			*pd;
static Anim_info	ainfo;
static Image_file	*pimage;
static Rcel 		screen_rcel;
static Cmap 		screen_cmap;

int stop_the_show = 0;

debug_output(int code, int codesz, int nextcode)
{
	static int count = 0;

	  if (count > 0x1000)
		  return stop_the_show = 1;
	printf("%6x = %6x (%2d)\n", nextcode, code, codesz);
	++count;
}

void pj_cmap_load(void *rast, Cmap *cmap)
{
	return;
}

Errcode pj_errno_errcode(void)
{
	return -1;
}

void pj_set_rast()
{
	return;
}
void pj_set_hline()
{
	return;
}

void pj_mask1blit(UBYTE *mbytes, Coor mbpr, Coor mx, Coor my,
			   RASType *r, Coor rx, Coor ry, Ucoor width, Ucoor height,
			   Pixel oncolor )
{
	printf("mask1blit writing at %5d,%5d,%5d,%5d\n", rx,ry,width,height);
	return;
}

void pj_put_hseg(RASType *r,void *pixbuf,Coor x,Ucoor y,Ucoor w)
{
	printf("put_hseg writing at %5d,%5d,%5d\n", x,y,w);
	return;
}

void pj_get_hseg(RASType *r,unsigned char *pixbuf,Coor x,Ucoor y,Ucoor w)
{
	extern unsigned int rand(void);
	printf("get_hseg reading at %5d,%5d,%5d\n", x,y,w);
	while (w--) {
		*pixbuf++ = rand();
	}
	return;
}

void *zalloc(int size)
{
	void *ptr;

	if (NULL == (ptr = malloc(size)))
		return NULL;
	memset(ptr, 0, size);
	return ptr;
}

int do_rgb_loop(Image_file *ifi, Rcel *screen, Pdr *pd, Anim_info *ai)
{
	int err;
	int y, dy;
	int w, h;
	Rgb3	linebuf[2048];

	w = ai->width;
	h = ai->height;

	err = pd->rgb_seekstart(ifi);
	if (err < Success)
		{
		printf("status from rgb_seekstart = %d\n",err);
		goto OUT;
		}
	else if (err == 0)
		{
		y = 0;
		dy = 1;
		}
	else
		{
		y = h-1;
		dy = -1;
		}

	while (--h >= 0)
		{
		if (y == 195)
			stop_the_show = 1;
		if (Success > (err = pd->rgb_readline(ifi, linebuf)))
			{
			printf("status from rgb_readline at line #%d = %d\n", y, err);
			goto OUT;
			}
		y += dy;
		}

OUT:
	return err;
}

static void cmap_init(UBYTE *ctab)
{
	int i = 3*256;
	while (i--)
		*ctab++ = rand();
}

void main(int argc, char *argv[])
/*****************************************************************************
 *
 ****************************************************************************/
{
	Errcode err;
	int 	counter;
	char	*argp;
	char	*fname = "";
	int 	io_option = IO_READ;
	Pdroptions *popt;

	pd = &rexlib_header;
	popt = pd->poptions;
	screen_rcel.cmap = &screen_cmap;
	cmap_init((UBYTE *)&screen_cmap.ctab);

	for (counter = 1; counter < argc; counter++)
		{
		argp = argv[counter];
		if (*argp == '-')
			{
			switch (toupper(*++argp))
				{
				case 'R':
					io_option = IO_READ;
					break;
				case 'W':
					io_option = IO_WRITE;
					break;
				default:
					break;
				}
			}
		else
			{
			fname = argp;		/* It's not a switch, must be the source file. */
			}
		}

	if (io_option == IO_READ)
		{
		if (Success != (err = pd->open_image_file(pd, fname, &pimage, &ainfo)))
			{
			printf("status from open_image is %d\n", err);
			goto OUT;
			}
		if (ainfo.depth <= 8)
			{
			err = pd->read_first_frame(pimage, &screen_rcel);
			printf("status from read_first is %d\n", err);
			}
		else
			{
			err = do_rgb_loop(pimage, &screen_rcel, pd, &ainfo);
			printf("status from do_rgb_loop = %d\n", err);
			}
		}
	else
		{
		popt->option1 = 0;			/* greyscale */
		popt->option2 = 2;			/* lzw */
		popt->options_valid = 1;

		memset(&ainfo, 0, sizeof(ainfo));
		ainfo.width = 640;
		ainfo.height = 480;
		ainfo.depth = 8;
		ainfo.num_frames = 1;
		pd->spec_best_fit(&ainfo);
		if (Success != (err = pd->create_image_file(pd, fname, &pimage, &ainfo)))
			printf("status from create_image is %d\n", err);
		else
			err = pd->save_frames(pimage, &screen_rcel, 1, NULL, NULL, NULL);
		printf("status from save_frames is %d\n", err);
		}
OUT:
	if (pimage != NULL)
		pd->close_image_file(&pimage);
	exit(0);
}
