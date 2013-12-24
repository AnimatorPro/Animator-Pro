#include "linklist.h"
#include "ptrmacro.h"

/**************************************************************/
Dlnode *circ_get_next(Dlnode *this)

/* starting with a node returns next node on list if on tail will return head
 * may not be called with an empty list or BOOM */
{
	this = this->next;
	if(this->next == NULL)
		return(TOSTRUCT(Dlheader,tail,this)->head);
	else
		return(this);
}
