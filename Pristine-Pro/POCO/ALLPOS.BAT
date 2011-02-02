  @set which=watcom
  @if not "%1" == "" set which=%1
  for %%f in (\paa\poco\test\pos\*.poc) do call testone %which% pos %%f
