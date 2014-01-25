#include "msfile.h"
#include "errcodes.h"
#include "ptrmacro.h"

static short err_trans_table[] = {
	Success,
	Err_nogood,
	Err_no_file,
	Err_no_path,
	Err_too_many_files,
	Err_access,
	Err_file_not_open,
	Err_sys_mem_bad,
	Err_no_memory,
	Err_bad_address,
	Err_bad_env,
	Err_disk_format,
	Err_file_access,
	Err_data,
	Err_nogood,
	Err_no_device,
	Err_nogood,
	Err_nogood,
	Err_no_more_files,
	Err_write_protected,
	Err_no_device,
	Err_disk_not_ready,
	Err_nogood,
	Err_disk_data,
	Err_nogood,
	Err_seek,
	Err_not_dos_disk,
	Err_sector,
	Err_no_paper,
	Err_read,
	Err_write,
	Err_general_failure,
	Err_file_share,
	Err_file_lock,
	Err_disk_change,
	Err_nogood, /* "No FCB available", */
	Err_nogood, /* "Sharing buffer overflow", */
};

static short err2_trans_table[] =  {
Err_network,
Err_no_remote,
Err_network,
Err_network,
Err_network_busy,
Err_network,
Err_network,
Err_network,
Err_network,
Err_network,
Err_network,
Err_nogood, /* "Print queue full", */
Err_nogood, /* "Not enough space for print file", */
Err_nogood, /* "Print file was deleted", */
Err_network,
Err_access,
Err_network,
Err_network,
Err_network,
Err_network,
Err_share_pause,
Err_network,
Err_redirect_pause, /* #72 */
Err_nogood,Err_nogood,Err_nogood,Err_nogood,Err_nogood,Err_nogood,Err_nogood,
								/* #73-79 */
Err_extant,
Err_nogood,
Err_directory_entry,
Err_nogood, 	/* critical error */
Err_network,
Err_network,
Err_nogood, 	/* "Invalid password", */
Err_nogood, 	/* "Invalid parameter", */
Err_network,
};

Errcode pj_mserror(Doserr derr)
/* return Errcode translation from return by pj_dget_err() dos error code */
{
extern short pj_crit_errval;
Errcode err;

	if (derr == 83) /* critical error */
		err = err_trans_table[pj_crit_errval+19];
	else if (derr >= 0 && (unsigned int)derr <= Array_els(err_trans_table))
		err = err_trans_table[derr];
	else if (derr >= 50 &&	derr <= 88)
		err = err2_trans_table[derr-50];
	else
		err = Err_nogood;
	return(err);
}
