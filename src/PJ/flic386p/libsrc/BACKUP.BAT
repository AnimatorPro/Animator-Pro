@echo off

rem **********************************************************************
rem * Batch file to make backup of FlicLib.
rem **********************************************************************

	cd ..\..

	del flic386p\libsrc\flc386bk.zip
	zip -r -P -x*.bak flic386p\libsrc\FLC386BK @flic386p\libsrc\flc386bk.inc

	cd flic386p\libsrc

