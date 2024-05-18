
PJREX_MAGIC equ 9abcdef0h
PJREX_MAGIC2 equ 0def0h
PJREX_VERSION equ 0h

rexentry struc  ;rexlib entry  (data) structure\n"
	re_safety	dw ? ; actually two one byte return instructions

    re_magic    dd ? ; magic number

    re_header   dd ? ; pointer to library header 

    re_magic2   dw ? ; second magic number
    re_version  dw ? ; version number

	re_data_end dd ? ;     Set to the symbol "_end" in pharlap and watcom
					 ; linkers, "_end" is specified as the address of the end of
					 ; the uninitialized data segment which is the maximum 
					 ; address we have to load.  If you want it to load to the
					 ; next even 4K page set this to 0.  
					 ;
					 ;     This value is used to eliminate page padding in the 
					 ; loader by specifying the total number of bytes to 
					 ; allocate for the rex module. It will override the 
					 ; last potentially padded page of uninitialized data
					 ; specified in the rex header if it is smaller.
rexentry ends

