rem %1 is public call name %2 is libcall %3 is asm name 
copy rast1jmp.i %3.asm 
rep XXXX %1 %3.asm -b -s
rep YYYY %2 %3.asm -b -s
