#include "commonst.h"
#include "softmenu.h"

char empty_str[] = "";
char space_str[] = " ";
char r_str[] = "r";
char w_str[] = "w";
char rb_str[] = "rb";
char wb_str[] = "wb";
char *unnamed_str;
char *any_continue;
char *enter_choice;
char *continue_str;
char *yes_str;
char *no_str;
char *ok_str;
char *cancel_str;
char *load_str;
char *save_str;
char *lrmenu_font_name;
char *menu_font_name;
char *hrmenu_font_name;
char *please_wait_str;


static Smu_name_scats common_keys[] = {
	{ "aalrmenu", { &lrmenu_font_name } },
	{ "aamenu",   { &menu_font_name } },
	{ "aahrmenu", { &hrmenu_font_name } },
	{ "continue", { &continue_str } },
	{ "yes",      { &yes_str } },
	{ "no",       { &no_str } },
	{ "ok",       { &ok_str } },
	{ "cancel",   { &cancel_str } },
	{ "load",     { &load_str } },
	{ "save",     { &save_str } },
	{ "any_key",  { &any_continue } },
	{ "enter",    { &enter_choice } },
	{ "unnamed",  { &unnamed_str } },
	{ "pls_wait", { &please_wait_str } },
};
static void *common_ss;

void default_common_str(void)
/* make 'ok', 'continue', etc same as they're symbol names so will
 * appear in English ok at least in error conditions */
{
Smu_name_scats *ss = common_keys;
Smu_name_scats *maxss = OPTR(common_keys,sizeof(common_keys));

	while(ss < maxss)
	{
		*(ss->toload.ps) = ss->name;	/* reasonable default on error */
		++ss;
	}
	any_continue = "Any key to continue >";
	enter_choice = "Enter choice >";
	please_wait_str = "Please Wait:";
}

Errcode init_common_str(Softmenu *sm)
{
	/* note: this will call default_common_str() on failure before
	 * reporting because SCT_STRINIT is in flags */

	return(smu_name_scatters(sm, "common_strings", common_keys,
				  Array_els(common_keys), &common_ss, SCT_STRINIT ));
}

void cleanup_common_str(void)
{
	smu_free_scatters(&common_ss);
}

