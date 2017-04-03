!ifndef %ROOT
!error ROOT is not defined. Please check your SETENV.CMD.
!endif
ROOT=$(%ROOT)

!include $(ROOT)\tools\header.mif

WMAKEOPTS=-h $(__MAKEOPTS__) KEE=$(KEE) DEBUG=$(DEBUG)

#===================================================================
#
#   List of source files
#
#===================================================================
FILES = sound.obj init.obj memory.obj memalloc.obj sgbuf.obj &
  control.obj device.obj misc.obj &
  sound_oss.obj info_oss.obj &
  pcm.obj pcm_native.obj pcm_lib.obj pcm_misc.obj pcm_timer.obj pcm_memory.obj &
  rawmidi.obj timer.obj rtctimer.obj hwdep.obj info.obj vmaster.obj

TARGET = core

LNKFILE = $(OBJDIR)\$(TARGET).lnk

#===================================================================
#
#   Specific dependencies
#
#===================================================================
all: $(OBJDIR)\$(TARGET).lib subdirs

$(LNKFILE): Makefile
    @%create $(LNKFILE)
    @for %f in ($(FILES)) do @%append $(LNKFILE) -+$(OBJDIR)\%f

$(OBJDIR)\$(TARGET).lib: $(LNKFILE) $(FILES)
    $(LIB) $(OBJDIR)\$(TARGET).lib @$(LNKFILE)
    copy $(OBJDIR)\$(TARGET).lib $(ALSA_LIB)


subdirs: .SYMBOLIC
    @echo BUILDING seq
    @cd seq
    @wmake $(WMAKEOPTS)
    @cd ..
    @echo BUILDING oss
    @cd oss
    @wmake $(WMAKEOPTS)
    @cd ..

clean: .SYMBOLIC
    @echo CLEANING seq
    @cd seq
    @wmake $(WMAKEOPTS) clean
    @cd ..
    @echo CLEARING oss
    @cd oss
    @wmake $(WMAKEOPTS) clean
    @cd ..
    -@rm *.err
    @if exist obj @rm.exe -r obj
    @if exist bin @rm.exe -r bin

