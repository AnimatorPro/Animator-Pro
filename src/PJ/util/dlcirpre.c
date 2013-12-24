#include "linklist.h"
#include "ptrmacro.h"

/**************************************************************/
Dlnode *circ_get_prev(Dlnode *this)

/* starting with a node returns previous node on list if on head will return 
 * tail, may not be called with an empty list or BOOM */
{
	this = this->prev;
	if(this->prev == NULL)
		return(TOSTRUCT(Dlheader,head,this)->tails_prev);
	else
		return(this);
}
