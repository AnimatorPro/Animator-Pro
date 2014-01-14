                NAME    wr1lib
DGROUP          GROUP   CONST,_DATA,_BSS
_TEXT           SEGMENT PUBLIC BYTE USE32 'CODE'
                ASSUME  CS:_TEXT,DS:DGROUP

include rastlib.i
include raster.i
include wndo.i

; ******************* get dot ************************

gdarg	struc	
	gdarg_ret dd ?
	gdarg_w	dd ?
	gdarg_x	dd ?
	gdarg_y dd ?
gdarg ends

	public wr1_get_dot
	public _wr1_get_dot

wr1_get_dot proc near

; clip here 

	mov		eax, dword ptr [esp].gdarg_w ; eax = stack wndo
    movzx   ecx,word ptr R_WIDTH[eax]	 ; (ULONG)ecx = w->width    
    cmp     ecx,dword ptr [esp].gdarg_x  ; if(ecx <= gdarg_x 
	jbe		wr1_gdot_ret					 ; goto ret
    movzx   ecx,word ptr R_HEIGHT[eax]
    cmp     ecx,dword ptr [esp].gdarg_y
	ja		wr1_gdot
wr1_gdot_ret:
	ret

_wr1_get_dot proc near
	mov		eax, dword ptr [esp].gdarg_w ; eax = stack wndo
wr1_gdot:
    movsx   ecx,word ptr W_ONERAST[eax]  ; ecx = w->onerast 
    mov     eax,dword ptr W_RASTS[eax]   ; eax = w->rasts
    mov     eax,dword ptr [eax+ecx*4]    ; eax = *(eax+ecx)
	mov 	dword ptr [esp].gdarg_w, eax  ; r = eax
   	mov     eax,dword ptr R_LIB[eax]     ; eax = r->lib
    jmp    	dword ptr RL_GET_DOT[eax]       ; jump eax[_GET_DOT]

_wr1_get_dot endp
wr1_get_dot endp

	public wr1os_get_dot
	public _wr1os_get_dot

wr1os_get_dot proc near

; clip here 

	mov		eax, dword ptr [esp].gdarg_w ; eax = stack wndo
    movzx   ecx,word ptr R_WIDTH[eax]	 ; (ULONG)ecx = w->width    
    cmp     ecx,dword ptr [esp].gdarg_x  ; if(ecx <= gdarg_x 
	jbe		wr1os_gdot_ret					 ; goto ret
    movzx   ecx,word ptr R_HEIGHT[eax]
    cmp     ecx,dword ptr [esp].gdarg_y
	ja		wr1os_gdot
wr1os_gdot_ret:
	ret

_wr1os_get_dot proc near
	mov		eax, dword ptr [esp].gdarg_w ; eax = stack wndo

wr1os_gdot:
;
; add behind x and y offsets
;
	mov     cx, word ptr W_BEHIND+R_X[eax] ; cx = w->behind.x
	add		word ptr [esp].gdarg_x, cx   ; arg_y += cx
	mov     cx, word ptr W_BEHIND+R_Y[eax] ; cx = w->behind.x
	add		word ptr [esp].gdarg_y, cx   ; arg_y += cx
;
; get raster 
;
    movsx   ecx,word ptr W_ONERAST[eax]  ; ecx = w->onerast 
    mov     eax,dword ptr W_RASTS[eax]   ; eax = w->rasts
    mov     eax,dword ptr [eax+ecx*4]    ; eax = *(eax+ecx)
	mov 	dword ptr [esp].gdarg_w, eax  ; r = eax
;
; subtract raster offsets now x = x + w->behind.x - w->rasts[w->onerast]->x 
;
	mov		cx, word ptr R_X[eax]        ; cx = rast->x 
	sub		word ptr [esp].gdarg_x, cx   ; arg_x -= cx
	mov		cx, word ptr R_Y[eax]        ; cx = rast->y 
	sub		word ptr [esp].gdarg_y, cx   ; arg_y -= cx
   	mov     eax,dword ptr R_LIB[eax]     ; eax = r->lib
    jmp    	dword ptr RL_GET_DOT[eax]      ; jump eax[_GET_DOT]

_wr1os_get_dot endp
wr1os_get_dot endp


_TEXT           ENDS

CONST           SEGMENT PUBLIC WORD USE32 'DATA'
CONST           ENDS

_DATA           SEGMENT PUBLIC WORD USE32 'DATA'
_DATA           ENDS

_BSS            SEGMENT PUBLIC WORD USE32 'BSS'
_BSS            ENDS

                END
