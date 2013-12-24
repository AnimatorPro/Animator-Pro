           include a8514.i


_DATA      segment   public word use32 'DATA'

opWordMess db        'Packet Count/Line Skip', 0
lCountMess db        'LineCount', 0
skpBytMess db        'ByteSkip', 0
addrMess   db        'Current Address', 0
opCodeMess db        'OpCode', 0
repValMess db        'RepeatData', 0
pixCntMess db        'RepeatCnt', 0

_DATA      ends

CONST      segment   public word use32 'DATA'
CONST      ends

_BSS       segment   PUBLIC WORD USE32 'BSS'
_BSS       ends

DGROUP     group     CONST,_DATA,_BSS

CGROUP     group     code

code       segment   dword 'CODE'
           assume    cs:CGROUP, ds:CGROUP

           extrn     PrintHex:near, PrintLHex:near, PrintVar:near
           extrn     PrintText:near, WaitKey:near, PrintLVar:near


PRINT_HEX  macro     reg               ; 'reg' must be 32 bit
           pushad
           push      reg
           call      PrintHex
           pop       reg
           popad
           endm

PRINT_LHEX macro     reg               ; 'reg' must be 32 bit
           pushad
           push      reg
           call      PrintLHex
           pop       reg
           popad
           endm

PRINTMESS  macro     ptr               ; Name of message
           pushad
           lea       EAX, ptr
           push      EAX
           call      PrintText
           popad
           endm

PRINTVAR   macro     ptr, reg
           pushad
           push      reg
           lea       EAX, ptr
           push      EAX
           call      PrintVar
           pop       EAX
           pop       EAX
           popad
           endm

PRINTADDR  macro
           pushad
           lea       EAX, addrMess
           push      ESI
           push      EAX
           call      PrintLVar
           pop       EAX
           pop       EAX
           popad
           endm

WAIT_KEY   macro
           pushad
           call      WaitKey
           popad
           endm

;set up mix register to take data from pixel transfer register
SET_PTRANS macro     ;corrupts dx and ax as usual!
           mov       AX, PTRANS_ACTIVE+MIX_SRC
           mov       DX, FGRD_MIX
           out       DX, AX
           endm

SET_FORE   macro     ;corrupts dx and ax as usual!
           mov       AX, F_CLR_ACTIVE+MIX_SRC
           mov       DX, FGRD_MIX
           out       DX, AX
           endm

pj_8514_unss2 proc near
           public pj_8514_unss2
;extern UBYTE *pj_8514_unss2(LONG x, LONG y, UBYTE *cbuf);
ussp       struc
luss_line_ops dd     ?
uss_ebp    dd        ?
uss_ret    dd        ?
uss_x      dd        ?
uss_y      dd        ?
uss_cbuf   dd        ?
uss_width  dd        ?
ussp       ends

           push      EBP
           push      EBP               ; Space for stack variable luss_line_ops
           mov       EBP, ESP
           push      EBX
           push      ECX
           push      EDX
           push      ESI
           push      EDI

           mov       ESI, [EBP].uss_cbuf ; Get pointer to buffer

           PRINTADDR

           xor       EAX, EAX
           lodsw
           mov       [EBP].luss_line_ops, EAX  ; Line count of decompress

           PRINTVAR  lCountMess, EAX

                                       ; Make sure have GP is free
           CLRCMD                      ; v1.00
           WAITQ     2                 ; v1.00

           SET_PTRANS                  ; Set us into transfer mode.

           jmp       line_loop         ; Jump here so can just fall through on line-skips

skiplines:
           cwde                        ; Sign extend AX into EAX
           sub       [EBP].uss_y, EAX  ; Reduce line count

line_loop:
           lodsw                       ; Get Op Word

           PRINTVAR  opWordMess, EAX
           WAIT_KEY

           test      AX, AX            ; Check polarity
           jns       do_ss2ops         ; if positive it's an ss2 opcount
           cmp       AH, 040h          ; if bit 0x40 of ah is on (unsigned >= C0h)
           jae       skiplines         ; we skip lines 
           jmp       line_loop
           
do_ss2ops:
           movsx     EDI, AX           ; Get packet count from the opword
           mov       EBX, [EBP].uss_x  ; Load X and Y positions

