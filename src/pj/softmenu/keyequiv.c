#include <ctype.h>
#include "pjbasics.h"
#include "softmenu.h"

Errcode load_key_equivs(char *symname, Keyequiv *kfin, unsigned int count)
/* currently limited to 150 items, does not require freeing  */
{
Errcode err;
Keyequiv *max;
Keyequiv *kf;
Smu_name_scats scts[150];
Smu_name_scats *sct = scts;
void *ss;

	kf = kfin;
	if(kf->flags & KE_LOADED)
		return(Success);

	if(count > 150)
		count = 150;

	max = kf + count;

	while(kf < max)
	{
		sct->name = kf->name;
		++sct;
		++kf;
	}
	if((err = soft_name_scatters(symname,scts,count,
								&ss,SCT_DIRECT)) < Success)
	{	
		return(err);
	}

	sct = scts;
	kf = kfin;
	while(kf < max)
	{
		kf->key = *((SHORT *)(sct->toload.s));
		kf->flags |= KE_LOADED;
		++kf;
		++sct;
	}
	smu_free_scatters(&ss);
	return(Success);
}
Boolean do_keyequiv(SHORT key, Keyequiv *kf, unsigned int count)
{
Keyequiv *max;

	if(!JSTHIT(KEYHIT))
		return(FALSE);

	if((UBYTE)key)
		key = tolower((UBYTE)key);

	max = kf + count;
	while(kf < max)
	{
		if(key == kf->key)
		{
			if(kf->flags & KE_HIDE)
				hide_mp();
			kf->doit();
			if(kf->flags & KE_HIDE)
				show_mp();
			return(TRUE);
		}
		++kf;
	}
	return(FALSE);
}
Boolean hit_keyequiv(Keyequiv *ke, SHORT key)
{
	if(!JSTHIT(KEYHIT))
		return(FALSE);

	if((UBYTE)key)
		return(tolower((UBYTE)key) == ke->key);
	else
		return(key == ke->key);
}
