cd flilo
wmake
@if ERRORLEVEL 1 goto ERROR
cd ..

cd rif
wmake
@if ERRORLEVEL 1 goto ERROR
cd ..

cd skeleton
wmake
@if ERRORLEVEL 1 goto ERROR
cd ..

cd targa
wmake
@if ERRORLEVEL 1 goto ERROR
cd ..

@goto OUT
:ERROR
	@echo ERROR
	@goto OUT
:OUT
