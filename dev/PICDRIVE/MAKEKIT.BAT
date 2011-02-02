@echo off
rem ************************************************************************
rem * Make PDRKIT...
rem *
rem *	If run without parms, this will update the contents of the
rem *	picdrive\inc and picdrive\lib directories.
rem *
rem *	If run with a parm of 'DIST', it will also build a zipfile which
rem *	contains the distribution of the PDRKIT.
rem *
rem *	Note that both steps have to handle the rexentry.obj file with a
rem *	a separate command, since cpynewer.exe treats obj files differently,
rem *	and the main zip command excludes obj files.
rem ************************************************************************

	cpynewer -v -i \paa\rexlib\inc\ -o inc\ @incfiles.inc
	cpynewer -v -i \paa\rexlib\ 	-o lib\ @libfiles.inc
	copy \paa\rexlib\rexentry.obj	   lib\rexentry.obj

	if NOT "%1"=="dist" goto out

	zip -r -P -x*.obj -x*.pdr -x*.bak pdrkit%2.zip @kitfiles.inc
	zip    -P  pdrkit%2.zip ..\picdrive\testmake.bat ..\picdrive\lib\rexentry.obj

:out
