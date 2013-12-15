cd poepatch
wmake
@if ERRORLEVEL 1 goto ERROR
cd ..

cd skeleton
wmake
@if ERRORLEVEL 1 goto ERROR
cd ..

cd colorutl
wmake
@if ERRORLEVEL 1 goto ERROR
cd ..

cd lookup
wmake
@if ERRORLEVEL 1 goto ERROR
cd ..

cd otdemo
wmake
@if ERRORLEVEL 1 goto ERROR
cd ..

cd pdrdemo
wmake
@if ERRORLEVEL 1 goto ERROR
cd ..

cd pstamp
wmake
@if ERRORLEVEL 1 goto ERROR
cd ..

@goto OUT
:ERROR
	@echo ERROR
	@goto OUT
:OUT
