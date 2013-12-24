;*****************************************************************************
;*
;*****************************************************************************

dosmem_block struc
far32	df	?
dosseg	dw	?
dosmem_block ends

	extrn	pj_dosmem_alloc:near
	extrn	pj_dosmem_free:near
