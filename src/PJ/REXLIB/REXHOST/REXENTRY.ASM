	include rexentry.i

	EXTRN	rexlib_header:WORD
	EXTRN	_end:WORD

DGROUP group _data
	assume	cs:_rexlib_entry_segment,ds:DGROUP

_rexlib_entry_segment	segment	para public use32 'code'

	public	_start_

_start_	proc	near	 ; This will be linked as a .REX file.
	rexentry <0c3c3h,PJREX_MAGIC,rexlib_header,PJREX_MAGIC2,PJREX_VERSION,_end>
_start_	endp

_rexlib_entry_segment	ends

_data	segment	para public use32 'data'
_data	ends

_stack	segment byte stack use32 'stack'
;define a minimal stack to keep linker from complaining
	dd	0
_stack	ends

	end _start_

