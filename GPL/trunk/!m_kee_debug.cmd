@echo off
set KEE=1
call tools\mkversion include\version.h include\version.mak
wmake -ms -f makefile.os2 DEBUG=1
set KEE=
