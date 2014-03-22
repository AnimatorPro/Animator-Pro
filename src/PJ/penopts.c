#include "jimk.h"
#include "brush.h"
#include "options.h"

extern Errcode box_tool(), circle_tool(), 
	draw_tool(), drizl_tool(), edge_tool(),
	fill_tool(), flood_tool(), gel_tool(), line_tool(), 
	move_tool(), copy_tool(), ovalf_tool(), petlf_tool(), 
	polyf_tool(), rpolyf_tool(), 
	sep_tool(), shapef_tool(), spiral_tool(), curve_tool(), spray_tool(), 
	starf_tool(), streak_tool(), text_tool();

extern Button text_group_sel, om_sratio_group_sel, om_osped_group_sel,
 curve_group_sel, fill2c_group_sel, sep_group_sel,
 om_points_group_sel, freepoly_group_sel, om_sratio_group_sel,
 fill2c_group_sel, fill2c_group_sel, box_group_sel, move_group_sel;

static void close_static_ptool(Option_tool *tool);

extern Cursorhdr fill_cursor;
extern Cursorhdr pen_cursor;
extern Cursorhdr shape_cursor;
extern Cursorhdr text_cursor;
extern Cursorhdr box_cursor;
extern Cursorhdr move_tool_cursor;
extern Cursorhdr sep_cursor;
extern Cursorhdr edge_cursor;
extern Cursorhdr pick_cursor;
extern Cursorhdr spray_cursor;
extern Cursorhdr star_cursor;
extern Cursorhdr plain_ptool_cursor;




extern void test_ptfunc();
extern void zoom_unundo();

#ifdef TESTING
	static Pentool test_ptool_opt = PTOOLINIT1(	
		NONEXT,	
		"Test",	
		PTOOL_OPT,	
		TEST_PTOOL,
		"For testing whatever",
		NO_SUBOPTS,
		close_static_ptool,
		test_ptfunc,	
		&plain_ptool_cursor,
		NOINSTALL,
		zoom_unundo
	); 
	#define LAST_TOOL &test_ptool_opt
#else
	#define LAST_TOOL NONEXT
#endif /* TESTING */

