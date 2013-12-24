@echo off
	call setwc386
	pushdir
	cd \paa\inc
	tcmake ilib
	popdir
	tcmake %1 %2 %3 %4 %5 %6 %7 %8 %9
@echo on
