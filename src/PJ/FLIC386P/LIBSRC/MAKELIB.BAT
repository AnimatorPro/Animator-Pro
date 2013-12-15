@echo off

rem **********************************************************************
rem * Batch file to make FlicLib source code into FLIC386P.LIB.
rem **********************************************************************

	call setwc386

	if "%1"=="l" goto do_lib

	cd drvcomn
	cd
	call makeobj  %1 %2 %3 %4
	cd ..

	cd vesa
	cd
	call makeobj  %1 %2 %3 %4
	cd ..

	cd svga
	cd
	call makeobj  %1 %2 %3 %4
	cd ..

	cd 8514
	cd
	call makeobj  %1 %2 %3 %4
	cd ..

	cd mcga
	cd
	call makeobj  %1 %2 %3 %4
	cd ..

	cd rastlib
	cd
	call makeobj  %1 %2 %3 %4
	cd ..

	cd util
	cd
	call makeobj  %1 %2 %3 %4
	cd ..

	cd gfx
	cd
	call makeobj  %1 %2 %3 %4
	cd ..

	cd fli
	cd
	call makeobj  %1 %2 %3 %4
	cd ..

	cd fileio
	cd
	call makeobj  %1 %2 %3 %4
	cd ..

	cd gluecode
	cd
	call makeobj  %1 %2 %3 %4
	cd ..

	cd glue_env
	cd
	call makeobj  %1 %2 %3 %4
	cd ..

:do_lib

	del  ..\flc386ws.lib
	wlib ..\flc386ws.lib @flc386ws.obs

	del  ..\flc386wr.lib
	wlib ..\flc386wr.lib @flc386wr.obs

	del  ..\flc386ps.lib
	wlib ..\flc386ps.lib @flc386ps.obs
	womp ..\flc386ps.lib /fp /d-

	del  ..\flc386po.lib
	wlib ..\flc386po.lib @flc386po.obs
	womp ..\flc386po.lib /fp /d-

	del  ..\flc386pr.lib
	wlib ..\flc386pr.lib @flc386pr.obs
	womp ..\flc386pr.lib /fp /d-

	del  ..\flc386cb.lib
	wlib ..\flc386cb.lib @flc386cb.obs
	womp ..\flc386cb.lib /fp /d-
	call setcb
	echo quit exit >convert.$$$
	lib32 ..\flc386cb.lib nobackup batch <convert.$$$
	del convert.$$$
	call setwc386

	echo makelib ended.
