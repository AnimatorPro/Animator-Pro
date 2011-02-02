#ifdef CCODE
static void tblit_line(Pixel *source_buf, Pixel *dest_buf, 
					   Coor width, const Tcolxldat *tcx)
/* (Private to grc_driver.) */
{
Pixel pix;
Pixel *maxdest;
Pixel tcolor = tcx->tcolor;

	maxdest = dest_buf + width;
	while (dest_buf < maxdest)
	{
		if ((pix = *source_buf++) == tcolor)
			dest_buf++;
		else
			*dest_buf++ = pix;
	}
}
#endif /* CCODE */
