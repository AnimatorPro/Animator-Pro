/*****************************************************************************
 * pocodraw.c - routines for poco to draw on the current frame
 *	12/08/90	(Ian)
 *				Fixed po_get_color_map(); its args were declared as int *
 *				types instead of Popot's.
 *	11/02/91	(Ian)
 *				SetColorMap() and SetScreenColorMap() now set dirties.
 ****************************************************************************/

#include "errcodes.h"
#include "ptrmacro.h"
#include "jimk.h"
#include "pocoface.h"
#include "pocolib.h"
#include "poly.h"
#include "inks.h"

extern Errcode builtin_err;
Errcode render_hline(register SHORT y,register SHORT x0,SHORT x1,Raster *r);
extern Poly working_poly;


/***** ink oriented graphics function (things that draw something in
  current color, ink, etc ****/

static void po_ink_dot(int x, int y)
/*****************************************************************************
 * void Dot(int x, int y);
 ****************************************************************************/
{
dirties();
render_dot(x,y);
}

static int po_get_dot(int x, int y)
/*****************************************************************************
 * int GetDot(int x, int y);
 ****************************************************************************/
{
return(pj_get_dot(vb.pencel, x, y));
}

void po_ink_line(int x1, int y1, int x2, int y2)
/*****************************************************************************
 * void Line(int x1, int y1, int x2, int y2);
 ****************************************************************************/
{
dirties();
render_line(x1,y1,x2,y2);
if (vs.cycle_draw) cycle_ccolor();
}

static void po_ink_box(int x, int y, int w, int h)
/*****************************************************************************
 * void Box(int x, int y, int w, int h);
 ****************************************************************************/
{
Rectangle rect;

rect.x = x;
rect.y = y;
rect.width = w;
rect.height = h;
free_render_cashes();		/* AAARRRR */
render_beveled_box(&rect, vs.box_bevel, vs.fillp);
make_render_cashes();
if (vs.cycle_draw) cycle_ccolor();
dirties();
}

static void po_ink_circle(int cx, int cy, int radius)
/*****************************************************************************
 * void Circle(int cx, int cy, int radius);
 ****************************************************************************/
{
extern render_dot(), render_brush();

dirties();
free_render_cashes();
rend_circ(cx,cy,radius);
make_render_cashes();
}

Errcode csd_render_poly(Poly *wply, Boolean filled, Boolean closed);

static Errcode po_some_poly(int ptcount, Popot *px, Popot *py, Boolean curved)
/*****************************************************************************
 * service routine
 ****************************************************************************/
{
Errcode err;
extern char curveflag;
Poly p;

if ((err  = po_arrays_to_poly(&p, ptcount, px, py)) < Success)
	return(err);
curveflag = curved;
err = csd_render_poly(&p, vs.fillp, vs.closed_curve);
pj_free(p.clipped_list);
curveflag = 0;
if (vs.cycle_draw) cycle_ccolor();
dirties();
return(err);
}

static Errcode po_ink_poly(int ptcount, Popot x, Popot y)
/*****************************************************************************
 * ErrCode Poly(int ptcount, int *x, int *y);
 ****************************************************************************/
{
return(po_some_poly(ptcount,&x,&y,FALSE));
}

static Errcode po_ink_spline(int ptcount, Popot x, Popot y)
/*****************************************************************************
 * ErrCode Spline(int ptcount, int *x, int *y);
 ****************************************************************************/
{
return(po_some_poly(ptcount,&x,&y,TRUE));
}

static Errcode po_ang_shape(int xcen, int ycen, int rad, double angle,
	int ppoints, int shape, int miscy)
/*****************************************************************************
 *
 ****************************************************************************/
{
Errcode err;
Poly poly;

clear_struct(&poly);
if (!make_sp_wpoly(&poly,
		xcen,ycen,rad,(int)(angle*1024.0/360.0),ppoints, shape, miscy))
	return(Err_no_memory);
err = csd_render_poly(&poly, vs.fillp, TRUE);
free_polypoints(&poly);
if (vs.cycle_draw) cycle_ccolor();
dirties();
return(err);
}

