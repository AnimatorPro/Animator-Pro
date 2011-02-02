/* qconfig.c - Interface menus and calls to manipulate the config 
 * file settings */

#include <ctype.h>
#include "jimk.h"
#include "aaconfig.h"
#include "fli.h"
#include "errcodes.h"
#include "tfile.h"
#include "idriver.h"
#include "softmenu.h"



static Boolean get_serial_port(void)
{
int choice;
USHORT cdis[9];

	clear_mem(cdis, sizeof(cdis));
	cdis[(7 & vconfg.comm_port)] |= QCF_ASTERISK;

	if((choice = soft_qchoice(cdis, "comm_port")) >= Success)
	{
		if (!pj_comm_exists(choice))
			soft_continu_box("!%d", "no_comm", choice + 1);
		else
			vconfg.comm_port = choice;
	}
	return(choice);
}

extern char resource_dir[];
extern char mouse_idriver_name[];
extern char summa_idriver_name[];

static void config_input(void)
{
int choice;
char dev_type;
char buf[PATH_SIZE];
USHORT idis[9];
UBYTE modes[IDR_MAX_OPTIONS];
char sbuf[80];

	clear_mem(idis,sizeof(idis));
	idis[vconfg.dev_type] = QCF_ASTERISK;
	if(!resource_exists(summa_idriver_name))
		idis[1] |= QCF_DISABLED;

	choice = soft_qchoice(idis, "input_device" );
	switch (choice)
	{
		default:
			return;
		case 0:
			dev_type = 0;
			strcpy(buf, mouse_idriver_name);
			break;
		case 1:
			dev_type = 1;
			strcpy(buf, summa_idriver_name);
			break;
		case 2:
			if(vconfg.dev_type == 2)
				strcpy(buf,vconfg.idr_name);
			else
				buf[0] = 0;
			if(!req_resource_name(buf,"*.idr", 
								 stack_string("idr_scroll", sbuf)))
			{
				return;
			}
			dev_type = 2;
			break;
		case 3:
			if (!get_serial_port())
				return;
			goto REWRITE;
	}

	memcpy(modes,vconfg.idr_modes,sizeof(modes));
	if(config_idriver(buf, modes, 
					   txtcmp(vconfg.idr_name,buf) == 0,
					   (SHORT)vconfg.comm_port) < Success)
	{
		return;
	}
	vconfg.dev_type = dev_type;
	memcpy(vconfg.idr_modes,modes,sizeof(modes));
	strcpy(vconfg.idr_name, buf);
REWRITE:
	rewrite_config();
	cleanup_idriver();
	init_input();
}


void config_path(void)
{
char pbuf[sizeof(vconfg.temp_path)];

	strcpy(pbuf, vconfg.temp_path);
	if(soft_qreq_string( pbuf, sizeof(pbuf)-1,"temp_path"))
	{
		if (change_temp_path(pbuf) >= Success)
		{
			strcpy(vconfg.temp_path, pbuf);
			rewrite_config();
		}
	}
}

void new_config(void)
/* configure numbered item menu */
{
USHORT cdis[10];

	clear_mem(cdis, sizeof(cdis));
	if (vs.dcoor)
		cdis[3] = QCF_ASTERISK;

	switch(soft_qchoice(cdis, "configuration"))
	{
		case 0:		/* set temp path */
			config_path();
			break;
		case 1:
			save_default_settings();
			break;
		case 2:
			config_input();
			break;
		case 3:
			vs.dcoor = !vs.dcoor;
			break;
		default:
			break;
	}
}
