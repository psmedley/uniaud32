!ifndef %ROOT
!error ROOT is not defined. Please check your SETENV.CMD.
!endif
ROOT=$(%ROOT)

!include $(ROOT)\tools\header.mif

CINCLUDES = ..\include;..\runtime;..\alsa-kernel\include;$(WATCOM)\H;$(WATCOM)\H\OS2;
AINCLUDES = $(DDK)\base\H

CDEFINES = -D__KERNEL__ -DMODULE -dTARGET_OS2 -dALSA_BUILD -D__ISAPNP__ -DCONFIG_ISAPNP -D__i386_

!if "$(IBM_BUILD)" == "1"
CDEFINES += -DIBM_BUILD
!endif

#===================================================================
#
#   List of source files
#
#===================================================================
FILES = spinlock.obj malloc.obj pci.obj irq.obj ioctl.obj &
  memory.obj debug.obj sound.obj soundmixer.obj soundmidi.obj &
  ossidc.obj semaphore.obj waitqueue.obj misc.obj &
  strncpy.obj task.obj &
  strncmp.obj timer.obj &
  instropl2.obj instropl3.obj vsprintf.obj
# ossidc.obj semaphore.obj waitqueue.obj misc.obj fminstrload.obj
!if $(KEE) == 0
FILES += stack.obj
!endif

TARGET = linuxlib

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

