@echo off

	if NOT "%1"=="RECURSE" goto WALK_DIRS

	del *.c
	del *.h
	del *.i
	del *.asm
	del *.obj
	del *.lib
	goto DONE

:WALK_DIRS

	cd 8514
	call ..\makclean RECURSE
	cd ..

	cd drvcomn
	call ..\makclean RECURSE
	cd ..

	cd fileio
	call ..\makclean RECURSE
	cd ..

	cd fli
	call ..\makclean RECURSE
	cd ..

	cd gfx
	call ..\makclean RECURSE
	cd ..

	cd inc
	call ..\makclean RECURSE
	cd ..

	cd mcga
	call ..\makclean RECURSE
	cd ..

	cd rastlib
	call ..\makclean RECURSE
	cd ..

	cd svga
	call ..\makclean RECURSE
	cd ..

	cd util
	call ..\makclean RECURSE
	cd ..

	cd vesa
	call ..\makclean RECURSE
	cd ..


:DONE

