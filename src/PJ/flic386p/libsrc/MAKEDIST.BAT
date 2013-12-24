@echo off

rem **********************************************************************
rem * batch file to make fliclib distribution zipfiles.
rem *  use 'makedist b' to make binaries but not sources.
rem **********************************************************************

	cd ..

	del flclib%1.zip
	zip -P flclib%1 @libsrc\flc386pb.inc

	if "%1"=="b" goto done

	del flcsrc%1.zip
	zip -r -P flcsrc%1 @libsrc\flc386ps.inc

:done

	cd libsrc