static Errcode po_ink_oval(
	double angle, int xcen, int ycen, int xrad, int yrad)
/*****************************************************************************
 * ErrCode Oval( double angle, int xcen, int ycen, int xrad, int yrad);
 ****************************************************************************/
{
extern int ell_points(int bothrad);

return(po_ang_shape(
	xcen,ycen,yrad,angle,ell_points(xrad+yrad),WP_ELLIPSE,xrad));
}

static Errcode po_ink_star(double angle, int xcen, int ycen, int rad)
/*****************************************************************************
 * ErrCode Star(double angle, int xcen, int ycen, int rad);
 ****************************************************************************/
{
return(po_ang_shape(
	xcen,ycen,rad,angle,vs.star_points,WP_STAR,vs.star_ratio));
}

static Errcode po_ink_petal(double angle, int xcen, int ycen, int rad)
/*****************************************************************************
 * ErrCode Petal(double angle, int xcen, int ycen, int rad);
 ****************************************************************************/
{
return(po_ang_shape(
	xcen,ycen,rad,angle,vs.star_points,WP_PETAL,vs.star_ratio));
}

static Errcode po_ink_rpoly(double angle, int xcen, int ycen, int rad)
/*****************************************************************************
 * ErrCode Rpoly(double angle, int xcen, int ycen, int rad);
 ****************************************************************************/
{
return(po_ang_shape(
	xcen,ycen,rad,angle,vs.star_points,WP_RPOLY,vs.star_ratio));
}

static Errcode po_ink_spiral(
	double angle, int xcen, int ycen, int rad, double turns)
/*****************************************************************************
 * ErrCode Spiral( double angle, int xcen, int ycen, int rad, double turns);
 ****************************************************************************/
{
Errcode err;

if (!make_spiral_poly(xcen,ycen,rad,(int)(angle*1024.0/320.0),
			(long)(turns*1024.0/320.0) ))
	return(Err_no_memory);
err = csd_render_poly(&working_poly, FALSE, FALSE);
free_polypoints(&working_poly);
if (vs.cycle_draw) cycle_ccolor();
dirties();
return(err);
}


static Errcode po_ink_fill(int x, int y)
/*****************************************************************************
 * ErrCode Fill(int x, int y);
 ****************************************************************************/
{
Errcode err;

err = csd_some_flood(x,y,0,pj_get_dot(vb.pencel,x,y),vb.pencel);
if (vs.cycle_draw) cycle_ccolor();
return(err);
}

static Errcode po_ink_fill_to(int x, int y, int to_color)
/*****************************************************************************
 * ErrCode FillTo(int x, int y, int to_color);
 ****************************************************************************/
{
Errcode err;

err = csd_some_flood(x,y,1,to_color,vb.pencel);
if (vs.cycle_draw) cycle_ccolor();
return(err);
}

static Errcode po_ink_edge(int color)
/*****************************************************************************
 * ErrCode Edge(int color);
 ****************************************************************************/
{
Errcode err;

save_undo();
err = csd_edge1(color);
if (vs.cycle_draw)
	cycle_redraw_ccolor();
return(err);
}


static void po_clear_pic(void)
/*****************************************************************************
 * void Clear(void);
 ****************************************************************************/
{
pj_set_rast(vb.pencel, vs.inks[0]);
dirties();
}


static void po_get_screen_dims(Popot width, Popot height)
/*****************************************************************************
 * void GetSize(int *width, int *height);
 ****************************************************************************/
{
if (width.pt == NULL || height.pt == NULL)
	{
	builtin_err = Err_null_ref;
	return;
	}
vass(width.pt,int) = vb.pencel->width;
vass(height.pt,int) = vb.pencel->height;
}

static void po_get_physical_size(Popot width, Popot height)
/*****************************************************************************
 * void GetPhysicalSize(int *width, int *height);
 ****************************************************************************/
{
if (width.pt == NULL || height.pt == NULL)
	{
	builtin_err = Err_null_ref;
	return;
	}
vass(width.pt,int)	= vb.screen->wndo.width;
vass(height.pt,int) = vb.screen->wndo.height;
}

