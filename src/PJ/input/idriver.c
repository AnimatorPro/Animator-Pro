
#include "errcodes.h"
#include "idriver.h"
#include "memory.h"
#include "jfile.h"
#include "commonst.h"
#include "menus.h"

char key_idriver_name[] = "=key.";
char mouse_idriver_name[] = "=mouse.";
char summa_idriver_name[] = "summa.idr";

extern Errcode init_key_idriver(Idriver *idr);
extern Errcode init_mouse_idriver(Idriver *idr);

static Errcode do_init_idriver(Idriver *idr)
{
	if (idr->hdr.init == NULL)
		return Success;
	return (*idr->hdr.init)(idr);
}
void close_idriver(Idriver **pidr)
{
Idriver *idr;

	if((idr = *pidr) == NULL)
		return;
	if (idr->hdr.cleanup != NULL)
		(*idr->hdr.cleanup)(idr);
	pj_freez(pidr);
}
static Errcode alloc_local_idr(Errcode (*initit)(Idriver *idr),Idriver **pidr)
{
Errcode err;

	if ((*pidr = pj_zalloc(sizeof(Idriver))) == NULL)
		return(Err_no_memory);
	if((err = (*initit)(*pidr)) < Success)
		pj_freez(pidr);
	return(err);
}
Errcode load_idriver(Idriver **pidr,char *iname,UBYTE *modes, SHORT comm_port)
{
int i;
Errcode err;
Idriver *idr = NULL;
char *local_name;

	local_name = pj_get_path_name(iname);
	if(!(txtcmp(local_name,key_idriver_name)))
		err = alloc_local_idr(init_key_idriver,pidr);
	else if (!(txtcmp(local_name,mouse_idriver_name)))
		err = alloc_local_idr(init_mouse_idriver,pidr);
	else 
		err = Err_unimpl;

	if(err < Success)
		goto error;

	idr = *pidr;

	/* load selected modes and com port */
	idr->comm_port = comm_port;

	if( modes != NULL 
		&& idr->options != NULL
		&& ((i = idr->num_options) < IDR_MAX_OPTIONS))
	{
		while(--i >= 0)
		{
			if((0x0001 << modes[i]) & idr->options[i].enabled)
				idr->options[i].mode = modes[i];
		}
	}

	/* call the initializer */
	if((err = do_init_idriver(idr)) < Success)
		goto error;
	return(Success);
error:
	close_idriver(pidr);
done:
	return(err);
}

void idr_clip(Idriver *idr,SHORT first_channel, SHORT last_channel)
/* Clip the requested channels to idr min and idr clipmax */
{
int i;

	if((USHORT)first_channel > idr->channel_count)
		return;
	if((++last_channel) > idr->channel_count)
		last_channel = idr->channel_count;

	for (i = first_channel;i<last_channel; i++)
	{
		if (idr->pos[i] < idr->min[i])
			idr->pos[i] = idr->min[i];
		if (idr->pos[i] > idr->clipmax[i])
			idr->pos[i] = idr->clipmax[i];
	}
}
