!ifndef %ROOT
!error ROOT is not defined. Please check your SETENV.CMD.
!endif
ROOT=$(%ROOT)

CINCLUDES = ..\..\..\include;..\..\include;$(%WATCOM)\H
AINCLUDES = $(DDK)\base\H

CDEFINES = -D__KERNEL__ -DMODULE -dTARGET_OS2 -dALSA_BUILD -D__ISAPNP__ -DCONFIG_ISAPNP -D__i386_

!include $(ROOT)\tools\header.mif


#===================================================================
#
#   List of source files
#
#===================================================================
FILES = opl3_lib.obj opl3_synth.obj &
  opl3_seq.obj opl3_midi.obj opl3_drums.obj

TARGET   = opl3

LNKFILE  = $(OBJDIR)\$(TARGET).lnk


#===================================================================
#
#   Specific dependencies
#
#===================================================================
all: $(OBJDIR)\$(TARGET).lib


$(LNKFILE): Makefile
    @%create $(LNKFILE)
    @for %f in ($(FILES)) do @%append $(LNKFILE) -+$(OBJDIR)\%f

$(OBJDIR)\$(TARGET).lib: $(LNKFILE) $(FILES)
    $(LIB) $(OBJDIR)\$(TARGET).lib @$(LNKFILE)
    copy $(OBJDIR)\$(TARGET).lib $(ALSA_LIB)


clean: .SYMBOLIC
  -@rm  *.err
  @if exist obj @rm.exe -r obj
  @if exist bin @rm.exe -r bin

