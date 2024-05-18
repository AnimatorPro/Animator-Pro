#include "jinclude.h"
#include "gfx.h"
#include "jpegpdr.h"
#include <setjmp.h>


METHODDEF void
input_init (compress_info_ptr cinfo)
/* Initialize for input; return image size and component data. */
{
  struct ifileref *input = cinfo->input_file;
  Rcel *image = input->image;
  /* This routine must return five pieces of information about the incoming
   * image, and must do any setup needed for the get_input_row routine.
   * The image information is returned in fields of the cinfo struct.
   * (If you don't care about modularity, you could initialize these fields
   * in the main JPEG calling routine, and make this routine be a no-op.)
   * We show some example values here.
   */
  cinfo->image_width = image->width;		/* width in pixels */
  cinfo->image_height = image->height;		/* height in pixels */
  /* JPEG views an image as being a rectangular array of pixels, with each
   * pixel having the same number of "component" values (color channels).
   * You must specify how many components there are and the colorspace
   * interpretation of the components.  Most applications will use RGB data or
   * grayscale data.  If you want to use something else, you'll need to study
   * and perhaps modify jcdeflts.c, jccolor.c, and jdcolor.c.
   */
  cinfo->input_components = 3;		/* or 1 for grayscale */
  cinfo->in_color_space = CS_RGB;	/* or CS_GRAYSCALE for grayscale */
  cinfo->data_precision = 8;		/* bits per pixel component value */
  /* In the current JPEG software, data_precision must be set equal to
   * BITS_IN_JSAMPLE, which is 8 unless you twiddle jconfig.h.  Future
   * versions might allow you to say either 8 or 12 if compiled with
   * 12-bit JSAMPLEs, or up to 16 in lossless mode.  In any case,
   * it is up to you to scale incoming pixel values to the range
   *   0 .. (1<<data_precision)-1.
   * If your image data format is fixed at a byte per component,
   * then saying "8" is probably the best long-term solution.
   */
}

METHODDEF void
get_input_row (compress_info_ptr cinfo, JSAMPARRAY pixel_row)
/* Read next row of pixels into pixel_row[][] */
{
  /* This example shows how you might read RGB data (3 components)
   * from an input file in which the data is stored 3 bytes per pixel
   * in left-to-right, top-to-bottom order.
   */
  struct ifileref *input = cinfo->input_file;
  Rcel *image = input->image;
  Rgb3 *cmap = image->cmap->ctab;
  Rgb3 *one;
  Pixel *buf = input->line_buf;
  JSAMPROW ptr0, ptr1, ptr2;
  long col;
  int width = image->width;
  
  pj_get_hseg(image, buf, 0, input->y++, width);
  ptr0 = pixel_row[0];
  ptr1 = pixel_row[1];
  ptr2 = pixel_row[2];
  for (col = 0; col < width; col++) 
  	{
	one = &cmap[*buf++];
    *ptr0++ = one->r;
	*ptr1++ = one->g;
	*ptr2++ = one->b;
  	}
}

METHODDEF void
input_term (compress_info_ptr cinfo)
/* Finish up at the end of the input */
{
  /* This termination routine will very often have no work to do, */
  /* but you must provide it anyway. */
  /* Note that the JPEG code will only call it during successful exit; */
  /* if you want it called during error exit, you gotta do that yourself. */
}

static void save(struct ifileref *input, FILE *output)
{
  /* These three structs contain JPEG parameters and working data.
   * They must survive for the duration of parameter setup and one
   * call to jpeg_compress; typically, making them local data in the
   * calling routine is the best strategy.
   */
  struct Compress_info_struct cinfo;
  struct Compress_methods_struct c_methods;
  struct External_methods_struct e_methods;

  /* Initialize the system-dependent method pointers. */
  cinfo.methods = &c_methods;	/* links to method structs */
  cinfo.emethods = &e_methods;

  /* Here we supply our own error handler; compare to use of standard error
   * handler in the previous write_JPEG_file example.
   */
  set_error_methods(&e_methods);

  /* Here we use the standard memory manager provided with the JPEG code.
   * In some cases you might want to replace the memory manager, or at
   * least the system-dependent part of it, with your own code.
   */
  jselmemmgr(&e_methods);	/* select std memory allocation routines */
  /* If the compressor requires full-image buffers (for entropy-coding
   * optimization or a noninterleaved JPEG file), it will create temporary
   * files for anything that doesn't fit within the maximum-memory setting.
   * (Note that temp files are NOT needed if you use the default parameters.)
   * You can change the default maximum-memory setting by changing
   * e_methods.max_memory_to_use after jselmemmgr returns.
   * On some systems you may also need to set up a signal handler to
   * ensure that temporary files are deleted if the program is interrupted.
   * (This is most important if you are on MS-DOS and use the jmemdos.c
   * memory manager back end; it will try to grab extended memory for
   * temp files, and that space will NOT be freed automatically.)
   * See jcmain.c or jdmain.c for an example signal handler.
   */

  /* Here, set up pointers to your own routines for input data handling
   * and post-init parameter selection.
   */
  c_methods.input_init = input_init;
  c_methods.get_input_row = get_input_row;
  c_methods.input_term = input_term;
  c_methods.c_ui_method_selection = jselwjfif;

  /* Set up default JPEG parameters in the cinfo data structure. */
  j_c_defaults(&cinfo, 85, FALSE);
  /* Note: 75 is the recommended default quality level; you may instead pass
   * a user-specified quality level.  Be aware that values below 25 will cause
   * non-baseline JPEG files to be created (and a warning message to that
   * effect to be emitted on stderr).  This won't bother our decoder, but some
   * commercial JPEG implementations may choke on non-baseline JPEG files.
   * If you want to force baseline compatibility, pass TRUE instead of FALSE.
   * (If non-baseline files are fine, but you could do without that warning
   * message, set e_methods.trace_level to -1.)
   */

  /* At this point you can modify the default parameters set by j_c_defaults
   * as needed.  For a minimal implementation, you shouldn't need to change
   * anything.  See jcmain.c for some examples of what you might change.
   */

  /* Select the input and output files.
   * Note that cinfo.input_file is only used if your input reading routines
   * use it; otherwise, you can just make it NULL.
   * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
   * requires it in order to write binary files.
   */

  /* Set up input structure. */
  cinfo.input_file = input;

  /* Set up output file. */
  cinfo.output_file = output;

  /* Here we go! */
  jpeg_compress(&cinfo);

  /* Note: if you want to compress more than one image, we recommend you
   * repeat this whole routine.  You MUST repeat the j_c_defaults()/alter
   * parameters/jpeg_compress() sequence, as some data structures allocated
   * in j_c_defaults are freed upon exit from jpeg_compress.
   */
}


extern jmp_buf setjmp_buffer;	/* for return to caller */

Errcode jpeg_save_frame(struct ifileref *input, FILE *output)
{
Errcode err;
if (err = setjmp(setjmp_buffer)) 
	{
	/* If we get here, the JPEG code has signaled an error.
	 */
	return err;
	}
else
	{
	save(input, output);
	return Success;
	}
}

