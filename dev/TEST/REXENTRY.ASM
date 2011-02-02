
	EXTRN	do_widget:WORD
	EXTRN	_end:WORD

DGROUP group _data
	assume	cs:_rexlib_entry_segment,ds:DGROUP

_rexlib_entry_segment	segment	para public use32 'code'

	public	_start_

_start_	proc	near	 ; This will be linked as a .REX file.
    DD      _end
_start_	endp

_rexlib_entry_segment	ends

_data	segment	para public use32 'data'
_data	ends

_stack	segment byte stack use32 'stack'
;define a minimal stack to keep linker from complaining
	db	'S','T','A','C','K'
	db	'S','T','A'
_stack	ends

	end _start_

