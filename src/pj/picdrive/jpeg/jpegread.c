#include "jinclude.h"
#include "picdrive.h"
#include "gfx.h"
#include "jpegpdr.h"
#include <setjmp.h>


/*
 * To accept the image data from decompression, you must define four routines
 * output_init, put_color_map, put_pixel_rows, and output_term.
 *
 * You must understand the distinction between full color output mode
 * (N independent color components) and colormapped output mode (a single
 * output component representing an index into a color map).  You should use
 * colormapped mode to write to a colormapped display screen or output file.
 * Colormapped mode is also useful for reducing grayscale output to a small
 * number of gray levels: when using the 1-pass quantizer on grayscale data,
 * the colormap entries will be evenly spaced from 0 to MAX_JSAMPLE, so you
 * can regard the indexes are directly representing gray levels at reduced
 * precision.  In any other case, you should not depend on the colormap
 * entries having any particular order.
 * To get colormapped output, set cinfo->quantize_colors to TRUE and set
 * cinfo->desired_number_of_colors to the maximum number of entries in the
 * colormap.  This can be done either in your main routine or in
 * d_ui_method_selection.  For grayscale quantization, also set
 * cinfo->two_pass_quantize to FALSE to ensure the 1-pass quantizer is used
 * (presently this is the default, but it may not be so in the future).
 *
 * The output file writing modules (jwrppm.c, jwrgif.c, jwrtarga.c, etc) may be
 * useful examples of what these routines should actually do, although each of
 * them is encrusted with a lot of specialized code for its own file format.
 */


METHODDEF void
output_init (decompress_info_ptr cinfo)
/* This routine should do any setup required */
{
  /* This routine can initialize for output based on the data passed in cinfo.
   * Useful fields include:
   *	image_width, image_height	Pretty obvious, I hope.
   *	data_precision			bits per pixel value; typically 8.
   *	out_color_space			output colorspace previously requested
   *	color_out_comps			number of color components in same
   *	final_out_comps			number of components actually output
   * final_out_comps is 1 if quantize_colors is true, else it is equal to
   * color_out_comps.
   *
   * If you have requested color quantization, the colormap is NOT yet set.
   * You may wish to defer output initialization until put_color_map is called.
   */
}


/*
 * This routine is called if and only if you have set cinfo->quantize_colors
 * to TRUE.  It is given the selected colormap and can complete any required
 * initialization.  This call will occur after output_init and before any
 * calls to put_pixel_rows.  Note that the colormap pointer is also placed
 * in a cinfo field, whence it can be used by put_pixel_rows or output_term.
 * num_colors will be less than or equal to desired_number_of_colors.
 *
 * The colormap data is supplied as a 2-D array of JSAMPLEs, indexed as
 *		JSAMPLE colormap[component][indexvalue]
 * where component runs from 0 to cinfo->color_out_comps-1, and indexvalue
 * runs from 0 to num_colors-1.  Note that this is actually an array of
 * pointers to arrays rather than a true 2D array, since C does not support
 * variable-size multidimensional arrays.
 * JSAMPLE is typically typedef'd as "unsigned char".  If you want your code
 * to be as portable as the JPEG code proper, you should always access JSAMPLE
 * values with the GETJSAMPLE() macro, which will do the right thing if the
 * machine has only signed chars.
 */

METHODDEF void
put_color_map (decompress_info_ptr cinfo, int num_colors, JSAMPARRAY colormap)
/* Write the color map */
{
int i;
Rcel *screen = cinfo->output_file->image;
Rgb3 *pj_cmap = screen->cmap->ctab;
int cshift = cinfo->data_precision - 8;

for (i=0; i<num_colors; ++i)
	{
	pj_cmap[i].r = GETJSAMPLE(colormap[0][i]) >> cshift;
	pj_cmap[i].g = GETJSAMPLE(colormap[1][i]) >> cshift;
	pj_cmap[i].b = GETJSAMPLE(colormap[2][i]) >> cshift;
	}
pj_cmap_load(screen,screen->cmap); /* update hardware cmap if needed */
}


/*
 * This function is called repeatedly, with a few more rows of pixels supplied
 * on each call.  With the current JPEG code, some multiple of 8 rows will be
 * passed on each call except the last, but it is extremely bad form to depend
 * on this.  You CAN assume num_rows > 0.
 * The data is supplied in top-to-bottom row order (the standard order within
 * a JPEG file).  If you cannot readily use the data in that order, you'll
 * need an intermediate array to hold the image.  See jwrrle.c for an example
 * of outputting data in bottom-to-top order.
 *
 * The data is supplied as a 3-D array of JSAMPLEs, indexed as
 *		JSAMPLE pixel_data[component][row][column]
 * where component runs from 0 to cinfo->final_out_comps-1, row runs from 0 to
 * num_rows-1, and column runs from 0 to cinfo->image_width-1 (column 0 is
 * left edge of image).  Note that this is actually an array of pointers to
 * pointers to arrays rather than a true 3D array, since C does not support
 * variable-size multidimensional arrays.
 * JSAMPLE is typically typedef'd as "unsigned char".  If you want your code
 * to be as portable as the JPEG code proper, you should always access JSAMPLE
 * values with the GETJSAMPLE() macro, which will do the right thing if the
 * machine has only signed chars.
 *
 * If quantize_colors is true, then there is only one component, and its values
 * are indexes into the previously supplied colormap.  Otherwise the values
 * are actual data in your selected output colorspace.
 */


