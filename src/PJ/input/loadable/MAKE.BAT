@echo off

	call setwc386
	echo !include "\paa\rexlib\inc\makewc.inc" >\paa\rexlib\inc\make.inc
	tcmake %1

@echo on
