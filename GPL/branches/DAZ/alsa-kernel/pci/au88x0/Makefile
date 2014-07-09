!ifndef %ROOT
!error ROOT is not defined. Please check your SETENV.CMD.
!endif
ROOT=$(%ROOT)

CINCLUDES = $(ROOT)\include;$(%WATCOM)\H
AINCLUDES = $(DDK)\base\H

CDEFINES = -D__KERNEL__ -DMODULE -dTARGET_OS2 -dALSA_BUILD -D__ISAPNP__ -DCONFIG_ISAPNP -D__i386_

!include $(ROOT)\tools\header.mif


#===================================================================
#
#   List of source files
#
#===================================================================
# au8810.obj au8810_core.obj au8810_pcm.obj au8810_mixer.obj au8810_mpu401.obj au8810_eq.obj
FILES = au8820.obj au8820_core.obj au8820_pcm.obj au8820_mixer.obj au8820_mpu401.obj au8820_synth.obj
# au8830.obj au8830_core.obj au8830_pcm.obj au8830_mixer.obj au8830_mpu401.obj au8830_eq.obj au8830_synth.obj

TARGET = au88xx

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
    -@rm  *.err
    @if exist obj @rm.exe -r obj
    @if exist bin @rm.exe -r bin

