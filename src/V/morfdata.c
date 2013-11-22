#ifdef MORPH
#include "jimk.h"
#include "flicmenu.h"

extern dcorner_text();
extern change_mode();

int morf_tool;

/* morfdata.c - the buttons for the metamorphic panel */

static Flicmenu mtg6_sel = {
	NONEXT,
	NOCHILD,
	83, 188, 38, 8,
	"MOVE",
	dcorner_text,
	change_mode,
	&morf_tool, 5,
	NOKEY,
	NOOPT,
	};
static Flicmenu mtg5_sel = {
	&mtg6_sel,
	NOCHILD,
	83, 178, 38, 8,
	"LINK",
	dcorner_text,
	change_mode,
	&morf_tool, 4,
	NOKEY,
	NOOPT,
	};
static Flicmenu mtg4_sel = {
	&mtg5_sel,
	NOCHILD,
	83, 168, 38, 8,
	"FACE",
	dcorner_text,
	change_mode,
	&morf_tool, 3,
	NOKEY,
	NOOPT,
	};
static Flicmenu mtg3_sel = {
	&mtg4_sel,
	NOCHILD,
	43, 188, 38, 8,
	"COLOR",
	dcorner_text,
	change_mode,
	&morf_tool, 2,
	NOKEY,
	NOOPT,
	};
static Flicmenu mtg2_sel = {
	&mtg3_sel,
	NOCHILD,
	43, 178, 38, 8,
	"INK",
	dcorner_text,
	change_mode,
	&morf_tool, 1,
	NOKEY,
	NOOPT,
	};
static Flicmenu mtg1_sel = {
	&mtg2_sel,
	NOCHILD,
	43, 168, 38, 8,
	"MAKE",
	dcorner_text,
	change_mode,
	&morf_tool, 0,
	NOKEY,
	NOOPT,
	};
Flicmenu morf_tool_group_sel = {
	NONEXT,
	&mtg1_sel,
	43, 168, 78, 28,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};

#endif MORPH