METHODDEF void
put_pixel_rows (decompress_info_ptr cinfo, int num_rows, JSAMPIMAGE pixel_data)
/* Write some rows of output data */
{
/* This example shows how you might write full-color RGB data (3 components)
* to an output file in which the data is stored 3 bytes per pixel.
*/
register JSAMPROW ptr0;
register long col;
register int row;
struct ifileref *output = cinfo->output_file;
Rcel *image = output->image;
int y = output->y;

for (row = 0; row < num_rows; row++) 
	{
	ptr0 = pixel_data[0][row];
	pj_put_hseg(image, ptr0, 0, y, image->width);
	++y;
	}
output->y = y;
}


METHODDEF void
output_term (decompress_info_ptr cinfo)
/* Finish up at the end of the output */
{
  /* This termination routine may not need to do anything. */
  /* Note that the JPEG code will only call it during successful exit; */
  /* if you want it called during error exit, you gotta do that yourself. */
}


/*
 * That's it for the routines that deal with writing the output image.
 * Now we have overall control and parameter selection routines.
 */


/*
 * This routine gets control after the JPEG file header has been read;
 * at this point the image size and colorspace are known.
 * The routine must determine what output routines are to be used, and make
 * any decompression parameter changes that are desirable.  For example,
 * if it is found that the JPEG file is grayscale, you might want to do
 * things differently than if it is color.  You can also delay setting
 * quantize_colors and associated options until this point. 
 *
 * j_d_defaults initializes out_color_space to CS_RGB.  If you want grayscale
 * output you should set out_color_space to CS_GRAYSCALE.  Note that you can
 * force grayscale output from a color JPEG file (though not vice versa).
 */

METHODDEF void
d_ui_method_selection (decompress_info_ptr cinfo)
{
  /* select output routines */
  cinfo->methods->output_init = output_init;
  cinfo->methods->put_color_map = put_color_map;
  cinfo->methods->put_pixel_rows = put_pixel_rows;
  cinfo->methods->output_term = output_term;
}


/*
 * OK, here is the main function that actually causes everything to happen.
 * We assume here that the JPEG filename is supplied by the caller of this
 * routine, and that all decompression parameters can be default values.
 * The routine returns 1 if successful, 0 if not.
 */

static void read(FILE *input, struct ifileref *output)
{
  /* These three structs contain JPEG parameters and working data.
   * They must survive for the duration of parameter setup and one
   * call to jpeg_decompress; typically, making them local data in the
   * calling routine is the best strategy.
   */
  struct Decompress_info_struct cinfo;
  struct Decompress_methods_struct dc_methods;
  struct External_methods_struct e_methods;

  /* Select the input and output files.
   */
  cinfo.output_file = output;
  cinfo.input_file = input;

  /* Initialize the system-dependent method pointers. */
  cinfo.methods = &dc_methods;	/* links to method structs */
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
  /* If the decompressor requires full-image buffers (for two-pass color
   * quantization or a noninterleaved JPEG file), it will create temporary
   * files for anything that doesn't fit within the maximum-memory setting.
   * You can change the default maximum-memory setting by changing
   * e_methods.max_memory_to_use after jselmemmgr returns.
   * On some systems you may also need to set up a signal handler to
   * ensure that temporary files are deleted if the program is interrupted.
   * (This is most important if you are on MS-DOS and use the jmemdos.c
   * memory manager back end; it will try to grab extended memory for
   * temp files, and that space will NOT be freed automatically.)
   * See jcmain.c or jdmain.c for an example signal handler.
   */

  /* Here, set up the pointer to your own routine for post-header-reading
   * parameter selection.  You could also initialize the pointers to the
   * output data handling routines here, if they are not dependent on the
   * image type.
   */
  dc_methods.d_ui_method_selection = d_ui_method_selection;

  /* Set up default decompression parameters. */
  j_d_defaults(&cinfo, TRUE);
  /* TRUE indicates that an input buffer should be allocated.
   * In unusual cases you may want to allocate the input buffer yourself;
   * see jddeflts.c for commentary.
   */

  /* At this point you can modify the default parameters set by j_d_defaults
   * as needed; for example, you can request color quantization or force
   * grayscale output.  See jdmain.c for examples of what you might change.
   */
  cinfo.desired_number_of_colors = 256;
  cinfo.quantize_colors = TRUE;

  /* Set up to read a JFIF or baseline-JPEG file. */
  /* This is the only JPEG file format currently supported. */
  jselrjfif(&cinfo);

  /* Here we go! */
  jpeg_decompress(&cinfo);

  /* You might want to test e_methods.num_warnings to see if bad data was
   * detected.  In this example, we just blindly forge ahead.
   */

  /* Note: if you want to decompress more than one image, we recommend you
   * repeat this whole routine.  You MUST repeat the j_d_defaults()/alter
   * parameters/jpeg_decompress() sequence, as some data structures allocated
   * in j_d_defaults are freed upon exit from jpeg_decompress.
   */
}


extern jmp_buf setjmp_buffer;	/* for return to caller */

Errcode jpeg_read_frame(FILE *input, struct ifileref *output)
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
	read(input, output);
	return Success;
	}
}


