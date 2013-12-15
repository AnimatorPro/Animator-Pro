

	assume	cs:_text,ds:_data

_text	segment	para public use32 'code'

	extrn entry:near

	public	_start_

_start_	proc	near	; This will be linked as a .REX file.
	jmp entry
_start_	endp

_text	ends

_data	segment	para public use32 'data'
_data	ends

_stack	segment byte stack use32 'stack'
;define a minimal stack to keep linker from complaining
	db	8 dup (?)
_stack	ends

	end _start_

