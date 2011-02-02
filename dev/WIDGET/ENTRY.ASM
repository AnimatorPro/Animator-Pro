
	EXTRN	widgelib:WORD
	EXTRN	_end:WORD

; We put the item "_end" in the first longword of the entry code. This will make
; the first longword of the rex code at the "entry point" equal to the size of 
; the buffer to allocate to run this rex.  This value is provided py the 
; Pharlap linker. calling the vector one word beyond the entry point will give
; you a call to the do_widget() function in widget.rex

DGROUP group _data
	assume	cs:_rexlib_entry_segment,ds:DGROUP

_rexlib_entry_segment	segment	para public use32 'code'

	public	_start_

_start_	proc	near	 ; This will be linked as a .REX file.
    DD _end
    DD widgelib  ; library structure in c module
_start_	endp


_rexlib_entry_segment	ends

_data	segment	para public use32 'data'
_data	ends

_stack	segment byte stack use32 'stack'
;define a minimal stack to keep linker from complaining
	db	0
_stack	ends

	end _start_

