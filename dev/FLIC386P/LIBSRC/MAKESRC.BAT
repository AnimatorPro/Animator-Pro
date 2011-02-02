@echo off

	cd inc
	cd
	call makesrc %1 %2 %3 %4 %5 %6 %7 %8 %9
	cd ..

	cd drvcomn
	cd
	call makesrc %1 %2 %3 %4 %5 %6 %7 %8 %9
	cd ..

	cd vesa
	cd
	call makesrc %1 %2 %3 %4 %5 %6 %7 %8 %9
	cd ..

	cd svga
	cd
	call makesrc %1 %2 %3 %4 %5 %6 %7 %8 %9
	cd ..

	cd 8514
	cd
	call makesrc %1 %2 %3 %4 %5 %6 %7 %8 %9
	cd ..

	cd mcga
	cd
	call makesrc %1 %2 %3 %4 %5 %6 %7 %8 %9
	cd ..

	cd rastlib
	cd
	call makesrc %1 %2 %3 %4 %5 %6 %7 %8 %9
	cd ..

	cd util
	cd
	call makesrc %1 %2 %3 %4 %5 %6 %7 %8 %9
	cd ..

	cd gfx
	cd
	call makesrc %1 %2 %3 %4 %5 %6 %7 %8 %9
	cd ..

	cd fli
	cd
	call makesrc %1 %2 %3 %4 %5 %6 %7 %8 %9
	cd ..

	cd fileio
	cd
	call makesrc %1 %2 %3 %4 %5 %6 %7 %8 %9
	cd ..

