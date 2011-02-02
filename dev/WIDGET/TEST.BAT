@echo off
pushdir
cd \paa
if not "%1"=="out" goto noout
shift
run386 test.exp %1 %2 %3 %4 %5 %6 %7 %8 %9 >\paa\widget\test.out
goto done
:noout
run386 test.exp %1 %2 %3 %4 %5 %6 %7 %8 %9
:done
popdir
@echo on
