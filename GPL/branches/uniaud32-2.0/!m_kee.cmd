@echo off
set KEE=1
call tools\mkversion
wmake -ms -f makefile.os2
set KEE=