static void po_get_aspect_ratio(Popot x, Popot y)
/*****************************************************************************
 * void GetAspectRatio(int *x, int *y);
 ****************************************************************************/
{
if (x.pt == NULL || y.pt ==  NULL)
	{
	builtin_err = Err_null_ref;
	return;
	}
vass(x.pt,int) = vb.pencel->aspect_dx;
vass(y.pt,int) = vb.pencel->aspect_dy;
}


static int po_get_ink_color(void)
/*****************************************************************************
 * int GetColor(void);
 ****************************************************************************/
{
return(vs.ccolor);
}

static void po_ink_color(int color)
/*****************************************************************************
 * void SetColor(int color);
 ****************************************************************************/
{
free_render_cashes();
vs.ccolor = color & 0xff;
make_render_cashes();
}

static void po_set_color_map(int color, int r, int g, int b)
/*****************************************************************************
 * void SetColorMap(int index, int r, int g, int b);
 ****************************************************************************/
{
Rgb3 *rgb;

dirties();			// added 11/02/91
color &= 0xff;
rgb = vb.pencel->cmap->ctab + color;
rgb->r = r&0xff;
rgb->g = g&0xff;
rgb->b = b&0xff;
pj_set_colors(vb.pencel, color, 1, (UBYTE *)rgb);
if (vs.ink_id != opq_INKID)
	{
	free_render_cashes();
	make_render_cashes();
	}
}

static void po_get_color_map(int color, Popot r, Popot g, Popot b)
/*****************************************************************************
 * void GetColorMap(int index, int *r, int *g, int *b);
 ****************************************************************************/
{
Rgb3 *rgb;

if (r.pt == NULL || g.pt == NULL || b.pt == NULL)
	{
	builtin_err = Err_null_ref;
	return;
	}

color &= 0xff;
rgb = vb.pencel->cmap->ctab + color;
vass(r.pt,int) = rgb->r;
vass(g.pt,int) = rgb->g;
vass(b.pt,int) = rgb->b;
}

static Errcode cmap_to_poco_cmap(int count, Rgb3 *source_rgb, Popot *d)
/*****************************************************************************
 * Convert a color map from byte-each RGB representation to int-each
 * representation for Poco.
 ****************************************************************************/
{
	int    i;
	int  *dest;
	UBYTE  *source;

	if (d->pt == NULL)
		return builtin_err = Err_null_ref;
	count *= 3;		/* do it for each component. */
	if (Popot_bufsize(d) < count*sizeof(int))
		return builtin_err = Err_index_big;
	dest = d->pt;
	source	= (UBYTE *)source_rgb;
	while (--count >= 0)
		*dest++ = *source++;
	return Success;
}

static void po_get_screen_color_map(Popot screen, Popot map)
/*****************************************************************************
 * GetScreenColorMap(Screen *screen, int *map);
 ****************************************************************************/
{
	Rcel *s;
	Cmap *cmap;

	if (screen.pt == NULL)
		s = vb.pencel;
	else
		s = (Rcel *)screen.pt;
	cmap = s->cmap;
	cmap_to_poco_cmap(cmap->num_colors, cmap->ctab, &map);
}

static Errcode poco_cmap_to_cmap(int count, Popot *po_cmap, Rgb3 *rgb)
/*****************************************************************************
 * Convert from int-each RGB component representation to 
 * byte-each reprentation.
 ****************************************************************************/
{
	int *source;
	UBYTE *dest;

	dest = (UBYTE *)rgb;
	if ((source = po_cmap->pt) == NULL)
		return builtin_err = Err_null_ref;
	count *= 3;
	if (Popot_bufsize(po_cmap) < count*sizeof(int))
		return builtin_err = Err_index_big;
	while (--count >= 0)
		*dest++ = *source++;
	return Success;
}

static void po_set_screen_color_map(Popot screen, Popot map)
/*****************************************************************************
 *
 ****************************************************************************/
{
	Rcel *s;
	Cmap *cmap;


	if (screen.pt == NULL)
		s = vb.pencel;
	else
		s = (Rcel *)screen.pt;
	if (s == vb.pencel)
		dirties();
	cmap = s->cmap;
	if ((poco_cmap_to_cmap(cmap->num_colors, &map, cmap->ctab)) >= Success)
		pj_cmap_load(s, s->cmap);
}

