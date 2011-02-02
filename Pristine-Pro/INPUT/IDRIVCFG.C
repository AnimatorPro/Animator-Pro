#include "errcodes.h"
#include "idriver.h"
#include "memory.h"
#include "jfile.h"
#include "commonst.h"
#include "menus.h"
#include "ptrmacro.h"
#include "softmenu.h"

Errcode config_idriver(char *name, UBYTE *modes, Boolean use_modes, SHORT port)

/* if use_modes is true then the modes in the input modes are used to set the 
 * asterisks in the choice menu. otherwise the asterisks are the defualts
 * provided by the driver being configured */
{
Idriver *idr;
Errcode err;
SHORT ocount;
char rname[PATH_SIZE];
Idr_option *iopt;
USHORT qcflags[10];
UBYTE mode;
char *ctext;
char *soft_text;

	make_resource_name(name, rname);
	if ((err = load_idriver(&idr,rname,NULL,port)) < Success)
		goto error;
	if ((err = (*(idr->lib->detect))(idr)) < Success)
		goto error;
	if ((err = (*(idr->lib->inquire))(idr)) < Success)
		goto error;
	if ((ocount = idr->num_options) < 1)
		err = Success;
	else
	{
		iopt = idr->options;
		if (ocount > IDR_MAX_OPTIONS || iopt == NULL)
			goto protocol_error;

		while(--ocount >= 0)
		{
			clear_mem(qcflags,sizeof(qcflags));
			if(use_modes)
				mode = *modes;
			else
				mode = iopt->mode;

			if(iopt->enabled & ((USHORT)0x0001<<mode))
				qcflags[mode] = QCF_ASTERISK;

			for(mode = 0;mode < Array_els(qcflags)-1;++mode)
			{
				if(!(iopt->enabled & ((USHORT)0x0001<<mode)))
					qcflags[mode] |= QCF_DISABLED;
			}

			if(((soft_text = rex_key_or_text(iopt->choice_text,&ctext))!=NULL)
				&& (smu_load_name_text(&smu_sm,"idriver_texts", 
									   soft_text, &soft_text) >= Success))
			{
				ctext = soft_text;
			}
			err = qchoicef(qcflags, ctext);
			smu_free_text(&soft_text);
			if(err < Success)
				goto error;
			*modes++ = err;
			++iopt;
			err = Success;
		}
	}
	goto done;

protocol_error:
	err = Err_driver_protocol;
error:
	err = cant_load(err,rname);
done:
	close_idriver(&idr);
	return(err);
}
