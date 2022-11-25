#include "jinclude.h"
#include "errcodes.h"
#include "rcel.h"
#include "jpegpdr.h"
#include <setjmp.h>

static int ret_width, ret_height;

METHODDEF void
output_init (decompress_info_ptr cinfo)
/* This routine should do any setup required */
{
ret_width = cinfo->image_width;
ret_height = cinfo->image_height;
bail_out(Err_abort);
}

METHODDEF void
put_pixel_rows (decompress_info_ptr cinfo, int num_rows, JSAMPIMAGE pixel_data)
/* Write some rows of output data */
{
}

METHODDEF void
output_term (decompress_info_ptr cinfo)
/* Finish up at the end of the output */
{
}

METHODDEF void
d_ui_method_selection (decompress_info_ptr cinfo)
{
  /* select output routines */
  cinfo->methods->output_init = output_init;
  cinfo->methods->put_pixel_rows = put_pixel_rows;
  cinfo->methods->output_term = output_term;
}

static void get_res(FILE *input, int *pwidth, int *pheight)
{
  struct Decompress_info_struct cinfo;
  struct Decompress_methods_struct dc_methods;
  struct External_methods_struct e_methods;

  cinfo.output_file = NULL;
  cinfo.input_file = input;
  cinfo.methods = &dc_methods;	/* links to method structs */
  cinfo.emethods = &e_methods;
  set_error_methods(&e_methods);
  jselmemmgr(&e_methods);	/* select std memory allocation routines */
  dc_methods.d_ui_method_selection = d_ui_method_selection;
  j_d_defaults(&cinfo, TRUE);
  jselrjfif(&cinfo);
  jpeg_decompress(&cinfo);
}



extern jmp_buf setjmp_buffer;	/* for return to caller */

static Errcode file_get_res(FILE *input, int *pwidth, int *pheight)
{
Errcode err;

if (err = setjmp(setjmp_buffer)) 
	{
	/* We'll always get here since the init_input routine forces an
	 * error to avoid processing the whole file. */
	if (err != Err_abort)
		return err;
	else
		{
		*pwidth = ret_width;
		*pheight = ret_height;
		return Success;
		}
	}
else
	{
	get_res(input, pwidth, pheight);
	return Success;		/* Should never get here. */
	}
}

Errcode jpeg_get_res(char *name, int *pwidth, int *pheight)
{
FILE *input;
Errcode err;

if ((input = fopen(name, "rb")) != NULL)
	{
	err = file_get_res(input, pwidth, pheight);
	fclose(input);
	}
else
	{
	err = pj_errno_errcode();
	}
return err;
}

