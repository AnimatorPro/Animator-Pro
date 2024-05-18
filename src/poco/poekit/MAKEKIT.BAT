@echo off
rem ************************************************************************
rem * Make POEKIT...
rem *
rem *	If run without parms, this will update the contents of the
rem *	poekit\inc and poekit\lib directories.
rem *
rem *	If run with a parm of 'DIST', it will also build a zipfile which
rem *	contains the distribution of the POEKIT.
rem *
rem *	Note that both steps have to handle the rexentry.obj file with a
rem *	a separate command, since cpynewer.exe treats obj files differently,
rem *	and the main zip command excludes obj files.
rem ************************************************************************

	cpynewer -v -i \paa\rexlib\inc\ -o inc\ @incfiles.inc
	cpynewer -v -i \paa\rexlib\ 	-o lib\ @libfiles.inc
	copy \paa\rexlib\rexentry.obj	   lib\rexentry.obj
	cd poepatch
	wmake
	cd ..

	if NOT "%1"=="dist" goto out

	zip -r -P -x*.obj -x*.poe -x*.bak poekit%2.zip @kitfiles.inc
	zip    -P						  poekit%2.zip @kit_objs.inc

:out