static int po_get_boxbevel(void)
/*****************************************************************************
 * int GetBoxBevel(void)
 *	 return the current bevel setting for the Box() builtin function/tool.
 ****************************************************************************/
{
	return vs.box_bevel;
}

static void po_set_boxbevel(int newbevel)
/*****************************************************************************
 * void SetBoxBevel(int newbevel)
 *	 set the new bevel for drawing via the Box() builtin function/tool.
 ****************************************************************************/
{
	if (newbevel < 0) {
		builtin_err = Err_parameter_range;
		return;
	}

	vs.box_bevel = newbevel;
}

static void po_set_cluster(int cluster_size, Popot pop_cluster)
/*****************************************************************************
 * void	 SetCluster(int cluster_size, int *cluster)
 *		Set the colors in the cluster.
 ****************************************************************************/
{
	struct bundle *b = &vs.buns[vs.use_bun];
	int i;
	int *pt;

	if (cluster_size <= 0)
		{
		builtin_err = Err_parameter_range;
		return;
		}
	if (Popot_bufcheck(&pop_cluster, cluster_size * sizeof(int)) < Success)
		return;
	if (cluster_size > Array_els(b->bundle))
		cluster_size = b->bun_count;
	pt = pop_cluster.pt;
	for (i=0; i<cluster_size; ++i)
		b->bundle[i] = *pt++;
	b->bun_count = cluster_size;
}

static Errcode po_get_cluster(Popot pop_cluster_size, Popot pop_cluster)
/*****************************************************************************
 * 	void GetCluster(int *cluster_size, int **cluster)
 *		Get the colors in the cluster.  
 ****************************************************************************/
{
	struct bundle *b = &vs.buns[vs.use_bun];
	int *pcluster_size;
	int cluster_size;
	Popot *pcluster;
	int *cluster;
	int i;

	if (pop_cluster_size.pt == NULL || pop_cluster.pt == NULL)
		return (builtin_err = Err_null_ref);
	pcluster_size = pop_cluster_size.pt;
	*pcluster_size = cluster_size = b->bun_count;
	pcluster = pop_cluster.pt;
	*pcluster = poco_lmalloc(cluster_size * sizeof(int));
	if ((cluster = pcluster->pt) == NULL)
		return Err_no_memory;
	for (i=0; i<cluster_size; ++i)
		*cluster++ = b->bundle[i];
	return Success;
}

void po_hls_to_rgb(int h, int l, int s, Popot r, Popot g, Popot b)
/*****************************************************************************
 * void	HLStoRGB(int h, int l, int s, int *r, int *g, int *b);
 * 		Convert hue, lightness, saturation values to RGB values.  Input
 * 		and output should be in range 0-255.
 ****************************************************************************/
{
SHORT sr, sg, sb;	/* Short result value for hls_to_rgb */

if (r.pt == NULL || g.pt == NULL || b.pt == NULL)
	{
	builtin_err = Err_null_ref;
	return;
	}
hls_to_rgb(&sr, &sg, &sb, h, l, s);
*((int *)r.pt) = sr;
*((int *)g.pt) = sg;
*((int *)b.pt) = sb;
}

void po_rgb_to_hls(int r, int g, int b, Popot h, Popot l, Popot s)
/*****************************************************************************
 * void	RGBtoHLS(int r, int g, int b, int *h, int *l, int *s);
 * 		Convert RGB to hue, lightness, saturation values.  Input
 * 		and output should be in range 0-255.
 ****************************************************************************/
{
SHORT sh, sl, ss;	/* Short result valur for rgb_to_hls */
if (h.pt == NULL || l.pt == NULL || s.pt == NULL)
	{
	builtin_err = Err_null_ref;
	return;
	}
rgb_to_hls(r, g, b, &sh, &sl, &ss);
*((int *)h.pt) = sh;
*((int *)l.pt) = sl;
*((int *)s.pt) = ss;
}

