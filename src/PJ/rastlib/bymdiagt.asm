;diag.asm - map a diagonal line to a horizontal line.  The core of
;the raster rotating/stretching machinery in Vpaint.

	include raster.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

;pj_bym_dto_ptable(Raster *sv, Pixel *dtable,
;				   Ucoor dsize,Coor x0,Coor y0,Coor x1,Coor y1)

	public pj_bym_dto_ptable
pj_bym_dto_ptable proc near

lvarspace equ 16

dttp	struc	;pj_bym_dto_ptable stack and argument structure

; start of local variables 

	dt_incx dd ?
	dt_incy dd ?
	dt_deltax dd ?
	dt_deltay dd ?

; end local variables

	dt_edi dd ?	;what's there from pushad
	dt_esi dd ?
	dt_ebp dd ?
	dt_esp dd ?
	dt_ebx dd ?
	dt_edx dd ?
	dt_ecx dd ?
	dt_eax dd ?

; pushad variables above

	dt_ret 	dd ?	;return address for function
	dt_sv	dd ?	;1st parameter - source screen
	dt_dtable	dd ?
	dt_dsize	dd ?
	dt_x0	dd ?
	dt_y0	dd ?
	dt_x1	dd ?
	dt_y1	dd ?

dttp ends

	pushad
	sub esp,lvarspace	;local space
	mov ebp,esp

	mov edi,[ebp].dt_sv

;compute address of x0/y0 into ds:si

	mov esi,[edi].bym_p
	mov eax,[ebp].dt_y0
	mul [edi].bym_bpr
	add esi,eax	;add in y0 component
	mov eax,[ebp].dt_x0   ;fetch x0
	add esi,eax   ;add in x0 component

;compute  [ebp].dt_deltax and [ebp].dt_incx

	mov edx,-1  ;edx = [ebp].dt_incx
	mov ebx,[ebp].dt_x1
	sub ebx,eax  ;ebx = x1-x0
	js nnx
	neg edx
	neg ebx
nnx: 
	mov [ebp].dt_incx,edx
	mov  [ebp].dt_deltax,ebx  ; [ebp].dt_deltax = -|x1-x0|

;compute [ebp].dt_deltay and [ebp].dt_incy

	mov edx,[edi].bym_bpr  ;edx = [ebp].dt_incy
	neg edx
	mov ebx,[ebp].dt_y1
	sub ebx,[ebp].dt_y0  ;ebx = y1-y0
	js nny
	neg edx
	neg ebx
nny: 
	mov [ebp].dt_incy,edx
	mov [ebp].dt_deltay,ebx	;[ebp].dt_deltay = -|y1-y0|

	mov edx,[ebp].dt_dsize ;get dsize into handy register for inner loop

	;eax = xerr = dsize + [ebp].dt_deltax/2
	mov eax, [ebp].dt_deltax
	sar eax,1
	add eax,edx

	;ebx = yerr = dsize + [ebp].dt_deltay/2
	mov ebx,[ebp].dt_deltay
	sar ebx,1
	add ebx,edx

	mov ecx,edx	;dot count into dx

	dec  [ebp].dt_deltax
	dec [ebp].dt_deltay

	mov edi,[ebp].dt_dtable

innerlp:

	movsb	;move that babe!
	dec	esi	;oops didn't mean to increment source here

	add eax, [ebp].dt_deltax
	jg	ystep

nextx: add esi,[ebp].dt_incx
	add eax,edx
	jle nextx

ystep:
	add ebx,[ebp].dt_deltay
	jg nextlp

nexty: add esi,[ebp].dt_incy
	add ebx,edx
	jle nexty

nextlp:
	loop innerlp

	add esp,lvarspace
	popad
	ret
pj_bym_dto_ptable endp

code	ends
	end
