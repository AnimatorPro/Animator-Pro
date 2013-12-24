
           include a8514.i

CGROUP     group      code

code       segment dword 'CODE'
           assume cs:CGROUP,ds:CGROUP

INS_SIZE equ 2                              ;size of tower_ins
INS_COUNT equ 128

tower_ins macro
           out dx,ax
           endm


;set up mix register to take data from pixel transfer register
Set_ptrans macro      ;corrupts dx and ax as usual!
           mov ax,PTRANS_ACTIVE+MIX_SRC
           mov dx,FGRD_MIX
           out dx,ax
           endm

Set_fore macro ;corrupts dx and ax as usual!
           mov ax,F_CLR_ACTIVE+MIX_SRC
           mov dx,FGRD_MIX
           out dx,ax
           endm

pj_8514_unss2 proc near
           public pj_8514_unss2
;extern UBYTE *pj_8514_unss2(LONG x, LONG y, UBYTE *cbuf);
ussp       struc
           luss_line_ops dd ?
           uss_ebp    dd ?
           uss_ret    dd ?
           uss_x      dd ?
           uss_y      dd ?
           uss_cbuf dd ?
           uss_width dd ?
ussp       ends

           push ebp
           push ebp              ;space for stack variable luss_line_ops
           mov ebp,esp
           push ebx
           push ecx
           push edx
           push esi
           push edi

           mov esi,[ebp].uss_cbuf
           xor eax,eax
           lodsw
           mov [ebp].luss_line_ops,eax

;make sure have GP is free
           CLRCMD                      ; v1.00
           WAITQ     2                 ; v1.00


           Set_ptrans

           jmp line_loop         ;jump here so can just fall through on line-skips

skiplines:
           cwde
           sub [ebp].uss_y, eax 
line_loop:
           lodsw
           test ax,ax 
           jns do_ss2ops                    ; if positive it's an ss2 opcount
           cmp ah,0C0h                                 ; if bit 0x40 of ah is on (unsigned >= C0h)
           jae skiplines                    ; we skip lines 

           ; put dot in al at x + width - 1 here

           lodsw ; get next ss2 opcount

do_ss2ops:
           movsx edi,ax
           mov ebx,[ebp].uss_x
;set Y position
           mov eax,[ebp].uss_y
           mov dx,CUR_Y_POS
           out dx,ax
           inc eax
           mov [ebp].uss_y,eax


op_loop:
           WAITQ     8                 ; v1.00

           xor eax,eax
           lodsb      ;fetch skip
           add ebx,eax           ;add skip to x

;set up x position on 8514
           mov eax,ebx
           mov dx,CUR_X_POS
           out dx,ax

           lodsb
           movsx ecx,al
           mov eax,ecx
           add eax,eax
           js repeat_op

           add ebx,eax
           dec eax
           mov dx,MAJ_AXIS_PCNT
           out dx,ax
           mov ax,03319h
           mov dx,COMMAND
           out dx,ax

           mov dx,PIX_TRANS
           rep outsw

           mov       DX, SUBSYS_CNTL   ; v1.00
           mov       AX, 4             ; Clear the "Data register read" flag.
           out       DX, AX

           dec edi
           jnz op_loop
           dec [ebp].luss_line_ops
           jnz line_loop
           jmp done

repeat_op:
           lea ecx,towerend[eax]
           neg eax
           add ebx,eax
           dec eax
           mov dx,MAJ_AXIS_PCNT
           out dx,ax
           lodsw
           cmp ah,al
           jnz do_tower

;here we've got a run that is solid color.  Whoopi!  have the GP do the
;work.

           WAITQ     8                 ; v1.00

           mov dx,FRGD_COLOR
           ;and ax,0ffh                     ;probably not necessary
           out dx,ax
           Set_fore
           mov ax,(WRITCMD+STROKE_ALG+DRAWCMD+LINE_DRAW)
           mov dx,COMMAND
           out dx,ax

           mov       DX, SUBSYS_CNTL   ; v1.00
           mov       AX, 4             ; Clear the "Data register read" flag.
           out       DX, AX

           CLRCMD
           WAITQ     2                 ; v1.00

           Set_ptrans
;usual end of the op loop processing...
           dec edi
           jnz op_loop
           dec [ebp].luss_line_ops
           jnz line_loop
           jmp done

do_tower:
           mov ax,03319h
           mov dx,COMMAND
           out dx,ax

           mov ax,-2[esi]        ;reload color

           mov dx,PIX_TRANS
           jmp ecx

           rept INS_COUNT
           tower_ins
           endm
towerend:
           dec edi
           jnz op_loop
           dec [ebp].luss_line_ops
           jnz line_loop

done:


           ;set foreground mix back for faster put_dot
           Set_fore

           pop edi
           pop esi
           pop edx
           pop ecx
           pop ebx
           pop ebp
           pop ebp
           ret

pj_8514_unss2 endp

code       ends
           end
