@echo off

rem **********************************************************************
rem * Batch file to update FlicLib source code then make FLIC386P.LIB.
rem **********************************************************************

	if NOT "%1"=="o" goto do_source
	shift
	goto do_object

:do_source
	call makesrc %1 %2 %3

:do_object
	call makelib %1 %2 %3

