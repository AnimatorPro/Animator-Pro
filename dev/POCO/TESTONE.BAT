  @echo off
  if "%1"=="watcom" goto WATCOM
  \paa\poco\poco.exe %3
  goto EXIT
:WATCOM
  run386 \paa\poco\poco.exp %3
:EXIT
  rem all done
