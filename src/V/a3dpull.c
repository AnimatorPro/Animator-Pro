/* a3dpull.c - pull-down system for optics */

#include "jimk.h"
#include "flicmenu.h"
#include "a3d.h"
#include "a3dpull.str"

			static Pull ado_outline_pull = 
				{
				NONEXT,
				1, 1+5*CH_HEIGHT,
				3+10*CH_WIDTH,1+CH_HEIGHT,
				NOCHILD,
				a3dpull_100 /* " OUTLINE" */,
				pull_text,
				};
			static Pull ado_dots2_pull = 
				{
				&ado_outline_pull,
				1, 1+4*CH_HEIGHT,
				3+10*CH_WIDTH,1+CH_HEIGHT,
				NOCHILD,
				a3dpull_101 /* "----------" */,
				pull_text,
				};
			static Pull ado_spline_pull = 
				{
				&ado_dots2_pull,
				1, 1+3*CH_HEIGHT,
				3+10*CH_WIDTH,1+CH_HEIGHT,
				NOCHILD,
				a3dpull_102 /* " SPLINE" */,
				pull_text,
				};
			static Pull ado_poly_pull = 
				{
				&ado_spline_pull,
				1, 1+2*CH_HEIGHT,
				3+10*CH_WIDTH,1+CH_HEIGHT,
				NOCHILD,
				a3dpull_103 /* " POLYGON" */,
				pull_text,
				};
			static Pull ado_cel_pull = 
				{
				&ado_poly_pull,
				1, 1+1*CH_HEIGHT,
				3+10*CH_WIDTH,1+CH_HEIGHT,
				NOCHILD,
				a3dpull_104 /* " CEL" */,
				pull_text,
				};
			static Pull ado_frames_pull = 
				{
				&ado_cel_pull,
				1, 1+0*CH_HEIGHT,
				3+10*CH_WIDTH,1+CH_HEIGHT,
				NOCHILD,
				a3dpull_105 /* " FLIC" */,
				pull_text,
				};
		static Pull rtest_pull =
			{
			NONEXT,
			-1,CH_HEIGHT,
			5+10*CH_WIDTH, 3+6*CH_HEIGHT,
			&ado_frames_pull,
			NODATA,
			pull_oblock,
			};
static Pull test_pull =
	{
	NONEXT,
	30*CH_WIDTH, -1,
	8*CH_WIDTH,CH_HEIGHT,
	&rtest_pull,
	a3dpull_106 /* "ELEMENT" */,
	pull_text,
	};
			static Pull ado_complete_pull =
				{
				NULL,
				1, 1+5*CH_HEIGHT,
				3+13*CH_WIDTH,1+CH_HEIGHT,
				NOCHILD,
				a3dpull_107 /* " COMPLETE" */,
				pull_text,
				};
			static Pull ado_reverse_pull =
				{
				&ado_complete_pull,
				1, 1+4*CH_HEIGHT,
				3+13*CH_WIDTH,1+CH_HEIGHT,
				NOCHILD,
				a3dpull_108 /* " REVERSE" */,
				pull_text,
				};
			static Pull ado_pong_pull =
				{
				&ado_reverse_pull,
				1, 1+3*CH_HEIGHT,
				3+13*CH_WIDTH,1+CH_HEIGHT,
				NOCHILD,
				a3dpull_109 /* " PING-PONG" */,
				pull_text,
				};
			static Pull ado_still_pull =
				{
				&ado_pong_pull,
				1, 1+2*CH_HEIGHT,
				3+13*CH_WIDTH,1+CH_HEIGHT,
				NOCHILD,
				a3dpull_110 /* " STILL" */,
				pull_text,
				};
			static Pull ado_easeo_pull = 
				{
				&ado_still_pull,
				1, 1+1*CH_HEIGHT,
				3+13*CH_WIDTH,1+CH_HEIGHT,
				NOCHILD,
				a3dpull_111 /* " OUT SLOW" */,
				pull_text,
				};
			static Pull ado_ease_pull = 
				{
				&ado_easeo_pull,
				1, 1+0*CH_HEIGHT,
				3+13*CH_WIDTH,1+CH_HEIGHT,
				NOCHILD,
				a3dpull_112 /* " IN SLOW" */,
				pull_text,
				};
		static Pull roptions_pull =
			{
			NONEXT,
			-1,CH_HEIGHT,
			5+13*CH_WIDTH, 3+6*CH_HEIGHT,
			&ado_ease_pull,
			NODATA,
			pull_oblock,
			};
