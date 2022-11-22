

; This file lets you access out-of-segment memory and do 
; in's and out's.  Also can put bytes out a port.

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

	;;  out_byte(int port, int value)
	;;
	;;  Send one byte out a port.
	public out_byte
out_byte proc near
ob_parameters struc
	ob_return		dd	?	;return address
	ob_old_ebp		dd	?	;old base-page
	ob_port			dd	?	;io port #
	ob_value		dd	?	;byte to send out port
ob_parameters ends
	push	ebp
	mov	ebp,esp
	mov	edx,[ebp].ob_port
	mov	eax,[ebp].ob_value
	out	dx,al
	pop	ebp
	ret
out_byte endp

	;;
	;;	far_copy_bytes(int source_offset, int source_segment, 
	;;		int dest_offset, int dest_segment, int size)
	;;
	;;  Copy data between two different segments.

	public far_copy_bytes
far_copy_bytes proc near
fcb_parameters	struc	;far_copy_bytes parameter structure
	fcb_return		dd	?	;return address
	fcb_old_ebp		dd	?	;old base-page
	fcb_source_offset	dd	?	;address offset to read
	fcb_source_segment	dd	?	;address segment to read
	fcb_dest_offset		dd	?	;address offset to write
	fcb_dest_segment	dd	?	;address segment to write
	fcb_size		dd	?	;size of data to transfer
fcb_parameters ends
;; Standard function preamble - set up stack frame to access parameters
;; and push the registers worth saving.
	push	ebp
	mov	ebp,esp
	push	ds
	push	es
	push	esi
	push	edi
;; Set up ds:esi to point to the far source location and es:edi
;; to point to the destination buffer, and get ready for
;; the (inevitable?) repl movsb.
	mov	eax,[ebp].fcb_source_segment
	mov 	ds,ax
	mov 	esi,[ebp].fcb_source_offset
	mov 	eax,[ebp].fcb_dest_segment
	mov 	es,ax
	mov 	edi,[ebp].fcb_dest_offset
	mov 	ecx,[ebp].fcb_size
	rep 	movsb
;; Standard function exit - restore all saved registers and return.
	pop	edi
	pop	esi
	pop	es
	pop	ds
	pop	ebp
	ret
far_copy_bytes endp

code ends
	end
