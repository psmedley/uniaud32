@echo off
SET KEE=0
wmake -ms -f makefile.os2 DEBUG=1 clean
SET KEE=1
wmake -ms -f makefile.os2 DEBUG=1 clean
SET KEE=