int po_closest_color_in_screen(Popot screen, int r, int g, int b)
/*****************************************************************************
 * int	ClosestColorInScreen(Screen *s, int r, int g, int b);
 * 		Find closest color in screen's color map to rgb.
 ****************************************************************************/
{
	Rcel *s;
	Cmap *cmap;
	UBYTE  *pctab;
	Rgb3 rgb;

	if (screen.pt == NULL)
		s = vb.pencel;
	else
		s = (Rcel *)screen.pt;
	rgb.r = r;
	rgb.g = g;
	rgb.b = b;
	cmap = s->cmap;
	return closestc(&rgb, cmap->ctab, cmap->num_colors);
}

static int filter_duplicate_colors(Rgb3 *source, int source_count, Rgb3 *dest)
/*****************************************************************************
 * Copy colors from source to dest but omit duplicate colors.  Source and
 * dest may be the same.  Returns # of unique colors.
 ****************************************************************************/
{
	int i;
	int dest_count = 0;

	for (i=0; i<source_count; ++i)
		{
		if (!in_ctable(&source[i], dest, dest_count))
			{
			dest[dest_count++] = source[i];
			}
		}
	return dest_count;
}

ErrCode po_squeeze_colors(Popot source_map, int source_count, Popot dest_map
, int dest_count)
/*****************************************************************************
 * ErrCode	SqueezeColors(int *source_map
 * , int source_count, int *dest_map, int dest_count);"
 *     Reduce the number of colors in source_map to dest_count.  
 *	   Put color map with this number of colors in dest_map.
 ****************************************************************************/
{
	Rgb3 *source_rgb;
	Rgb3 *dest_rgb;
	Errcode err = Err_no_memory;
	int dest_alloc;

	if (source_count <= 0 || dest_count <= 0)
		return Err_parameter_range;
	dest_alloc = dest_count;
	if (dest_alloc < COLORS)
		dest_alloc = COLORS;
	if ((source_rgb = pj_malloc(source_count * sizeof(Rgb3))) != NULL)
		{
		if ((err = poco_cmap_to_cmap(source_count, &source_map, source_rgb))
		>= Success)
			{
			if ((dest_rgb = pj_malloc(dest_alloc * sizeof(Rgb3))) != NULL)
				{
				source_count  = filter_duplicate_colors(source_rgb
				, source_count, source_rgb);
				pack_ctable(source_rgb, source_count, dest_rgb, dest_count);
				err = cmap_to_poco_cmap(dest_count, dest_rgb, &dest_map);
				pj_free(dest_rgb);
				}
			pj_free(source_rgb);
			}
		}
	return err;
}

Errcode po_fit_screen_to_color_map(Popot screen
, Popot new_colors, Boolean keep_key)
/*****************************************************************************
 * ErrCode	FitScreenToColorMap(Screen *s, int *new_colors, Boolean keep_key);
 *		Update the screen's color map with new_colors,  and remap the
 *		pixel values so that the screen looks as much as possible the
 *		same as it ever did.
 *	    If keep_key is TRUE then pixels the key-color will not be changed.
 ****************************************************************************/
{
	Cmap new_cmap;
	Rcel *s;
	Errcode err;
	UBYTE ctable[COLORS];

	if ((s = screen.pt) == NULL)
		return builtin_err = Err_null_ref;
	new_cmap.num_colors = COLORS;
	if ((err = poco_cmap_to_cmap(COLORS, &new_colors, new_cmap.ctab)) 
	>= Success)
		{
		make_cfit_table(s->cmap->ctab, new_cmap.ctab, (Pixel *)ctable
		,	keep_key ? vs.inks[0] : -1);
		xlat_rast(s, ctable, 1);
		*(s->cmap) = new_cmap;
		pj_cmap_load(s, &new_cmap);
		if (s == vb.pencel)
			dirties();
		}
	return err;
}


