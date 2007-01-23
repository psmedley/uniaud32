@echo off
set KEE=0
call tools\mkversion include\version.h include\version.mak
wmake -ms -f makefile.os2
set KEE=
