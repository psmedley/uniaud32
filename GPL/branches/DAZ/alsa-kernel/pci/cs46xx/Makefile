!ifndef %ROOT
!error ROOT is not defined. Please check your SETENV.CMD.
!endif
ROOT=$(%ROOT)

CINCLUDES = ..\..\..\include;..\..\include;$(%WATCOM)\H
AINCLUDES = $(DDK)\base\H

CDEFINES = -D__KERNEL__ -DMODULE -dTARGET_OS2 -dALSA_BUILD -D__ISAPNP__ -DCONFIG_ISAPNP -D__i386_
#-DCONFIG_SND_CS46XX_NEW_DSP

!include $(ROOT)\tools\header.mif


#===================================================================
#
#   List of source files
#
#===================================================================
FILES = cs46xx.obj cs46xx_lib.obj
# dsp_spos.obj dsp_spos_scb_lib.obj

TARGET = cs46xx

LNKFILE = $(OBJDIR)\$(TARGET).lnk


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
    -@rm *.err
    @if exist obj @rm.exe -r obj
    @if exist bin @rm.exe -r bin

