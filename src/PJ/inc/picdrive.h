#ifndef PICDRIVE_H
#define PICDRIVE_H

#ifndef REXLIB_H
	#include "rexlib.h"
#endif

#ifndef RCEL_H
	#include "rcel.h"
#endif

#ifndef ANIMINFO_H
	#include "animinfo.h"
#endif

#ifndef CMAP_H
	#include "cmap.h"
#endif

#define PDR_VERSION 0
#define PDR_SUFFI_SIZE 15

/*----------------------------------------------------------------------------
 * structure used by PDR to specify that the CONVERT program is to conduct
 * Qchoice dialogs before calling create_image_file().
 * (Need a bit pithier discussion of the usage of this here).
 *--------------------------------------------------------------------------*/

typedef struct pdr_options {
	char	*choicelst1;
	char	*choicelst2;
	char	*choicelst3;
	char	*choicelst4;
	UBYTE	options_valid;
	UBYTE	option1;
	UBYTE	option2;
	UBYTE	option3;
	UBYTE	option4;
	} Pdroptions;

/*----------------------------------------------------------------------------
 * Data structure returned by create_image_file() or open_image_file(), and
 * passed back as a parameter to other functions of the image file driver.
 * The driver is free to extend this structure to include other data it finds
 * useful, but the first 3 fields must be those listed below and must remain
 * unaltered until close_image_file is called.
 *--------------------------------------------------------------------------*/

typedef struct image_file {
	struct pdr *pd; 	/* pointer to module header vectors and data
						 * set by host */

	UBYTE write_mode;	/* set to TRUE if this file is opened by
						 * create_image_file()
						 * FALSE if opened by open_image_file() set by host */

	UBYTE needs_work_cel; /* if set means that write_frames() needs a work cel
						   * that is the same size and depth of the source
						   * screen set by open and create image file */
} Image_file;


/*----------------------------------------------------------------------------
 * Structure which contains the rexlib header for the PDR, as well as the
 * pointers to data and functions which comprise the host<->PDR interface.
 *--------------------------------------------------------------------------*/

struct pdr {
	Rexlib hdr; 	/* REX_PICDRIVER, PDR_VERSION, rex library header */

	char *title_info;		/* string with capsule description of picture file
							 * format type. used for titles on menus and
							 * format directory. only 34 characters used.
							 * example "BrandX 8 bit picture format." */

	char *long_info;		/* optional long info text string to be displayed
							 * in a text window under the title info.
							 * The text will be word wrapped. newlines force
							 * a new line */

	char default_suffi[PDR_SUFFI_SIZE+1];	/* most commonly used file suffi
							 * for this image file type separated by
							 * semi-colons.
							 *	  Default suffix for saving files is the first
							 * one.  Suffi include "dots" as in ".pic", etc.
							 * sample:	".GIF;.XXX;.YYY" note no ";" after
							 * last suffix. Maximum of 3 suffi even if less
							 * than 3 chars */

	ULONG max_write_frames;  /* maximum number of image frames that can be
							  * written to a single file, including the first
							  * but not including the ring frame.
							  * 1 for still pictures, 0 if this module
							  * doesn't do writing */

	ULONG max_read_frames;	 /* maximum number of frames that can be read from
							  * an image file, including the first but not
							  * including the ring frame.
							  * 1 for still pictures, 0 if this module doesn't
							  * do reading */

	/********************/

	Boolean (*spec_best_fit)(Anim_info *spec);
	/**************************************************************************
	 * this is called before trying to write an image file to verify whether
	 * or not the images can be saved exactly as specified.
	 * If they can't the "best fit" specification
	 * will be loaded into *spec. Like, we can save a 320 X 200 but not a
	 * 300 X 180 picture. so 320 X 300 will be loaded into the width and height
	 * field of spec and all other fields will remain unchanged and
	 * FALSE (it's altered) will be returned. If the images can be saved
	 * so they will be	* read unchanged from the original
	 * return TRUE (its ok) and leave spec unaltered
	 *************************************************************************/

	Errcode (*create_image_file)(struct pdr *pd, char *path, Image_file **pif,
								 Anim_info *spec );
	/**************************************************************************
	 * create_image_file() does all preparatory work to open things up
	 * to recieve picture data for a new image file it will overwrite existing
	 * files or create a new file if none exists. if Successful it will
	 * set *pif to a valid Image_file pointer, if Failure it should clean up
	 * the mess and set *pif to NULL. The ainfo will contain the desired
	 * specs of the animation or picture that is going to be saved.
	 * May be NOFUNC (NULL) if max_write_frames == 0
	 *************************************************************************/