static Pull options_pull =
	{
	&test_pull,
	20*CH_WIDTH, -1,
	8*CH_WIDTH,CH_HEIGHT,
	&roptions_pull,
	a3dpull_113 /* "MOVEMENT" */,
	pull_text,
	};
			static Pull a3dfiles_pull =
				{
				NONEXT,
				1, 1+7*CH_HEIGHT,
				3+12*CH_WIDTH,1+CH_HEIGHT,
				NOCHILD,
				a3dpull_114 /* "FILES..." */,
				pull_text,
				};
			static Pull p6_pull =
				{
				&a3dfiles_pull,
				1, 1+6*CH_HEIGHT,
				3+12*CH_WIDTH,1+CH_HEIGHT,
				NOCHILD,
				a3dpull_115 /* "SQUASH" */,
				pull_text,
				};
			static Pull p5_pull =
				{
				&p6_pull,
				1, 1+5*CH_HEIGHT,
				3+12*CH_WIDTH,1+CH_HEIGHT,
				NOCHILD,
				a3dpull_116 /* "SPIN SMALL" */,
				pull_text,
				};
			static Pull p4_pull =
				{
				&p5_pull,
				1, 1+4*CH_HEIGHT,
				3+12*CH_WIDTH,1+CH_HEIGHT,
				NOCHILD,
				a3dpull_117 /* "WHIRL" */,
				pull_text,
				};
			static Pull p3_pull =
				{
				&p4_pull,
				1, 1+3*CH_HEIGHT,
				3+12*CH_WIDTH,1+CH_HEIGHT,
				NOCHILD,
				a3dpull_118 /* "TWIRL" */,
				pull_text,
				};
			static Pull p2_pull =
				{
				&p3_pull,
				1, 1+2*CH_HEIGHT,
				3+12*CH_WIDTH,1+CH_HEIGHT,
				NOCHILD,
				a3dpull_119 /* "SPIN" */,
				pull_text,
				};
			static Pull p1_pull =
				{
				&p2_pull,
				1, 1+1*CH_HEIGHT,
				3+12*CH_WIDTH,1+CH_HEIGHT,
				NOCHILD,
				a3dpull_120 /* "PULL BACK" */,
				pull_text,
				};
			static Pull p0_pull = 
				{
				&p1_pull,
				1, 1+0*CH_HEIGHT,
				3+12*CH_WIDTH,1+CH_HEIGHT,
				NOCHILD,
				a3dpull_121 /* "CLEAR ALL" */,
				pull_text,
				};
		static Pull rpresets_pull =
			{
			NONEXT,
			-1,CH_HEIGHT,
			5+12*CH_WIDTH, 3+8*CH_HEIGHT,
			&p0_pull,
			NODATA,
			pull_oblock,
			};
Pull presets_pull =
	{
	&options_pull,
	10*CH_WIDTH, -1,
	7*CH_WIDTH,CH_HEIGHT,
	&rpresets_pull,
	a3dpull_122 /* "PRESETS" */,
	pull_text,
	};


set_ado_asterisks()
{
xonflag(&ado_still_pull, !vs.ado_tween);
xonflag(&ado_pong_pull, vs.ado_pong);
xonflag(&ado_ease_pull, vs.ado_ease);
xonflag(&ado_easeo_pull, vs.ado_ease_out);
xonflag(&ado_reverse_pull, vs.ado_reverse);
xonflag(&ado_complete_pull, vs.ado_complete);
xonflag(&ado_cel_pull, vs.ado_source == OPS_CEL);
xonflag(&ado_frames_pull, vs.ado_source == OPS_SCREEN);
xonflag(&ado_spline_pull, vs.ado_source == OPS_SPLINE);
xonflag(&ado_poly_pull, vs.ado_source == OPS_POLY);
xonflag(&ado_outline_pull, vs.ado_outline);
}

a3d_pull_disables()
{
enable_pulls(&root_pull);
if (cel == NULL)
	{
	if (vs.ado_source == OPS_CEL)
		vs.ado_source = OPS_SCREEN;
	ado_cel_pull.disabled = 1;
	}
if (!jexists(poly_name))
	{
	if (vs.ado_source == OPS_SPLINE || vs.ado_source == OPS_POLY)
		vs.ado_source = OPS_SCREEN;
	ado_spline_pull.disabled = 1;
	ado_poly_pull.disabled = 1;
	}
}
