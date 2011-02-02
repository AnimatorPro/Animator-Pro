;**********************************************************************
;
; OUTPUT.ASM -- Special DEBUG code written for the AA386 TIGA driver.
;
; Written by Panacea Inc.
;
; Panacea Inc.
; 50 Nashua Road, Suite 305
; Londonderry, New Hampshire, 03053-3444
; (603) 437-5022
;
; NOTICE:
;   This code may be freely distributed and modified as long as the
;   Panacea copyright notice above is maintained intact.
;
;
;Revision History:
;
;When     Who   What
;======== ===   =======================================================
;09/13/90 JBR   Start of development.
;
;**********************************************************************

_DATA      segment   public word use32 'DATA'
_DATA      ends

CONST      segment   public word use32 'DATA'
CONST      ends

_BSS       segment   PUBLIC WORD USE32 'BSS'
_BSS       ends

DGROUP     group     CONST,_DATA,_BSS

_TEXT	segment	para public use32 'CODE'
CGROUP	group	_TEXT
	assume    CS:CGROUP, DS:DGROUP


;
; PrintText(char *string);
;
; Uses INT 10H to print a string to the VGA (or other IBM compatible
; text display) - this works with TIGA, since most TIGA systems use
; a VGA pass-through.
;
           public    PrintText
PrintText  proc      near
           push      EBP
           mov       EBP, ESP
           push      ESI
           mov       ESI, [EBP+8]      ; Get pointer to string

PrnTxt_Loop:
           lodsb                       ; Get a byte from the string
           cmp       AL, 0             ; End of string?
           je        PrnTxt_Done
           mov       AH, 14            ; Use Print TTY function
           int       10H
           jmp       PrnTxt_Loop

PrnTxt_Done:
           pop       ESI
           pop       EBP
           ret
PrintText  endp

;
; WaitKey()
;
; Waits for a key to be pressed.
;
           public    WaitKey
WaitKey    proc      near
           xor       AX, AX
           int       16H
           ret
WaitKey    endp

_TEXT      ends
	end