	Errcode (*open_image_file)(struct pdr *pd, char *path, Image_file **pif,
							   Anim_info *ainfo );
	/**************************************************************************
	 * open_image_file() will open and verify the file type of
	 * an existing image file and return info about the contents of the file.
	 * If not the proper file type or other error it will return
	 * an error code.  If successful it will set *pif to a valid Image_file
	 * pointer, ainfo will contain the specs of width, height, and pixel depth
	 * to match if the file or the loader has a resolution independent format
	 * (other fields may contain garbage). If the open is successful the actual
	 * specs of the file are loaded into *ainfo.  If the open fails it should
	 * clean up the mess and set *pif to NULL.
	 * This may may be NOFUNC (NULL) if max_read_frames == 0
	 * The processing overhead of this call should be kept to the minimum
	 * needed to open the file and verify it's validity since since an image
	 * file may be checked for validity with many module types. Significant
	 * processing should not take place until read_first_frame()
	 *************************************************************************/

	void (*close_image_file)(Image_file **pif);
	/**************************************************************************
	 * This will close io and deallocate
	 * all resources allocated by open_image_file() or create_image_file()
	 * and set *pif to NULL, If *pif is NULL it should do nothing
	 *************************************************************************/

	Errcode (*read_first_frame)(Image_file *ifile, Rcel *screen);
	/**************************************************************************
	 * Will read first image in image_file and put it in screen.
	 * Screen will be the size retrieved in the ainfo when open_image_file()
	 * is called.  If the raster type of screen is a CLIPBOX it may not be
	 * readable on all pixels.
	 * It must call load_cmap() to load the colors into hardware when
	 * appropriate to make things look the best may only be called
	 * if file opened by open_image_file() the screen will contain garbage
	 * when this is called, may be called at any time after the file
	 * is opened with open_image_file().
	 * May be NOFUNC (NULL) if max_read_frames == 0
	 *************************************************************************/

	Errcode (*read_delta_next)(Image_file *ifile,Rcel *screen);
	/**************************************************************************
	 * Assuming:
	 *		screen - screen to contain next frame. will contain previous
	 *				 frame. will be same size as ainfo returned by
	 *				 open_image_file() or else.
	 *
	 * This call will alter the contents of screen to contain the next image
	 * in the image file. It may only be called after read_first_frame().
	 * If the current frame is the last frame in the file it will install
	 * the first frame in screen and leave the state appropriate for a
	 * subsequent call to read_delta_next(). It will call load_cmap() to load
	 * the colors into hardware when appropriate to make things look the best.
	 * It may only be called if the file is opened by open_image_file()
	 * May be NOFUNC (NULL) if max_read_frames == 0
	 *************************************************************************/

	Errcode (*save_frames)(Image_file *ifile,
						   Rcel *screen,
						   int num_frames,
						   Errcode (*seek_frame)(int ix,void *seek_data),
						   void *seek_data,
						   Rcel *work_screen );
	/**************************************************************************
	 * This is only called after a call to create_image_file() and will
	 * write num_frames images to the image file.
	 * when called the input args will be:
	 * May be NOFUNC (NULL) if max_write_frames == 0
	 *
	 *		screen - will contain the first frame image (index 0) and is the
	 *				screen that the results of seek_frame() will be put into.
	 *				It is read only and should not be altered except through
	 *				seek_frame(). This will be the same size as the ainfo
	 *				returned by spec_best_fit();
	 *
	 *		num_frames - is the number of frames desired to be put in the
	 *				image file. (indices 0 to num_frames - 1).	>= 1
	 *
	 *		(*seek_frame)(ix,dat) - is a function that will put the contents
	 *				of the frame indexed by the input index into screen.
	 *				This is for use by the saver code.	It must be provided
	 *				with the seek_data in the dat field. Care should be taken
	 *				to minimize seeking for the sake of speed.
	 *					Seek frame will if it fails return an error code and
	 *				also will poll for user abort, and will return Err_abort
	 *				if an abort is requested. If errcode < Success
	 *				this error code must be passed back to the host by
	 *				save_frames() and a subsequent call to close_image_file()
	 *				should work. Seeking to the same index as the previous
	 *				seek will only check for user abort and may be used to do
	 *				this. Index 0 is the first frame, (num_frames-1) is the
	 *				last frame.
	 *					If num_frames == 1 then seek_frame() will only check
	 *				for user abort and will not alter screen.
	 *
	 *		seek_data - This value always to be input with ix to seek frame
	 *				and must not be altered.
	 *
	 *		work_screen - Only be provided if num_frames > 1 and
	 *				the flag needs_work_cel is TRUE when the file is opened.
	 *				will be the same size as screen.
	 *
	 *				This is a cel of equal or greater size than the
	 *				screen, it is for use by the save code and may be altered.
	 *
	 *				If it is a raster type CLIPBOX it may only accurately
	 *				contain copies of screen. ie if it is a virtual cel into
	 *				a cel that is smaller areas outside the root cel will
	 *				always read 0s
	 *************************************************************************/

