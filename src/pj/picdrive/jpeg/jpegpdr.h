/* jpegpdr.h - This contains mostly glue between the PDR side and
 * the jpeg side.
 */

struct ifileref
/*
 * This is the input structure to the JPEG compressor.
 */
	{
	Rcel *image;
	Pixel *line_buf;
	int y;
	};

Errcode jpeg_save_frame(struct ifileref *input, FILE *output);
Errcode jpeg_read_frame(FILE *input, struct ifileref *output);
Errcode jpeg_get_res(char *file_name, int *pwidth, int *pheight);

void set_error_methods(external_methods_ptr e_methods);
void bail_out(Errcode err);

