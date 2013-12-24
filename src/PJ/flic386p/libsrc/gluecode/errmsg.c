/*****************************************************************************
 * ERRMSG.C - Translate a PJ standard error code into an English-text message.
 ****************************************************************************/

#include "flicglue.h"
#include "ptrmacro.h"

typedef struct {Errcode err; char *msg;} ErrMsg;

static ErrMsg messages[] = {
{Err_access,			"Access denied.  (File read only?)"},
{Err_bad_address,		"Bad address to DOS.  (Internal error)"},
{Err_bad_env,			"Bad environment block.  Suggest you reboot"},
{Err_bad_input, 		"Bad input data"},
{Err_bad_magic, 		"File is not correct type"},
{Err_corrupted, 		"File corrupted"},
{Err_data,				"Invalid data to DOS. (Internal error)"},
{Err_directory_entry,	"Can't create directory entry"},
{Err_disk_change,		"Invalid disk  change"},
{Err_disk_data, 		"Disk data error, sorry"},
{Err_disk_format,		"Unrecognized disk format"},
{Err_disk_not_ready,	"Disk drive not ready. (No disk in drive or door open?)"},
{Err_eof,				"End of file"},
{Err_extant,			"File already exists"},
{Err_file_access,		"Invalid file access code. (Internal error)"},
{Err_file_lock, 		"File lock error"},
{Err_file_not_open, 	"File is not open"},
{Err_file_share,		"File sharing error"},
{Err_general_failure,	"General failure"},
{Err_internal_parameter,"Internal: routine detected bad parameter value"},
{Err_internal_pointer,	"Internal: routine detected bad pointer"},
{Err_network,			"General network failure"},
{Err_network_busy,		"Network busy"},
{Err_no_device, 		"Device doesn't exist"},
{Err_no_display,		"Display hardware not found or invalid"},
{Err_no_file,			"File not found. (File name misspelled?)"},
{Err_no_memory, 		"Out of Memory"},
{Err_no_more_files, 	"No more files"},
{Err_no_paper,			"Printer out of paper"},
{Err_no_path,			"Path not found.  (Misspelled directory?)"},
{Err_no_remote, 		"Remote computer not listening"},
{Err_no_such_mode,		"Asking for a mode that isn't there"},
{Err_no_vram,			"Out of video display memory"},
{Err_nogood,			"ERROR!"},
{Err_not_dos_disk,		"Not a DOS disk"},
{Err_null_ref,			"Attempting to use a NULL pointer"},
{Err_rast_type, 		"Invalid raster type for operation"},
{Err_read,				"Read error"},
{Err_redirect_pause,	"File/printer redirection paused"},
{Err_sector,			"Disk sector not found"},
{Err_seek,				"Seek error"},
{Err_share_pause,		"Sharing temporarily paused"},
{Err_sys_mem_bad,		"MS-DOS system tables damaged.  (Reboot time?)"},
{Err_too_big,			"Object too big to handle"},
{Err_too_many_files,	"Too many open files.  (Set FILES in config.sys to something higher?)"},
{Err_truncated, 		"File truncated"},
{Err_unimpl,			"Feature unimplemented"},
{Err_uninit,			"Subsystem not initialized"},
{Err_write, 			"Write error"},
{Err_write_protected,	"Disk is write protected"},
{Err_wrong_res, 		"Can't deal with object of this dimension"},
};

char *pj_error_get_message(Errcode err)
/*****************************************************************************
 * return error message, or empty string ("") for Success, Abort, or
 * Already Reported.
 ****************************************************************************/
{
	int i;

	if (err >= Success || err == Err_reported || err == Err_abort)
		return("");
	for (i=0; i<Array_els(messages); ++i) {
		if (err == messages[i].err)
			return(messages[i].msg);
	}
	return "Unknown error";
}
