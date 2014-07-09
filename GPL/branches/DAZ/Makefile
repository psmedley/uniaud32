!ifndef %ROOT
%ROOT=$+$(%cwd)$-
!endif
ROOT=$(%ROOT)

NOOBJ=1
!include $(ROOT)\tools\header.mif

WMAKEOPTS=-h $(__MAKEOPTS__) KEE=1

!if "$(IBM_BUILD)" == "1"
WMAKEOPTS += IBM_BUILD=1
!endif

!if "$(DEBUG)" == "1"
WMAKEOPTS += DEBUG=1
!Message BUILDING DEBUG VERSION
!else
WMAKEOPTS += DEBUG=0
!Message BUILDING RELEASE VERSION
!endif

#===================================================================
#
#   Specific dependencies
#
#===================================================================
all: subdirs

subdirs: .SYMBOLIC
  @if not exist $(ALSA_LIB) $(CREATEPATH) $(ALSA_LIB)
  @if not exist $(ALSA_BIN) $(CREATEPATH) $(ALSA_BIN)
  @$(ROOT)\tools\mkversion.cmd
  @echo BUILDING alsa-kernel
  @cd alsa-kernel
  @wmake $(WMAKEOPTS)
  @cd ..
  @echo BUILDING lib32
  @cd lib32
  @wmake $(WMAKEOPTS)
  @cd ..
  @echo BUILDING drv32
  @cd drv32
  @wmake $(WMAKEOPTS)
  @cd ..

clean: .SYMBOLIC
  @echo CLEANING alsa-kernel
  @cd alsa-kernel
  @wmake $(WMAKEOPTS) clean
  @cd ..
  @echo CLEANING lib32
  @cd lib32
  @wmake $(WMAKEOPTS) clean
  @cd ..
  @%create $(ROOT)\include\version.mak
  @echo CLEANING drv32
  @cd drv32
  @wmake $(WMAKEOPTS) clean
  @cd $(ROOT)
  @if exist $(ROOT)\include\version.h @del $(ROOT)\include\version.h
  @if exist $(ROOT)\include\version.mak @del $(ROOT)\include\version.mak
  @if exist bin @rm.exe -r bin
  @if exist obj @rm.exe -r obj
  @if exist lib @rm.exe -r lib

