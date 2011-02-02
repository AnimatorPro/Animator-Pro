#ifndef TRUEPDR_H
#define TRUEPDR_H

#ifndef REXLIB_H
	#include "rexlib.h"
#endif

#ifndef RCEL_H
	#include "rcel.h"
#endif 

#ifndef ANIMINFO_H
	#include "animinfo.h"
#endif

#define TPDR_VERSION 0
#define TPDR_SUFFI_SIZE 15

/* Data structure returned by create_image_file() or open_image_file(),
 * and passed back as a parameter to other functions of the image
 * file driver.   The driver is free to extend this structure to
 * include other data it finds useful,  but the first 3 fields
 * must be those listed below and must remain unaltered until close_image_file
 * is called. */

typedef struct timage_file {
	struct tpdr *pd; 	/* pointer to module header vectors and data 
						 * set by host */

	UBYTE write_mode;	/* set to TRUE if this file is opened by 
						 * create_image_file() 
					 	 * FALSE if opened by open_image_file() set by host */

	UBYTE needs_work_cel; /* if set means that write_frames() needs a work cel
						   * that is the same size and depth of the source 
						   * screen set by open and create image file */
} Timage_file;


struct tpdr {
	Rexlib hdr;  	/* REX_TRUEPDR, TPDR_VERSION, rex library header */

	char *title_info;		/* string with capsule description of picture file
							 * format type used for titles on menus and 
							 * format directory only 34 characters used 
							 * example "BrandX 8 bit picture format." */

	char *long_info;		/* optional long info text string to be displayed
							 * in a text window under the title info.  
							 * The text will be word wrapped. newlines force 
							 * a new line */

	char default_suffi[TPDR_SUFFI_SIZE+1];	/* most commonly used file suffi 
							 * for this image file type separated by 
							 * semi-colons. 
							 *    Default suffix for saving files is the first
							 * one.  Suffi include "dots" as in ".pic", etc. 
							 * sample:  ".GIF;.XXX;.YYY" note no ";" after 
							 * last suffix Maximum of 3 suffi even if less than
							 * 3 chars */

	ULONG max_write_frames;  /* maximum number of image frames that can be 
							  * written to a single file including the first 
							  * but not including the ring frame 
							  * 1 for still pictures 0 if this module
							  * doesn't do writing */

	ULONG max_read_frames;   /* maximum number of frames that can be read from
							  * an image file including the first but not 
							  * including the ring frame 
							  * 1 for still pictures 0 if this module doesn't
							  * do reading */

	/********************/

	Boolean (*spec_best_fit)(Anim_info *spec);

    /* this is called before trying to write an image file to verify whether
     * or not the images can be saved exactly as specified.
     * If they can't the "best fit" specification
     * will be loaded into *spec. Like, we can save a 320 X 200 but not a
     * 300 X 180 picture. so 320 X 300 will be loaded into the width and height
     * field of spec and all other fields will remain unchanged and 
     * FALSE (it's altered) will be returned. If the images can be saved 
	 * so they will be  * read unchanged from the original 
	 * return TRUE (its ok) and leave spec unaltered */

	Errcode (*create_image_file)(struct tpdr *pd, char *path, 
								 Timage_file **pif, Anim_info *spec );

	/* create_image_file() does all preparatory work to open things up
	 * to recieve picture data for a new image file it will overwrite existing
	 * files or create a new file if none exists. if Successful it will 
	 * set *pif to a valid Timage_file pointer, if Failure it should clean up
	 * the mess and set *pif to NULL. The ainfo will contain the desired
	 * specs of the animation or picture that is going to be saved. 
	 * May be NOFUNC (NULL) if max_write_frames == 0 */

	/********************/
	Errcode (*open_image_file)(struct tpdr *pd, char *path, Timage_file **pif,
							   Anim_info *ainfo );

	/********************/
	void (*close_image_file)(Timage_file **pif);

	/********************/
	Errcode (*seek_frame)(Timage_file *ifile, LONG frame_ix);

	/* Prepares image file to start reading lines from frame of index provided
	 * not in any way guaranteed to be efficient, especially if going backwards
	 * incrementing by one forward is it's primary function bytes_per_row
	 * is so it can handle buffers that may desire roundoff of rectangle
	 * blit type loading */

	Errcode (*read_frame_lines)(Timage_file *ifile, void *pixelbuf, 
								SHORT num_lines, SHORT bytes_per_row);

	/* Reads pixel lines from image frame starting with the first line
	 * after frame seeked to and reads subsequent lines after a call to 
	 * itself will return Err_end_of_record if no more lines are available */

};
#define Tpdr struct tpdr  /* argh... typedefs cant be set by struct *s 
						 * but I want to save typeing (ha ha) */

#ifndef REXLIB_CODE 

Errcode load_tpdr(char *path, Tpdr **ppdr);
void free_tpdr(Tpdr **ppdr);

int tpdr_get_title(Tpdr *pd, char *buf, int maxlen);
int tpdr_get_suffi(Tpdr *pd, char *buf);

char *tpdr_alloc_info(Tpdr *pd);
void tpdr_free_info(char *info);

Boolean tpdr_best_fit(Tpdr *pd, Anim_info *spec);
Errcode tpdr_create_ifile(Tpdr *pd, char *path, Timage_file **pifile,
						 Anim_info *spec );
Errcode tpdr_open_ifile(Tpdr *pd, char *path, Timage_file **pifile,
					   Anim_info *ainfo );
void tpdr_close_ifile(Timage_file **pifile);

Errcode tpdr_seek_frame(Timage_file *ifile, LONG frame_ix);

Errcode tpdr_read_frame_lines(Timage_file *ifile, void *pixelbuf, 
						      SHORT num_lines, SHORT bytes_per_row);

#endif /* REXLIB_CODE */

#endif /* TRUEPDR_H */