static Pentool text_ptool_opt = PTOOLINIT0(	
	LAST_TOOL,
	RL_KEYTEXT("text_n"),
	PTOOL_OPT,	
	TEXT_PTOOL,
	RL_KEYTEXT("text_help"),
	&text_group_sel,
	&text_cursor,
	text_tool,	
	close_static_ptool
); 
#undef LAST_TOOL
static Pentool streak_ptool_opt = PTOOLINIT0(	
	&text_ptool_opt,	
	RL_KEYTEXT("streak_n"),
	PTOOL_OPT,	
	STREAK_PTOOL,
	RL_KEYTEXT("streak_help"),
	&pen_brush_group,
	&pen_cursor,	
	streak_tool,	
	close_static_ptool
); 
static Pentool starf_ptool_opt = PTOOLINIT0(	
	&streak_ptool_opt,	
	RL_KEYTEXT("star_n"),
	PTOOL_OPT,	
	STARF_PTOOL,
	RL_KEYTEXT("star_help"),
	&om_sratio_group_sel,
	&star_cursor,	
	starf_tool,
	close_static_ptool
); 
static Pentool spray_ptool_opt = PTOOLINIT0(	
	&starf_ptool_opt,	
	RL_KEYTEXT("spray_n"),
	PTOOL_OPT,	
	SPRAY_PTOOL,
	RL_KEYTEXT("spray_help"),
	&om_osped_group_sel,
	&spray_cursor,	
	spray_tool,
	close_static_ptool
); 
static Pentool curve_ptool_opt = PTOOLINIT0(	
	&spray_ptool_opt,	
	RL_KEYTEXT("spline_n"),
	PTOOL_OPT,	
	CURVE_PTOOL,
	RL_KEYTEXT("spline_help"),
	&curve_group_sel,
	&shape_cursor,	
	curve_tool,
	close_static_ptool
); 
static Pentool spiral_ptool_opt = PTOOLINIT0(	
	&curve_ptool_opt,	
	RL_KEYTEXT("spiral_n"),
	PTOOL_OPT,	
	SPIRAL_PTOOL,
	RL_KEYTEXT("spiral_help"),
	NO_SUBOPTS,
	&pen_cursor,	
	spiral_tool,
	close_static_ptool
); 
static Pentool shapef_ptool_opt = PTOOLINIT0(	
	&spiral_ptool_opt,	
	RL_KEYTEXT("shape_n"),
	PTOOL_OPT,	
	SHAPEF_PTOOL,
	RL_KEYTEXT("shape_help"),
	&fill2c_group_sel,
	&shape_cursor,	
	shapef_tool,
	close_static_ptool
); 
Pentool sep_ptool_opt = PTOOLINIT0(	
	&shapef_ptool_opt,	
	RL_KEYTEXT("sep_n"),
	PTOOL_OPT,	
	SEP_PTOOL,
	RL_KEYTEXT("sep_help"),
	&sep_group_sel,
	&sep_cursor,	
	sep_tool,
	close_static_ptool
); 
static Pentool rpolyf_ptool_opt = PTOOLINIT0(	
	&sep_ptool_opt,	
	RL_KEYTEXT("rpoly_n"),
	PTOOL_OPT,	
	RPOLYF_PTOOL,	
	RL_KEYTEXT("rpoly_help"),
	&om_points_group_sel,
	&shape_cursor,	
	rpolyf_tool,
	close_static_ptool
); 
static Pentool polyf_ptool_opt = PTOOLINIT0(	
	&rpolyf_ptool_opt,	
	RL_KEYTEXT("poly_n"),
	PTOOL_OPT,	
	POLYF_PTOOL,
	RL_KEYTEXT("poly_help"),
	&freepoly_group_sel,
	&shape_cursor,	
	polyf_tool,
	close_static_ptool
); 
static Pentool petlf_ptool_opt = PTOOLINIT0(	
	&polyf_ptool_opt,	
	RL_KEYTEXT("petal_n"),
	PTOOL_OPT,	
	PETLF_PTOOL,
	RL_KEYTEXT("petal_help"),
	&om_sratio_group_sel,
	&shape_cursor,	
	petlf_tool,
	close_static_ptool
); 
static Pentool ovalf_ptool_opt = PTOOLINIT0(	
	&petlf_ptool_opt,	
	RL_KEYTEXT("oval_n"),
	PTOOL_OPT,	
	OVALF_PTOOL,
	RL_KEYTEXT("oval_help"),
	&fill2c_group_sel,
	&shape_cursor,
	ovalf_tool,
	close_static_ptool
); 
static Pentool move_ptool_opt = PTOOLINIT0(	
	&ovalf_ptool_opt,	
	RL_KEYTEXT("move_n"),
	PTOOL_OPT,	
	MOVE_PTOOL,
	RL_KEYTEXT("move_help"),
	&move_group_sel,
	&move_tool_cursor,	
	move_tool,
	close_static_ptool
); 
static Pentool line_ptool_opt = PTOOLINIT0(	
	&move_ptool_opt,	
	RL_KEYTEXT("line_n"),
	PTOOL_OPT,	
	LINE_PTOOL,
	RL_KEYTEXT("line_help"),
	&pen_brush_group,
	&pen_cursor,	
	line_tool,
	close_static_ptool
); 
static Pentool gel_ptool_opt = PTOOLINIT0(	
	&line_ptool_opt,	
	RL_KEYTEXT("gel_n"),
	PTOOL_OPT,	
	GEL_PTOOL,
	RL_KEYTEXT("gel_help"),
	&gel_brush_group,
	&pen_cursor,	
	gel_tool,
	close_static_ptool
); 
static Pentool flood_ptool_opt = PTOOLINIT0(	
	&gel_ptool_opt,	
	RL_KEYTEXT("fillto_n"),
	PTOOL_OPT,	
	FLOOD_PTOOL,
	RL_KEYTEXT("fillto_help"),
	NO_SUBOPTS,
	&pick_cursor,	
	flood_tool,
	close_static_ptool
); 
static Pentool fill_ptool_opt = PTOOLINIT0(	
	&flood_ptool_opt,	
	RL_KEYTEXT("fill_n"),
	PTOOL_OPT,	
	FILL_PTOOL,
	RL_KEYTEXT("fill_help"),
	NO_SUBOPTS,
	&fill_cursor,	
	fill_tool,
	close_static_ptool
); 
static Pentool edge_ptool_opt = PTOOLINIT0(	
	&fill_ptool_opt,	
	RL_KEYTEXT("edge_n"),
	PTOOL_OPT,	
	EDGE_PTOOL,
	RL_KEYTEXT("edge_help"),
	&pen_brush_group,
	&edge_cursor,	
	edge_tool,
	close_static_ptool
); 
static Pentool driz_ptool_opt = PTOOLINIT0(	
	&edge_ptool_opt,	
	RL_KEYTEXT("driz_n"),
	PTOOL_OPT,	
	DRIZ_PTOOL,
	RL_KEYTEXT("driz_help"),
	&pen_brush_group,
	&pen_cursor,
	drizl_tool,
	close_static_ptool
); 
static Pentool draw_ptool_opt = PTOOLINIT0(	
	&driz_ptool_opt,	
	RL_KEYTEXT("draw_n"),
	PTOOL_OPT,	
	DRAW_PTOOL,
	RL_KEYTEXT("draw_help"),
	&pen_brush_group,
	&pen_cursor,
	draw_tool,
	close_static_ptool
); 
static Pentool copy_ptool_opt = PTOOLINIT0(	
	&draw_ptool_opt,	
	RL_KEYTEXT("copy_n"),
	PTOOL_OPT,	
	COPY_PTOOL,
	RL_KEYTEXT("copy_help"),
	&move_group_sel,
	&move_tool_cursor,	
	copy_tool,
	close_static_ptool
); 
static Pentool circle_ptool_opt = PTOOLINIT0(	
	&copy_ptool_opt,	
	RL_KEYTEXT("circle_n"),
	PTOOL_OPT,	
	CIRCLE_PTOOL,
	RL_KEYTEXT("circle_help"),
	&fill2c_group_sel,
	&shape_cursor,
	circle_tool,
	close_static_ptool
); 
Pentool box_ptool_opt = PTOOLINIT0(	
	&circle_ptool_opt,	
	RL_KEYTEXT("box_n"),
	PTOOL_OPT,	
	BOX_PTOOL,
	RL_KEYTEXT("box_help"),
	&box_group_sel,
	&box_cursor,
	box_tool,
	close_static_ptool
); 


