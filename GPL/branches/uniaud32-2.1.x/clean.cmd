@echo off
set path=e:\watcom\binp;u:\ddk\tools;u:\bin;%path%
set watcom=e:\watcom
set beginlibpath=e:\watcom\binp\dll
SET KEE=0
wmake -ms -f makefile.os2 DEBUG=1 clean
SET KEE=1
wmake -ms -f makefile.os2 DEBUG=1 clean
SET KEE=
del drv32\bin\Release\*
