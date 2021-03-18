@echo off

rem If you already have these set in your environment, then you do not
rem need to set them here
set WATCOM=e:\Watcom
set DDK=u:\ddk

rem To build the release version
wmake -a DEBUG=0

rem To build the debug version
wmake -a DEBUG=1