/* in the future we may have tools loaded from files or some such.
 * this is a start on what would be needed ***/

static Option_tool *static_ptools = (Option_tool *)&box_ptool_opt;
static void *tool_ss = NULL;
Option_tool *ptool_list = NULL;

static void close_static_ptool(Option_tool *tool)
{
	tool->next = static_ptools;
	static_ptools = tool;
}

Errcode init_ptools()
{
Errcode err;
Option_tool *tool;

	while(static_ptools)
	{
		tool = static_ptools;
		static_ptools = tool->next;
		tool->next = ptool_list;
		ptool_list = tool;
	}

	if((err = load_option_names((Option_tool *)ptool_list, "tool_texts",
						         &tool_ss, TRUE)) < Success)
	{
		goto error;	
	}
	ptool_list = (Option_tool *)sort_names((Names *)ptool_list);
error:
	return(err);
}
void cleanup_ptools()
{
	close_option_tools(&ptool_list);
	smu_free_scatters(&tool_ss);
}

void attatch_tools(void)
{
extern Button pen_opts_sel;
int i;
int id;
Button *ob;
Option_tool *o;
Pentool *ptool;

static Pentool *default_ptools[] = {
	&draw_ptool_opt,	
	&polyf_ptool_opt,	
	&spray_ptool_opt,	
	&box_ptool_opt,
	&text_ptool_opt,	
	&fill_ptool_opt,	
	&text_ptool_opt,	
	&fill_ptool_opt,	
};

	ob = &pen_opts_sel;
	for (i=0; i<8; i++)
	{
		id = vs.tool_slots[i];
		if(NULL == (o = id_find_option(ptool_list, id)))
			o = (Option_tool *)default_ptools[i];
		ob->identity = o->id;
		ob->datme = o;
		ob = ob->next;
	}
	if(NULL == (ptool = id_find_option(ptool_list, vs.ptool_id)))
		set_quickptool(default_ptools[0]);
	else
		set_quickptool(ptool);
}
