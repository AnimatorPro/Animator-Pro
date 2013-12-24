rem makecall  lib_symbol, asmname(.asm) ** if no asmname then symbol(.asm)
rem
call makecall pj__get_pto_errno ptoerrno
call makecall clearerr
call makecall feof
call makecall ferror
call makecall pj_errno_errcode errcode
call makecall fopen
call makecall fclose
call makecall fseek
call makecall ftell
call makecall fflush
call makecall rewind
call makecall fread
call makecall fgetc
call makecall fgets
call makecall ungetc
call makecall fwrite
call makecall fputc
call makecall fputs
call makecall fprintf
call makecall printf
call makecall sprintf

