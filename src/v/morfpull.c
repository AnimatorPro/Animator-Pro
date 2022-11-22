#ifdef MORPH

/* generated with makepull */
#include "jimk.h"
#include "flicmenu.h"


			static struct pull ddm_hhm_pull = {
				NONEXT,
				1+0*CH_WIDTH, 1+7*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"HhMorfSel4",
				pull_text,
				0,
				};
			static struct pull ddm_ggm_pull = {
				&ddm_hhm_pull,
				1+0*CH_WIDTH, 1+6*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"GgMorfSel4",
				pull_text,
				0,
				};
			static struct pull ddm_ffm_pull = {
				&ddm_ggm_pull,
				1+0*CH_WIDTH, 1+5*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"FfMorfSel4",
				pull_text,
				0,
				};
			static struct pull ddm_eem_pull = {
				&ddm_ffm_pull,
				1+0*CH_WIDTH, 1+4*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"EeMorfSel4",
				pull_text,
				0,
				};
			static struct pull ddm_ddm_pull = {
				&ddm_eem_pull,
				1+0*CH_WIDTH, 1+3*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"DdMorfSel4",
				pull_text,
				0,
				};
			static struct pull ddm_ccm_pull = {
				&ddm_ddm_pull,
				1+0*CH_WIDTH, 1+2*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"New Polyh ",
				pull_text,
				0,
				};
			static struct pull ddm_bbm_pull = {
				&ddm_ccm_pull,
				1+0*CH_WIDTH, 1+1*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"Save Polyh",
				pull_text,
				0,
				};
			static struct pull ddm_aam_pull = {
				&ddm_bbm_pull,
				1+0*CH_WIDTH, 1+0*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"Load Polyh",
				pull_text,
				0,
				};
		static struct pull rddm_pull = {
			NONEXT,
			-1+0*CH_WIDTH, 0+1*CH_HEIGHT, 5+10*CH_WIDTH, 3+8*CH_HEIGHT, 
			&ddm_aam_pull,
			NODATA,
			pull_oblock,
			0,
			};
	static struct pull ddm_pull = {
		NONEXT,
		0+40*CH_WIDTH, -1+0*CH_HEIGHT, 0+13*CH_WIDTH, 0+1*CH_HEIGHT, 
		&rddm_pull,
		"DdMorfMenu",
		pull_text,
		0,
		};
			static struct pull ccm_hhm_pull = {
				NONEXT,
				1+0*CH_WIDTH, 1+7*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"HhMorfSel3",
				pull_text,
				0,
				};
			static struct pull ccm_ggm_pull = {
				&ccm_hhm_pull,
				1+0*CH_WIDTH, 1+6*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"GgMorfSel3",
				pull_text,
				0,
				};
			static struct pull ccm_ffm_pull = {
				&ccm_ggm_pull,
				1+0*CH_WIDTH, 1+5*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"FfMorfSel3",
				pull_text,
				0,
				};
			static struct pull ccm_eem_pull = {
				&ccm_ffm_pull,
				1+0*CH_WIDTH, 1+4*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"EeMorfSel3",
				pull_text,
				0,
				};
			static struct pull ccm_ddm_pull = {
				&ccm_eem_pull,
				1+0*CH_WIDTH, 1+3*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"DdMorfSel3",
				pull_text,
				0,
				};
			static struct pull ccm_ccm_pull = {
				&ccm_ddm_pull,
				1+0*CH_WIDTH, 1+2*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"CcMorfSel3",
				pull_text,
				0,
				};
			static struct pull ccm_bbm_pull = {
				&ccm_ccm_pull,
				1+0*CH_WIDTH, 1+1*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"BbMorfSel3",
				pull_text,
				0,
				};
			static struct pull ccm_aam_pull = {
				&ccm_bbm_pull,
				1+0*CH_WIDTH, 1+0*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"AaMorfSel3",
				pull_text,
				0,
				};
		static struct pull rccm_pull = {
			NONEXT,
			-1+0*CH_WIDTH, 0+1*CH_HEIGHT, 5+10*CH_WIDTH, 3+8*CH_HEIGHT, 
			&ccm_aam_pull,
			NODATA,
			pull_oblock,
			0,
			};
	static struct pull ccm_pull = {
		&ddm_pull,
		0+27*CH_WIDTH, -1+0*CH_HEIGHT, 0+13*CH_WIDTH, 0+1*CH_HEIGHT, 
		&rccm_pull,
		"CcMorfMenu",
		pull_text,
		0,
		};
			static struct pull bbm_hhm_pull = {
				NONEXT,
				1+0*CH_WIDTH, 1+7*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"HhMorfSel2",
				pull_text,
				0,
				};
			static struct pull bbm_ggm_pull = {
				&bbm_hhm_pull,
				1+0*CH_WIDTH, 1+6*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"GgMorfSel2",
				pull_text,
				0,
				};
			static struct pull bbm_ffm_pull = {
				&bbm_ggm_pull,
				1+0*CH_WIDTH, 1+5*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"FfMorfSel2",
				pull_text,
				0,
				};
			static struct pull bbm_eem_pull = {
				&bbm_ffm_pull,
				1+0*CH_WIDTH, 1+4*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"EeMorfSel2",
				pull_text,
				0,
				};
			static struct pull bbm_ddm_pull = {
				&bbm_eem_pull,
				1+0*CH_WIDTH, 1+3*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"DdMorfSel2",
				pull_text,
				0,
				};
			static struct pull bbm_ccm_pull = {
				&bbm_ddm_pull,
				1+0*CH_WIDTH, 1+2*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"CcMorfSel2",
				pull_text,
				0,
				};
			static struct pull bbm_bbm_pull = {
				&bbm_ccm_pull,
				1+0*CH_WIDTH, 1+1*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"BbMorfSel2",
				pull_text,
				0,
				};
			static struct pull bbm_aam_pull = {
				&bbm_bbm_pull,
				1+0*CH_WIDTH, 1+0*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"AaMorfSel2",
				pull_text,
				0,
				};
		static struct pull rbbm_pull = {
			NONEXT,
			-1+0*CH_WIDTH, 0+1*CH_HEIGHT, 5+10*CH_WIDTH, 3+8*CH_HEIGHT, 
			&bbm_aam_pull,
			NODATA,
			pull_oblock,
			0,
			};
	static struct pull bbm_pull = {
		&ccm_pull,
		0+14*CH_WIDTH, -1+0*CH_HEIGHT, 0+13*CH_WIDTH, 0+1*CH_HEIGHT, 
		&rbbm_pull,
		"BbMorfMenu",
		pull_text,
		0,
		};
			static struct pull aam_hhm_pull = {
				NONEXT,
				1+0*CH_WIDTH, 1+7*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"HhMorfSel1",
				pull_text,
				0,
				};
			static struct pull aam_ggm_pull = {
				&aam_hhm_pull,
				1+0*CH_WIDTH, 1+6*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"GgMorfSel1",
				pull_text,
				0,
				};
			static struct pull aam_ffm_pull = {
				&aam_ggm_pull,
				1+0*CH_WIDTH, 1+5*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"FfMorfSel1",
				pull_text,
				0,
				};
			static struct pull aam_eem_pull = {
				&aam_ffm_pull,
				1+0*CH_WIDTH, 1+4*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"EeMorfSel1",
				pull_text,
				0,
				};
			static struct pull aam_ddm_pull = {
				&aam_eem_pull,
				1+0*CH_WIDTH, 1+3*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"DdMorfSel1",
				pull_text,
				0,
				};
			static struct pull aam_ccm_pull = {
				&aam_ddm_pull,
				1+0*CH_WIDTH, 1+2*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"CcMorfSel1",
				pull_text,
				0,
				};
			static struct pull aam_bbm_pull = {
				&aam_ccm_pull,
				1+0*CH_WIDTH, 1+1*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"BbMorfSel1",
				pull_text,
				0,
				};
			static struct pull aam_aam_pull = {
				&aam_bbm_pull,
				1+0*CH_WIDTH, 1+0*CH_HEIGHT, 3+10*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				"AaMorfSel1",
				pull_text,
				0,
				};
		static struct pull raam_pull = {
			NONEXT,
			-1+0*CH_WIDTH, 0+1*CH_HEIGHT, 5+10*CH_WIDTH, 3+8*CH_HEIGHT, 
			&aam_aam_pull,
			NODATA,
			pull_oblock,
			0,
			};
	struct pull morf_pull = {
		&bbm_pull,
		0+1*CH_WIDTH, -1+0*CH_HEIGHT, 0+13*CH_WIDTH, 0+1*CH_HEIGHT, 
		&raam_pull,
		"AaMorfMenu",
		pull_text,
		0,
		};
#endif /* MORPH */
