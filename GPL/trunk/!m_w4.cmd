@echo off
set KEE=0
call tools\mkversion
wmake -ms -f makefile.os2
set KEE=
