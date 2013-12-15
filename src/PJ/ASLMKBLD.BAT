pushdir
cd asl
pkzip -es -P i:\aslbuild bin\*.*
c:
cd \
pkzip -es -P i:\aslbuild @f:\paa\aslmkbld.inc
popdir
