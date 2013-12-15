@echo off
echo >foo xxxxxxxx
rep " xxxxxxxx" "y\ny\ny\ny" foo -b -s
deldir <foo h:\paat
echo . 
deldir <foo \paat
echo . 
del default.flx
del foo
@echo on
