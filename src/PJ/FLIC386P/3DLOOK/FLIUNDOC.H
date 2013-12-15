/* This describes some undocumented FlicLib routines.
 */

FlicRaster *pj_raster_center_virtual(FlicRaster *root,
								   FlicRaster *virt,
								   int width, int height);
/* This one make a virtual raster of width/height centered on the
 * root raster.  The advantage of a virtual raster is that it is
 * of known dimensions,   and it will clip to these dimensions.
 */

#ifdef __WATCOMC__
	#pragma aux (FLICLIB3S) pj_raster_center_virtual;
#endif /* __WATCOMC__ */