op_loop:
           WAITQ     8                 ; v1.00

           mov       EAX, [EBP].uss_y  ; Set Y position
           mov       DX, CUR_Y_POS
           out       DX, AX

           xor       EAX, EAX
           lodsb                       ; fetch skip

           PRINTVAR  skpBytMess, EAX

           add       EBX, EAX          ; add skip to x

           mov       EAX, EBX          ; Set up x position on 8514
           mov       DX, CUR_X_POS
           out       DX, AX

           xor       EAX, EAX

           lodsb                       ; Get op byte (neg is duplicate)

           PRINTVAR  opCodeMess, EAX
           WAIT_KEY

           movsx     ECX, AL           ; ...into counter
           or        ECX, ECX
           js        repeat_op

CopyData:
           add       EBX, EAX          ; Start of copy loop, update the X for later
           shl       AX, 1
           dec       AX                ; Pixel count use # - 1 
           mov       DX, MAJ_AXIS_PCNT
           out       DX, AX
           mov       AX, 03319h        ; Start the copy command
           mov       DX, COMMAND
           out       DX, AX

           mov       DX, PIX_TRANS
           rep       outsw

           mov       DX, SUBSYS_CNTL   ; v1.00
           mov       AX, 4             ; Clear the "Data register read" flag.
           out       DX, AX

PacketEnd:
           CLRCMD
           WAITQ     2                 ; v1.00

           PRINTADDR

           dec       EDI               ; Decrement packet count
           jnz       op_loop
           inc       [EBP].uss_y       ; Bump to next line for future use
           dec       [EBP].luss_line_ops ; Decrement line count
           jnz       line_loop
           jmp       done

repeat_op:
           neg       ECX               ; Negate the negative repeat count.
           mov       EAX, ECX
           shl       AX, 1             ; * 2 - Since it's a word count
           dec       AX                ; Pixel count is actual # - 1

           PRINTVAR  pixCntMess, EAX

           mov       DX, MAJ_AXIS_PCNT
           out       DX, AX

           lodsw                       ; Get the duplication word.

           PRINTVAR  repValMess, EAX

           jcxz      PacketEnd         ; On zero count, skip dupes.

           cmp       AH, AL
           jne       DupeUpload

;here we've got a run that is solid color.  Whoopi!  have the GP do the
;work.

           push      AX
           WAITQ     7                 ; v1.00
           pop       AX

           mov       DX, FRGD_COLOR
           out       DX, AX
           SET_FORE

           mov       AX, (WRITCMD+STROKE_ALG+DRAWCMD+PLANAR+LINE_DRAW)
           mov       DX, COMMAND
           out       DX, AX

           CLRCMD
           WAITQ     2                 ; v1.00

           SET_PTRANS
;usual end of the op loop processing...

           PRINTADDR

           dec       EDI               ; Decrement the op word count
           jnz       op_loop
           inc       [EBP].uss_y       ; Bump to next line for future use
           dec       [EBP].luss_line_ops ; Oops, new line
           jnz       line_loop
           jmp       done

DupeUpload:
           push      AX
           mov       AX, 03319H        ; Command to copy a line
           mov       DX, COMMAND
           out       DX, AX
           pop       AX

;           mov       AX, [ESI-2]       ; Reload color

           mov       DX, PIX_TRANS

SendWord:
           out       DX, AX
           jmp       short $+2
           loop      SendWord          ; Send out all the words.

           mov       DX, SUBSYS_CNTL   ; v1.00
           mov       AX, 4             ; Clear the "Data register read" flag.
           out       DX, AX

           CLRCMD
           WAITQ     2                 ; v1.00

           PRINTADDR

           dec       EDI
           jnz       op_loop
           inc       [EBP].uss_y       ; Bump to next line for future use
           dec       [EBP].luss_line_ops
           jnz       line_loop
done:

           ;set foreground mix back for faster put_dot
           SET_FORE

           pop       EDI
           pop       ESI
           pop       EDX
           pop       ECX
           pop       EBX
           pop       EBP
           pop       EBP
           ret

pj_8514_unss2 endp

code       ends
           end