/*---------------------------------------------------------------------------- 
 * library protos...
 *
 * Maintenance notes:
 *	To add a function to this library, add the pointer and prototype string
 *	TO THE END of the following list.  Then go to POCOLIB.H and add a
 *	corresponding prototype to the library structure typedef therein which
 *	matches the name of the structure below.  When creating the prototype
 *	in POCOLIB.H, remember that all arguments prototyped below as pointers
 *	must be defined as Popot types in the prototype in pocolib.h; any number
 *	of stars in the Poco proto still equate to a Popot with no stars in the
 *	pocolib.h prototype.
 *
 *	DO NOT ADD NEW PROTOTYPES OTHER THAN AT THE END OF AN EXISTING STRUCTURE!
 *	DO NOT DELETE A PROTOTYPE EVER! (The best you could do would be to point
 *	the function to a no-op service routine).  Breaking these rules will
 *	require the recompilation of every POE module in the world.  These
 *	rules apply only to library functions which are visible to POE modules
 *	(ie, most of them).  If a specific typedef name exists in pocolib.h, the
 *	rules apply.  If the protos are coded as a generic array of Lib_proto
 *	structs without an explicit typedef in pocolib.h, the rules do not apply.
 *--------------------------------------------------------------------------*/

PolibDraw po_libdraw = {
po_get_screen_dims,
	"void    GetSize(int *width, int *height);",
po_get_aspect_ratio,
	"void    GetAspectRatio(int *x, int *y);",
po_get_ink_color,
	"int     GetColor(void);",
po_ink_color,
	"void    SetColor(int color);",
po_clear_pic,
	"void    Clear(void);",
po_ink_dot,
	"void    Dot(int x, int y);",
po_get_dot,
	"int     GetDot(int x, int y);",
po_ink_line,
	"void    Line(int x1, int y1, int x2, int y2);",
po_ink_box,
	"void    Box(int x, int y, int w, int h);",
po_ink_circle,
	"void    Circle(int cx, int cy, int radius);",
po_ink_poly,
	"ErrCode Poly(int ptcount, int *x, int *y);",
po_ink_spline,
	"ErrCode Spline(int ptcount, int *x, int *y);",
po_ink_oval,
	"ErrCode Oval(double angle, int xcen, int ycen, int xrad, int yrad);",
po_ink_star,
	"ErrCode Star(double angle, int xcen, int ycen, int rad);",
po_ink_petal,
	"ErrCode Petal(double angle, int xcen, int ycen, int rad);",
po_ink_rpoly,
	"ErrCode Rpoly(double angle, int xcen, int ycen, int rad);",
po_ink_spiral,
	"ErrCode Spiral(double angle, int xcen, int ycen, int rad, double turns);",
po_ink_fill,
	"ErrCode Fill(int x, int y);",
po_ink_fill_to,
	"ErrCode FillTo(int x, int y, int to_color);",
po_ink_edge,
	"ErrCode Edge(int color);",
po_set_color_map,
	"void    SetColorMap(int index, int r, int g, int b);",
po_get_color_map,
	"void    GetColorMap(int index, int *r, int *g, int *b);",
po_get_screen_color_map,
	"void    GetScreenColorMap(Screen *s, int *maparray);",
po_set_screen_color_map,
	"void    SetScreenColorMap(Screen *s, int *maparray);",
po_get_physical_size,
	"void    GetPhysicalSize(int *width, int *height);",
po_get_boxbevel,
	"int     GetBoxBevel(void);",
po_set_boxbevel,
	"void    SetBoxBevel(int new_bevel);",
po_set_cluster,
	"void	 SetCluster(int cluster_size, int *cluster);",
po_get_cluster,
	"ErrCode GetCluster(int *cluster_size, int **cluster);",
po_hls_to_rgb,
	"void	HLStoRGB(int h, int l, int s, int *r, int *g, int *b);",
po_rgb_to_hls,
	"void	RGBtoHLS(int r, int g, int b, int *h, int *l, int *s);",
po_closest_color_in_screen,
	"int	ClosestColorInScreen(Screen *s, int r, int g, int b);",
po_squeeze_colors,
	"ErrCode	SqueezeColors(int *source_map, int source_count, int *dest_map, int dest_count);",
po_fit_screen_to_color_map,
	"ErrCode	FitScreenToColorMap(Screen *s, int *new_colors, Boolean keep_key);",
};

Poco_lib po_draw_lib =
	{
	NULL, "Graphics",
	(Lib_proto *)&po_libdraw, POLIB_DRAW_SIZE,
	};

