#include "rastcall.ih"

void pj_xor_rast(Raster *source, Raster *dest)
/* xors one raster with raster of same dimensions */
{
	if(source->type == dest->type)
	{
		(source->lib->xor_rast[RL_TO_SAME])( source, dest );
		return;
	}
	else if(dest->type == RT_BYTEMAP)
	{
		(source->lib->xor_rast[RL_TO_BYTEMAP])( source, dest );
		return;
	}
	else if(source->type == RT_BYTEMAP)
	{
		(dest->lib->xor_rast[RL_FROM_BYTEMAP])( source, dest );
		return;
	}
	(source->lib->xor_rast[RL_TO_OTHER])( source, dest );
	return;
}
