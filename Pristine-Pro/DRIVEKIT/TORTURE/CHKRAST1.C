#include "torture.h"

static char *rastlib_fmt_strings[] = {
	 "    close_raster    = %p  %s\n",
	 "    cput_dot        = %p  %s\n",
	 "    put_dot         = %p  %s\n",
	 "    cget_dot        = %p  %s\n",
	 "    get_dot         = %p  %s\n",
	 "    put_hseg        = %p  %s\n",
	 "    get_hseg        = %p  %s\n",
	 "    put_vseg        = %p  %s\n",
	 "    get_vseg        = %p  %s\n",
	 "    put_rectpix     = %p  %s\n",
	 "    get_rectpix     = %p  %s\n",
	 "    set_hline       = %p  %s\n",
	 "    set_vline       = %p  %s\n",
	 "    set_rect        = %p  %s\n",
	 "    set_rast        = %p  %s\n",
	 "    xor_rect        = %p  %s\n",
	 "    mask1blit       = %p  %s\n",
	 "    mask2blit       = %p  %s\n",
	 "    unbrun_rect     = %p  %s\n",
	 "    unlccomp_rect   = %p  %s\n",
	 "    unss2_rect      = %p  %s\n",
	 "    reserved1       = %p  %s\n",
	 "    reserved2       = %p  %s\n",
	 "    blit_in_card    = %p  %s\n",
	 "    blit_to_ram     = %p  %s\n",
	 "    blit_from_ram   = %p  %s\n",
	 "    blit_reserved   = %p  %s\n",
	 "    swap_in_card    = %p  %s\n",
	 "    swap_to_ram     = %p  %s\n",
	 "    swap_from_ram   = %p  %s\n",
	 "    swap_reserved   = %p  %s\n",
	 "    tblit_in_card   = %p  %s\n",
	 "    tblit_to_ram    = %p  %s\n",
	 "    tblit_from_ram  = %p  %s\n",
	 "    tblit_reserved  = %p  %s\n",
	 "    xor_in_card     = %p  %s\n",
	 "    xor_to_ram      = %p  %s\n",
	 "    xor_from_ram    = %p  %s\n",
	 "    xor_reserved    = %p  %s\n",
	 "    zoom_in_card    = %p  %s\n",
	 "    zoom_to_ram     = %p  %s\n",
	 "    zoom_from_ram   = %p  %s\n",
	 "    zoom_reserved   = %p  %s\n",
	 "    set_colors      = %p  %s\n",
	 "    uncc64          = %p  %s\n",
	 "    uncc256         = %p  %s\n",
	 "    wait_vsync      = %p  %s\n",
	};

#define RASTLIB_FUNC_COUNT (sizeof(rastlib_fmt_strings) / sizeof(char *))

void check_raster_sanity(Raster *r, USHORT rastnum)
/*****************************************************************************
 * perform sanity check on Raster and Rastlib structures.
 ****************************************************************************/
{
Rastlib 	*rlib = r->lib;
void		**glib_array = (void **)(tcb.vd->grclib);
void		**rlib_array = (void **)(rlib);
USHORT		counter;
char		rastname[50];
char		fromdriver[] = "* (Driver  Library)";
char		fromgrc[]	 = "  (Generic Library)";
char		*fromlib;
void		*libroutine;

	if (rastnum == RASTER_PRIMARY)
		sprintf(rastname,"primary display raster");
	else
		sprintf(rastname,"secondary raster %hu",rastnum);

	/*------------------------------------------------------------------------
	 * print the contents of the raster and rastlib structures...
	 *----------------------------------------------------------------------*/

	log_data("  Contents of Raster structure for %s:\n"
			 "    type            = %hd\n"
			 "    pdepth          = %hd\n"
			 "    lib             = %p\n"
			 "    aspect_dx       = %hd\n"
			 "    aspect_dy       = %hd\n"
			 "    width           = %hu\n"
			 "    height          = %hu\n"
			 "    x               = %hd\n"
			 "    y               = %hd\n\n",
			 rastname,
			 r->type,
			 r->pdepth,
			 r->lib,
			 r->aspect_dx,
			 r->aspect_dy,
			 r->width,
			 r->height,
			 r->x,
			 r->y
			);

	log_data("  Contents of Rastlib structure for %s:\n", rastname);
	for (counter = 0; counter < RASTLIB_FUNC_COUNT; counter++)
		{
		if (rlib_array[counter] == NULL ||
			rlib_array[counter] == glib_array[counter])
			{
			libroutine = glib_array[counter];
			fromlib = fromgrc;
			}
		else
			{
			libroutine = rlib_array[counter];
			fromlib = fromdriver;
			}
		log_data(rastlib_fmt_strings[counter], libroutine, fromlib);
		rlib_array[counter] = libroutine;
		}
	log_data("\n");

	/*-----------------------------------------------------------------------
	 * do sanity check on Raster structure...
	 *---------------------------------------------------------------------*/

	log_progress("Verifying contents of Raster structure for %s...\n", rastname);

	if (r->type < tcb.vd->first_rtype)
		{
		log_error("Raster structure element 'type' has a value of %d, it should\n"
				  "be greater than or equal to 'vdev->first_rtype'.\n", r->type);
		return;
		}

	if (r->pdepth != 8)
		{
		log_error("Raster structure element 'pdepth' currently must be 8.\n");
		return;
		}

	if (r->width == 0 || r->height == 0)
		{
		log_error("Raster structure elements 'width' and 'height'"
					"must not be zero.\n");
		}

	if (r->aspect_dx == 0 || r->aspect_dy == 0)
		{
		log_warning("Raster structure elements 'aspect_dx' and 'aspect_dy'"
					"should not be zero.\n");
		}

	if (rlib == NULL)
		{
		log_error("Raster structure element 'lib' must not be a NULL pointer.\n");
		return;
		}

	log_progress("...verification of Raster structure complete.\n\n");

	/*-----------------------------------------------------------------------
	 * do sanity check on Rastlib structure...
	 *---------------------------------------------------------------------*/

	log_progress("Verifying contents of Rastlib structure for %s...\n",rastname);

	if (rlib->set_colors == NULL)
		log_error("Required raster function 'set_colors' not provided by driver.\n");

	if (rlib->get_dot == NULL)
		log_error("Required raster function 'get_dot' not provided by driver.\n");

	if (rlib->put_dot == NULL)
		log_error("Required raster function 'put_dot' not provided by driver.\n");


	log_progress("...verification of Rastlib structure complete.\n\n");

}

void test_close_raster(Raster *r)
/*****************************************************************************
 *
 ****************************************************************************/
{
	if (r == NULL || r->lib == NULL || r->lib->close_raster == NULL ||
		is_generic(r->lib, close_raster))
		return;

	log_progress("Testing close_raster()...\n");
	r->lib->close_raster(r);
	log_progress("...testing of close_raster() completed.\n\n");
}
