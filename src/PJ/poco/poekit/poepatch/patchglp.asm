;*****************************************************************************
;* PATCHGLP.ASM - Patch for Ani Pro v1.0 po_get_libproto_line() function.
;*
;* This code replaces the version of po_get_libproto_line() which is compiled
;* into Ani Pro v1.0.  A bug in the v1.0 routine prevents the use of multiple
;* POE modules in a single Poco program.  The bug occurs when multiple #pragma
;* statements appear in one Poco program.  Upon reaching the end of the
;* protos in the second POE module, the v1.0 routine follows the pl->next
;* link back into the prior POE module, and then dies with a 'Function XXXX()
;* redefined" error.  The fix (present in Ani Pro v1.1 and up) is to not
;* follow this link for a loaded library.  (The links must still be walked
;* for builtin libraries, however.)  Comments in poco\pocoface.c describe
;* the fix in more detail.
;*
;* The code below may be applied as a patch to the existing code in Ani Pro
;* v1.0.  The code was created by compiling the new (v1.1) routine, then
;* hand-tweaking it to make it smaller.  This allows it to overlay the
;* existing v1.0 routine even though it has a few more instructions than
;* the original version did.  The actual patching is done by the poepatch.c
;* module; a static array in that module holds the binary code that this
;* module will generate if it is assembled.
;*
;* If you don't have the Ani Pro source code, this module will mean nothing
;* to you; it exists just in case I need to re-gen the patch data someday.
;*****************************************************************************

	include stdmacro.i

_TEXT	segment

poe_patch_getlibproto proc near

	push	ebx			; save ebx

	mov	ebx,+08h[esp]		; ebx = pcb
	mov	eax,+6Bh[ebx]		; eax = (fs) pcb->file_stack
	mov	edx,+4h[eax]		; edx = (pl) fs->source.lib

	mov	ecx,+0Ch[eax]		; ecx = fs->line_count
	cmp	ecx,+0Ch[edx]		; cmp to pl->count
	jl	short L1		; if LT, return next proto

	cmp	dword ptr +38h[ebx],0	; pcb->run.loaded_libs == NULL?
	jne	short Lpunt		; nope, punt (this is the patch!)

	mov	edx,[edx]		; edx = pl->next
	test	edx,edx 		; is it NULL?
	jne	short L2		; nope, move to next library

Lpunt:
	xor	eax,eax 		; NULL return value
	mov	dword ptr +58h[ebx],eax ; NULL pcb->libfunc
	jmp	short L3		; return

L2:
	mov	+4h[eax],edx		; fs->source.lib = pl
	xor	ecx,ecx 		; zero out reg'ized line count
	mov	dword ptr +0Ch[eax],ecx ; fs->line_count = 0

L1:
	inc	dword ptr +0Ch[eax]	; ++fs->line_count
	mov	eax,+8h[edx]		; eax = pl->lib
	mov	edx,[eax+ecx*8] 	; edx = pl->lib[line_count].func
	mov	+58h[ebx],edx		; pcb->libfunc = edx
	mov	eax,+4h[eax+ecx*8]	; eax = pl->lib[line_count].proto

L3:
	pop	ebx			; restore ebx
	ret				; return to caller

magic	db	'patchglp',0            ; this says "patch has been applied"

poe_patch_getlibproto endp

_TEXT	ends
	end