	Pdroptions *poptions;	 /* pointer to Pdroptions structure used to query
							  * (via qchoicef) output options from the
							  * user.  this is currently used by TIFF & TARGA.
							  * must be set to NULL if unneeded.
							  */

	Errcode (*rgb_seekstart)(Image_file *ifile);
	/**************************************************************************
	 * This will prepare for reading the first line of data from an RGB-type
	 * image file (currently TIFF and TARGA files).  This may be called any
	 * number of times during the processing of an RGB image, but it will only
	 * be called when the file has already been opened via open_ifile().  Each
	 * time this routine is called, it must take whatever action is necessary
	 * internal to the driver to ensure that the next call to rgb_readline()
	 * will obtain the first line of data from the image file.	(EG, fseek to
	 * the start of the data area in the file, resync data decompression, etc).
	 *
	 * The return value, if negative, is a standard Errcode value.	If zero,
	 * it indicates that the 1st data line corresponds to the first screen line.
	 * If greater than zero, it indicates that the 1st data line corresponds
	 * to the last screen line (ie, image is stored upside down in file).
	 *
	 * This is for RGB drivers only; other drivers should set this field to
	 * NOFUNC/NULL.
	 *
	 * This function will only be called if the PDR has set a value greater
	 * than 8 into the anim_info.depth field during open_image_file()
	 * processing.
	 *************************************************************************/

	Errcode (*rgb_readline)(Image_file *ifile, Rgb3 *linebuf);
	/**************************************************************************
	 * This will return the next line of RGB data from the file into linebuf.
	 * The data placed into linebuf by the PDR must be in Rgb3 format.	This
	 * routine will only be called after open_image_file() put a value greater
	 * than 8 into anim_info.depth, and rgb_seekstart() has been called at
	 * least once. The linebuf will have room for 3*anim_info.width bytes, ie,
	 * one line of data.
	 *************************************************************************/

	long reserved[4];	/* PDR should init these fields to NULL */

};
#define Pdr struct pdr


#ifndef REXLIB_CODE

#define LOCAL_PDR_STR "="
#define LOCAL_PDR_CHAR '='

typedef struct local_pdr {
	void *next;
	char *name;
	Pdr *header;
} Local_pdr;

extern char 	 gif_pdr_name[];
extern char 	 fli_pdr_name[];
extern Local_pdr fli_local_pdr;
extern char 	 pic_pdr_name[];
extern Local_pdr pic_local_pdr;

/*
 * following items found in picdrive\host\picdrive.c...
 */

extern	Local_pdr *local_pdrs;

void	add_local_pdr(Local_pdr *lpd); /* make a locally linked in pdr available */
void	remove_local_pdr(Local_pdr *lpd); /* make local pdr un-available */

Errcode load_pdr(char *path, Pdr **ppdr);
void	free_pdr(Pdr **ppdr);

int 	pdr_get_title(Pdr *pd, char *buf, int maxlen);
int 	pdr_get_suffi(Pdr *pd, char *buf);

char	*pdr_alloc_info(Pdr *pd);
void	pdr_free_info(char *info);

Boolean pdr_best_fit(Pdr *pd, Anim_info *spec);
Errcode pdr_create_ifile(Pdr *pd, char *path, Image_file **pifile,
						 Anim_info *spec );
Errcode pdr_open_ifile(Pdr *pd, char *path, Image_file **pifile,
					   Anim_info *ainfo );
void	pdr_close_ifile(Image_file **pifile);
Errcode pdr_read_first(Image_file *ifile, Rcel *screen);
Errcode pdr_read_next(Image_file *ifile,Rcel *screen);

Errcode pdr_save_frames(Image_file *ifile,	Rcel *screen,  int num_frames,
						Errcode (*seek_frame)(int ix,void *seek_data),
						void *seek_data, Rcel *work_screen );

Errcode pdr_rgb_seekstart(Image_file *ifile);
Errcode pdr_rgb_readline(Image_file *ifile, Rgb3 *linebuf);

#endif /* REXLIB_CODE */

#endif /* PICDRIVE_H */
