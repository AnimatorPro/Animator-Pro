/* cleanup.c - stuff to cleanup what vpaint opens in init.c and subsequent
 * code */

#include "jimk.h"
#include "aaconfig.h"
#include "brush.h"
#include "flx.h"
#include "inks.h"
#include "pentools.h"
#include "rastcurs.h"
#include "resource.h"
#include "softmenu.h"
#include "vdevcall.h"
#include "wndo.h"
#include "zoom.h"

void close_downto_screen(void)
/* closes all that may be opened after init_screen() */
{
	free_buffers();
	close_zwinmenu();
	cleanup_toptext();
	close_all_menus(vb.screen,0);
	close_temp_flx();
	cleanup_wait_box();
}
static void cleanup_low(Errcode err)
/* cleans up stuff opened before screen */
{
	close_macro();
	free_undof();
	cleanup_brushes();
	cleanup_cursors();
	cleanup_inks();
	cleanup_ptools();
 	cleanup_resources();
	if(smu_is_open(&smu_sm))
		softerr(err,"fatal_exit");
	cleanup_startup();

}

void cleanup_all(Errcode err)
/* Remove interrupt handlers, restore device and directory, change back to
   start-up video mode, and do other miscellanious program cleanup/termination
   stuff */
{
	if (vb.screen != NULL)
		close_downto_screen();
	cleanup_screen();
	if(err >= Success) /* flush if exiting ok */
		rewrite_config();
	cleanup_low(err);
}
